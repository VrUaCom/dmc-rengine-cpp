#pragma once

#include <array>
#include <cstddef>
#include <cstdint>

namespace dmc::rengine::reverse {

using PtxPayload = std::array<std::byte, 0x208>;

struct PtxCacheEntryImage {
    std::uint64_t resource_key;
    std::uint32_t references;
    std::array<std::byte, 4> preserved_0c;
    PtxPayload payload;
};
static_assert(sizeof(PtxCacheEntryImage) == 0x218);
static_assert(offsetof(PtxCacheEntryImage, payload) == 0x10);

// A canonical-image state model, not a host object with an executable vtable.
// vtable_va is an address tag; it must never be called as a native function.
struct PtxManagerImage {
    std::uint64_t vtable_va;
    std::array<PtxCacheEntryImage, 32> entries;
};
static_assert(offsetof(PtxManagerImage, entries) == 8);
static_assert(sizeof(PtxManagerImage) == 0x4308);

inline constexpr std::uint64_t ptx_manager_vtable = 0x140507b60;
inline constexpr std::uint64_t ptx_interface_vtable = 0x140507b30;

// Explicit unresolved service boundary. Functions must be present when their
// path is reached. These model normal returns, not Windows exception unwinding.
struct PtxServices {
    void* context;
    int (*load_basic)(void*, std::uint64_t, PtxPayload&) noexcept; // 0x140336BB0
    int (*load_variant)(void*, std::uint64_t, PtxPayload&, std::uint64_t) noexcept; // 0x140336A70
    void (*release_payload)(void*, PtxPayload&) noexcept; // 0x1403317D0
    void (*deallocate)(void*, PtxManagerImage*, std::size_t) noexcept; // 0x140345554
};

// 0x140314CB0 and 0x140314C50: key/count/padding are deliberately preserved.
void construct_ptx_entry(PtxCacheEntryImage&) noexcept;
PtxManagerImage& construct_ptx_manager(PtxManagerImage&) noexcept;
// 0x140315150 clears keys/counts, retaining payloads and padding; no releases.
void reset_ptx_keys(PtxManagerImage&) noexcept;
// 0x140314E00 / 0x140314FA0. Both variants share the same key-only cache.
PtxPayload* acquire_ptx(PtxManagerImage&, std::uint64_t key, const PtxServices&) noexcept;
PtxPayload* acquire_ptx_variant(PtxManagerImage&, std::uint64_t key,
                                std::uint64_t variant_argument, const PtxServices&) noexcept;
// 0x140315180 returns true only when a matching count reaches zero and releases.
bool release_ptx(PtxManagerImage&, std::uint64_t key, const PtxServices&) noexcept;
// 0x140314D00: array callbacks are ret-only; payload release is not performed.
void destroy_ptx_manager(PtxManagerImage&) noexcept;
// 0x140314D60. Bit 0 controls the sized delete. Returned address can be dangling.
PtxManagerImage* deleting_destroy_ptx_manager(PtxManagerImage&, std::uint32_t flags,
                                             const PtxServices&) noexcept;

} // namespace dmc::rengine::reverse
