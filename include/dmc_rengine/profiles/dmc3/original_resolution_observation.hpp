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

// Identity of one successfully mounted numbered archive. The archive index must
// be below first_missing_archive_volume, but the mounted set is allowed to be a
// strict subset of the discovered 0..first-missing-1 filenames because archive
// initialization can fail without stopping later discovery/mount attempts.
struct OriginalArchiveArtifactIdentity final {
    std::uint32_t volume_index{};
    std::string filename;
    std::string sha256;
    std::uint64_t size{};

    [[nodiscard]] bool valid() const noexcept;
};

// One actually executed candidate-provider operation shaped after the recovered
// original policy. Archive candidates emit one probe per successfully mounted
// archive in recovered mount-list precedence (highest successful discovery index
// first). Discovered-but-failed archives do not become lookup probes. Physical
// candidates emit exactly one probe. Structure alone is never trusted original-
// process evidence; trusted origin belongs to the external publisher gate.
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

    std::optional<std::uint32_t> archive_volume_index;
    std::string archive_member_path;

    // Mounted-root-relative resource identity, never a private absolute path.
    std::string physical_relative_path;

    [[nodiscard]] bool valid_shape() const noexcept;
};

// Content candidate shaped after one protected-process resolver observation.
// This aggregate is intentionally forgeable for parser/contract tests, so
// valid() means structural/content validity only. It must pass the external
// normalization + artifact-backed + trusted-origin gates before promotion.
struct OriginalResolutionObservation final {
    std::string executable_sha256;
    std::uint64_t executable_size{};

    std::string runtime_mapping_packet_sha256;

    std::string observer_id;
    std::string observer_version;
    std::string observer_build_sha256;

    bool trace_complete{};
    std::uint64_t dropped_event_count{};

    std::uint32_t pid{};
    // Windows process creation FILETIME. This must match the R2B mapping packet
    // before any candidate can be bound to that runtime session.
    std::uint64_t process_creation_filetime{};
    std::uint64_t module_base{};
    std::uint32_t flags{1U};
    std::string request;
    std::string basename;

    // Discovery stop: first numbered DMC3-N.nbz filename that was absent.
    // This does NOT imply every lower discovered archive mounted successfully.
    std::uint32_t first_missing_archive_volume{};

    // Successfully mounted numbered archives only, sorted by ascending volume
    // index for deterministic serialization. Resolver traversal uses reverse
    // order because successful registrations are prepended during bootstrap.
    std::vector<OriginalArchiveArtifactIdentity> archives;

    std::vector<OriginalResolutionProbe> probes;
    std::optional<OriginalSelectedResourceIdentity> selected;

    [[nodiscard]] bool valid() const noexcept;
};

// Metadata-only producer. The emitted v2 surface carries the process-instance
// creation identity and explicit mounted archive set but remains non-trusted
// content. Consumers must normalize and artifact-bind it before promotion.
[[nodiscard]] std::string original_resolution_observation_to_json(
    const OriginalResolutionObservation& observation);

enum class OriginalProductComparisonStatus {
    matched,
    invalid_original_observation,
    invalid_product_configuration,
    product_not_resolved,
    provider_identity_mismatch,
    resource_identity_mismatch,
};

[[nodiscard]] constexpr std::string_view to_string(
    OriginalProductComparisonStatus status) noexcept {
    switch (status) {
    case OriginalProductComparisonStatus::matched: return "candidate-content-match";
    case OriginalProductComparisonStatus::invalid_original_observation:
        return "invalid-content-candidate";
    case OriginalProductComparisonStatus::invalid_product_configuration:
        return "invalid-product-configuration";
    case OriginalProductComparisonStatus::product_not_resolved:
        return "product-not-resolved";
    case OriginalProductComparisonStatus::provider_identity_mismatch:
        return "provider-identity-mismatch";
    case OriginalProductComparisonStatus::resource_identity_mismatch:
        return "resource-identity-mismatch";
    }
    return "invalid-content-candidate";
}

struct OriginalProductResolutionComparison final {
    OriginalProductComparisonStatus status{
        OriginalProductComparisonStatus::invalid_original_observation};
    std::string detail;

    // This is a content-only comparison. It is never a trusted/original-process
    // evidence predicate and must not drive promotion.
    [[nodiscard]] bool candidate_content_matched() const noexcept {
        return status == OriginalProductComparisonStatus::matched;
    }

    [[deprecated("matched() is candidate-content-only; it is not original-process evidence. Use candidate_content_matched().")]]
    [[nodiscard]] bool matched() const noexcept {
        return candidate_content_matched();
    }
};

// Content-only helper for development diagnostics. It compares the candidate
// selected identity with one product resolution result. It does not bind observer
// artifacts, archive artifacts or trusted capture origin and therefore must never
// be used as an R3 promotion predicate.
[[nodiscard]] OriginalProductResolutionComparison compare_original_to_product(
    const OriginalResolutionObservation& candidate,
    const RuntimeResolutionReport& product,
    const RuntimeSourceBindings& bindings) noexcept;

} // namespace dmc::rengine::profiles::dmc3
