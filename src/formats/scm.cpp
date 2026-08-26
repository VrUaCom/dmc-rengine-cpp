#include "dmc_rengine/formats/scm.hpp"

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
    const auto total = static_cast<std::uint64_t>(bytes.size());
    if (total < Contract::group_table_offset) {
        return fail(
            ScmParseError::truncated_header,
            "scene model is smaller than its own header");
    }
    for (std::size_t index = 0U; index < Contract::magic_bytes; ++index) {
        if (std::to_integer<char>(bytes[index]) != Contract::magic[index]) {
            return fail(
                ScmParseError::invalid_magic,
                "scene model does not open with the recovered SCM tag");
        }
    }

    ScmDocument document;
    document.document_size = total;
    // Read as a byte because the routine reads a byte. Treating this dword as
    // a count would produce millions of groups out of what is really three
    // other fields sharing the word.
    document.group_count = std::to_integer<std::uint32_t>(
        bytes[Contract::group_count_offset]);
    document.document_pointer =
        read_u64_le(bytes, Contract::document_pointer_offset);

    if (document.group_count > k_max_group_count) {
        return fail(
            ScmParseError::group_limit,
            "declared group count exceeds the product parser safety limit");
    }
    if (document.document_pointer >= total) {
        return fail(
            ScmParseError::group_out_of_bounds,
            "the document pointer relocates outside the scene model");
    }

    const auto table_bytes = static_cast<std::uint64_t>(document.group_count) *
        Contract::group_stride;
    if (!fits(Contract::group_table_offset, table_bytes, total)) {
        return fail(
            ScmParseError::truncated_group_table,
            "the group table extends past the end of the scene model");
    }

    document.groups.reserve(document.group_count);
    for (std::uint32_t index = 0U; index < document.group_count; ++index) {
        const auto group_offset = Contract::group_table_offset +
            static_cast<std::uint64_t>(index) * Contract::group_stride;
        const auto batch_count = std::to_integer<std::uint32_t>(
            bytes[static_cast<std::size_t>(
                group_offset + Contract::group_batch_count_offset)]);
        const auto first_batch = read_u64_le(
            bytes,
            static_cast<std::size_t>(
                group_offset + Contract::group_batch_pointer_offset));

        if (batch_count > k_max_batch_count) {
            return fail(
                ScmParseError::group_limit,
                "a group declares more batches than the safety limit allows");
        }
        if (batch_count != 0U &&
            !fits(first_batch, Contract::batch_stride, total)) {
            return fail(
                ScmParseError::batch_out_of_bounds,
                "a group's first batch relocates outside the scene model");
        }

        document.groups.push_back(ScmGroup{
            .group_index = index,
            .group_offset = group_offset,
            .batch_count = batch_count,
            .first_batch_offset = first_batch,
        });

        // Walk the batches first, then check the array packing across the
        // whole group: the arrays are grouped by kind, so no single batch can
        // be validated on its own.
        const auto first = document.batches.size();
        auto batch_offset = first_batch;
        for (std::uint32_t batch = 0U; batch < batch_count; ++batch) {
            if (!fits(batch_offset, Contract::batch_stride, total)) {
                return fail(
                    ScmParseError::batch_out_of_bounds,
                    "a batch header extends past the end of the scene model");
            }
            const auto at = [&](std::size_t field) {
                return static_cast<std::size_t>(batch_offset) + field;
            };

            ScmPrimitiveBatch record{
                .group_index = index,
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
            batch_offset += read_u64_le(bytes, at(Contract::batch_next_stride_offset));
        }

        // The packing check. Each array kind holds every batch of the group in
        // order, 16-byte aligned, and the recorded offsets must reproduce
        // exactly — which is what proves the batch walk found the real batches
        // rather than plausible-looking ones.
        const std::pair<std::size_t, std::uint64_t> kinds[]{
            {offsetof(ScmPrimitiveBatch, position_offset), Contract::position_element_bytes},
            {offsetof(ScmPrimitiveBatch, normal_offset), Contract::normal_element_bytes},
            {offsetof(ScmPrimitiveBatch, attribute_offset), Contract::attribute_element_bytes},
            {offsetof(ScmPrimitiveBatch, index_offset), Contract::index_element_bytes},
        };
        if (batch_count != 0U) {
            auto cursor = document.batches[first].position_offset;
            for (const auto& [field, element_bytes] : kinds) {
                for (std::size_t batch = 0U; batch < batch_count; ++batch) {
                    const auto& record = document.batches[first + batch];
                    const auto recorded = *reinterpret_cast<const std::uint64_t*>(
                        reinterpret_cast<const std::byte*>(&record) + field);
                    if (recorded != cursor) {
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
