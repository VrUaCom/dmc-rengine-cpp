#pragma once

#include <cstddef>
#include <cstdint>

namespace dmc::rengine::analysis::mod {

// Evidence-only contract for MOD fields that were historically left unnamed.
//
// IMPORTANT:
// - this does not rename serialized ABI fields;
// - "unconsumed" always means unconsumed by the specifically audited canonical
//   runtime path, not proof that no code anywhere can ever inspect the bytes;
// - preservation remains mandatory until writer authority is independently
//   established.
enum class UnknownFieldDisposition : std::uint8_t {
    active,
    inactive_in_audited_mod_runtime,
    unused_by_canonical_render_shaders,
    format_specific_auxiliary_stream_slot,
    alignment_or_reserved_candidate,
    preserved_undecoded,
};

struct CanonicalExeUnknownFieldEvidence final {
    static constexpr std::uintptr_t mod_post_load = 0x1402FE3B0ULL;
    static constexpr std::uintptr_t mod_runtime_mesh_builder = 0x1402FE6A0ULL;
    static constexpr std::uintptr_t efm_post_load = 0x1402F7A90ULL;
    static constexpr std::uintptr_t efm_runtime_mesh_builder = 0x1402F7D60ULL;
    static constexpr std::uintptr_t common_material_helper = 0x1402F9890ULL;
    static constexpr std::uintptr_t mod_efm_transform_initializer = 0x1402FA080ULL;
    static constexpr std::uintptr_t rotation_xyz_helper = 0x140330450ULL;
    static constexpr std::uintptr_t translation_helper = 0x140031200ULL;
    static constexpr std::uintptr_t mod_manager_initializer = 0x1402F9570ULL;
    static constexpr std::uintptr_t cpu_blend_lane1_consumer = 0x1402F3D0AULL;

    // Serialized transform record +0x1C is copied as the fourth float of the
    // 16-byte rotation scratch block by 0x1402FA080. The called rotation
    // helper 0x140330450 reads only scratch +0x00/+0x04/+0x08 (serialized
    // rotation X/Y/Z) and never reads scratch +0x0C. Therefore +0x1C has no
    // effect on canonical MOD/EFM local-matrix construction.
    static constexpr std::size_t transform_reserved1c_offset = 0x1CU;
    static constexpr bool transform_1c_used_by_local_matrix_builder = false;
    static constexpr std::size_t mod_transform_1c_zero_count = 285U;
    static constexpr std::size_t known_efm_transform_1c_zero_count = 5U;
    static constexpr UnknownFieldDisposition transform_1c_disposition =
        UnknownFieldDisposition::inactive_in_audited_mod_runtime;

    // Contrast/control: serialized transform +0x0C is the fourth float of the
    // translation vector passed to 0x140031200. That helper loads the complete
    // 16-byte vector and adds it to matrix row 3, so +0x0C is live while the
    // homologous rotation-tail +0x1C is not consumed by 0x140330450.
    static constexpr bool translation_magnitude_fourth_lane_is_consumed = true;

    // Mesh +0x38 is NOT generic padding. EFM post-load 0x1402F7A90 relocates
    // it and EFM runtime builder 0x1402F7D60 forwards it to runtime auxiliary
    // stream slot +0x160. Existing EFM/HLSL evidence binds this stream to
    // per-vertex COLOR0. MOD post-load does not relocate +0x38 and MOD runtime
    // builder explicitly zeros both +0x160 auxiliary-stream pointers.
    static constexpr std::size_t mesh_auxiliary_stream_offset = 0x38U;
    static constexpr std::size_t runtime_auxiliary_stream_offset = 0x160U;
    static constexpr bool efm_mesh_38_is_runtime_active = true;
    static constexpr bool mod_mesh_38_is_runtime_active = false;
    static constexpr bool mod_builder_explicitly_zeros_runtime_auxiliary_stream = true;
    static constexpr std::size_t mod_mesh_38_zero_count = 180U;
    static constexpr UnknownFieldDisposition mesh_38_disposition =
        UnknownFieldDisposition::format_specific_auxiliary_stream_slot;

    // +0x0C lies between four GS CLAMP u16 fields (+0x04..+0x0A) and the first
    // 8-byte stream pointer at +0x10. It is zero in all 180 current MOD meshes
    // and the two bound EFM meshes, and is not consumed by the audited MOD/EFM
    // post-load paths nor common material helper 0x1402F9890. This is strong
    // alignment/reserved evidence but is deliberately not a global padding
    // declaration.
    static constexpr std::size_t mesh_0c_offset = 0x0CU;
    static constexpr std::size_t mod_mesh_0c_zero_count = 180U;
    static constexpr std::size_t known_efm_mesh_0c_zero_count = 2U;
    static constexpr UnknownFieldDisposition mesh_0c_disposition =
        UnknownFieldDisposition::alignment_or_reserved_candidate;

    // +0x4C is the final dword after generated topology count +0x48 and before
    // the 0x50 record boundary. It is zero in the same bounded MOD/EFM corpus
    // and is not consumed by the audited MOD/EFM load/build paths. Preserve it
    // until a global ABI/version census authorizes a stronger claim.
    static constexpr std::size_t mesh_4c_offset = 0x4CU;
    static constexpr std::size_t mod_mesh_4c_zero_count = 180U;
    static constexpr std::size_t known_efm_mesh_4c_zero_count = 2U;
    static constexpr UnknownFieldDisposition mesh_4c_disposition =
        UnknownFieldDisposition::alignment_or_reserved_candidate;

    // Canonical embedded MOD shader families DMC3_MOD, DMC3_MOD_SP and
    // DMC3_MOD_STX contain no matIndex.x/matIndxX use. Their skin code uses
    // y/z/w. The EFM shader variants show the same lane selection. CPU code at
    // 0x1402F3D0A independently reads lane[1] and shifts right by two.
    // lane[0] is therefore render-skin-unused, but remains preserved until a
    // complete non-render CPU census rules out another purpose.
    static constexpr std::size_t blendindices_x_lane = 0U;
    static constexpr std::size_t blendindices_first_active_skin_lane = 1U;
    static constexpr bool blendindices_x_used_by_canonical_mod_efm_skin_shaders = false;
    static constexpr bool blendindices_yzw_used_by_canonical_mod_efm_skin_shaders = true;
    static constexpr std::size_t mod_blendindices_x_zero_count = 20976U;
    static constexpr UnknownFieldDisposition blendindices_x_disposition =
        UnknownFieldDisposition::unused_by_canonical_render_shaders;

    // Header +0x14 is copied verbatim by 0x1402F9570 to manager +0xE4. A
    // targeted direct-displacement census of the core model subsystem
    // 0x1402F9000..0x14030D000 found the write but no direct read of that
    // manager field. This is negative evidence only; external manager users
    // still need type-aware whole-EXE tracing.
    static constexpr std::size_t header_runtime_metadata_offset = 0x14U;
    static constexpr std::size_t manager_runtime_metadata_offset = 0xE4U;
    static constexpr std::uintptr_t core_model_scan_begin = 0x1402F9000ULL;
    static constexpr std::uintptr_t core_model_scan_end = 0x14030D000ULL;
    static constexpr bool header_14_is_runtime_carried = true;
    static constexpr std::size_t core_model_direct_manager_e4_writes = 1U;
    static constexpr std::size_t core_model_direct_manager_e4_reads = 0U;
    static constexpr UnknownFieldDisposition header_14_disposition =
        UnknownFieldDisposition::preserved_undecoded;
};

static_assert(CanonicalExeUnknownFieldEvidence::transform_reserved1c_offset == 0x1CU);
static_assert(!CanonicalExeUnknownFieldEvidence::transform_1c_used_by_local_matrix_builder);
static_assert(CanonicalExeUnknownFieldEvidence::mod_transform_1c_zero_count == 285U);
static_assert(CanonicalExeUnknownFieldEvidence::mesh_auxiliary_stream_offset == 0x38U);
static_assert(CanonicalExeUnknownFieldEvidence::runtime_auxiliary_stream_offset == 0x160U);
static_assert(CanonicalExeUnknownFieldEvidence::efm_mesh_38_is_runtime_active);
static_assert(!CanonicalExeUnknownFieldEvidence::mod_mesh_38_is_runtime_active);
static_assert(CanonicalExeUnknownFieldEvidence::mod_mesh_0c_zero_count == 180U);
static_assert(CanonicalExeUnknownFieldEvidence::mod_mesh_4c_zero_count == 180U);
static_assert(!CanonicalExeUnknownFieldEvidence::blendindices_x_used_by_canonical_mod_efm_skin_shaders);
static_assert(CanonicalExeUnknownFieldEvidence::blendindices_yzw_used_by_canonical_mod_efm_skin_shaders);
static_assert(CanonicalExeUnknownFieldEvidence::mod_blendindices_x_zero_count == 20976U);
static_assert(CanonicalExeUnknownFieldEvidence::header_14_is_runtime_carried);
static_assert(CanonicalExeUnknownFieldEvidence::core_model_direct_manager_e4_writes == 1U);
static_assert(CanonicalExeUnknownFieldEvidence::core_model_direct_manager_e4_reads == 0U);

} // namespace dmc::rengine::analysis::mod
