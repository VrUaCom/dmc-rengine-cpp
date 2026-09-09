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
// Zero corpus values and initializer-local no-read evidence do not authorize
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
// NOT read serialized object +0x18/+0x1C. Downstream code may therefore retain
// access to the entire serialized record through runtime +0x18; all unresolved
// serialized ranges must remain byte-preserved until their consumer census is
// closed.
inline constexpr std::size_t runtime_serialized_object_pointer_offset = 0x18U;

// Hash-verified direct-disassembly follow-up (canonical dmc3.exe):
//
// MOD runtime mesh builder 0x1402FE6A0..0x1402FE921 derives a canonical
// runtime object with the 0x380 stride. At 0x1402FE6F4 it loads runtime +0x18,
// and at 0x1402FE700 the retained serialized pointer is dereferenced only at
// serialized object +0x08 to recover the mesh-table pointer. The function then
// switches provenance into the 0x50-byte serialized mesh-record domain. This
// is a positive control proving that the retained pointer is live while not
// reading object +0x04..07/+0x14..17/+0x20..2F in this path.
inline constexpr std::uintptr_t mod_runtime_mesh_builder_begin = 0x1402FE6A0ULL;
inline constexpr std::uintptr_t mod_runtime_mesh_builder_end = 0x1402FE921ULL;
inline constexpr std::uintptr_t mod_runtime_mesh_retained_pointer_load = 0x1402FE6F4ULL;
inline constexpr std::uintptr_t mod_runtime_mesh_table_pointer_read = 0x1402FE700ULL;
inline constexpr std::size_t serialized_object_mesh_table_offset = 0x08U;

// MOD render-command builder 0x1402FE930..0x1402FF563 performs another
// provenance-confirmed load of runtime +0x18 at 0x1402FED8E and stores it in
// local rbp+0x30 at 0x1402FED92. Full bounded-function disassembly contains no
// later read of that local slot before return. The homologous EFM builder
// 0x1402F8000..0x1402F8C4A has the same dead-after-load pattern at
// 0x1402F845E/0x1402F8462. These are bounded negative results, not global proof
// that the serialized object secondary regions are unused.
inline constexpr std::uintptr_t mod_render_builder_begin = 0x1402FE930ULL;
inline constexpr std::uintptr_t mod_render_builder_end = 0x1402FF563ULL;
inline constexpr std::uintptr_t mod_render_retained_pointer_load = 0x1402FED8EULL;
inline constexpr std::uintptr_t mod_render_retained_pointer_local_store = 0x1402FED92ULL;
inline constexpr std::size_t mod_render_saved_pointer_reads_after_store = 0U;

inline constexpr std::uintptr_t efm_render_builder_begin = 0x1402F8000ULL;
inline constexpr std::uintptr_t efm_render_builder_end = 0x1402F8C4AULL;
inline constexpr std::uintptr_t efm_render_retained_pointer_load = 0x1402F845EULL;
inline constexpr std::uintptr_t efm_render_retained_pointer_local_store = 0x1402F8462ULL;
inline constexpr std::size_t efm_render_saved_pointer_reads_after_store = 0U;

// Important provenance rejection: 0x140302F10..0x14030345A also retains a
// serialized 0x40-record pointer at runtime +0x18, but its runtime record stride
// is 0x3C0 rather than MOD/EFM's canonical 0x380 path. Independent SCM evidence
// identifies this function as the SCM object initializer and its C4/EA narrow
// compatibility rewrites. Equal physical offsets and source-record stride do
// not make it a MOD retained-pointer consumer.
inline constexpr std::uintptr_t scm_object_initializer_begin = 0x140302F10ULL;
inline constexpr std::uintptr_t scm_object_initializer_end = 0x14030345AULL;
inline constexpr std::size_t scm_runtime_object_stride = 0x3C0U;
inline constexpr std::size_t mod_efm_runtime_object_stride = 0x380U;

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

} // namespace dmc::rengine::analysis::mod
