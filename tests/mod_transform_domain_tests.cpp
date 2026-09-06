#include "dmc_rengine/formats/mod/transform_domain.hpp"
#include "dmc_rengine/formats/mod/world_transform.hpp"

#include <bit>
#include <cassert>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <vector>

namespace domain = dmc::rengine::formats::mod::transform_domain;
namespace world = dmc::rengine::formats::mod::world_transform;

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

[[nodiscard]] bool near(const float a, const float b) {
    return std::fabs(a - b) < 0.000001F;
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

    // Non-identity topological order is intentional. The third parent is node
    // 2, not inverse(order)[2] == 1. This guards the provenance-corrected
    // parentByOrderPosition semantics against the old inverse heuristic.
    bytes[0x60U] = std::byte{0xFFU};
    bytes[0x61U] = std::byte{0U};
    bytes[0x62U] = std::byte{2U};
    bytes[0x64U] = std::byte{0U};
    bytes[0x65U] = std::byte{2U};
    bytes[0x66U] = std::byte{1U};
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
    assert(result.hierarchy_is_topological);
    assert(result.hierarchy_candidate_is_acyclic);
    assert(result.node_at_order_position.size() == 3U);
    assert(result.node_at_order_position[0] == 0U);
    assert(result.node_at_order_position[1] == 2U);
    assert(result.node_at_order_position[2] == 1U);
    assert(result.parent_by_order_position.size() == 3U);
    assert(result.parent_by_order_position[0] == -1);
    assert(result.parent_by_order_position[1] == 0);
    assert(result.parent_by_order_position[2] == 2);
    assert(result.derived_hierarchy_candidate == result.parent_by_order_position);

    assert(result.adapter_table.size() == 3U);
    assert(result.adapter_table[0] == 7U);
    assert(result.adapter_table[1] == 8U);
    assert(result.adapter_table[2] == 9U);

    assert(result.transform_records_complete);
    assert(result.transform_records_finite);
    assert(result.local_transform_records_by_node_index.size() == 3U);
    assert(result.local_transform_records_by_node_index[0].record_offset == 0x70U);
    assert(result.local_transform_records_by_node_index[0].translation.x == 1.0F);
    assert(result.local_transform_records_by_node_index[0].translation.y == 2.0F);
    assert(result.local_transform_records_by_node_index[0].translation.z == 2.0F);
    assert(result.local_transform_records_by_node_index[0].translation_magnitude == 3.0F);
    assert(std::fabs(result.local_transform_records_by_node_index[0].rotation_xyz_radians.z - 0.3F) < 0.000001F);
    assert(result.local_transform_records_by_node_index[2].translation.x == -1.0F);
    assert(world::supports_spatial_hierarchy(result));
    assert(world::build_model_space_world_matrices(result).has_value());

    // A translation-only hierarchy makes the world-space expectations
    // independent of the rotation implementation. The non-linear evaluation
    // order also proves that parents are node indices, not order positions.
    domain::ParseResult spatial{};
    spatial.recognized = true;
    spatial.raw_domain_count = 3U;
    spatial.permutation_is_complete = true;
    spatial.hierarchy_is_topological = true;
    spatial.transform_records_complete = true;
    spatial.transform_records_finite = true;
    spatial.node_at_order_position = {0U, 2U, 1U};
    spatial.parent_by_order_position = {-1, 0, 2};
    spatial.local_transform_records_by_node_index.resize(3U);
    spatial.local_transform_records_by_node_index[0].translation =
        domain::Vec3f{10.0F, 0.0F, 0.0F};
    spatial.local_transform_records_by_node_index[1].translation =
        domain::Vec3f{0.0F, 0.0F, 2.0F};
    spatial.local_transform_records_by_node_index[2].translation =
        domain::Vec3f{0.0F, 5.0F, 0.0F};

    assert(world::supports_spatial_hierarchy(spatial));
    const auto model_world = world::build_model_space_world_matrices(spatial);
    assert(model_world.has_value());
    const auto root_position = world::world_position((*model_world)[0]);
    const auto mid_position = world::world_position((*model_world)[2]);
    const auto tip_position = world::world_position((*model_world)[1]);
    assert(near(root_position.x, 10.0F));
    assert(near(root_position.y, 0.0F));
    assert(near(root_position.z, 0.0F));
    assert(near(mid_position.x, 10.0F));
    assert(near(mid_position.y, 5.0F));
    assert(near(mid_position.z, 0.0F));
    assert(near(tip_position.x, 10.0F));
    assert(near(tip_position.y, 5.0F));
    assert(near(tip_position.z, 2.0F));

    auto root_base = world::identity_matrix();
    root_base.values[12] = 100.0F;
    const auto rooted_world = world::build_world_matrices(spatial, root_base);
    assert(rooted_world.has_value());
    const auto rooted_tip = world::world_position((*rooted_world)[1]);
    assert(near(rooted_tip.x, 110.0F));
    assert(near(rooted_tip.y, 5.0F));
    assert(near(rooted_tip.z, 2.0F));

    auto malformed_hierarchy = bytes;
    malformed_hierarchy[0x62U] = std::byte{1U};
    const auto malformed_hierarchy_result = domain::parse(malformed_hierarchy);
    assert(malformed_hierarchy_result.ok());
    assert(!malformed_hierarchy_result.hierarchy_is_topological);
    assert(!world::supports_spatial_hierarchy(malformed_hierarchy_result));
    assert(!world::build_model_space_world_matrices(
        malformed_hierarchy_result).has_value());

    auto truncated = bytes;
    truncated.resize(0xCFU);
    const auto truncated_result = domain::parse(truncated);
    assert(truncated_result.recognized);
    assert(!truncated_result.ok());
    assert(!truncated_result.transform_records_complete);
    assert(!world::supports_spatial_hierarchy(truncated_result));

    return 0;
}
