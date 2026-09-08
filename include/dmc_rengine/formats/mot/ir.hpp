#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <vector>

namespace dmc::rengine::formats::mot {

struct QuantizedKey2 final {
    std::uint16_t time_control{};
    std::uint16_t value{};
};

struct QuantizedKey3 final {
    std::uint16_t time_control{};
    std::uint16_t value{};
    std::uint16_t auxiliary_a{};
    std::uint16_t auxiliary_b{};
};

struct TrackRecord final {
    std::size_t offset{};
    std::uint16_t span{};
    std::uint16_t key_count{};
    std::uint16_t compression{};
    std::uint16_t start_time_raw{};

    // Known compression-2 records use [0..1], compression-3 records use [0..5].
    // High-level min/range/tangent semantics remain analysis authority.
    std::array<float, 6U> quantization_raw{};
    std::size_t quantization_float_count{};

    std::vector<QuantizedKey2> keys2;
    std::vector<QuantizedKey3> keys3;
    std::vector<std::byte> source_bytes;
};

struct Document final {
    std::size_t physical_size{};
    std::uint32_t header_size{};
    std::uint32_t raw_u32_08{};
    float raw_f32_0c{};
    float raw_f32_10{};
    float raw_f32_14{};
    std::uint16_t raw_u16_18{};
    std::uint16_t raw_u16_1a{};
    std::uint16_t channel_domain_count{};
    std::vector<std::uint16_t> channel_masks;
    std::vector<std::byte> raw_header_tail;
    std::uint32_t record_count{};
    std::vector<TrackRecord> tracks;
    std::size_t zero_padding_size{};

    [[nodiscard]] bool valid() const noexcept;
};

} // namespace dmc::rengine::formats::mot
