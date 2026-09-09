#pragma once

// A volume record shaped the way the corpus shapes one.
//
// These tests used to hand the volume reader a zero-filled buffer, which the
// reader accepted because it checked nothing but the length. It now enforces
// what the em000 extraction shows every record agreeing on — a known kind, a
// reserved run of zeros, and a first vector that is a position in homogeneous
// coordinates — so a fixture has to be a volume record rather than a buffer of
// the right size.

#include "dmc_rengine/formats/so/volume_table.hpp"

#include <cstddef>
#include <cstdint>
#include <cstring>
#include <vector>

namespace so_test_fixture {

inline void put_u32(
    std::vector<std::byte>& bytes, std::size_t offset, std::uint32_t value) {
    for (std::size_t index = 0U; index < 4U; ++index) {
        bytes[offset + index] =
            static_cast<std::byte>((value >> (8U * index)) & 0xFFU);
    }
}

inline void put_f32(
    std::vector<std::byte>& bytes, std::size_t offset, float value) {
    std::uint32_t raw{};
    std::memcpy(&raw, &value, sizeof(raw));
    put_u32(bytes, offset, raw);
}

/// `count` spheres, each centred at the origin with a unit radius.
[[nodiscard]] inline std::vector<std::byte> volume_records(std::size_t count) {
    namespace volumes = dmc::rengine::formats::so::volume_table;
    std::vector<std::byte> bytes(count * volumes::record_size, std::byte{0});
    for (std::size_t index = 0U; index < count; ++index) {
        const auto record = index * volumes::record_size;
        put_u32(bytes, record, volumes::sphere_record_type);
        // vector0 is a position, so its w is 1 and not 0.
        put_f32(bytes, record + volumes::first_vector_offset + 0x0CU,
            volumes::position_w);
        // vector1.x is the radius.
        put_f32(bytes, record + 0x20U, 1.0F);
    }
    return bytes;
}

} // namespace so_test_fixture
