#pragma once

#include "dmc_rengine/gdspaces/resource_payload.hpp"
#include "dmc_rengine/gdspaces/working_copy.hpp"

#include <cstddef>
#include <cstdint>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace dmc::rengine::profiles::dmc3 {

enum class RelativeSlotRebuildStatus : std::uint8_t {
    ok,
    invalid_source,
    resource_mismatch,
    source_hash_mismatch,
    unsupported_format,
    size_changed,
    source_parse_failed,
    output_parse_failed,
    topology_changed,
    protected_prefix_changed,
};

[[nodiscard]] constexpr std::string_view to_string(
    RelativeSlotRebuildStatus status) noexcept {
    switch (status) {
    case RelativeSlotRebuildStatus::ok: return "ok";
    case RelativeSlotRebuildStatus::invalid_source: return "invalid-source";
    case RelativeSlotRebuildStatus::resource_mismatch: return "resource-mismatch";
    case RelativeSlotRebuildStatus::source_hash_mismatch: return "source-hash-mismatch";
    case RelativeSlotRebuildStatus::unsupported_format: return "unsupported-format";
    case RelativeSlotRebuildStatus::size_changed: return "size-changed";
    case RelativeSlotRebuildStatus::source_parse_failed: return "source-parse-failed";
    case RelativeSlotRebuildStatus::output_parse_failed: return "output-parse-failed";
    case RelativeSlotRebuildStatus::topology_changed: return "topology-changed";
    case RelativeSlotRebuildStatus::protected_prefix_changed:
        return "protected-prefix-changed";
    }
    return "invalid-source";
}

struct RelativeSlotTopologyEntry final {
    std::uint32_t slot_index{};
    std::uint64_t offset{};
    std::uint64_t size{};
    bool populated{false};

    friend bool operator==(
        const RelativeSlotTopologyEntry&,
        const RelativeSlotTopologyEntry&) = default;
};

struct RelativeSlotTopology final {
    std::string format;
    std::uint32_t declared_slot_count{};
    std::uint64_t container_size{};
    std::uint64_t protected_prefix_size{};
    std::vector<RelativeSlotTopologyEntry> entries;

    [[nodiscard]] bool valid() const noexcept;

    friend bool operator==(
        const RelativeSlotTopology&,
        const RelativeSlotTopology&) = default;
};

struct LayoutPreservingRebuildReceipt final {
    gdspaces::ResourceId resource;
    std::string source_sha256;
    std::string output_sha256;
    std::uint64_t working_revision{};
    RelativeSlotTopology source_topology;
    RelativeSlotTopology output_topology;

    [[nodiscard]] bool valid() const noexcept;
};

struct RelativeSlotRebuildResult final {
    RelativeSlotRebuildStatus status{RelativeSlotRebuildStatus::invalid_source};
    std::vector<std::byte> bytes;
    std::optional<LayoutPreservingRebuildReceipt> receipt;
    std::string detail;

    [[nodiscard]] bool ok() const noexcept {
        return status == RelativeSlotRebuildStatus::ok &&
            receipt.has_value() && receipt->valid();
    }
};

class RelativeSlotLayoutWriter final {
public:
    // Product writer for the bounded first Layer-1 authoring tier. The
    // original materialized PAC/PNST image remains layout authority; this
    // does not claim Capcom offline-packer equivalence.
    [[nodiscard]] static RelativeSlotRebuildResult rebuild(
        const gdspaces::ResourcePayload& source,
        const gdspaces::WorkingCopy& working_copy);
};

} // namespace dmc::rengine::profiles::dmc3
