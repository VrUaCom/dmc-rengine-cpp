#pragma once

#include "runtime/resources/resource_lifecycle.hpp"

#include <array>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <span>

namespace dmc::recovered::dmc3::runtime::resources {

// Minimum recovered in-memory ABI for one entry in the 363-entry resource pool.
// Unknown bytes stay explicit. This is intentionally a 64-bit target because the
// canonical DMC3 executable is PE32+ x86-64.
struct ResourceRuntimeEntry final {
    std::uint32_t group_index{};                                  // +0x00
    ResourceLoadState state{ResourceLoadState::free_or_unstarted}; // +0x04
    std::uint16_t subtype_index{};                                // +0x08
    std::array<std::byte, 0x0E> unresolved_0A_17{};              // +0x0A..+0x17
    std::uintptr_t source_descriptor{};                           // +0x18
    std::uintptr_t loaded_payload{};                              // +0x20
    std::uintptr_t owned_state{};                                 // +0x28
    std::array<std::byte, 0x18> unresolved_30_47{};              // +0x30..+0x47
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
    slot_out_of_range,
    wrong_state,
    payload_missing,
    unsupported_postload_format,
    postload_failed,
};

class IConfirmedPostLoadBackend {
public:
    virtual ~IConfirmedPostLoadBackend() = default;

    // Implementations reconstruct the in-place algorithm for the corresponding
    // confirmed helper. Until a format-specific implementation exists, callers
    // can supply a research/test backend, but state 3 is reached only after this
    // callback returns true.
    [[nodiscard]] virtual bool apply(
        PostLoadFormat format,
        std::span<std::byte> bytes) = 0;
};

[[nodiscard]] std::optional<PostLoadFormat> confirmed_postload_format(
    std::span<const std::byte> bytes) noexcept;

// Apply only the directly observed state-4 -> state-0 cleanup edge. This does
// not claim the still-unrecovered source-state domain that enters state 4, nor
// does it invent field-clearing/destructor behavior around that transition.
[[nodiscard]] ResourceTransitionResult apply_observed_cleanup_transition(
    ResourceRuntimeEntry& entry) noexcept;

class ResourceRuntimeManager final {
public:
    static constexpr std::size_t slot_count = 363U;

    ResourceRuntimeManager() noexcept;

    [[nodiscard]] const ResourceRuntimeEntry* entry(
        std::size_t slot) const noexcept;

    [[nodiscard]] ResourceRuntimeEntry* entry(
        std::size_t slot) noexcept;

    [[nodiscard]] std::optional<std::uint32_t> group_for_slot(
        std::size_t slot) const noexcept;

    // Confirmed successful lifecycle edge 0 -> 1. Slot-selection policy is not
    // guessed here; the caller supplies the exact pool slot.
    [[nodiscard]] ResourceTransitionResult start_loading(
        std::size_t slot,
        std::uint16_t subtype_index,
        std::uintptr_t source_descriptor) noexcept;

    // Confirmed successful lifecycle edge 1 -> 2. The payload/owned-state values
    // are stored in their recovered ABI fields, but their deeper pointee types
    // remain intentionally opaque.
    [[nodiscard]] ResourceTransitionResult mark_io_complete(
        std::size_t slot,
        std::uintptr_t loaded_payload,
        std::uintptr_t owned_state = 0U) noexcept;

    // Confirmed state-2 typed-postload boundary. State 3 is reached only when a
    // confirmed MOD/EFM/SCM/SHW magic is recognized and the supplied recovered
    // fixup backend succeeds.
    [[nodiscard]] ResourceTransitionResult run_confirmed_postload(
        std::size_t slot,
        std::span<std::byte> bytes,
        IConfirmedPostLoadBackend& backend) noexcept;

    // Only the observed 4 -> 0 transition is implemented. The transition into
    // state 4 remains outside this API until its source-state domain is closed.
    [[nodiscard]] ResourceTransitionResult cleanup_teardown_pending(
        std::size_t slot) noexcept;

private:
    std::array<ResourceRuntimeEntry, slot_count> entries_{};
};

} // namespace dmc::recovered::dmc3::runtime::resources
