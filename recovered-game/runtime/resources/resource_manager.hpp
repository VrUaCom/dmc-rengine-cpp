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

namespace detail {

[[nodiscard]] constexpr bool starts_with_magic(
    std::span<const std::byte> bytes,
    char a,
    char b,
    char c) noexcept {
    return bytes.size() >= 3U &&
        bytes[0] == static_cast<std::byte>(a) &&
        bytes[1] == static_cast<std::byte>(b) &&
        bytes[2] == static_cast<std::byte>(c);
}

} // namespace detail

[[nodiscard]] inline std::optional<PostLoadFormat> confirmed_postload_format(
    std::span<const std::byte> bytes) noexcept {
    if (detail::starts_with_magic(bytes, 'M', 'O', 'D')) {
        return PostLoadFormat::mod;
    }
    if (detail::starts_with_magic(bytes, 'E', 'F', 'M')) {
        return PostLoadFormat::efm;
    }
    if (detail::starts_with_magic(bytes, 'S', 'C', 'M')) {
        return PostLoadFormat::scm;
    }
    if (detail::starts_with_magic(bytes, 'S', 'H', 'W')) {
        return PostLoadFormat::shw;
    }
    return std::nullopt;
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

class ResourceRuntimeManager final {
public:
    static constexpr std::size_t slot_count = 363U;

    ResourceRuntimeManager() noexcept {
        const auto& model = wave2_resource_runtime_model();
        for (const auto& group : model.pool.groups) {
            const auto first = static_cast<std::size_t>(group.first_entry);
            const auto end = static_cast<std::size_t>(group.end_entry());
            for (std::size_t slot = first;
                 slot < end && slot < entries_.size();
                 ++slot) {
                entries_[slot].group_index = group.index;
                entries_[slot].state = ResourceLoadState::free_or_unstarted;
            }
        }
    }

    [[nodiscard]] const ResourceRuntimeEntry* entry(
        std::size_t slot) const noexcept {
        return slot < entries_.size() ? &entries_[slot] : nullptr;
    }

    [[nodiscard]] ResourceRuntimeEntry* entry(
        std::size_t slot) noexcept {
        return slot < entries_.size() ? &entries_[slot] : nullptr;
    }

    [[nodiscard]] std::optional<std::uint32_t> group_for_slot(
        std::size_t slot) const noexcept {
        const auto* current = entry(slot);
        if (current == nullptr) {
            return std::nullopt;
        }
        return current->group_index;
    }

    // Confirmed successful lifecycle edge 0 -> 1. Slot-selection policy is not
    // guessed here; the caller supplies the exact pool slot.
    [[nodiscard]] ResourceTransitionResult start_loading(
        std::size_t slot,
        std::uint16_t subtype_index,
        std::uintptr_t source_descriptor) noexcept {
        auto* current = entry(slot);
        if (current == nullptr) {
            return ResourceTransitionResult::slot_out_of_range;
        }
        if (current->state != ResourceLoadState::free_or_unstarted) {
            return ResourceTransitionResult::wrong_state;
        }

        current->subtype_index = subtype_index;
        current->source_descriptor = source_descriptor;
        current->state = ResourceLoadState::io_scheduled_or_loading;
        return ResourceTransitionResult::applied;
    }

    // Confirmed successful lifecycle edge 1 -> 2. The payload/owned-state values
    // are stored in their recovered ABI fields, but their deeper pointee types
    // remain intentionally opaque.
    [[nodiscard]] ResourceTransitionResult mark_io_complete(
        std::size_t slot,
        std::uintptr_t loaded_payload,
        std::uintptr_t owned_state = 0U) noexcept {
        auto* current = entry(slot);
        if (current == nullptr) {
            return ResourceTransitionResult::slot_out_of_range;
        }
        if (current->state != ResourceLoadState::io_scheduled_or_loading) {
            return ResourceTransitionResult::wrong_state;
        }
        if (loaded_payload == 0U) {
            return ResourceTransitionResult::payload_missing;
        }

        current->loaded_payload = loaded_payload;
        current->owned_state = owned_state;
        current->state = ResourceLoadState::io_complete_pending_postprocess;
        return ResourceTransitionResult::applied;
    }

    // Confirmed state-2 typed-postload boundary. State 3 is reached only when a
    // confirmed MOD/EFM/SCM/SHW magic is recognized and the supplied recovered
    // fixup backend succeeds.
    [[nodiscard]] ResourceTransitionResult run_confirmed_postload(
        std::size_t slot,
        std::span<std::byte> bytes,
        IConfirmedPostLoadBackend& backend) noexcept {
        auto* current = entry(slot);
        if (current == nullptr) {
            return ResourceTransitionResult::slot_out_of_range;
        }
        if (current->state != ResourceLoadState::io_complete_pending_postprocess) {
            return ResourceTransitionResult::wrong_state;
        }
        if (current->loaded_payload == 0U || bytes.empty()) {
            return ResourceTransitionResult::payload_missing;
        }

        const auto format = confirmed_postload_format(bytes);
        if (!format.has_value()) {
            return ResourceTransitionResult::unsupported_postload_format;
        }
        if (!backend.apply(*format, bytes)) {
            return ResourceTransitionResult::postload_failed;
        }

        current->state = ResourceLoadState::ready_postprocessed;
        return ResourceTransitionResult::applied;
    }

    // Only the observed 4 -> 0 transition is implemented. The transition into
    // state 4 remains outside this API until its source-state domain is closed.
    [[nodiscard]] ResourceTransitionResult cleanup_teardown_pending(
        std::size_t slot) noexcept {
        auto* current = entry(slot);
        if (current == nullptr) {
            return ResourceTransitionResult::slot_out_of_range;
        }
        return apply_observed_cleanup_transition(*current);
    }

private:
    std::array<ResourceRuntimeEntry, slot_count> entries_{};
};

} // namespace dmc::recovered::dmc3::runtime::resources
