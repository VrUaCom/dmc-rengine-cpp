#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <span>

namespace dmc::rengine::analysis::mod {

struct SerializedRegion final {
    std::size_t offset{};
    std::size_t size{};
};

// Canonical MOD secondary serialized regions observed zero in the bounded
// retail corpus. These names intentionally describe location, not semantics.
// Zero corpus values and canonical runtime dormancy do not authorize
// padding/reserved semantics or writer normalization.
inline constexpr SerializedRegion header_08_0f{0x08U, 0x08U};
inline constexpr SerializedRegion header_18_1f{0x18U, 0x08U};
inline constexpr SerializedRegion header_28_3f{0x28U, 0x18U};

inline constexpr SerializedRegion object_04_07{0x04U, 0x04U};
inline constexpr SerializedRegion object_14_17{0x14U, 0x04U};
inline constexpr SerializedRegion object_20_2f{0x20U, 0x10U};

inline constexpr SerializedRegion node_domain_10_1f{0x10U, 0x10U};

[[nodiscard]] constexpr bool region_in_bounds(
    std::span<const std::byte> bytes,
    std::size_t base,
    SerializedRegion region) noexcept {
    if (base > bytes.size()) return false;
    const auto remaining = bytes.size() - base;
    if (region.offset > remaining) return false;
    return region.size <= remaining - region.offset;
}

template <std::size_t N>
[[nodiscard]] constexpr std::array<std::byte, N> snapshot_region(
    std::span<const std::byte> bytes,
    std::size_t base,
    SerializedRegion region) noexcept {
    std::array<std::byte, N> out{};
    if (region.size != N || !region_in_bounds(bytes, base, region)) return out;
    for (std::size_t i = 0; i < N; ++i) {
        out[i] = bytes[base + region.offset + i];
    }
    return out;
}

// Executable provenance correction:
// 0x140302A78 derives serialized object = source_header + 0x40 + index*0x40.
// 0x140302AAA stores that pointer itself into runtime object +0x18. It does
// NOT read serialized object +0x18/+0x1C.
inline constexpr std::size_t runtime_serialized_object_pointer_offset = 0x18U;
inline constexpr std::size_t serialized_object_mesh_table_offset = 0x08U;
inline constexpr std::size_t mod_efm_runtime_object_stride = 0x380U;
inline constexpr std::size_t scm_runtime_object_stride = 0x3C0U;

// 2026-09-11 whole-image closure. Runtime object provenance is identified by
// manager +0xE8 count, manager +0x100 array and index*0x380. The previous
// model-core-bounded count of 31 missed 0x14029F0B0; the complete executable
// contains 32 exact roots. Conservative propagation from those roots reaches
// 61 typed states / 49 pointer-tagged direct calls. Only five paths load the
// retained serialized-object pointer at runtime +0x18, and none exports that
// serialized pointer or passes it through an indirect call.
inline constexpr std::size_t canonical_runtime_object_root_count = 32U;
inline constexpr std::size_t retained_pointer_typed_state_count = 61U;
inline constexpr std::size_t retained_pointer_direct_call_count = 49U;
inline constexpr std::size_t retained_serialized_object_load_count = 5U;
inline constexpr std::size_t retained_serialized_object_escape_count = 0U;
inline constexpr std::size_t retained_serialized_object_indirect_call_count = 0U;

// The five provenance-confirmed retained-pointer loads are:
//
// 0x14029F0F0  same model manager via 0x140089DE0; reads source object +0x08
// 0x1402F7DB4  EFM sibling builder; reads source object +0x08
// 0x1402F845E  EFM render builder; saved local is dead before return
// 0x1402FE6F4  MOD runtime mesh builder; reads source object +0x08
// 0x1402FED8E  MOD render builder; saved local is dead before return
//
// Thus object +0x04..07/+0x14..17/+0x20..2F have canonical runtime behavior
// EXE_CONFIRMED dormant/no effect. This is a behavioral result only: all three
// serialized ranges remain PRESERVED_UNDECODED and exact-byte writer
// obligations.
inline constexpr std::uintptr_t external_model_retained_pointer_load = 0x14029F0F0ULL;
inline constexpr std::uintptr_t efm_runtime_mesh_retained_pointer_load = 0x1402F7DB4ULL;
inline constexpr std::uintptr_t efm_render_retained_pointer_load = 0x1402F845EULL;
inline constexpr std::uintptr_t mod_runtime_mesh_retained_pointer_load = 0x1402FE6F4ULL;
inline constexpr std::uintptr_t mod_render_retained_pointer_load = 0x1402FED8EULL;

inline constexpr std::uintptr_t mod_runtime_mesh_builder_begin = 0x1402FE6A0ULL;
inline constexpr std::uintptr_t mod_runtime_mesh_builder_end = 0x1402FE921ULL;
inline constexpr std::uintptr_t mod_runtime_mesh_table_pointer_read = 0x1402FE700ULL;

inline constexpr std::uintptr_t mod_render_builder_begin = 0x1402FE930ULL;
inline constexpr std::uintptr_t mod_render_builder_end = 0x1402FF563ULL;
inline constexpr std::uintptr_t mod_render_retained_pointer_local_store = 0x1402FED92ULL;
inline constexpr std::size_t mod_render_saved_pointer_reads_after_store = 0U;

inline constexpr std::uintptr_t efm_render_builder_begin = 0x1402F8000ULL;
inline constexpr std::uintptr_t efm_render_builder_end = 0x1402F8C4AULL;
inline constexpr std::uintptr_t efm_render_retained_pointer_local_store = 0x1402F8462ULL;
inline constexpr std::size_t efm_render_saved_pointer_reads_after_store = 0U;

// Independent direct-source control: whole-image manager/source +0x108,
// index*0x40, source_header+0x40 derivation occurs in exactly five functions.
// The MOD/EFM/shared planning paths consume known fields only. 0x140302F10 is
// the SCM object initializer and 0x1402F9F20 belongs to a separate 0x3C0
// runtime-owner domain, so neither is promoted as MOD 0x380 provenance.
inline constexpr std::size_t direct_source_object_derivation_function_count = 5U;
inline constexpr std::uintptr_t direct_source_layout_planner_a = 0x1402FDB40ULL;
inline constexpr std::uintptr_t direct_source_layout_planner_b = 0x1402FDD10ULL;
inline constexpr std::uintptr_t mod_efm_object_initializer = 0x1403029E0ULL;
inline constexpr std::uintptr_t scm_object_initializer_begin = 0x140302F10ULL;
inline constexpr std::uintptr_t scm_object_initializer_end = 0x14030345AULL;

// Compile-time preservation probe. Synthetic non-zero values in every object
// secondary region must survive the raw snapshot helper exactly. Canonical
// runtime dormancy must never become implicit zero-normalization.
[[nodiscard]] constexpr std::array<std::byte, 0x40U>
make_object_secondary_preservation_probe() noexcept {
    std::array<std::byte, 0x40U> bytes{};
    for (std::size_t i = 0; i < object_04_07.size; ++i)
        bytes[object_04_07.offset + i] = static_cast<std::byte>(0xA1U + i);
    for (std::size_t i = 0; i < object_14_17.size; ++i)
        bytes[object_14_17.offset + i] = static_cast<std::byte>(0xB1U + i);
    for (std::size_t i = 0; i < object_20_2f.size; ++i)
        bytes[object_20_2f.offset + i] = static_cast<std::byte>(0xC0U + i);
    return bytes;
}

inline constexpr auto object_secondary_preservation_probe =
    make_object_secondary_preservation_probe();
inline constexpr auto object_secondary_probe_04_07 = snapshot_region<4U>(
    std::span<const std::byte>(object_secondary_preservation_probe), 0U,
    object_04_07);
inline constexpr auto object_secondary_probe_14_17 = snapshot_region<4U>(
    std::span<const std::byte>(object_secondary_preservation_probe), 0U,
    object_14_17);
inline constexpr auto object_secondary_probe_20_2f = snapshot_region<16U>(
    std::span<const std::byte>(object_secondary_preservation_probe), 0U,
    object_20_2f);

static_assert(header_08_0f.offset == 0x08U && header_08_0f.size == 0x08U);
static_assert(header_18_1f.offset == 0x18U && header_18_1f.size == 0x08U);
static_assert(header_28_3f.offset == 0x28U && header_28_3f.size == 0x18U);
static_assert(object_04_07.offset == 0x04U && object_04_07.size == 0x04U);
static_assert(object_14_17.offset == 0x14U && object_14_17.size == 0x04U);
static_assert(object_20_2f.offset == 0x20U && object_20_2f.size == 0x10U);
static_assert(runtime_serialized_object_pointer_offset == 0x18U);
static_assert(serialized_object_mesh_table_offset == 0x08U);
static_assert(mod_render_saved_pointer_reads_after_store == 0U);
static_assert(efm_render_saved_pointer_reads_after_store == 0U);
static_assert(scm_runtime_object_stride != mod_efm_runtime_object_stride);
static_assert(canonical_runtime_object_root_count == 32U);
static_assert(retained_serialized_object_escape_count == 0U);
static_assert(retained_serialized_object_indirect_call_count == 0U);
static_assert(object_secondary_probe_04_07[0] == std::byte{0xA1U});
static_assert(object_secondary_probe_04_07[3] == std::byte{0xA4U});
static_assert(object_secondary_probe_14_17[0] == std::byte{0xB1U});
static_assert(object_secondary_probe_14_17[3] == std::byte{0xB4U});
static_assert(object_secondary_probe_20_2f[0] == std::byte{0xC0U});
static_assert(object_secondary_probe_20_2f[15] == std::byte{0xCFU});

} // namespace dmc::rengine::analysis::mod
