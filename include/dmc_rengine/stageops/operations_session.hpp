#pragma once

#include "dmc_rengine/gdspaces/working_copy.hpp"
#include "dmc_rengine/integration/project_workspace.hpp"
#include "dmc_rengine/stageops/assembly_workspace.hpp"

#include <cstddef>
#include <cstdint>
#include <map>
#include <string>
#include <string_view>

namespace dmc::rengine::stageops {

struct StageOperationResult final {
    bool applied{false};
    std::uint64_t stage_revision{};
    std::uint64_t resource_revision{};
    std::string error;
};

struct StageOperationsSnapshot final {
    std::uint64_t stage_revision{};
    StageAssemblyStatus assembly_status{StageAssemblyStatus::invalid};
    StageGameReadiness game_readiness{StageGameReadiness::unproven};
    bool derived_state_stale{false};
    std::size_t dirty_resource_count{};
    std::size_t missing_project_session_count{};
    std::size_t validation_request_count{};

    [[nodiscard]] bool valid() const noexcept {
        return assembly_status != StageAssemblyStatus::invalid;
    }
};

// Stage-scoped product operations over one canonical StageAssemblyWorkspace.
//
// This session intentionally does not implement vanilla DMC runtime behavior.
// It coordinates DMC Rengine WorkingCopy/event operations through
// ProjectWorkspace and conservatively marks derived scene/domain state stale
// whenever bytes change. Fine-grained dependency invalidation is added only
// when Stage Ops owns an evidence-backed domain/dependency relationship.
class StageOperationsSession final {
public:
    StageOperationsSession(
        StageAssemblyWorkspace assembly,
        integration::ProjectWorkspace& project);

    [[nodiscard]] bool valid() const noexcept;
    [[nodiscard]] const StageAssemblyWorkspace& assembly() const noexcept;
    [[nodiscard]] integration::ProjectWorkspace& project() noexcept;
    [[nodiscard]] const integration::ProjectWorkspace& project() const noexcept;

    [[nodiscard]] std::uint64_t stage_revision() const noexcept;
    [[nodiscard]] bool derived_state_stale() const noexcept;
    [[nodiscard]] StageOperationsSnapshot snapshot() const;

    // Creates/retains the shared ProjectWorkspace WorkingCopy for one resource
    // already owned by this StageAssemblyWorkspace. It does not mutate bytes and
    // therefore does not invalidate derived scene state.
    [[nodiscard]] bool enable_editing(std::string_view canonical_resource_id);

    [[nodiscard]] StageOperationResult apply_edit(
        std::string_view canonical_resource_id,
        gdspaces::EditOperation operation);

    [[nodiscard]] StageOperationResult undo_last_edit(
        std::string_view canonical_resource_id);

    [[nodiscard]] StageOperationResult reset_resource(
        std::string_view canonical_resource_id);

    // Emits Build & Test Lab validation requests for every assembled resource
    // that currently has a ProjectWorkspace session and a Stage Ops tool route.
    // Missing sessions remain visible in snapshot() and are never fabricated.
    [[nodiscard]] std::size_t request_validation(
        std::string validation_id,
        std::string message);

    // Detects byte-state changes performed through another tool/session after
    // the last observed revision. One or more changes increment the Stage Ops
    // stage revision once and mark derived scene/domain state stale.
    [[nodiscard]] bool detect_external_project_changes();

    // Called only after Stage Ops derived/domain projections have actually been
    // rebuilt against the expected revision. This clears the conservative stale
    // gate and establishes a new observed WorkingCopy baseline.
    [[nodiscard]] bool commit_derived_refresh(
        std::uint64_t expected_stage_revision);

private:
    struct ObservedResourceState final {
        bool project_session_present{false};
        bool working_copy_present{false};
        std::uint64_t working_copy_revision{};
        bool dirty{false};

        friend bool operator==(
            const ObservedResourceState&,
            const ObservedResourceState&) = default;
    };

    [[nodiscard]] const StageAssemblyResource* stage_resource(
        std::string_view canonical_resource_id) const noexcept;
    [[nodiscard]] ObservedResourceState observe(
        const StageAssemblyResource& resource) const noexcept;
    void capture_observed_state();
    void update_observed_state(const StageAssemblyResource& resource);
    [[nodiscard]] StageOperationResult rejected(
        std::string error,
        std::uint64_t resource_revision = 0U) const;
    [[nodiscard]] StageOperationResult committed(
        const StageAssemblyResource& resource);

    StageAssemblyWorkspace assembly_;
    integration::ProjectWorkspace* project_{nullptr};
    std::uint64_t stage_revision_{};
    bool derived_state_stale_{false};
    std::map<std::string, ObservedResourceState, std::less<>> observed_;
};

} // namespace dmc::rengine::stageops
