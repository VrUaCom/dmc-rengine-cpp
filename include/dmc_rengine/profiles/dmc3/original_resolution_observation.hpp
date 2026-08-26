#pragma once

#include "dmc_rengine/profiles/dmc3/resource_lookup_policy.hpp"
#include "dmc_rengine/profiles/dmc3/runtime_resource_resolver.hpp"

#include <cstddef>
#include <cstdint>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace dmc::rengine::profiles::dmc3 {

enum class OriginalResolutionProbeOutcome {
    miss,
    selected,
};

[[nodiscard]] constexpr std::string_view to_string(
    OriginalResolutionProbeOutcome outcome) noexcept {
    switch (outcome) {
    case OriginalResolutionProbeOutcome::miss: return "miss";
    case OriginalResolutionProbeOutcome::selected: return "selected";
    }
    return "miss";
}

struct OriginalArchiveArtifactIdentity final {
    std::uint32_t volume_index{};
    std::string filename;
    std::string sha256;
    std::uint64_t size{};

    [[nodiscard]] bool valid() const noexcept;
};

// One directly observed provider operation from the protected original process.
// Archive candidates can emit several probes at the same lookup_attempt_index,
// one per mounted volume in recovered highest-to-lowest precedence order.
// Physical candidates emit exactly one probe.
struct OriginalResolutionProbe final {
    std::size_t sequence_index{};
    std::size_t lookup_attempt_index{};
    ResourceProviderClass provider{ResourceProviderClass::archive};
    std::string candidate;
    std::string provider_key;
    std::optional<std::uint32_t> archive_volume_index;
    OriginalResolutionProbeOutcome outcome{OriginalResolutionProbeOutcome::miss};

    [[nodiscard]] bool valid_shape() const noexcept;
};

struct OriginalSelectedResourceIdentity final {
    ResourceProviderClass provider{ResourceProviderClass::archive};
    std::size_t lookup_attempt_index{};
    std::string candidate;
    std::string provider_key;

    // Archive selection authority.
    std::optional<std::uint32_t> archive_volume_index;
    std::string archive_member_path;

    // Physical selection authority. This is the mounted-root-relative resource
    // identity, not an absolute private workstation path.
    std::string physical_relative_path;

    [[nodiscard]] bool valid_shape() const noexcept;
};

struct OriginalResolutionObservation final {
    // Must classify as the canonical protected-distribution/original-execution
    // candidate, never the canonical analysis executable.
    std::string executable_sha256;
    std::uint64_t executable_size{};

    // SHA-256 of the validated dmc-rengine.gdspaces-l2-runtime-mapping.v1 packet
    // used to authorize these runtime probe locations. This field binds the
    // observation to mapping evidence but does not by itself validate that
    // packet; the acquisition pipeline must validate the packet first.
    std::string runtime_mapping_packet_sha256;

    std::uint32_t pid{};
    std::uint64_t module_base{};
    std::uint32_t flags{1U};
    std::string request;
    std::string basename;

    // Recovered numbered-volume bootstrap is contiguous [0, first_missing).
    std::uint32_t first_missing_archive_volume{};
    std::vector<OriginalArchiveArtifactIdentity> archives;

    std::vector<OriginalResolutionProbe> probes;
    std::optional<OriginalSelectedResourceIdentity> selected;

    [[nodiscard]] bool valid() const noexcept;
};

// Metadata-only public receipt. Raw executable/resource bytes are never
// serialized. Empty string means structural validation failed.
[[nodiscard]] std::string original_resolution_observation_to_json(
    const OriginalResolutionObservation& observation);

enum class OriginalProductComparisonStatus {
    matched,
    invalid_original_observation,
    product_not_resolved,
    provider_identity_mismatch,
    resource_identity_mismatch,
};

[[nodiscard]] constexpr std::string_view to_string(
    OriginalProductComparisonStatus status) noexcept {
    switch (status) {
    case OriginalProductComparisonStatus::matched: return "matched";
    case OriginalProductComparisonStatus::invalid_original_observation:
        return "invalid-original-observation";
    case OriginalProductComparisonStatus::product_not_resolved:
        return "product-not-resolved";
    case OriginalProductComparisonStatus::provider_identity_mismatch:
        return "provider-identity-mismatch";
    case OriginalProductComparisonStatus::resource_identity_mismatch:
        return "resource-identity-mismatch";
    }
    return "invalid-original-observation";
}

struct OriginalProductResolutionComparison final {
    OriginalProductComparisonStatus status{
        OriginalProductComparisonStatus::invalid_original_observation};
    std::string detail;

    [[nodiscard]] bool matched() const noexcept {
        return status == OriginalProductComparisonStatus::matched;
    }
};

// Compares only the selected identity. It never upgrades product probes into
// original-process evidence and never treats a synthetic/product match as proof
// that the original process emitted the observation.
[[nodiscard]] OriginalProductResolutionComparison compare_original_to_product(
    const OriginalResolutionObservation& original,
    const RuntimeResolutionReport& product,
    const RuntimeSourceBindings& bindings) noexcept;

} // namespace dmc::rengine::profiles::dmc3
