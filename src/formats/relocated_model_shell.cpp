#include "dmc_rengine/formats/relocated_model_shell.hpp"

#include <utility>

namespace dmc::rengine::formats {
namespace {

[[nodiscard]] bool fits(
    std::uint64_t offset,
    std::uint64_t size,
    std::uint64_t total) noexcept {
    return offset <= total && size <= total - offset;
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

[[nodiscard]] ModelShellResult fail(ModelShellError error, std::string message) {
    return ModelShellResult{
        .shell = std::nullopt,
        .error = error,
        .message = std::move(message),
    };
}

} // namespace

ModelShellResult RelocatedModelShell::parse(
    std::span<const std::byte> bytes,
    std::string_view magic,
    std::size_t batch_stride) {
    const auto total = static_cast<std::uint64_t>(bytes.size());
    if (total < group_table_offset) {
        return fail(
            ModelShellError::truncated_header,
            "model document is smaller than its own header");
    }
    for (std::size_t index = 0U; index < magic.size(); ++index) {
        if (std::to_integer<char>(bytes[index]) != magic[index]) {
            return fail(
                ModelShellError::invalid_magic,
                "model document does not open with the recovered tag");
        }
    }

    ModelShell shell;
    shell.document_size = total;
    // Read as a byte because the routines read a byte. Treating this dword as
    // a count would produce millions of groups out of what is really several
    // fields sharing the word.
    shell.group_count = std::to_integer<std::uint32_t>(bytes[group_count_offset]);
    shell.document_mode = std::to_integer<std::uint8_t>(bytes[document_mode_offset]);
    shell.document_pointer = read_u64_le(bytes, document_pointer_offset);

    if (shell.group_count > max_group_count) {
        return fail(
            ModelShellError::group_limit,
            "declared group count exceeds the product parser safety limit");
    }
    if (shell.document_pointer >= total) {
        return fail(
            ModelShellError::pointer_out_of_bounds,
            "the document pointer relocates outside the model document");
    }

    const auto table_bytes =
        static_cast<std::uint64_t>(shell.group_count) * group_stride;
    if (!fits(group_table_offset, table_bytes, total)) {
        return fail(
            ModelShellError::truncated_group_table,
            "the group table extends past the end of the model document");
    }

    shell.groups.reserve(shell.group_count);
    for (std::uint32_t index = 0U; index < shell.group_count; ++index) {
        const auto group_offset =
            group_table_offset + static_cast<std::uint64_t>(index) * group_stride;
        const auto batch_count = std::to_integer<std::uint32_t>(
            bytes[static_cast<std::size_t>(group_offset + group_batch_count_offset)]);
        const auto first_batch = read_u64_le(
            bytes,
            static_cast<std::size_t>(group_offset + group_batch_pointer_offset));

        if (batch_count > max_batch_count) {
            return fail(
                ModelShellError::group_limit,
                "a group declares more batches than the safety limit allows");
        }
        if (batch_count != 0U && !fits(first_batch, batch_stride, total)) {
            return fail(
                ModelShellError::pointer_out_of_bounds,
                "a group's first batch relocates outside the model document");
        }

        shell.groups.push_back(ModelShellGroup{
            .group_index = index,
            .group_offset = group_offset,
            .batch_count = batch_count,
            .first_batch_offset = first_batch,
        });
    }

    return ModelShellResult{
        .shell = std::move(shell),
        .error = ModelShellError::none,
        .message = {},
    };
}

} // namespace dmc::rengine::formats
