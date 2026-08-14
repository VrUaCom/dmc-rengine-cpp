#pragma once

#include "dmc_rengine/stageops/operations_session.hpp"
#include "dmc_rengine/stageops/runtime_link_validation.hpp"
#include "dmc_rengine/stageops/scene_snapshot.hpp"

#include <cstdint>
#include <functional>
#include <string>
#include <string_view>
#include <vector>

namespace dmc::rengine::stageops {

enum class StageSceneRefreshStatus {
    invalid_session,
    analysis_failed,
    missing_project_sessions,
    runtime_link_provider_failed,
    runtime_links_invalid,
    provisional_snapshot_invalid,
    concurrent_change,
    final_snapshot_invalid,
    refreshed,
};

[[nodiscard]] constexpr std::string_view to_string(
    StageSceneRefreshStatus status) noexcept {
    switch (status) {
    case StageSceneRefreshStatus::invalid_session:
        return "invalid-session";
    case StageSceneRefreshStatus::analysis_failed:
        return "analysis-failed";
    case StageSceneRefreshStatus::missing_project_sessions:
        return "missing-project-sessions";
    case StageSceneRefreshStatus::runtime_link_provider_failed:
        return "runtime-link-provider-failed";
    case StageSceneRefreshStatus::runtime_links_invalid:
        return "runtime-links-invalid";
    case StageSceneRefreshStatus::provisional_snapshot_invalid:
        return "provisional-snapshot-invalid";
    case StageSceneRefreshStatus::concurrent_change:
        return "concurrent-change";
    case StageSceneRefreshStatus::final_snapshot_invalid:
        return "final-snapshot-invalid";
    case StageSceneRefreshStatus::refreshed:
        return "refreshed";
    }
    return "invalid-session";
}

struct StageSceneRefreshResult final {
    StageSceneRefreshStatus status{StageSceneRefreshStatus::invalid_session};
    std::uint64_t target_stage_revision{};
    std::uint64_t final_stage_revision{};
    bool external_change_detected{false};
    StageAnalysisRefreshResult analysis;
    StageRuntimeLinkValidationReport runtime_link_validation;
    StageSceneSnapshot snapshot;
    std::string error;

    [[nodiscard]] bool refreshed() const noexcept {
        return status == StageSceneRefreshStatus::refreshed &&
            snapshot.valid() &&
            snapshot.current_for_active_bytes();
    }

    [[nodiscard]] bool retryable_concurrency_conflict() const noexcept {
        return status == StageSceneRefreshStatus::concurrent_change;
    }
};

// Provider failure is not equivalent to an empty, valid runtime-link set. A
// profile/reverse integration that cannot validate its recovered contract must
// fail closed rather than silently publishing a scene without those semantics.
struct StageRuntimeLinkProviderResult final {
    bool valid{true};
    std::vector<StageRuntimeLink> links;
    std::string error;

    [[nodiscard]] bool coherent() const noexcept {
        return valid ? error.empty() : !error.empty();
    }
};

// Called only after canonical analysis has rebuilt StageDomainWorkspace for the
// exact target Stage Ops revision. Profile/reverse integrations can therefore
// derive offset-sensitive runtime links from current domain identities without
// duplicating the Stage Ops refresh transaction.
using StageRuntimeLinkProvider = std::function<
    StageRuntimeLinkProviderResult(const StageDomainWorkspace&)>;

// Canonical high-level Stage Ops refresh transaction.
//
// UI, ModViz and MCP/agent callers should use this instead of manually ordering
// detect_external_project_changes(), parser refresh, snapshot construction and
// commit_derived_refresh(). The controller owns no bytes and performs no
// resource resolution. It coordinates already-established Stage Ops and
// ProjectWorkspace authorities and fails closed on parser/evidence/concurrency
// inconsistencies.
class StageSceneController final {
public:
    [[nodiscard]] static StageSceneRefreshResult refresh(
        StageOperationsSession& session,
        std::vector<StageRuntimeLink> explicit_runtime_links = {});

    // Preferred composition form for profile/recovered-runtime bridges. The
    // provider executes against the exact post-analysis domains@revision before
    // runtime-link validation and the guarded commit.
    [[nodiscard]] static StageSceneRefreshResult refresh_with_provider(
        StageOperationsSession& session,
        StageRuntimeLinkProvider provider);
};

} // namespace dmc::rengine::stageops
