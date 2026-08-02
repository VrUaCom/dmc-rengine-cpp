#include "dmc_rengine/formats/synthetic/slot_container_parser.hpp"

#include "dmc_rengine/binary/reader.hpp"

#include <algorithm>
#include <cstdint>
#include <iomanip>
#include <limits>
#include <optional>
#include <sstream>
#include <string>
#include <vector>

namespace dmc::rengine::formats::synthetic {
namespace {

constexpr std::size_t header_size = 16U;
constexpr std::size_t entry_size = 16U;
constexpr std::uint32_t populated_flag = 0x1U;
constexpr std::uint32_t maximum_slots = 4096U;
constexpr std::size_t maximum_name_length = 256U;

[[nodiscard]] std::string synthetic_name(
    std::uint32_t slot,
    std::string_view suffix) {
    std::ostringstream output;
    output << "slot_" << std::setfill('0') << std::setw(4) << slot << suffix;
    return output.str();
}

[[nodiscard]] std::optional<std::string> read_c_string(
    const binary::Reader& reader,
    std::size_t offset) {
    std::string value;
    value.reserve(32U);

    for (std::size_t index = 0; index < maximum_name_length; ++index) {
        const auto byte = reader.u8(offset + index);
        if (!byte.has_value()) {
            return std::nullopt;
        }
        if (*byte == 0U) {
            return value;
        }
        value.push_back(static_cast<char>(*byte));
    }

    return std::nullopt;
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

std::string_view SlotContainerParser::id() const noexcept {
    return "synthetic-slot-container-v1";
}

std::string_view SlotContainerParser::format() const noexcept {
    return "synthetic-slot-container";
}

int SlotContainerParser::probe(
    std::span<const std::byte> bytes,
    std::string_view) const noexcept {
    return binary::Reader(bytes).matches(0U, "SLTC") ? 100 : 0;
}

ContainerParseResult SlotContainerParser::parse(
    std::span<const std::byte> bytes,
    std::string_view) const {
    ContainerParseResult result;
    result.document.format = std::string{format()};
    result.document.container_size = static_cast<std::uint64_t>(bytes.size());

    const binary::Reader reader(bytes);
    if (!reader.matches(0U, "SLTC")) {
        add_diagnostic(
            result,
            ParseSeverity::warning,
            "synthetic_slot.unrecognized",
            "The resource does not contain the synthetic SLTC magic.",
            0U);
        return result;
    }
    result.recognized = true;

    if (!reader.contains(0U, header_size)) {
        add_diagnostic(
            result,
            ParseSeverity::error,
            "synthetic_slot.truncated_header",
            "The synthetic slot-container header is truncated.",
            0U);
        return result;
    }

    const auto version = reader.u16_le(4U);
    const auto slot_count = reader.u16_le(6U);
    const auto table_offset = reader.u32_le(8U);
    const auto strings_offset = reader.u32_le(12U);
    if (!version.has_value() || !slot_count.has_value() ||
        !table_offset.has_value() || !strings_offset.has_value()) {
        add_diagnostic(
            result,
            ParseSeverity::error,
            "synthetic_slot.invalid_header",
            "Required synthetic slot-container header fields are unavailable.",
            0U);
        return result;
    }

    result.document.schema_version = *version;
    result.document.declared_slot_count = *slot_count;

    if (*version != 1U) {
        add_diagnostic(
            result,
            ParseSeverity::error,
            "synthetic_slot.unsupported_version",
            "Only synthetic slot-container schema version 1 is supported.",
            4U);
        return result;
    }

    if (*slot_count > maximum_slots) {
        add_diagnostic(
            result,
            ParseSeverity::error,
            "synthetic_slot.slot_limit",
            "The declared slot count exceeds the safety limit.",
            6U);
        return result;
    }

    const auto table_size = static_cast<std::uint64_t>(*slot_count) * entry_size;
    if (*table_offset > bytes.size() ||
        table_size > static_cast<std::uint64_t>(bytes.size() - *table_offset)) {
        add_diagnostic(
            result,
            ParseSeverity::error,
            "synthetic_slot.truncated_table",
            "The declared slot table is outside the resource.",
            8U);
        return result;
    }

    if (*strings_offset > bytes.size()) {
        add_diagnostic(
            result,
            ParseSeverity::warning,
            "synthetic_slot.invalid_strings_offset",
            "The string-table offset is outside the resource; fallback names will be used.",
            12U);
    }

    result.document.entries.reserve(*slot_count);
    for (std::uint32_t slot = 0; slot < *slot_count; ++slot) {
        const auto entry_offset = static_cast<std::size_t>(*table_offset) +
            static_cast<std::size_t>(slot) * entry_size;
        const auto data_offset = reader.u32_le(entry_offset);
        const auto data_size = reader.u32_le(entry_offset + 4U);
        const auto name_offset = reader.u32_le(entry_offset + 8U);
        const auto flags = reader.u32_le(entry_offset + 12U);

        if (!data_offset.has_value() || !data_size.has_value() ||
            !name_offset.has_value() || !flags.has_value()) {
            add_diagnostic(
                result,
                ParseSeverity::error,
                "synthetic_slot.truncated_entry",
                "A slot entry is truncated.",
                static_cast<std::uint64_t>(entry_offset));
            return result;
        }

        const auto populated = (*flags & populated_flag) != 0U;
        if (!populated) {
            if (*data_offset != 0U || *data_size != 0U || *name_offset != 0U) {
                add_diagnostic(
                    result,
                    ParseSeverity::warning,
                    "synthetic_slot.empty_slot_residue",
                    "An empty slot contains non-zero residual fields; the fields were ignored.",
                    static_cast<std::uint64_t>(entry_offset));
            }

            result.document.entries.push_back(ContainerEntry{
                .slot_index = slot,
                .offset = 0U,
                .size = 0U,
                .logical_name = synthetic_name(slot, ".empty"),
                .populated = false,
                .synthetic_name = true,
            });
            continue;
        }

        const auto offset64 = static_cast<std::uint64_t>(*data_offset);
        const auto size64 = static_cast<std::uint64_t>(*data_size);
        if (size64 == 0U || offset64 > bytes.size() ||
            size64 > static_cast<std::uint64_t>(bytes.size()) - offset64) {
            add_diagnostic(
                result,
                ParseSeverity::error,
                "synthetic_slot.entry_out_of_range",
                "A populated slot points outside the container and was preserved as an invalid empty slot.",
                static_cast<std::uint64_t>(entry_offset));
            result.document.entries.push_back(ContainerEntry{
                .slot_index = slot,
                .offset = 0U,
                .size = 0U,
                .logical_name = synthetic_name(slot, ".invalid"),
                .populated = false,
                .synthetic_name = true,
            });
            continue;
        }

        std::string name;
        bool used_synthetic_name = false;
        if (*name_offset == 0U || *strings_offset > bytes.size()) {
            name = synthetic_name(slot, ".bin");
            used_synthetic_name = true;
        } else {
            const auto absolute_name_offset =
                static_cast<std::uint64_t>(*strings_offset) + *name_offset;
            if (absolute_name_offset > bytes.size()) {
                name = synthetic_name(slot, ".bin");
                used_synthetic_name = true;
                add_diagnostic(
                    result,
                    ParseSeverity::warning,
                    "synthetic_slot.name_out_of_range",
                    "A slot name points outside the string table; a fallback name was assigned.",
                    static_cast<std::uint64_t>(entry_offset + 8U));
            } else {
                const auto parsed_name = read_c_string(
                    reader, static_cast<std::size_t>(absolute_name_offset));
                if (!parsed_name.has_value() || parsed_name->empty()) {
                    name = synthetic_name(slot, ".bin");
                    used_synthetic_name = true;
                    add_diagnostic(
                        result,
                        ParseSeverity::warning,
                        "synthetic_slot.invalid_name",
                        "A slot name is missing or unterminated; a fallback name was assigned.",
                        absolute_name_offset);
                } else {
                    name = *parsed_name;
                }
            }
        }

        result.document.entries.push_back(ContainerEntry{
            .slot_index = slot,
            .offset = offset64,
            .size = size64,
            .logical_name = std::move(name),
            .populated = true,
            .synthetic_name = used_synthetic_name,
        });
    }

    for (std::size_t left = 0; left < result.document.entries.size(); ++left) {
        const auto& first = result.document.entries[left];
        if (!first.populated) {
            continue;
        }
        for (std::size_t right = left + 1U;
             right < result.document.entries.size();
             ++right) {
            const auto& second = result.document.entries[right];
            if (!second.populated) {
                continue;
            }

            const auto first_end = first.offset + first.size;
            const auto second_end = second.offset + second.size;
            if (first.offset < second_end && second.offset < first_end) {
                add_diagnostic(
                    result,
                    ParseSeverity::warning,
                    "synthetic_slot.entry_overlap",
                    "Two populated slots overlap in the container.",
                    std::max(first.offset, second.offset));
            }
        }
    }

    return result;
}

} // namespace dmc::rengine::formats::synthetic
