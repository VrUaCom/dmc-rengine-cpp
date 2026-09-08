#include "dmc_rengine/formats/mot.hpp"

#include <bit>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <vector>

namespace mot = dmc::rengine::formats::mot;

namespace {

void put_u16(std::vector<std::byte>& bytes, std::size_t offset, std::uint16_t value) {
    bytes[offset + 0U] = static_cast<std::byte>(value & 0xFFU);
    bytes[offset + 1U] = static_cast<std::byte>((value >> 8U) & 0xFFU);
}

void put_u32(std::vector<std::byte>& bytes, std::size_t offset, std::uint32_t value) {
    bytes[offset + 0U] = static_cast<std::byte>(value & 0xFFU);
    bytes[offset + 1U] = static_cast<std::byte>((value >> 8U) & 0xFFU);
    bytes[offset + 2U] = static_cast<std::byte>((value >> 16U) & 0xFFU);
    bytes[offset + 3U] = static_cast<std::byte>((value >> 24U) & 0xFFU);
}

void put_f32(std::vector<std::byte>& bytes, std::size_t offset, float value) {
    put_u32(bytes, offset, std::bit_cast<std::uint32_t>(value));
}

void append_u16(std::vector<std::byte>& bytes, std::uint16_t value) {
    bytes.push_back(static_cast<std::byte>(value & 0xFFU));
    bytes.push_back(static_cast<std::byte>((value >> 8U) & 0xFFU));
}

void append_f32(std::vector<std::byte>& bytes, float value) {
    const auto raw = std::bit_cast<std::uint32_t>(value);
    bytes.push_back(static_cast<std::byte>(raw & 0xFFU));
    bytes.push_back(static_cast<std::byte>((raw >> 8U) & 0xFFU));
    bytes.push_back(static_cast<std::byte>((raw >> 16U) & 0xFFU));
    bytes.push_back(static_cast<std::byte>((raw >> 24U) & 0xFFU));
}

void append_compression3_track(
    std::vector<std::byte>& bytes,
    std::uint16_t time_control,
    std::uint16_t value) {
    append_u16(bytes, 0x28U);
    append_u16(bytes, 1U);
    append_u16(bytes, mot::TrackAbi::compression_3);
    append_u16(bytes, 0U);
    append_f32(bytes, 0.0F);
    append_f32(bytes, 1.0F);
    append_f32(bytes, 0.0F);
    append_f32(bytes, 1.0F);
    append_f32(bytes, 0.0F);
    append_f32(bytes, 1.0F);
    append_u16(bytes, time_control);
    append_u16(bytes, value);
    append_u16(bytes, 0U);
    append_u16(bytes, 0U);
}

[[nodiscard]] std::vector<std::byte> make_valid_motion() {
    constexpr std::uint32_t header_size = 0x30U;
    std::vector<std::byte> bytes(header_size + 4U, std::byte{0});
    put_u32(bytes, 0x00U, header_size);
    bytes[0x04U] = std::byte{'M'};
    bytes[0x05U] = std::byte{'O'};
    bytes[0x06U] = std::byte{'T'};
    bytes[0x07U] = std::byte{0};
    put_f32(bytes, 0x0CU, 140.0F);
    put_f32(bytes, 0x14U, 140.0F);
    put_u16(bytes, 0x1CU, 3U);
    put_u16(bytes, 0x1EU, 0U);
    put_u16(bytes, 0x20U, mot::ChannelMaskAbi::high_triplet);
    put_u16(bytes, 0x22U, 0U);
    put_u32(bytes, header_size, 3U);

    append_compression3_track(bytes, 0x8000U, 0U);
    append_compression3_track(bytes, 0x8000U, 0x7FFFU);
    append_compression3_track(bytes, 0x8000U, 0xFFFFU);
    while ((bytes.size() & 0x0FU) != 0U) {
        bytes.push_back(std::byte{0});
    }
    return bytes;
}

} // namespace

int main() {
    static_assert(mot::header_size_for_channel_domain(3U) == 0x30U);
    static_assert(mot::header_size_for_channel_domain(22U) == 0x50U);
    static_assert(mot::selected_track_count(0x1C0U) == 3U);
    static_assert(mot::selected_track_count(0x1F8U) == 6U);
    static_assert(mot::key_time_index(0x80AFU) == 0x00AFU);
    static_assert(mot::key_high_flag(0x8000U));

    const auto valid = make_valid_motion();
    const auto parsed = mot::Parser::parse(valid);
    assert(parsed.ok());
    assert(parsed.document->valid());
    assert(parsed.document->header_size == 0x30U);
    assert(parsed.document->channel_domain_count == 3U);
    assert(parsed.document->record_count == 3U);
    assert(parsed.document->tracks.size() == 3U);
    assert(parsed.document->tracks[0].compression == mot::TrackAbi::compression_3);
    assert(parsed.document->tracks[0].span == 0x28U);
    assert(parsed.document->tracks[0].keys3.size() == 1U);
    assert(mot::key_time_index(parsed.document->tracks[0].keys3[0].time_control) == 0U);

    auto bad_count = valid;
    put_u32(bad_count, 0x30U, 2U);
    const auto count_result = mot::Parser::parse(bad_count);
    assert(!count_result.ok());
    assert(count_result.error == mot::ParseError::record_count_mask_mismatch);

    auto bad_header = valid;
    put_u32(bad_header, 0x00U, 0x50U);
    const auto header_result = mot::Parser::parse(bad_header);
    assert(!header_result.ok());
    assert(header_result.error == mot::ParseError::invalid_header_size);

    auto bad_mask = valid;
    put_u16(bad_mask, 0x20U, 0x0200U);
    const auto mask_result = mot::Parser::parse(bad_mask);
    assert(!mask_result.ok());
    assert(mask_result.error == mot::ParseError::unknown_channel_mask_bits);

    return 0;
}
