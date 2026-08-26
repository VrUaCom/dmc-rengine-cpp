#include "dmc_rengine/formats/scm.hpp"

#include "dmc_rengine/formats/relocated_model_shell.hpp"
#include "dmc_rengine/profiles/dmc3/scm_contract.hpp"

#include <cstddef>
#include <utility>

namespace dmc::rengine::formats {
namespace {

using Contract = profiles::dmc3::ScmContract;

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

[[nodiscard]] ScmParseResult fail(ScmParseError error, std::string message) {
    return ScmParseResult{
        .document = std::nullopt,
        .error = error,
        .message = std::move(message),
    };
}

} // namespace

bool ScmPrimitiveBatch::valid(std::uint64_t document_size) const noexcept {
    if (index_count == 0U) {
        return false;
    }
    const auto vector_bytes =
        static_cast<std::uint64_t>(index_count) * Contract::position_element_bytes;
    const auto word_bytes =
        static_cast<std::uint64_t>(index_count) * Contract::index_element_bytes;
    return fits(position_offset, vector_bytes, document_size) &&
        fits(normal_offset, vector_bytes, document_size) &&
        fits(attribute_offset, word_bytes, document_size) &&
        fits(index_offset, word_bytes, document_size) &&
        strip_offset + Contract::strip_element_bytes <= document_size;
}

std::uint64_t ScmDocument::total_index_count() const noexcept {
    std::uint64_t total = 0U;
    for (const auto& batch : batches) {
        total += batch.index_count;
    }
    return total;
}

bool ScmDocument::valid() const noexcept {
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

ScmParseResult ScmParser::parse(std::span<const std::byte> bytes) {
    // The document and group layer is shared with MOD, byte for byte, because
    // both recovered routines read it the same way. Everything below the group
    // is where the two formats stop agreeing.
    const auto shell = RelocatedModelShell::parse(
        bytes, Contract::magic, Contract::batch_stride);
    if (!shell.ok()) {
        switch (shell.error) {
        case ModelShellError::invalid_magic:
            return fail(ScmParseError::invalid_magic, shell.message);
        case ModelShellError::truncated_header:
            return fail(ScmParseError::truncated_header, shell.message);
        case ModelShellError::group_limit:
            return fail(ScmParseError::group_limit, shell.message);
        case ModelShellError::truncated_group_table:
            return fail(ScmParseError::truncated_group_table, shell.message);
        case ModelShellError::pointer_out_of_bounds:
        case ModelShellError::none:
            break;
        }
        return fail(ScmParseError::group_out_of_bounds, shell.message);
    }

    const auto total = shell.shell->document_size;

    ScmDocument document;
    document.document_size = total;
    document.group_count = shell.shell->group_count;
    document.document_pointer = shell.shell->document_pointer;
    document.groups.reserve(document.group_count);

    for (const auto& group : shell.shell->groups) {
        document.groups.push_back(ScmGroup{
            .group_index = group.group_index,
            .group_offset = group.group_offset,
            .batch_count = group.batch_count,
            .first_batch_offset = group.first_batch_offset,
        });

        // Walk the batches first, then check the array packing across the
        // whole group: the arrays are grouped by kind, so no single batch can
        // be validated on its own.
        const auto first = document.batches.size();
        auto batch_offset = group.first_batch_offset;
        for (std::uint32_t batch = 0U; batch < group.batch_count; ++batch) {
            if (!fits(batch_offset, Contract::batch_stride, total)) {
                return fail(
                    ScmParseError::batch_out_of_bounds,
                    "a batch header extends past the end of the scene model");
            }
            const auto at = [&](std::size_t field) {
                return static_cast<std::size_t>(batch_offset) + field;
            };

            ScmPrimitiveBatch record{
                .group_index = group.group_index,
                .batch_index = batch,
                .batch_offset = batch_offset,
                .index_count = read_u16_le(bytes, at(Contract::batch_index_count_offset)),
                .position_offset = read_u64_le(bytes, at(Contract::batch_position_offset)),
                .normal_offset = read_u64_le(bytes, at(Contract::batch_normal_offset)),
                .attribute_offset = read_u64_le(bytes, at(Contract::batch_attribute_offset)),
                .index_offset = read_u64_le(bytes, at(Contract::batch_index_offset)),
                .strip_offset = batch_offset +
                    read_u64_le(bytes, at(Contract::batch_strip_offset)),
                .stored_strip_length = read_i32_le(bytes, at(Contract::batch_strip_length_offset)),
                .skipped_index_count = 0U,
            };

            if (!record.valid(total)) {
                return fail(
                    ScmParseError::array_out_of_bounds,
                    "a batch array relocates outside the scene model");
            }
            if (read_u16_le(bytes, static_cast<std::size_t>(record.strip_offset)) !=
                Contract::strip_buffer_marker) {
                // The routine only updates a strip length when the buffer is
                // marked. An unmarked buffer means this file does not work the
                // way the recovered routine expects, and guessing past that
                // would be inventing a second format.
                return fail(
                    ScmParseError::missing_strip_marker,
                    "a batch strip buffer does not carry the recovered marker");
            }

            for (std::uint32_t element = 0U; element < record.index_count; ++element) {
                const auto flag = bytes[static_cast<std::size_t>(
                    record.index_offset +
                    static_cast<std::uint64_t>(element) * Contract::index_element_bytes +
                    Contract::index_flag_byte)];
                if ((std::to_integer<std::uint8_t>(flag) &
                     Contract::index_skip_mask) != 0U) {
                    ++record.skipped_index_count;
                }
            }

            document.batches.push_back(record);
            // Chained, not indexed: this format stores the step to the next
            // batch where MOD stores an array pointer.
            batch_offset += read_u64_le(bytes, at(Contract::batch_next_stride_offset));
        }

        // The packing check. Each array kind holds every batch of the group in
        // order, 16-byte aligned, and the recorded offsets must reproduce
        // exactly — which is what proves the batch walk found the real batches
        // rather than plausible-looking ones.
        const std::pair<std::uint64_t ScmPrimitiveBatch::*, std::uint64_t> kinds[]{
            {&ScmPrimitiveBatch::position_offset, Contract::position_element_bytes},
            {&ScmPrimitiveBatch::normal_offset, Contract::normal_element_bytes},
            {&ScmPrimitiveBatch::attribute_offset, Contract::attribute_element_bytes},
            {&ScmPrimitiveBatch::index_offset, Contract::index_element_bytes},
        };
        if (group.batch_count != 0U) {
            auto cursor = document.batches[first].position_offset;
            for (const auto& [field, element_bytes] : kinds) {
                for (std::uint32_t batch = 0U; batch < group.batch_count; ++batch) {
                    const auto& record = document.batches[first + batch];
                    if (record.*field != cursor) {
                        return fail(
                            ScmParseError::array_packing_mismatch,
                            "a batch array is not where the group's packing puts it");
                    }
                    cursor += aligned(
                        static_cast<std::uint64_t>(record.index_count) * element_bytes);
                }
            }
        }
    }

    if (!document.valid()) {
        return fail(
            ScmParseError::invalid_document,
            "scene model decoded to an inconsistent document");
    }

    return ScmParseResult{
        .document = std::move(document),
        .error = ScmParseError::none,
        .message = {},
    };
}

} // namespace dmc::rengine::formats
