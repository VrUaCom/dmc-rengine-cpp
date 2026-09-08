#include "dmc_rengine/formats/mot.hpp"

#include <bit>
#include <utility>

namespace dmc::rengine::formats::mot {
namespace {

[[nodiscard]] ParseResult fail(ParseError error, std::string message) {
    return ParseResult{
        .document = std::nullopt,
        .error = error,
        .message = std::move(message),
    };
}

[[nodiscard]] std::uint16_t read_u16(
    std::span<const std::byte> bytes, std::size_t offset) noexcept {
    return static_cast<std::uint16_t>(
        std::to_integer<std::uint16_t>(bytes[offset + 0U]) |
        static_cast<std::uint16_t>(
            std::to_integer<std::uint16_t>(bytes[offset + 1U]) << 8U));
}

[[nodiscard]] std::uint32_t read_u32(
    std::span<const std::byte> bytes, std::size_t offset) noexcept {
    return std::to_integer<std::uint32_t>(bytes[offset + 0U]) |
        (std::to_integer<std::uint32_t>(bytes[offset + 1U]) << 8U) |
        (std::to_integer<std::uint32_t>(bytes[offset + 2U]) << 16U) |
        (std::to_integer<std::uint32_t>(bytes[offset + 3U]) << 24U);
}

[[nodiscard]] float read_f32(
    std::span<const std::byte> bytes, std::size_t offset) noexcept {
    return std::bit_cast<float>(read_u32(bytes, offset));
}

[[nodiscard]] bool magic_ok(std::span<const std::byte> bytes) noexcept {
    return bytes.size() >= 8U &&
        bytes[4U] == std::byte{'M'} &&
        bytes[5U] == std::byte{'O'} &&
        bytes[6U] == std::byte{'T'} &&
        bytes[7U] == std::byte{0};
}

} // namespace

bool Document::valid() const noexcept {
    return physical_size != 0U &&
        (header_size == Reader::k_observed_header_small ||
         header_size == Reader::k_observed_header_large) &&
        raw_header.size() == header_size &&
        records.size() == static_cast<std::size_t>(record_count);
}

ParseResult Reader::parse(std::span<const std::byte> bytes) {
    if (bytes.size() < 0x18U) {
        return fail(ParseError::truncated_header, "MOT header prefix is truncated");
    }
    if (!magic_ok(bytes)) {
        return fail(ParseError::invalid_magic, "MOT\\0 marker at +0x04 is absent");
    }

    const auto header_size = read_u32(bytes, 0U);
    if (header_size != k_observed_header_small &&
        header_size != k_observed_header_large) {
        return fail(
            ParseError::unsupported_observed_header_size,
            "MOT header size is outside the 0x30/0x50 em000 evidence set");
    }
    const auto header_extent = static_cast<std::size_t>(header_size);
    if (header_extent > bytes.size() || bytes.size() - header_extent < 4U) {
        return fail(ParseError::truncated_header, "MOT header/count envelope is truncated");
    }

    const auto record_count = read_u32(bytes, header_extent);
    if (record_count > k_safety_max_records) {
        return fail(
            ParseError::unreasonable_record_count,
            "MOT record count exceeds the product-side safety bound");
    }

    Document document{
        .physical_size = bytes.size(),
        .header_size = header_size,
        .raw_time_a = read_f32(bytes, 0x0CU),
        .raw_time_b = read_f32(bytes, 0x10U),
        .raw_time_c = read_f32(bytes, 0x14U),
        .raw_header = std::vector<std::byte>(bytes.begin(), bytes.begin() +
            static_cast<std::ptrdiff_t>(header_extent)),
        .record_count = record_count,
    };
    document.records.reserve(static_cast<std::size_t>(record_count));

    std::size_t cursor = header_extent + 4U;
    for (std::uint32_t index = 0U; index < record_count; ++index) {
        static_cast<void>(index);
        if (cursor > bytes.size() || bytes.size() - cursor < 2U) {
            return fail(ParseError::truncated_record, "MOT record header crosses EOF");
        }
        const auto span = read_u16(bytes, cursor);
        if (span < 2U) {
            return fail(ParseError::invalid_record_span, "MOT record span is smaller than its u16 header");
        }
        const auto extent = static_cast<std::size_t>(span);
        if (extent > bytes.size() - cursor) {
            return fail(ParseError::truncated_record, "MOT record span crosses EOF");
        }
        document.records.push_back(Record{
            .offset = cursor,
            .span = span,
        });
        cursor += extent;
    }

    for (std::size_t i = cursor; i < bytes.size(); ++i) {
        if (bytes[i] != std::byte{0}) {
            return fail(ParseError::nonzero_tail, "MOT post-record tail is not zero padding");
        }
    }
    document.zero_padding_size = bytes.size() - cursor;

    return ParseResult{
        .document = std::move(document),
        .error = ParseError::none,
        .message = {},
    };
}

bool Reader::structurally_valid(std::span<const std::byte> bytes) noexcept {
    return parse(bytes).ok();
}

} // namespace dmc::rengine::formats::mot
