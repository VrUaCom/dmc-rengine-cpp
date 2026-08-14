#pragma once

#include "runtime/resources/resource_lifecycle.hpp"

#include <array>
#include <cstddef>
#include <cstdint>
#include <span>

namespace dmc::recovered::dmc3::runtime::resources {

// Minimum recovered in-memory ABI for one entry in the 363-entry resource pool.
// Unknown bytes stay explicit. This is intentionally a 64-bit target because the
// canonical DMC3 executable is PE32+ x86-64.
//
// IMPORTANT: known field locations do not by themselves prove which original
// transition writer assigns each field. The helpers below therefore change only
// lifecycle state. Field-writer ownership remains a separate reverse target.
struct ResourceRuntimeEntry final {
    std::uint32_t group_index{};                                   // +0x00
    ResourceLoadState state{ResourceLoadState::free_or_unstarted}; // +0x04
    std::uint16_t subtype_index{};                                 // +0x08
    std::array<std::byte, 0x0E> unresolved_0A_17{};               // +0x0A..+0x17
    std::uintptr_t source_descriptor{};                            // +0x18
    std::uintptr_t loaded_payload{};                               // +0x20
    std::uintptr_t owned_state{};                                  // +0x28
    std::array<std::byte, 0x18> unresolved_30_47{};               // +0x30..+0x47
};

static_assert(sizeof(std::uintptr_t) == 8U);
static_assert(sizeof(ResourceRuntimeEntry) == 0x48U);
static_assert(offsetof(ResourceRuntimeEntry, group_index) == 0x00U);
static_assert(offsetof(ResourceRuntimeEntry, state) == 0x04U);
static_assert(offsetof(ResourceRuntimeEntry, subtype_index) == 0x08U);
static_assert(offsetof(ResourceRuntimeEntry, source_descriptor) == 0x18U);
static_assert(offsetof(ResourceRuntimeEntry, loaded_payload) == 0x20U);
static_assert(offsetof(ResourceRuntimeEntry, owned_state) == 0x28U);

enum class ResourceTransitionResult {
    applied,
    wrong_state,
    unsupported_postload_format,
    postload_failed,
};

class IConfirmedPostLoadBackend {
public:
    virtual ~IConfirmedPostLoadBackend() = default;

    // Execution boundary for one explicitly selected, confirmed Wave-2 typed
    // post-load helper. The backend is responsible for the actual in-place
    // algorithm. Selecting the format is kept outside this ABI slice because
    // Wave-2 proves typed dispatch, but not yet the complete dispatcher key/ABI.
    [[nodiscard]] virtual bool apply(
        PostLoadFormat format,
        std::span<std::byte> bytes) = 0;
};

// Directly observed successful lifecycle edge 0 -> 1. No subtype, descriptor,
// callback, ownership, or scheduling side effects are invented here.
[[nodiscard]] inline ResourceTransitionResult transition_free_to_loading(
    ResourceRuntimeEntry& entry) noexcept {
    if (entry.state != ResourceLoadState::free_or_unstarted) {
        return ResourceTransitionResult::wrong_state;
    }
    entry.state = ResourceLoadState::io_scheduled_or_loading;
    return ResourceTransitionResult::applied;
}

// Directly observed successful lifecycle edge 1 -> 2. Known payload/owned-state
// fields are deliberately not assigned here until the original writer(s) and
// ordering are recovered from executable evidence.
[[nodiscard]] inline ResourceTransitionResult transition_loading_to_io_complete(
    ResourceRuntimeEntry& entry) noexcept {
    if (entry.state != ResourceLoadState::io_scheduled_or_loading) {
        return ResourceTransitionResult::wrong_state;
    }
    entry.state = ResourceLoadState::io_complete_pending_postprocess;
    return ResourceTransitionResult::applied;
}

// Confirmed state-2 typed post-load boundary. The caller explicitly supplies a
// confirmed format; this avoids claiming that the original dispatcher chooses
// MOD/EFM/SCM/SHW by a three-byte magic test. State 3 is reached only when the
// corresponding recovered backend succeeds.
[[nodiscard]] inline ResourceTransitionResult run_confirmed_postload(
    ResourceRuntimeEntry& entry,
    PostLoadFormat format,
    std::span<std::byte> bytes,
    IConfirmedPostLoadBackend& backend) noexcept {
    if (entry.state != ResourceLoadState::io_complete_pending_postprocess) {
        return ResourceTransitionResult::wrong_state;
    }
    if (wave2_resource_runtime_model().fixup(format) == nullptr) {
        return ResourceTransitionResult::unsupported_postload_format;
    }
    if (!backend.apply(format, bytes)) {
        return ResourceTransitionResult::postload_failed;
    }
    entry.state = ResourceLoadState::ready_postprocessed;
    return ResourceTransitionResult::applied;
}

// Apply only the directly observed state-4 -> state-0 cleanup edge. This does
// not claim the still-unrecovered source-state domain that enters state 4, nor
// does it invent field-clearing/destructor behavior around that transition.
[[nodiscard]] inline ResourceTransitionResult apply_observed_cleanup_transition(
    ResourceRuntimeEntry& entry) noexcept {
    if (entry.state != ResourceLoadState::teardown_or_cancel_pending) {
        return ResourceTransitionResult::wrong_state;
    }
    entry.state = ResourceLoadState::free_or_unstarted;
    return ResourceTransitionResult::applied;
}

} // namespace dmc::recovered::dmc3::runtime::resources
