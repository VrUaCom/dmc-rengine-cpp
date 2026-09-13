#include "dmc_rengine/runtime/resource_bridge.hpp"

#include "dmc_rengine/gdspaces/diagnostic.hpp"

#include <string>
#include <utility>

namespace dmc::rengine::runtime {

ResourceBridge::ResourceBridge(const gdspaces::SourceRegistry& registry) noexcept
    : registry_(&registry) {}

Expected<const gdspaces::ResourcePayload*> ResourceBridge::load(const gdspaces::ResourceId& id) {
    if (!id.valid()) {
        ++stats_.failures;
        return fail(make_error(RuntimeErrorCode::invalid_argument, "resource-bridge",
                               "resource id is not valid"));
    }

    const std::string key = id.canonical();
    if (const auto cached = cache_.find(key); cached != cache_.end()) {
        ++stats_.hits;
        return &cached->second;
    }

    ++stats_.misses;

    auto payload = registry_->read(id);
    if (!payload.has_value()) {
        ++stats_.failures;
        return fail(make_error(RuntimeErrorCode::resource_missing, "resource-bridge",
                               "GDSpaces returned no payload for '" + key + "'"));
    }
    if (!payload->readable()) {
        ++stats_.failures;

        std::string detail;
        for (const auto& diagnostic : payload->diagnostics) {
            if (diagnostic.severity == gdspaces::DiagnosticSeverity::error) {
                detail = diagnostic.code + ": " + diagnostic.message;
                break;
            }
        }
        if (detail.empty()) {
            detail = "payload reported itself unreadable";
        }

        return fail(make_error(RuntimeErrorCode::resource_unreadable, "resource-bridge",
                               "'" + key + "' " + detail));
    }

    resident_bytes_ += static_cast<std::uint64_t>(payload->bytes.size());
    const auto inserted = cache_.emplace(key, std::move(*payload));
    return &inserted.first->second;
}

Expected<std::span<const std::byte>> ResourceBridge::bytes(const gdspaces::ResourceId& id) {
    auto payload = load(id);
    if (!payload.has_value()) {
        return fail(payload.error());
    }
    return std::span<const std::byte>{(*payload)->bytes};
}

Expected<const gdspaces::ResourcePayload*> ResourceBridge::payload(
    const gdspaces::ResourceId& id) {
    return load(id);
}

bool ResourceBridge::resident(const gdspaces::ResourceId& id) const {
    return cache_.find(id.canonical()) != cache_.end();
}

bool ResourceBridge::evict(const gdspaces::ResourceId& id) {
    const auto cached = cache_.find(id.canonical());
    if (cached == cache_.end()) {
        return false;
    }

    resident_bytes_ -= static_cast<std::uint64_t>(cached->second.bytes.size());
    cache_.erase(cached);
    return true;
}

void ResourceBridge::clear() noexcept {
    cache_.clear();
    resident_bytes_ = 0U;
}

std::size_t ResourceBridge::resident_resources() const noexcept {
    return cache_.size();
}

std::uint64_t ResourceBridge::resident_bytes() const noexcept {
    return resident_bytes_;
}

const ResourceBridgeStats& ResourceBridge::stats() const noexcept {
    return stats_;
}

} // namespace dmc::rengine::runtime
