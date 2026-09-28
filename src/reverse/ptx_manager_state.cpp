#include "dmc_rengine/reverse/ptx_manager_state.hpp"

namespace dmc::rengine::reverse {

void construct_ptx_entry(PtxCacheEntryImage& entry) noexcept {
    entry.payload.fill(std::byte{0});
}

PtxManagerImage& construct_ptx_manager(PtxManagerImage& state) noexcept {
    state.vtable_va = ptx_manager_vtable;
    for (auto& entry : state.entries) construct_ptx_entry(entry);
    return state;
}

void reset_ptx_keys(PtxManagerImage& state) noexcept {
    for (auto& entry : state.entries) {
        entry.resource_key = 0;
        entry.references = 0;
    }
}

namespace {
PtxPayload* acquire(PtxManagerImage& state, std::uint64_t key,
                    std::uint64_t argument, bool variant, const PtxServices& services) noexcept {
    if (key == 0) return nullptr;
    for (auto& entry : state.entries) {
        if (entry.resource_key == key) {
            ++entry.references; // Preserve the EXE's modulo-2^32 behavior.
            return &entry.payload;
        }
    }
    for (auto& entry : state.entries) {
        if (entry.resource_key != 0) continue;
        PtxPayload temporary{};
        const int loaded = variant
            ? services.load_variant(services.context, key, temporary, argument)
            : services.load_basic(services.context, key, temporary);
        if (loaded == 0) return nullptr;
        entry.resource_key = key;
        entry.references = 1;
        entry.payload = temporary;
        return &entry.payload;
    }
    return nullptr;
}
} // namespace

PtxPayload* acquire_ptx(PtxManagerImage& state, std::uint64_t key,
                        const PtxServices& services) noexcept {
    return acquire(state, key, 0, false, services);
}

PtxPayload* acquire_ptx_variant(PtxManagerImage& state, std::uint64_t key,
                                std::uint64_t argument, const PtxServices& services) noexcept {
    return acquire(state, key, argument, true, services);
}

bool release_ptx(PtxManagerImage& state, std::uint64_t key, const PtxServices& services) noexcept {
    if (key == 0) return false;
    for (auto& entry : state.entries) {
        if (entry.resource_key != key) continue;
        --entry.references; // Underflow also wraps in the original EXE.
        if (entry.references != 0) return false;
        entry.resource_key = 0;
        services.release_payload(services.context, entry.payload);
        return true;
    }
    return false;
}

void destroy_ptx_manager(PtxManagerImage& state) noexcept {
    // Original: derived vptr, 32 ret-only element callbacks, base vptr.
    // This projects the resulting state; there are no payload-release calls.
    state.vtable_va = ptx_interface_vtable;
}

PtxManagerImage* deleting_destroy_ptx_manager(PtxManagerImage& state, std::uint32_t flags,
                                             const PtxServices& services) noexcept {
    auto* address = &state;
    destroy_ptx_manager(state);
    if ((flags & 1U) != 0) services.deallocate(services.context, address, 0x4308);
    return address;
}

} // namespace dmc::rengine::reverse
