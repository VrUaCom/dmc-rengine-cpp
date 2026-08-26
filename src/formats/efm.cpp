#include "dmc_rengine/formats/efm.hpp"

#include "dmc_rengine/profiles/dmc3/efm_contract.hpp"

#include <cstddef>
#include <utility>

namespace dmc::rengine::formats {
namespace {

using Contract = profiles::dmc3::EfmContract;

[[nodiscard]] bool fits(
    std::uint64_t offset,
    std::uint64_t size,
    std::uint64_t total) noexcept {
    return offset <= total && size <= total - offset;
}

[[nodiscard]] std::uint16_t read_u16_le(
    std::span<const std::byte> bytes,
    std::size_t offset) noexcept {
    return static_cast<std::uint16_t>(
        std::to_integer<std::uint16_t>(bytes[offset]) |
        static_cast<std::uint16_t>(
            std::to_integer<std::uint16_t>(bytes[offset + 1U]) << 8U));
}

[[nodiscard]] std::int32_t read_i32_le(
    std::span<const std::byte> bytes,
    std::size_t offset) noexcept {
    std::uint32_t value = 0U;
    for (std::size_t index = 0U; index < 4U; ++index) {
        value |= std::to_integer<std::uint32_t>(bytes[offset + index])
            << (8U * index);
    }
    return static_cast<std::int32_t>(value);
}

[[nodiscard]] std::uint64_t read_u64_le(
    std::span<const std::byte> bytes,
    std::size_t offset) noexcept {
    std::uint64_t value = 0U;
    for (std::size_t index = 0U; index < 8U; ++index) {
        value |= std::to_integer<std::uint64_t>(bytes[offset + index])
            << (8U * index);
    }
    return value;
}

[[nodiscard]] std::uint64_t aligned(std::uint64_t raw) noexcept {
    return (raw + Contract::array_alignment - 1U) /
        Contract::array_alignment * Contract::array_alignment;
}

[[nodiscard]] EfmParseResult fail(EfmParseError error, std::string message) {
    return EfmParseResult{
        .document = std::nullopt,
        .error = error,
        .message = std::move(message),
    };
}

} // namespace

bool EfmPrimitiveBatch::valid(std::uint64_t document_size) const noexcept {
    if (vertex_count == 0U) {
        return false;
    }
    const auto count = static_cast<std::uint64_t>(vertex_count);
    return fits(position_offset, count * Contract::position_element_bytes, document_size) &&
        fits(normal_offset, count * Contract::normal_element_bytes, document_size) &&
        fits(attribute_offset, count * Contract::attribute_element_bytes, document_size) &&
        fits(secondary_offset, count * Contract::secondary_element_bytes, document_size) &&
        fits(control_offset, count * Contract::index_element_bytes, document_size) &&
        // Base only: the routine never indexes through this one, so claiming
        // an extent for it would be inventing the element width.
        extra_offset < document_size &&
        strip_offset + Contract::strip_element_bytes <= document_size;
}

std::uint64_t EfmDocument::total_vertex_count() const noexcept {
    std::uint64_t total = 0U;
    for (const auto& batch : batches) {
        total += batch.vertex_count;
    }
    return total;
}

bool EfmDocument::valid() const noexcept {
    if (document_size == 0U || groups.size() != group_count) {
        return false;
    }
    for (const auto& batch : batches) {
        if (!batch.valid(document_size)) {
            return false;
        }
    }
    return true;
}

EfmParseResult EfmParser::parse(std::span<const std::byte> bytes) {
    const auto shell = RelocatedModelShell::parse(
        bytes, Contract::magic, Contract::batch_stride);
    if (!shell.ok()) {
        return fail(
            EfmParseError::shell_rejected,
            shell.message.empty() ? "model shell rejected the byte image"
                                  : shell.message);
    }

    EfmDocument document;
    document.document_size = shell.shell->document_size;
    document.group_count = shell.shell->group_count;
    document.document_mode = shell.shell->document_mode;
    document.document_pointer = shell.shell->document_pointer;
    document.groups = shell.shell->groups;

    const auto total = document.document_size;
    for (const auto& group : document.groups) {
        const auto first = document.batches.size();
        for (std::uint32_t batch = 0U; batch < group.batch_count; ++batch) {
            const auto batch_offset = group.first_batch_offset +
                static_cast<std::uint64_t>(batch) * Contract::batch_stride;
            if (!fits(batch_offset, Contract::batch_stride, total)) {
                return fail(
                    EfmParseError::batch_out_of_bounds,
                    "a batch header extends past the end of the effect model");
            }
            const auto at = [&](std::size_t field) {
                return static_cast<std::size_t>(batch_offset) + field;
            };

            EfmPrimitiveBatch record{
                .group_index = group.group_index,
                .batch_index = batch,
                .batch_offset = batch_offset,
                .vertex_count = read_u16_le(bytes, at(Contract::batch_index_count_offset)),
                .position_offset = read_u64_le(bytes, at(Contract::batch_position_offset)),
                .normal_offset = read_u64_le(bytes, at(Contract::batch_normal_offset)),
                .attribute_offset = read_u64_le(bytes, at(Contract::batch_attribute_offset)),
                .secondary_offset = read_u64_le(bytes, at(Contract::batch_secondary_offset)),
                .control_offset = read_u64_le(bytes, at(Contract::batch_index_offset)),
                .extra_offset = read_u64_le(bytes, at(Contract::batch_extra_offset)),
                .strip_offset = batch_offset +
                    read_u64_le(bytes, at(Contract::batch_strip_offset)),
                .stored_strip_length = read_i32_le(bytes, at(Contract::batch_strip_length_offset)),
                .strip_marker_present = false,
                .break_count = 0U,
            };

            if (!record.valid(total)) {
                return fail(
                    EfmParseError::array_out_of_bounds,
                    "a batch array relocates outside the effect model");
            }
            record.strip_marker_present =
                read_u16_le(bytes, static_cast<std::size_t>(record.strip_offset)) ==
                Contract::strip_buffer_marker;

            for (std::uint32_t vertex = 0U; vertex < record.vertex_count; ++vertex) {
                const auto control = read_u16_le(
                    bytes,
                    static_cast<std::size_t>(
                        record.control_offset +
                        static_cast<std::uint64_t>(vertex) * Contract::index_element_bytes));
                if ((control & Contract::index_break_mask) != 0U) {
                    ++record.break_count;
                }
            }

            document.batches.push_back(record);
        }

        // The extra array is deliberately absent from this check: without an
        // element width there is no span to place it by, and guessing one to
        // make the arithmetic close would be manufacturing the fact.
        const std::pair<std::uint64_t EfmPrimitiveBatch::*, std::uint64_t> kinds[]{
            {&EfmPrimitiveBatch::position_offset, Contract::position_element_bytes},
            {&EfmPrimitiveBatch::normal_offset, Contract::normal_element_bytes},
            {&EfmPrimitiveBatch::attribute_offset, Contract::attribute_element_bytes},
            {&EfmPrimitiveBatch::secondary_offset, Contract::secondary_element_bytes},
            {&EfmPrimitiveBatch::control_offset, Contract::index_element_bytes},
        };
        if (group.batch_count != 0U) {
            auto cursor = document.batches[first].position_offset;
            for (const auto& [field, element_bytes] : kinds) {
                for (std::uint32_t batch = 0U; batch < group.batch_count; ++batch) {
                    const auto& record = document.batches[first + batch];
                    if (record.*field != cursor) {
                        return fail(
                            EfmParseError::array_packing_mismatch,
                            "a batch array is not where the group's packing puts it");
                    }
                    cursor += aligned(
                        static_cast<std::uint64_t>(record.vertex_count) * element_bytes);
                }
            }
        }
    }

    if (!document.valid()) {
        return fail(
            EfmParseError::invalid_document,
            "effect model decoded to an inconsistent document");
    }

    return EfmParseResult{
        .document = std::move(document),
        .error = EfmParseError::none,
        .message = {},
    };
}

} // namespace dmc::rengine::formats
