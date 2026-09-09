#pragma once

#include <cstddef>
#include <cstdint>

namespace dmc::rengine::analysis::mod {

// Canonical evidence vocabulary for MOD reverse work. Do not add ad-hoc status
// names here: receipts/docs and C++ evidence must use this exact closed set.
enum class EvidenceStatus : std::uint8_t {
    EXE_CONFIRMED,
    CORPUS_CONFIRMED,
    EXE_AND_CORPUS_CONFIRMED,
    STRUCTURAL_CONFIRMED,
    SEMANTIC_CANDIDATE,
    PRESERVED_UNDECODED,
    RESERVED_OBSERVED_ZERO,
    REJECTED,
};

// Runtime-role classification is intentionally separate from evidence status.
// It describes only the specifically audited path and must never be treated as
// permission to discard serialized bytes.
enum class UnknownFieldDisposition : std::uint8_t {
    active,
    inactive_in_audited_mod_runtime,
    unused_by_canonical_render_shaders,
    format_specific_auxiliary_stream_slot,
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

    // Serialized transform +0x1C is physically copied into the fourth float of
    // the initializer scratch block. Rotation helper 0x140330450 reads only
    // scratch +0x00/+0x04/+0x08. Independently, CMotion transfer 0x14030F850
    // copies source +0x00/+04/+08/+10/+14/+18 and advances the source pointer
    // by 0x20 at 0x14030FA2F, skipping +0x1C. This closes non-consumption in
    // the two audited local-transform paths, but not every possible subsystem.
    static constexpr std::size_t transform_raw1c_offset = 0x1CU;
    static constexpr bool transform_1c_used_by_local_matrix_builder = false;
    static constexpr bool transform_1c_transferred_by_cmotion_binding = false;
    static constexpr std::size_t mod_transform_1c_zero_count = 285U;
    static constexpr std::size_t known_efm_transform_1c_zero_count = 5U;
    static constexpr UnknownFieldDisposition transform_1c_disposition =
        UnknownFieldDisposition::inactive_in_audited_mod_runtime;
    static constexpr EvidenceStatus transform_1c_status =
        EvidenceStatus::PRESERVED_UNDECODED;

    // Mesh +0x38 is not generic padding. EFM post-load 0x1402F7A90 relocates
    // it and EFM runtime builder 0x1402F7D60 forwards it to runtime auxiliary
    // stream +0x160; existing EFM/HLSL evidence binds that stream to COLOR0.
    // MOD post-load does not relocate +0x38 and MOD builder explicitly zeros
    // the corresponding auxiliary runtime stream slots. This closes the
    // MOD-specific canonical runtime role without importing EFM semantics.
    static constexpr std::size_t mesh_auxiliary_stream_offset = 0x38U;
    static constexpr std::size_t runtime_auxiliary_stream_offset = 0x160U;
    static constexpr bool efm_mesh_38_is_runtime_active = true;
    static constexpr bool mod_mesh_38_is_runtime_active = false;
    static constexpr bool mod_builder_explicitly_zeros_runtime_auxiliary_stream = true;
    static constexpr std::size_t mod_mesh_38_zero_count = 180U;
    static constexpr UnknownFieldDisposition mesh_38_disposition =
        UnknownFieldDisposition::format_specific_auxiliary_stream_slot;
    static constexpr EvidenceStatus mesh_38_status =
        EvidenceStatus::EXE_AND_CORPUS_CONFIRMED;

    // +0x0C lies between the four GS CLAMP u16 fields and the first u64 stream
    // pointer. It is zero in all 180 current MOD meshes and two bound EFM
    // meshes, and no positive consumer was established in the audited
    // load/build/material paths. Physical placement plus zeros is not proof of
    // padding, alignment, or reservation.
    static constexpr std::size_t mesh_0c_offset = 0x0CU;
    static constexpr std::size_t mod_mesh_0c_zero_count = 180U;
    static constexpr std::size_t known_efm_mesh_0c_zero_count = 2U;
    static constexpr UnknownFieldDisposition mesh_0c_disposition =
        UnknownFieldDisposition::preserved_undecoded;
    static constexpr EvidenceStatus mesh_0c_status =
        EvidenceStatus::PRESERVED_UNDECODED;

    // +0x4C is the trailing dword after generated topology count +0x48. +0x48
    // is the positive control: canonical MOD post-load generates it and runtime
    // builder forwards it, while no +0x4C companion use is established. Zero
    // corpus values still do not authorize a padding/reserved promotion.
    static constexpr std::size_t mesh_4c_offset = 0x4CU;
    static constexpr std::size_t mod_mesh_4c_zero_count = 180U;
    static constexpr std::size_t known_efm_mesh_4c_zero_count = 2U;
    static constexpr UnknownFieldDisposition mesh_4c_disposition =
        UnknownFieldDisposition::preserved_undecoded;
    static constexpr EvidenceStatus mesh_4c_status =
        EvidenceStatus::PRESERVED_UNDECODED;

    // BLENDINDICES.x indirect GPU escape is now closed for the canonical
    // executable. Whole-image census found 69 DXBC blobs; 8 input signatures
    // expose BLENDINDICES as uint4 register 3 with Mask=0xF and
    // ReadWriteMask=0xE, proving compiled shaders read Y/Z/W but not X.
    // Embedded DMC3_MOD/DMC3_MOD_SP/DMC3_MOD_STX HLSL independently contains
    // no matIndex.x/matIndxX use. Provenance-confirmed direct CPU consumers
    // contain the lane-Y positive control at 0x1402F3D0A and no lane-X read.
    // The raw serialized X byte remains part of the ABI and must be preserved.
    static constexpr std::size_t blendindices_x_lane = 0U;
    static constexpr std::size_t blendindices_first_active_skin_lane = 1U;
    static constexpr std::size_t canonical_dxbc_blob_count = 69U;
    static constexpr std::size_t blendindices_dxbc_signature_count = 8U;
    static constexpr std::uint8_t blendindices_dxbc_mask = 0x0FU;
    static constexpr std::uint8_t blendindices_dxbc_read_write_mask = 0x0EU;
    static constexpr bool blendindices_x_gpu_escape_closed = true;
    static constexpr bool blendindices_x_used_by_canonical_mod_efm_skin_shaders = false;
    static constexpr bool blendindices_yzw_used_by_canonical_mod_efm_skin_shaders = true;
    static constexpr std::size_t mod_blendindices_x_zero_count = 20976U;
    static constexpr UnknownFieldDisposition blendindices_x_disposition =
        UnknownFieldDisposition::unused_by_canonical_render_shaders;
    static constexpr EvidenceStatus blendindices_x_status =
        EvidenceStatus::PRESERVED_UNDECODED;

    // Header +0x14 is copied verbatim by 0x1402F9570 to manager +0xE4:
    // 0x1402F95C2 -> 0x1402F95C5. A targeted model-core scan finds the single
    // provenance-confirmed write and no direct read. A whole-EXE raw +0xE4
    // displacement census contains 83 candidates, but offset equality across
    // unrelated object layouts is not a type-aware xref. No identity/resource
    // semantic is promoted from the old decimal hypothesis.
    static constexpr std::size_t header_runtime_metadata_offset = 0x14U;
    static constexpr std::size_t manager_runtime_metadata_offset = 0xE4U;
    static constexpr std::uintptr_t core_model_scan_begin = 0x1402F9000ULL;
    static constexpr std::uintptr_t core_model_scan_end = 0x14030D000ULL;
    static constexpr bool header_14_is_runtime_carried = true;
    static constexpr std::size_t core_model_direct_manager_e4_writes = 1U;
    static constexpr std::size_t core_model_direct_manager_e4_reads = 0U;
    static constexpr std::size_t whole_exe_raw_e4_displacement_candidates = 83U;
    static constexpr UnknownFieldDisposition header_14_disposition =
        UnknownFieldDisposition::preserved_undecoded;
    static constexpr EvidenceStatus header_14_status =
        EvidenceStatus::PRESERVED_UNDECODED;
};

static_assert(CanonicalExeUnknownFieldEvidence::transform_raw1c_offset == 0x1CU);
static_assert(!CanonicalExeUnknownFieldEvidence::transform_1c_used_by_local_matrix_builder);
static_assert(!CanonicalExeUnknownFieldEvidence::transform_1c_transferred_by_cmotion_binding);
static_assert(CanonicalExeUnknownFieldEvidence::mod_transform_1c_zero_count == 285U);
static_assert(CanonicalExeUnknownFieldEvidence::mesh_auxiliary_stream_offset == 0x38U);
static_assert(CanonicalExeUnknownFieldEvidence::runtime_auxiliary_stream_offset == 0x160U);
static_assert(CanonicalExeUnknownFieldEvidence::efm_mesh_38_is_runtime_active);
static_assert(!CanonicalExeUnknownFieldEvidence::mod_mesh_38_is_runtime_active);
static_assert(CanonicalExeUnknownFieldEvidence::mod_mesh_0c_zero_count == 180U);
static_assert(CanonicalExeUnknownFieldEvidence::mod_mesh_4c_zero_count == 180U);
static_assert(CanonicalExeUnknownFieldEvidence::blendindices_x_gpu_escape_closed);
static_assert(CanonicalExeUnknownFieldEvidence::canonical_dxbc_blob_count == 69U);
static_assert(CanonicalExeUnknownFieldEvidence::blendindices_dxbc_signature_count == 8U);
static_assert(CanonicalExeUnknownFieldEvidence::blendindices_dxbc_mask == 0x0FU);
static_assert(CanonicalExeUnknownFieldEvidence::blendindices_dxbc_read_write_mask == 0x0EU);
static_assert(!CanonicalExeUnknownFieldEvidence::blendindices_x_used_by_canonical_mod_efm_skin_shaders);
static_assert(CanonicalExeUnknownFieldEvidence::blendindices_yzw_used_by_canonical_mod_efm_skin_shaders);
static_assert(CanonicalExeUnknownFieldEvidence::mod_blendindices_x_zero_count == 20976U);
static_assert(CanonicalExeUnknownFieldEvidence::header_14_is_runtime_carried);
static_assert(CanonicalExeUnknownFieldEvidence::core_model_direct_manager_e4_writes == 1U);
static_assert(CanonicalExeUnknownFieldEvidence::core_model_direct_manager_e4_reads == 0U);

} // namespace dmc::rengine::analysis::mod
