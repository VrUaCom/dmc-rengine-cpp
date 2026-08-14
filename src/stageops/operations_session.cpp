#include "dmc_rengine/stageops/operations_session.hpp"

#include <utility>

namespace dmc::rengine::stageops {
namespace {

[[nodiscard]] bool byte_state_changed(
    const StageOperationsSession::ObservedResourceState& left,
    const StageOperationsSession::ObservedResourceState& right) noexcept {
    return left.working_copy_revision != right.working_copy_revision ||
        left.dirty != right.dirty;
}

} // namespace

StageOperationsSession::StageOperationsSession(
    StageAssemblyWorkspace assembly,
    integration::ProjectWorkspace& project)
    : assembly_(std::move(assembly)),
      project_(&project) {
    capture_observed_state();
}

bool StageOperationsSession::valid() const noexcept {
    return project_ != nullptr && assembly_.valid();
}

const StageAssemblyWorkspace& StageOperationsSession::assembly() const noexcept {
    return assembly_;
}

integration::ProjectWorkspace& StageOperationsSession::project() noexcept {
    return *project_;
}

const integration::ProjectWorkspace&
StageOperationsSession::project() const noexcept {
    return *project_;
}

std::uint64_t StageOperationsSession::stage_revision() const noexcept {
    return stage_revision_;
}

bool StageOperationsSession::derived_state_stale() const noexcept {
    return derived_state_stale_;
}

StageOperationsSnapshot StageOperationsSession::snapshot() const {
    StageOperationsSnapshot result{
        .stage_revision = stage_revision_,
        .assembly_status = assembly_.status(),
        .game_readiness = assembly_.game_readiness(),
        .derived_state_stale = derived_state_stale_,
        .dirty_resource_count = 0U,
        .missing_project_session_count = 0U,
        .validation_request_count = 0U,
    };

    if (!valid()) {
        result.assembly_status = StageAssemblyStatus::invalid;
        return result;
    }

    for (const auto& resource : assembly_.resources) {
        const auto* session = project_->find_session(resource.resource.id);
        if (session == nullptr) {
            ++result.missing_project_session_count;
            continue;
        }
        const auto* working = session->working_copy();
        if (working != nullptr && working->dirty()) {
            ++result.dirty_resource_count;
        }
        result.validation_request_count += session->events().by_type(
            integration::WorkspaceEventType::validation_requested).size();
    }
    return result;
}

bool StageOperationsSession::enable_editing(
    std::string_view canonical_resource_id) {
    const auto* resource = stage_resource(canonical_resource_id);
    if (!valid() || resource == nullptr) {
        return false;
    }
    if (!project_->enable_working_copy(resource->resource.id)) {
        return false;
    }
    // Enabling a revision-0 clean WorkingCopy does not alter scene bytes. Record
    // it as the new operational baseline without invalidating derived state.
    update_observed_state(*resource);
    return true;
}

StageOperationResult StageOperationsSession::apply_edit(
    std::string_view canonical_resource_id,
    gdspaces::EditOperation operation) {
    const auto* resource = stage_resource(canonical_resource_id);
    if (!valid() || resource == nullptr) {
        return rejected("Resource is not owned by this Stage Ops assembly.");
    }

    const auto* session = project_->find_session(resource->resource.id);
    if (session == nullptr) {
        return rejected("Stage resource has no ProjectWorkspace session.");
    }
    const auto* working = session->working_copy();
    if (working == nullptr) {
        return rejected("WorkingCopy is not enabled for this stage resource.");
    }

    const auto result = project_->apply_edit(
        resource->resource.id,
        std::move(operation),
        gdspaces::ToolTarget::stage_ops);
    if (!result.applied) {
        return rejected(result.error, result.revision);
    }
    return committed(*resource);
}

StageOperationResult StageOperationsSession::undo_last_edit(
    std::string_view canonical_resource_id) {
    const auto* resource = stage_resource(canonical_resource_id);
    if (!valid() || resource == nullptr) {
        return rejected("Resource is not owned by this Stage Ops assembly.");
    }

    const auto* session = project_->find_session(resource->resource.id);
    if (session == nullptr || session->working_copy() == nullptr) {
        return rejected("Stage resource has no enabled WorkingCopy.");
    }
    const auto previous_revision = session->working_copy()->revision();
    if (!project_->undo_last_edit(
            resource->resource.id,
            gdspaces::ToolTarget::stage_ops)) {
        return rejected(
            "The last stage resource edit could not be undone.",
            previous_revision);
    }
    return committed(*resource);
}

StageOperationResult StageOperationsSession::reset_resource(
    std::string_view canonical_resource_id) {
    const auto* resource = stage_resource(canonical_resource_id);
    if (!valid() || resource == nullptr) {
        return rejected("Resource is not owned by this Stage Ops assembly.");
    }

    const auto* session = project_->find_session(resource->resource.id);
    if (session == nullptr || session->working_copy() == nullptr) {
        return rejected("Stage resource has no enabled WorkingCopy.");
    }
    const auto before = observe(*resource);
    if (!project_->reset_working_copy(
            resource->resource.id,
            gdspaces::ToolTarget::stage_ops)) {
        return rejected(
            "The stage resource WorkingCopy could not be reset.",
            before.working_copy_revision);
    }

    const auto after = observe(*resource);
    update_observed_state(*resource);
    if (byte_state_changed(before, after)) {
        ++stage_revision_;
        derived_state_stale_ = true;
    }
    return StageOperationResult{
        .applied = true,
        .stage_revision = stage_revision_,
        .resource_revision = after.working_copy_revision,
        .error = {},
    };
}

std::size_t StageOperationsSession::request_validation(
    std::string validation_id,
    std::string message) {
    if (!valid() || validation_id.empty() || message.empty()) {
        return 0U;
    }

    std::size_t requested = 0U;
    for (const auto& resource : assembly_.resources) {
        if (project_->find_session(resource.resource.id) == nullptr) {
            continue;
        }
        if (project_->request_validation(
                resource.resource.id,
                gdspaces::ToolTarget::stage_ops,
                validation_id,
                message)) {
            ++requested;
        }
    }
    return requested;
}

bool StageOperationsSession::detect_external_project_changes() {
    if (!valid()) {
        return false;
    }

    bool changed = false;
    for (const auto& resource : assembly_.resources) {
        const auto canonical = resource.resource.id.canonical();
        const auto current = observe(resource);
        const auto iterator = observed_.find(canonical);
        if (iterator == observed_.end()) {
            observed_.emplace(canonical, current);
            if (current.working_copy_revision != 0U || current.dirty) {
                changed = true;
            }
            continue;
        }

        if (byte_state_changed(iterator->second, current)) {
            changed = true;
        }
        iterator->second = current;
    }

    if (changed) {
        ++stage_revision_;
        derived_state_stale_ = true;
    }
    return changed;
}

bool StageOperationsSession::commit_derived_refresh(
    std::uint64_t expected_stage_revision) {
    if (!valid() || expected_stage_revision != stage_revision_) {
        return false;
    }
    capture_observed_state();
    derived_state_stale_ = false;
    return true;
}

const StageAssemblyResource* StageOperationsSession::stage_resource(
    std::string_view canonical_resource_id) const noexcept {
    return assembly_.find_resource(canonical_resource_id);
}

StageOperationsSession::ObservedResourceState StageOperationsSession::observe(
    const StageAssemblyResource& resource) const noexcept {
    ObservedResourceState result;
    if (project_ == nullptr) {
        return result;
    }

    const auto* session = project_->find_session(resource.resource.id);
    if (session == nullptr) {
        return result;
    }
    result.project_session_present = true;
    const auto* working = session->working_copy();
    if (working == nullptr) {
        return result;
    }
    result.working_copy_present = true;
    result.working_copy_revision = working->revision();
    result.dirty = working->dirty();
    return result;
}

void StageOperationsSession::capture_observed_state() {
    observed_.clear();
    if (!assembly_.valid()) {
        return;
    }
    for (const auto& resource : assembly_.resources) {
        observed_.emplace(
            resource.resource.id.canonical(),
            observe(resource));
    }
}

void StageOperationsSession::update_observed_state(
    const StageAssemblyResource& resource) {
    observed_[resource.resource.id.canonical()] = observe(resource);
}

StageOperationResult StageOperationsSession::rejected(
    std::string error,
    std::uint64_t resource_revision) const {
    return StageOperationResult{
        .applied = false,
        .stage_revision = stage_revision_,
        .resource_revision = resource_revision,
        .error = std::move(error),
    };
}

StageOperationResult StageOperationsSession::committed(
    const StageAssemblyResource& resource) {
    ++stage_revision_;
    derived_state_stale_ = true;
    update_observed_state(resource);

    const auto* session = project_->find_session(resource.resource.id);
    const auto* working = session == nullptr ? nullptr : session->working_copy();
    return StageOperationResult{
        .applied = true,
        .stage_revision = stage_revision_,
        .resource_revision = working == nullptr ? 0U : working->revision(),
        .error = {},
    };
}

} // namespace dmc::rengine::stageops
