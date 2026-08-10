#include "dmc_rengine/formats/dmc3/pac_container_parser.hpp"

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <span>
#include <string_view>
#include <vector>

namespace {

void write_u32(
    std::vector<std::byte>& bytes,
    std::size_t offset,
    std::uint32_t value) {
    for (std::size_t index = 0; index < 4U; ++index) {
        bytes[offset + index] = static_cast<std::byte>(
            (value >> static_cast<unsigned>(index * 8U)) & 0xFFU);
    }
}

void write_ascii(
    std::vector<std::byte>& bytes,
    std::size_t offset,
    std::string_view value) {
    for (std::size_t index = 0; index < value.size(); ++index) {
        bytes[offset + index] = static_cast<std::byte>(value[index]);
    }
}

[[nodiscard]] std::vector<std::byte> make_valid_fixture() {
    std::vector<std::byte> bytes(80U, std::byte{0});
    write_ascii(bytes, 0U, std::string_view{"PAC\0", 4U});
    write_u32(bytes, 4U, 5U);

    // Sparse runtime slot table. Slots 2 and 3 intentionally alias one entry.
    write_u32(bytes, 8U, 32U);
    write_u32(bytes, 12U, 0U);
    write_u32(bytes, 16U, 48U);
    write_u32(bytes, 20U, 48U);
    write_u32(bytes, 24U, 64U);

    write_ascii(bytes, 32U, "DDS ");
    write_ascii(bytes, 48U, std::string_view{"SCM\0", 4U});
    write_ascii(bytes, 64U, "HITS");
    return bytes;
}

} // namespace

int main() {
    using dmc::rengine::formats::ParseSeverity;
    using dmc::rengine::formats::dmc3::PacContainerParser;

    PacContainerParser parser;
    assert(parser.id() == "dmc3-pac-runtime-v1");
    assert(parser.format() == "pac");

    const auto bytes = make_valid_fixture();
    assert(parser.probe(std::span<const std::byte>{bytes}, "stage.pac") == 100);

    const auto parsed = parser.parse(
        std::span<const std::byte>{bytes}, "stage.pac");
    assert(parsed.recognized);
    assert(parsed.ok());
    assert(parsed.document.valid());
    assert(parsed.document.declared_slot_count == 5U);
    assert(parsed.document.entries.size() == 5U);

    assert(parsed.document.entries[0].populated);
    assert(parsed.document.entries[0].offset == 32U);
    assert(parsed.document.entries[0].size == 16U);
    assert(parsed.document.entries[0].slot_index == 0U);
    assert(parsed.document.entries[0].synthetic_name);

    assert(!parsed.document.entries[1].populated);
    assert(parsed.document.entries[1].slot_index == 1U);

    assert(parsed.document.entries[2].populated);
    assert(parsed.document.entries[2].offset == 48U);
    assert(parsed.document.entries[2].size == 16U);
    assert(parsed.document.entries[3].populated);
    assert(parsed.document.entries[3].offset == 48U);
    assert(parsed.document.entries[3].size == 16U);

    assert(parsed.document.entries[4].populated);
    assert(parsed.document.entries[4].offset == 64U);
    assert(parsed.document.entries[4].size == 16U);

    bool found_alias_warning = false;
    for (const auto& diagnostic : parsed.diagnostics) {
        if (diagnostic.severity == ParseSeverity::warning &&
            diagnostic.code == "dmc3.pac.duplicate_offset") {
            found_alias_warning = true;
        }
    }
    assert(found_alias_warning);

    auto malformed_bytes = make_valid_fixture();
    write_u32(malformed_bytes, 24U, 12U);
    const auto malformed = parser.parse(
        std::span<const std::byte>{malformed_bytes}, "malformed.pac");
    assert(malformed.recognized);
    assert(!malformed.ok());
    assert(malformed.document.entries.size() == 5U);
    assert(!malformed.document.entries[4].populated);

    bool found_range_error = false;
    for (const auto& diagnostic : malformed.diagnostics) {
        if (diagnostic.severity == ParseSeverity::error &&
            diagnostic.code == "dmc3.pac.offset_out_of_range") {
            found_range_error = true;
        }
    }
    assert(found_range_error);

    auto not_pac = bytes;
    not_pac[2U] = std::byte{'X'};
    const auto unrecognized = parser.parse(
        std::span<const std::byte>{not_pac}, "not-pac.bin");
    assert(!unrecognized.recognized);
    assert(!unrecognized.ok());

    return 0;
}
