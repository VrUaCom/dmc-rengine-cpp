#include "runtime/resources/resource_manager.hpp"

#include <algorithm>

namespace dmc::recovered::dmc3::runtime::resources {
namespace {

[[nodiscard]] bool starts_with_magic(
    std::span<const std::byte> bytes,
    char a,
    char b,
    char c) noexcept {
    return bytes.size() >= 3U &&
        bytes[0] == static_cast<std::byte>(a) &&
        bytes[1] == static_cast<std::byte>(b) &&
        bytes[2] == static_cast<std::byte>(c);
}

} // namespace

std::optional<PostLoadFormat> confirmed_postload_format(
    std::span<const std::byte> bytes) noexcept {
    if (starts_with_magic(bytes, 'M', 'O', 'D')) {
        return PostLoadFormat::mod;
    }
    if (starts_with_magic(bytes, 'E', 'F', 'M')) {
        return PostLoadFormat::efm;
    }
    if (starts_with_magic(bytes, 'S', 'C', 'M')) {
        return PostLoadFormat::scm;
    }
    if (starts_with_magic(bytes, 'S', 'H', 'W')) {
        return PostLoadFormat::shw;
    }
    return std::nullopt;
}

ResourceTransitionResult apply_observed_cleanup_transition(
    ResourceRuntimeEntry& entry) noexcept {
    if (entry.state != ResourceLoadState::teardown_or_cancel_pending) {
        return ResourceTransitionResult::wrong_state;
    }

    entry.state = ResourceLoadState::free_or_unstarted;
    return ResourceTransitionResult::applied;
}

ResourceRuntimeManager::ResourceRuntimeManager() noexcept {
    const auto& model = wave2_resource_runtime_model();
    for (const auto& group : model.pool.groups) {
        const auto first = static_cast<std::size_t>(group.first_entry);
        const auto end = static_cast<std::size_t>(group.end_entry());
        for (std::size_t slot = first; slot < end && slot < entries_.size(); ++slot) {
            entries_[slot].group_index = group.index;
            entries_[slot].state = ResourceLoadState::free_or_unstarted;
        }
    }
}

const ResourceRuntimeEntry* ResourceRuntimeManager::entry(
    std::size_t slot) const noexcept {
    return slot < entries_.size() ? &entries_[slot] : nullptr;
}

ResourceRuntimeEntry* ResourceRuntimeManager::entry(
    std::size_t slot) noexcept {
    return slot < entries_.size() ? &entries_[slot] : nullptr;
}

std::optional<std::uint32_t> ResourceRuntimeManager::group_for_slot(
    std::size_t slot) const noexcept {
    const auto* current = entry(slot);
    if (current == nullptr) {
        return std::nullopt;
    }
    return current->group_index;
}

ResourceTransitionResult ResourceRuntimeManager::start_loading(
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

ResourceTransitionResult ResourceRuntimeManager::mark_io_complete(
    std::size_t slot,
    std::uintptr_t loaded_payload,
    std::uintptr_t owned_state) noexcept {
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

ResourceTransitionResult ResourceRuntimeManager::run_confirmed_postload(
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

ResourceTransitionResult ResourceRuntimeManager::cleanup_teardown_pending(
    std::size_t slot) noexcept {
    auto* current = entry(slot);
    if (current == nullptr) {
        return ResourceTransitionResult::slot_out_of_range;
    }
    return apply_observed_cleanup_transition(*current);
}

} // namespace dmc::recovered::dmc3::runtime::resources
