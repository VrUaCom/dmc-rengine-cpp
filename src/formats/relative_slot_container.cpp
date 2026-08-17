#include "dmc_rengine/formats/relative_slot_container.hpp"

#include <algorithm>
#include <iomanip>
#include <limits>
#include <sstream>
#include <utility>
#include <vector>

namespace dmc::rengine::formats {
namespace {

[[nodiscard]] std::uint32_t read_u32_le(
    std::span<const std::byte> bytes,
    std::size_t offset) noexcept {
    return std::to_integer<std::uint32_t>(bytes[offset]) |
        (std::to_integer<std::uint32_t>(bytes[offset + 1U]) << 8U) |
        (std::to_integer<std::uint32_t>(bytes[offset + 2U]) << 16U) |
        (std::to_integer<std::uint32_t>(bytes[offset + 3U]) << 24U);
}

[[nodiscard]] bool matches_magic(
    std::span<const std::byte> bytes,
    std::string_view magic) noexcept {
    if (magic.size() != 4U || bytes.size() < magic.size()) {
        return false;
    }
    for (std::size_t index = 0; index < magic.size(); ++index) {
        if (std::to_integer<unsigned char>(bytes[index]) !=
            static_cast<unsigned char>(magic[index])) {
            return false;
        }
    }
    return true;
}

[[nodiscard]] std::string synthetic_slot_name(
    std::uint32_t slot,
    bool populated) {
    std::ostringstream output;
    output << "slot_" << std::setfill('0') << std::setw(4) << slot
           << (populated ? ".bin" : ".empty");
    return output.str();
}

[[nodiscard]] RelativeSlotParseResult fail(
    RelativeSlotParseError error,
    std::string message) {
    return RelativeSlotParseResult{
        .document = std::nullopt,
        .error = error,
        .message = std::move(message),
    };
}

} // namespace

RelativeSlotParseResult parse_relative_slot_container(
    std::span<const std::byte> bytes,
    const RelativeSlotContainerSpec& spec) {
    if (spec.format.empty() || spec.magic.size() != 4U ||
        spec.display_label.empty()) {
        return fail(
            RelativeSlotParseError::invalid_document,
            "Relative-slot parser specification is invalid");
    }

    if (bytes.size() < 8U) {
        return fail(
            RelativeSlotParseError::truncated_header,
            std::string{spec.display_label} + " header requires at least 8 bytes");
    }

    if (!matches_magic(bytes, spec.magic)) {
        return fail(
            RelativeSlotParseError::invalid_magic,
            std::string{spec.display_label} + " magic does not match");
    }

    const auto slot_count = read_u32_le(bytes, 4U);
    if (slot_count > k_relative_slot_max_slot_count) {
        return fail(
            RelativeSlotParseError::slot_count_limit,
            std::string{spec.display_label} +
                " slot count exceeds the product parser safety limit");
    }

    if (slot_count >
        (std::numeric_limits<std::size_t>::max() - 8U) / 4U) {
        return fail(
            RelativeSlotParseError::truncated_offset_table,
            std::string{spec.display_label} +
                " offset table size overflows the host size type");
    }

    const auto table_end =
        8U + static_cast<std::size_t>(slot_count) * 4U;
    if (table_end > bytes.size()) {
        return fail(
            RelativeSlotParseError::truncated_offset_table,
            std::string{spec.display_label} +
                " offset table exceeds the supplied byte span");
    }

    std::vector<std::uint32_t> offsets(slot_count, 0U);
    std::vector<std::uint32_t> distinct_populated_offsets;
    distinct_populated_offsets.reserve(slot_count);

    for (std::uint32_t slot = 0U; slot < slot_count; ++slot) {
        const auto table_offset =
            8U + static_cast<std::size_t>(slot) * 4U;
        const auto offset = read_u32_le(bytes, table_offset);
        offsets[slot] = offset;

        if (offset == 0U) {
            continue;
        }
        if (static_cast<std::size_t>(offset) < table_end) {
            return fail(
                RelativeSlotParseError::slot_offset_before_payload,
                std::string{spec.display_label} +
                    " populated slot points into the header/offset table");
        }
        if (static_cast<std::size_t>(offset) >= bytes.size()) {
            return fail(
                RelativeSlotParseError::slot_offset_out_of_bounds,
                std::string{spec.display_label} +
                    " populated slot offset lies outside the supplied byte span");
        }
        distinct_populated_offsets.push_back(offset);
    }

    std::sort(
        distinct_populated_offsets.begin(),
        distinct_populated_offsets.end());
    distinct_populated_offsets.erase(
        std::unique(
            distinct_populated_offsets.begin(),
            distinct_populated_offsets.end()),
        distinct_populated_offsets.end());

    ContainerDocument document;
    document.format = std::string{spec.format};
    document.schema_version = 1U;
    document.declared_slot_count = slot_count;
    document.container_size = static_cast<std::uint64_t>(bytes.size());
    document.entries.resize(slot_count);

    for (std::uint32_t slot = 0U; slot < slot_count; ++slot) {
        auto& entry = document.entries[slot];
        entry.slot_index = slot;
        entry.synthetic_name = true;

        const auto offset = offsets[slot];
        if (offset == 0U) {
            entry.logical_name = synthetic_slot_name(slot, false);
            continue;
        }

        const auto next = std::upper_bound(
            distinct_populated_offsets.begin(),
            distinct_populated_offsets.end(),
            offset);
        const auto end = next == distinct_populated_offsets.end()
            ? static_cast<std::uint64_t>(bytes.size())
            : static_cast<std::uint64_t>(*next);

        entry.offset = offset;
        entry.size = end - static_cast<std::uint64_t>(offset);
        entry.logical_name = synthetic_slot_name(slot, true);
        entry.populated = true;
    }

    if (!document.valid()) {
        return fail(
            RelativeSlotParseError::invalid_document,
            std::string{spec.display_label} +
                " decoded to an invalid ContainerDocument");
    }

    return RelativeSlotParseResult{
        .document = std::move(document),
        .error = RelativeSlotParseError::none,
        .message = {},
    };
}

} // namespace dmc::rengine::formats
