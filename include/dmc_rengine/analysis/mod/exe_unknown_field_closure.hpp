#pragma once

#include <cstddef>
#include <cstdint>

namespace dmc::rengine::analysis::mod {

// Hash-bound negative/low-level executable evidence for the remaining MOD
// unknown-field closure. These constants intentionally describe observed
// canonical behavior without inventing high-level artistic semantics.
struct ExeUnknownFieldClosure final {
    static constexpr std::uint64_t transform_initializer = 0x1402FA080ULL;
    static constexpr std::uint64_t rotation_helper = 0x140330450ULL;
    static constexpr bool transform_1c_read_by_rotation_helper = false;
    static constexpr bool transform_1c_transferred_to_runtime_node = false;

    static constexpr std::uint64_t mod_post_load = 0x1402FE3B0ULL;
    static constexpr std::uint64_t mod_runtime_mesh_builder = 0x1402FE6A0ULL;
    static constexpr bool mesh_0c_consumed_in_canonical_load_path = false;
    static constexpr bool mesh_38_consumed_in_canonical_load_path = false;
    static constexpr bool mesh_4c_consumed_in_canonical_load_path = false;

    // Cross-format positive control: EFM uses the homologous +0x38 slot.
    static constexpr std::uint64_t efm_post_load = 0x1402F7A90ULL;
    static constexpr bool efm_mesh_38_consumed = true;

    // Full embedded-source ASCII census across DMC3_MOD, DMC3_MOD_SP and
    // DMC3_MOD_STX source material in the hash-verified executable.
    static constexpr std::size_t shader_mat_index_x_refs = 0U;
    static constexpr std::size_t shader_mat_index_y_refs = 112U;
    static constexpr std::size_t shader_mat_index_z_refs = 16U;
    static constexpr std::size_t shader_mat_index_w_refs = 16U;

    static constexpr std::uint64_t cpu_blend_lane_y_read = 0x1402F3D0AULL;
    static constexpr std::uint8_t cpu_blend_lane_y_byte_offset = 1U;

    static constexpr std::uint64_t object_render_packet_helper = 0x140302640ULL;
    static constexpr std::uint32_t source_flag_00100000 = 0x00100000U;
    static constexpr std::uint64_t packet08_when_flag_clear = 0x000000000005000DULL;
    static constexpr std::uint64_t packet08_when_flag_set = 0x000000000005010DULL;
    static constexpr std::uint64_t packet00_clear_path_mask = 0x0000000100000000ULL;
};

static_assert(!ExeUnknownFieldClosure::transform_1c_read_by_rotation_helper);
static_assert(!ExeUnknownFieldClosure::transform_1c_transferred_to_runtime_node);
static_assert(!ExeUnknownFieldClosure::mesh_0c_consumed_in_canonical_load_path);
static_assert(!ExeUnknownFieldClosure::mesh_38_consumed_in_canonical_load_path);
static_assert(!ExeUnknownFieldClosure::mesh_4c_consumed_in_canonical_load_path);
static_assert(ExeUnknownFieldClosure::efm_mesh_38_consumed);
static_assert(ExeUnknownFieldClosure::shader_mat_index_x_refs == 0U);
static_assert(ExeUnknownFieldClosure::packet08_when_flag_set -
              ExeUnknownFieldClosure::packet08_when_flag_clear == 0x100ULL);

} // namespace dmc::rengine::analysis::mod
