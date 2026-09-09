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

static_assert(header_08_0f.offset == 0x08U && header_08_0f.size == 0x08U);
static_assert(header_18_1f.offset == 0x18U && header_18_1f.size == 0x08U);
static_assert(header_28_3f.offset == 0x28U && header_28_3f.size == 0x18U);
static_assert(object_04_07.offset == 0x04U && object_04_07.size == 0x04U);
static_assert(object_14_17.offset == 0x14U && object_14_17.size == 0x04U);
static_assert(object_20_2f.offset == 0x20U && object_20_2f.size == 0x10U);
static_assert(runtime_serialized_object_pointer_offset == 0x18U);

} // namespace dmc::rengine::analysis::mod
