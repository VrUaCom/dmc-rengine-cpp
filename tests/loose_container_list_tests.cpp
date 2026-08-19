#include "dmc_rengine/profiles/dmc3/loose_container_list.hpp"

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <map>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace {

using dmc::rengine::profiles::dmc3::LooseContainerListPolicy;
using dmc::rengine::profiles::dmc3::LooseContainerRepresentation;
using dmc::rengine::profiles::dmc3::LooseContainerSafetyLimits;
using dmc::rengine::profiles::dmc3::LooseContainerStatus;

[[nodiscard]] std::vector<std::byte> bytes(std::string_view text) {
    std::vector<std::byte> result;
    result.reserve(text.size());
    for (const auto value : text) {
        result.push_back(static_cast<std::byte>(
            static_cast<unsigned char>(value)));
    }
    return result;
}

[[nodiscard]] std::uint32_t read_u32(
    const std::vector<std::byte>& value, std::size_t offset) {
    assert(offset + 4U <= value.size());
    return std::to_integer<std::uint32_t>(value[offset + 0U]) |
        (std::to_integer<std::uint32_t>(value[offset + 1U]) << 8U) |
        (std::to_integer<std::uint32_t>(value[offset + 2U]) << 16U) |
        (std::to_integer<std::uint32_t>(value[offset + 3U]) << 24U);
}

[[nodiscard]] std::string magic_string(
    const std::vector<std::byte>& value) {
    assert(value.size() >= 4U);
    std::string result;
    result.reserve(4U);
    for (std::size_t index = 0U; index < 4U; ++index) {
        result.push_back(static_cast<char>(
            std::to_integer<unsigned char>(value[index])));
    }
    return result;
}

class Fixture final {
public:
    void put(std::string path, std::vector<std::byte> value) {
        files_.insert_or_assign(std::move(path), std::move(value));
    }

    [[nodiscard]] auto reader() {
        return [this](std::string_view path)
            -> std::optional<std::vector<std::byte>> {
            reads_.emplace_back(path);
            const auto iterator = files_.find(std::string{path});
            if (iterator == files_.end()) {
                return std::nullopt;
            }
            return iterator->second;
        };
    }

    [[nodiscard]] const std::vector<std::string>& reads() const noexcept {
        return reads_;
    }

private:
    std::map<std::string, std::vector<std::byte>> files_;
    std::vector<std::string> reads_;
};

} // namespace

int main() {
    // Recovered representation precedence: a positive-size packed image wins
    // and the .lst fallback is not queried.
    {
        Fixture fixture;
        fixture.put("room\\stage.pac", bytes("PACKED"));
        fixture.put("room\\stage.lst", bytes("child.bin\r\n"));
        const auto report = LooseContainerListPolicy::materialize(
            "room\\stage.pac", fixture.reader());
        assert(report.ok());
        assert(report.representation == LooseContainerRepresentation::packed);
        assert(report.selected_path == "room\\stage.pac");
        assert(report.bytes == bytes("PACKED"));
        assert(fixture.reads().size() == 1U);
        assert(fixture.reads()[0] == "room\\stage.pac");
    }

    // A path without an extension cannot enter the recovered .lst rewrite.
    {
        Fixture fixture;
        const auto report = LooseContainerListPolicy::materialize(
            "room\\stage", fixture.reader());
        assert(report.status == LooseContainerStatus::invalid_path);
        assert(!report.ok());
        assert(fixture.reads().size() == 1U);
    }

    // Neither positive-size packed representation nor list fallback exists.
    {
        Fixture fixture;
        const auto report = LooseContainerListPolicy::materialize(
            "room\\missing.pac", fixture.reader());
        assert(report.status == LooseContainerStatus::not_found);
        assert(fixture.reads().size() == 2U);
        assert(fixture.reads()[1] == "room\\missing.lst");
    }

    // Direct grammar: '/' is comment/skip, '#' is magic directive, blank
    // CRLF does not count, dummy does count as a declared sparse slot.
    {
        const auto document = LooseContainerListPolicy::parse(bytes(
            "/comment\r\n"
            "#PNST\r\n"
            "\r\n"
            "dummy\r\n"
            "child.bin\r\n"));
        assert(document.ok());
        assert(document.magic_from_directive);
        assert(document.entries.size() == 2U);
        assert(document.entries[0].token == "dummy");
        assert(document.entries[0].dummy);
        assert(document.entries[1].token == "child.bin");
        assert(!document.entries[1].dummy);
    }

    // A '#' directive captures immediate bytes. Whitespace is data, not
    // automatically skipped before magic capture.
    {
        const auto document = LooseContainerListPolicy::parse(bytes(
            "# PNST\r\n"
            "child.bin\r\n"));
        assert(document.ok());
        assert(document.magic_from_directive);
        assert(std::to_integer<unsigned char>(document.magic[0]) == ' ');
        assert(std::to_integer<unsigned char>(document.magic[1]) == 'P');
        assert(std::to_integer<unsigned char>(document.magic[2]) == 'N');
        assert(std::to_integer<unsigned char>(document.magic[3]) == 'S');
        assert(document.entries.size() == 1U);
    }

    // No magic directive -> PAC\0 default. Ordinary child tokens are loaded as
    // baseDirectory + raw token; there is no generic rewrite to .pac.
    {
        Fixture fixture;
        fixture.put("room\\stage.lst", bytes(
            "/ ignored\r\n"
            "first.bin\r\n"
            "dummy\r\n"
            "second.dat\r\n"));
        fixture.put("room\\first.bin", bytes("ONE"));
        fixture.put("room\\second.dat", bytes("TWO2"));

        const auto report = LooseContainerListPolicy::materialize(
            "room\\stage.pac", fixture.reader());
        assert(report.ok());
        assert(report.representation == LooseContainerRepresentation::loose_list);
        assert(report.selected_path == "room\\stage.lst");
        assert(report.bytes.size() == 0xC0U);
        assert(magic_string(report.bytes) == std::string("PAC\0", 4U));
        assert(read_u32(report.bytes, 4U) == 3U);
        assert(read_u32(report.bytes, 8U) == 0x40U);
        assert(read_u32(report.bytes, 12U) == 0U);
        assert(read_u32(report.bytes, 16U) == 0x80U);
        assert(report.bytes[0x40U] == static_cast<std::byte>('O'));
        assert(report.bytes[0x80U] == static_cast<std::byte>('T'));

        bool read_first_bin = false;
        bool read_first_pac = false;
        for (const auto& path : fixture.reads()) {
            read_first_bin = read_first_bin || path == "room\\first.bin";
            read_first_pac = read_first_pac || path == "room\\first.pac";
        }
        assert(read_first_bin);
        assert(!read_first_pac);
    }

    // Nested .lst uses the specifically recovered packed sibling precedence.
    {
        Fixture fixture;
        fixture.put("root\\parent.lst", bytes("nested.lst\r\n"));
        fixture.put("root\\nested.pac", bytes("NESTED-PACKED"));
        fixture.put("root\\nested.lst", bytes("loose.bin\r\n"));
        fixture.put("root\\loose.bin", bytes("SHOULD-NOT-BE-READ"));

        const auto report = LooseContainerListPolicy::materialize(
            "root\\parent.pac", fixture.reader());
        assert(report.ok());
        assert(read_u32(report.bytes, 4U) == 1U);
        assert(read_u32(report.bytes, 8U) == 0x40U);
        assert(report.bytes[0x40U] == static_cast<std::byte>('N'));

        bool read_nested_pac = false;
        bool read_nested_lst = false;
        bool read_loose = false;
        for (const auto& path : fixture.reads()) {
            read_nested_pac = read_nested_pac || path == "root\\nested.pac";
            read_nested_lst = read_nested_lst || path == "root\\nested.lst";
            read_loose = read_loose || path == "root\\loose.bin";
        }
        assert(read_nested_pac);
        assert(!read_nested_lst);
        assert(!read_loose);
    }

    // If nested sibling .pac is unavailable, the nested .lst is synthesized in
    // place and its own magic/layout is preserved in the parent child span.
    {
        Fixture fixture;
        fixture.put("root\\parent.lst", bytes("nested.lst\r\n"));
        fixture.put("root\\nested.lst", bytes(
            "#PNST\r\n"
            "leaf.bin\r\n"));
        fixture.put("root\\leaf.bin", bytes("LEAF"));

        const auto report = LooseContainerListPolicy::materialize(
            "root\\parent.pac", fixture.reader());
        assert(report.ok());
        const auto nested_offset = read_u32(report.bytes, 8U);
        assert(nested_offset == 0x40U);
        assert(report.bytes.size() == 0xC0U);
        assert(static_cast<char>(std::to_integer<unsigned char>(
                   report.bytes[nested_offset + 0U])) == 'P');
        assert(static_cast<char>(std::to_integer<unsigned char>(
                   report.bytes[nested_offset + 1U])) == 'N');
        assert(static_cast<char>(std::to_integer<unsigned char>(
                   report.bytes[nested_offset + 2U])) == 'S');
        assert(static_cast<char>(std::to_integer<unsigned char>(
                   report.bytes[nested_offset + 3U])) == 'T');
        assert(read_u32(report.bytes, nested_offset + 4U) == 1U);
        assert(read_u32(report.bytes, nested_offset + 8U) == 0x40U);
    }

    // Ordinary missing/zero-size child is not the recovered explicit `dummy`.
    {
        Fixture fixture;
        fixture.put("room\\stage.lst", bytes("missing.bin\r\n"));
        const auto report = LooseContainerListPolicy::materialize(
            "room\\stage.pac", fixture.reader());
        assert(report.status == LooseContainerStatus::child_unavailable);
    }
    {
        Fixture fixture;
        fixture.put("room\\stage.lst", bytes("empty.bin\r\n"));
        fixture.put("room\\empty.bin", {});
        const auto report = LooseContainerListPolicy::materialize(
            "room\\stage.pac", fixture.reader());
        assert(report.status == LooseContainerStatus::child_unavailable);
    }

    // LF-only normal resource lines are not promoted as equivalent to the
    // recovered CRLF-oriented grammar.
    {
        const auto document = LooseContainerListPolicy::parse(
            bytes("child.bin\n"));
        assert(document.status == LooseContainerStatus::lf_only_normal_line);
    }

    // Recovered token and scan ceilings fail closed.
    {
        std::string token(LooseContainerListPolicy::token_limit + 1U, 'a');
        token += "\r\n";
        const auto document = LooseContainerListPolicy::parse(bytes(token));
        assert(document.status == LooseContainerStatus::token_limit_exceeded);
    }
    {
        std::string oversized(
            LooseContainerListPolicy::scan_limit + 1U, '/');
        const auto document = LooseContainerListPolicy::parse(bytes(oversized));
        assert(document.status == LooseContainerStatus::scan_limit_exceeded);
    }

    // Cycle and depth guards are product safety policy, deliberately separate
    // from the recovered grammar/layout claims.
    {
        Fixture fixture;
        fixture.put("root\\a.lst", bytes("a.lst\r\n"));
        const auto report = LooseContainerListPolicy::materialize(
            "root\\a.pac", fixture.reader());
        assert(report.status == LooseContainerStatus::recursion_cycle);
    }
    {
        Fixture fixture;
        fixture.put("root\\a.lst", bytes("b.lst\r\n"));
        fixture.put("root\\b.lst", bytes("leaf.bin\r\n"));
        fixture.put("root\\leaf.bin", bytes("LEAF"));
        const auto report = LooseContainerListPolicy::materialize(
            "root\\a.pac", fixture.reader(),
            LooseContainerSafetyLimits{
                .max_recursion_depth = 1U,
                .max_output_bytes = 0x10000U,
            });
        assert(report.status == LooseContainerStatus::recursion_limit);
    }

    // Helper boundaries.
    assert(LooseContainerListPolicy::list_path_for("a\\b.PAC") == "a\\b.lst");
    assert(!LooseContainerListPolicy::list_path_for("a\\b").has_value());
    assert(LooseContainerListPolicy::packed_sibling_for_list("a\\b.lst") ==
           "a\\b.pac");
    assert(!LooseContainerListPolicy::packed_sibling_for_list("a\\b.LST").has_value());
    assert(LooseContainerListPolicy::header_size(0U) == 0x40U);
    assert(LooseContainerListPolicy::header_size(15U) == 0x80U);

    return 0;
}
