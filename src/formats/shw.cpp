#include "dmc_rengine/formats/shw.hpp"

#include "dmc_rengine/profiles/dmc3/shw_contract.hpp"

#include <utility>

namespace dmc::rengine::formats {
namespace {

using Contract = profiles::dmc3::ShwContract;

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

[[nodiscard]] ShwParseResult fail(ShwParseError error, std::string message) {
    return ShwParseResult{
        .document = std::nullopt,
        .error = error,
        .message = std::move(message),
    };
}

} // namespace

bool ShwDocument::valid() const noexcept {
    if (document_size == 0U || entries.size() != entry_count) {
        return false;
    }
    for (const auto& entry : entries) {
        for (const auto offset : entry.array_offsets) {
            if (offset >= document_size) {
                return false;
            }
        }
    }
    return true;
}

ShwParseResult ShwParser::parse(std::span<const std::byte> bytes) {
    const auto total = static_cast<std::uint64_t>(bytes.size());
    if (total < Contract::entry_table_offset) {
        return fail(
            ShwParseError::truncated_header,
            "shadow document is smaller than its own header");
    }
    for (std::size_t index = 0U; index < Contract::magic_bytes; ++index) {
        if (std::to_integer<char>(bytes[index]) != Contract::magic[index]) {
            return fail(
                ShwParseError::invalid_magic,
                "shadow document does not open with the recovered SHW tag");
        }
    }

    ShwDocument document;
    document.document_size = total;
    document.entry_count =
        std::to_integer<std::uint32_t>(bytes[Contract::entry_count_offset]);
    if (document.entry_count > Contract::max_entry_count) {
        return fail(
            ShwParseError::entry_limit,
            "declared entry count exceeds the product parser safety limit");
    }

    const auto table_bytes =
        static_cast<std::uint64_t>(document.entry_count) * Contract::entry_stride;
    if (!fits(Contract::entry_table_offset, table_bytes, total)) {
        return fail(
            ShwParseError::truncated_entry_table,
            "the entry table extends past the end of the shadow document");
    }

    document.entries.reserve(document.entry_count);
    for (std::uint32_t index = 0U; index < document.entry_count; ++index) {
        const auto entry_offset = Contract::entry_table_offset +
            static_cast<std::uint64_t>(index) * Contract::entry_stride;

        ShwEntry entry{.entry_index = index, .entry_offset = entry_offset, .array_offsets = {}};
        for (std::size_t pointer = 0U; pointer < Contract::entry_pointer_count; ++pointer) {
            const auto value = read_u64_le(
                bytes,
                static_cast<std::size_t>(
                    entry_offset + pointer * Contract::entry_pointer_stride));
            if (value >= total) {
                return fail(
                    ShwParseError::pointer_out_of_bounds,
                    "an entry array relocates outside the shadow document");
            }
            entry.array_offsets[pointer] = value;
        }
        document.entries.push_back(entry);
    }

    if (!document.valid()) {
        return fail(
            ShwParseError::invalid_document,
            "shadow document decoded to an inconsistent document");
    }

    return ShwParseResult{
        .document = std::move(document),
        .error = ShwParseError::none,
        .message = {},
    };
}

} // namespace dmc::rengine::formats
