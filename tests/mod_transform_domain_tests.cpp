#include "dmc_rengine/formats/mod/transform_domain.hpp"

#include <bit>
#include <cassert>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <vector>

namespace domain = dmc::rengine::formats::mod::transform_domain;

namespace {
void put_u32(std::vector<std::byte>& bytes, const std::size_t offset, const std::uint32_t value) {
    for (std::size_t i = 0U; i < 4U; ++i) {
        bytes[offset + i] = static_cast<std::byte>((value >> (8U * i)) & 0xFFU);
    }
}

void put_u64(std::vector<std::byte>& bytes, const std::size_t offset, const std::uint64_t value) {
    put_u32(bytes, offset, static_cast<std::uint32_t>(value & 0xFFFFFFFFULL));
    put_u32(bytes, offset + 4U, static_cast<std::uint32_t>(value >> 32U));
}

void put_f32(std::vector<std::byte>& bytes, const std::size_t offset, const float value) {
    put_u32(bytes, offset, std::bit_cast<std::uint32_t>(value));
}

void put_transform(std::vector<std::byte>& bytes,
                   const std::size_t offset,
                   const float tx,
                   const float ty,
                   const float tz,
                   const float magnitude,
                   const float rx,
                   const float ry,
                   const float rz,
                   const float reserved) {
    put_f32(bytes, offset + 0x00U, tx);
    put_f32(bytes, offset + 0x04U, ty);
    put_f32(bytes, offset + 0x08U, tz);
    put_f32(bytes, offset + 0x0CU, magnitude);
    put_f32(bytes, offset + 0x10U, rx);
    put_f32(bytes, offset + 0x14U, ry);
    put_f32(bytes, offset + 0x18U, rz);
    put_f32(bytes, offset + 0x1CU, reserved);
}
} // namespace

int main() {
    std::vector<std::byte> bytes(0xD0U);
    bytes[0U] = std::byte{'M'};
    bytes[1U] = std::byte{'O'};
    bytes[2U] = std::byte{'D'};
    bytes[3U] = std::byte{' '};
    bytes[0x11U] = std::byte{3U};
    put_u64(bytes, 0x20U, 0x40U);

    // Node-domain core at document 0x40:
    // parent 0x20, order 0x24, adapter 0x28, transforms 0x30.
    put_u32(bytes, 0x40U, 0x20U);
    put_u32(bytes, 0x44U, 0x24U);
    put_u32(bytes, 0x48U, 0x28U);
    put_u32(bytes, 0x4CU, 0x30U);

    bytes[0x60U] = std::byte{0xFFU};
    bytes[0x61U] = std::byte{0U};
    bytes[0x62U] = std::byte{1U};
    bytes[0x64U] = std::byte{0U};
    bytes[0x65U] = std::byte{1U};
    bytes[0x66U] = std::byte{2U};
    bytes[0x68U] = std::byte{7U};
    bytes[0x69U] = std::byte{8U};
    bytes[0x6AU] = std::byte{9U};

    put_transform(bytes, 0x70U,
                  1.0F, 2.0F, 2.0F, 3.0F,
                  0.1F, 0.2F, 0.3F, 0.0F);
    put_transform(bytes, 0x90U,
                  0.0F, 4.0F, 0.0F, 4.0F,
                  -0.1F, 0.0F, 0.0F, 0.0F);
    put_transform(bytes, 0xB0U,
                  -1.0F, 0.0F, 0.0F, 1.0F,
                  0.0F, 0.5F, 0.0F, 0.0F);

    const auto result = domain::parse(bytes);
    assert(result.ok());
    assert(result.raw_domain_count == 3U);
    assert(result.parent_relative_offset == 0x20U);
    assert(result.order_relative_offset == 0x24U);
    assert(result.adapter_relative_offset == 0x28U);
    assert(result.transform_relative_offset == 0x30U);
    assert(result.serialized_layout_matches_core);

    assert(result.permutation_is_complete);
    assert(result.hierarchy_candidate_is_acyclic);
    assert(result.derived_hierarchy_candidate.size() == 3U);
    assert(result.derived_hierarchy_candidate[0] == -1);
    assert(result.derived_hierarchy_candidate[1] == 0);
    assert(result.derived_hierarchy_candidate[2] == 1);

    assert(result.adapter_table.size() == 3U);
    assert(result.adapter_table[0] == 7U);
    assert(result.adapter_table[1] == 8U);
    assert(result.adapter_table[2] == 9U);

    assert(result.transform_records_complete);
    assert(result.transform_records_finite);
    assert(result.local_transform_records.size() == 3U);
    assert(result.local_transform_records[0].record_offset == 0x70U);
    assert(result.local_transform_records[0].translation.x == 1.0F);
    assert(result.local_transform_records[0].translation.y == 2.0F);
    assert(result.local_transform_records[0].translation.z == 2.0F);
    assert(result.local_transform_records[0].translation_magnitude == 3.0F);
    assert(std::fabs(result.local_transform_records[0].rotation_xyz_radians.z - 0.3F) < 0.000001F);
    assert(result.local_transform_records[2].translation.x == -1.0F);

    auto truncated = bytes;
    truncated.resize(0xCFU);
    const auto truncated_result = domain::parse(truncated);
    assert(truncated_result.recognized);
    assert(!truncated_result.ok());
    assert(!truncated_result.transform_records_complete);

    return 0;
}
