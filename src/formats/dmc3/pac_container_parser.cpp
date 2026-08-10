#include "dmc_rengine/formats/dmc3/pac_container_parser.hpp"

#include "dmc_rengine/binary/reader.hpp"

#include <algorithm>
#include <cstdint>
#include <iomanip>
#include <sstream>
#include <string>
#include <vector>

namespace dmc::rengine::formats::dmc3 {
namespace {

constexpr std::size_t header_size = 8U;
constexpr std::size_t offset_size = 4U;
constexpr std::uint32_t maximum_slots = 65536U;
constexpr std::string_view pac_magic{"PAC\0", 4U};

[[nodiscard]] std::string synthetic_slot_name(
    std::uint32_t slot,
    std::string_view suffix) {
    std::ostringstream output;
    output << "slot_" << std::setfill('0') << std::setw(4) << slot << suffix;
    return output.str();
}

void add_diagnostic(
    ContainerParseResult& result,
    ParseSeverity severity,
    std::string code,
    std::string message,
    std::uint64_t offset) {
    result.diagnostics.push_back(ParseDiagnostic{
        .severity = severity,
        .code = std::move(code),
        .message = std::move(message),
        .offset = offset,
    });
}

} // namespace

std::string_view PacContainerParser::id() const noexcept {
    return "dmc3-pac-runtime-v1";
}

std::string_view PacContainerParser::format() const noexcept {
    return "pac";
}

int PacContainerParser::probe(
    std::span<const std::byte> bytes,
    std::string_view) const noexcept {
    return binary::Reader(bytes).matches(0U, pac_magic) ? 100 : 0;
}

ContainerParseResult PacContainerParser::parse(
    std::span<const std::byte> bytes,
    std::string_view) const {
    ContainerParseResult result;
    result.document.format = std::string{format()};
    result.document.schema_version = 1U;
    result.document.container_size = static_cast<std::uint64_t>(bytes.size());

    const binary::Reader reader(bytes);
    if (!reader.matches(0U, pac_magic)) {
        add_diagnostic(
            result,
            ParseSeverity::warning,
            "dmc3.pac.unrecognized",
            "The resource does not contain the confirmed PAC\\0 magic.",
            0U);
        return result;
    }
    result.recognized = true;

    if (!reader.contains(0U, header_size)) {
        add_diagnostic(
            result,
            ParseSeverity::error,
            "dmc3.pac.truncated_header",
            "The PAC header is truncated before slotCount at +0x04.",
            0U);
        return result;
    }

    const auto slot_count = reader.u32_le(4U);
    if (!slot_count.has_value()) {
        add_diagnostic(
            result,
            ParseSeverity::error,
            "dmc3.pac.missing_slot_count",
            "The PAC slotCount field at +0x04 could not be read.",
            4U);
        return result;
    }
    result.document.declared_slot_count = *slot_count;

    if (*slot_count > maximum_slots) {
        add_diagnostic(
            result,
            ParseSeverity::error,
            "dmc3.pac.slot_limit",
            "The PAC slotCount exceeds the bounded read-only safety limit.",
            4U);
        return result;
    }

    const auto table_bytes = static_cast<std::uint64_t>(*slot_count) * offset_size;
    if (table_bytes > static_cast<std::uint64_t>(bytes.size()) ||
        header_size > bytes.size() ||
        table_bytes > static_cast<std::uint64_t>(bytes.size() - header_size)) {
        add_diagnostic(
            result,
            ParseSeverity::error,
            "dmc3.pac.truncated_offset_table",
            "The relative-offset table at +0x08 extends outside the PAC resource.",
            8U);
        return result;
    }

    const auto table_end = header_size + static_cast<std::size_t>(table_bytes);
    std::vector<std::uint32_t> slot_offsets(*slot_count, 0U);
    std::vector<std::uint32_t> populated_offsets;
    populated_offsets.reserve(*slot_count);

    for (std::uint32_t slot = 0; slot < *slot_count; ++slot) {
        const auto field_offset = header_size +
            static_cast<std::size_t>(slot) * offset_size;
        const auto relative_offset = reader.u32_le(field_offset);
        if (!relative_offset.has_value()) {
            add_diagnostic(
                result,
                ParseSeverity::error,
                "dmc3.pac.truncated_offset",
                "A PAC relative-offset entry could not be read.",
                static_cast<std::uint64_t>(field_offset));
            continue;
        }

        if (*relative_offset == 0U) {
            continue;
        }

        if (*relative_offset < table_end || *relative_offset >= bytes.size()) {
            add_diagnostic(
                result,
                ParseSeverity::error,
                "dmc3.pac.offset_out_of_range",
                "A populated PAC slot points into the header/table or outside the resource.",
                static_cast<std::uint64_t>(field_offset));
            continue;
        }

        slot_offsets[slot] = *relative_offset;
        populated_offsets.push_back(*relative_offset);
    }

    std::sort(populated_offsets.begin(), populated_offsets.end());
    const auto unique_end = std::unique(populated_offsets.begin(), populated_offsets.end());
    const auto had_duplicate_offsets = unique_end != populated_offsets.end();
    populated_offsets.erase(unique_end, populated_offsets.end());

    if (had_duplicate_offsets) {
        add_diagnostic(
            result,
            ParseSeverity::warning,
            "dmc3.pac.duplicate_offset",
            "Multiple PAC slots reference the same relative offset; the shared offset is preserved without assigning alias semantics.",
            8U);
    }

    result.document.entries.reserve(*slot_count);
    for (std::uint32_t slot = 0; slot < *slot_count; ++slot) {
        const auto offset = slot_offsets[slot];
        if (offset == 0U) {
            result.document.entries.push_back(ContainerEntry{
                .slot_index = slot,
                .offset = 0U,
                .size = 0U,
                .logical_name = synthetic_slot_name(slot, ".empty"),
                .populated = false,
                .synthetic_name = true,
            });
            continue;
        }

        const auto next = std::upper_bound(
            populated_offsets.begin(), populated_offsets.end(), offset);
        const auto end_offset = next == populated_offsets.end()
            ? static_cast<std::uint64_t>(bytes.size())
            : static_cast<std::uint64_t>(*next);
        const auto offset64 = static_cast<std::uint64_t>(offset);

        if (end_offset <= offset64) {
            add_diagnostic(
                result,
                ParseSeverity::error,
                "dmc3.pac.invalid_inferred_range",
                "A PAC slot did not produce a positive bounded inferred byte range.",
                offset64);
            result.document.entries.push_back(ContainerEntry{
                .slot_index = slot,
                .offset = 0U,
                .size = 0U,
                .logical_name = synthetic_slot_name(slot, ".invalid"),
                .populated = false,
                .synthetic_name = true,
            });
            continue;
        }

        result.document.entries.push_back(ContainerEntry{
            .slot_index = slot,
            .offset = offset64,
            .size = end_offset - offset64,
            .logical_name = synthetic_slot_name(slot, ".bin"),
            .populated = true,
            .synthetic_name = true,
        });
    }

    return result;
}

} // namespace dmc::rengine::formats::dmc3
