#pragma once

#include "dmc_rengine/gdspaces/resource_id.hpp"
#include "dmc_rengine/gdspaces/resource_payload.hpp"
#include "dmc_rengine/gdspaces/source_registry.hpp"
#include "dmc_rengine/runtime/status.hpp"

#include <cstddef>
#include <cstdint>
#include <map>
#include <span>
#include <string>

namespace dmc::rengine::runtime {

struct ResourceBridgeStats final {
    std::uint64_t hits{};
    std::uint64_t misses{};
    std::uint64_t failures{};

    friend bool operator==(const ResourceBridgeStats&, const ResourceBridgeStats&) = default;
};

/// The runtime's only door to resource bytes (Constitution, Article I).
///
/// Every byte the runtime consumes is read through a `SourceRegistry`. The
/// bridge adds a residency cache and typed failures on top; it does not
/// resolve paths, expand containers or invent identities, because that
/// authority belongs to GDSpaces alone.
class ResourceBridge final {
public:
    explicit ResourceBridge(const gdspaces::SourceRegistry& registry) noexcept;

    /// Reads a resource, serving it from the cache when already resident.
    [[nodiscard]] Expected<std::span<const std::byte>> bytes(const gdspaces::ResourceId& id);

    /// Cached payload including GDSpaces diagnostics and provenance.
    [[nodiscard]] Expected<const gdspaces::ResourcePayload*> payload(
        const gdspaces::ResourceId& id);

    [[nodiscard]] bool resident(const gdspaces::ResourceId& id) const;
    bool evict(const gdspaces::ResourceId& id);
    void clear() noexcept;

    [[nodiscard]] std::size_t resident_resources() const noexcept;
    [[nodiscard]] std::uint64_t resident_bytes() const noexcept;
    [[nodiscard]] const ResourceBridgeStats& stats() const noexcept;

private:
    [[nodiscard]] Expected<const gdspaces::ResourcePayload*> load(const gdspaces::ResourceId& id);

    const gdspaces::SourceRegistry* registry_{};
    std::map<std::string, gdspaces::ResourcePayload, std::less<>> cache_{};
    std::uint64_t resident_bytes_{};
    ResourceBridgeStats stats_{};
};

} // namespace dmc::rengine::runtime
