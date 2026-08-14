#include "dmc_rengine/stageops/workspace_view.hpp"

#include <algorithm>
#include <string>

namespace dmc::rengine::stageops {
namespace {

void count_diagnostic(
    StageWorkspaceView& view,
    const gdspaces::Diagnostic& diagnostic) {
    if (diagnostic.severity == gdspaces::DiagnosticSeverity::error) {
        ++view.error_count;
    } else if (diagnostic.severity == gdspaces::DiagnosticSeverity::warning) {
        ++view.warning_count;
    }
}

void attach_project_state(
    StageWorkspaceView& view,
    StageResourceView& resource,
    const integration::ResourceWorkspaceSession* session) {
    if (session == nullptr) {
        return;
    }

    resource.project_session_attached = true;
    resource.workspace_status = session->status();
    const auto* binary = session->binary_document();
    const auto* working = session->working_copy();
    resource.binary_document = binary != nullptr;
    resource.binary_coverage_bytes = binary == nullptr
        ? 0U
        : binary->coverage_bytes();
    resource.evidence_record_count = session->evidence_record_ids().size();
    resource.diagnostic_count = session->diagnostics().size();
    resource.working_copy_revision = working == nullptr
        ? 0U
        : working->revision();
    resource.dirty = working != nullptr && working->dirty();

    if (resource.dirty) {
        ++view.dirty_resource_count;
    }
    view.validation_request_count += session->events().by_type(
        integration::WorkspaceEventType::validation_requested).size();
    for (const auto& diagnostic : session->diagnostics()) {
        count_diagnostic(view, diagnostic);
    }
}

} // namespace

bool StageResourceView::has_category(
    gdspaces::StageResourceCategory category) const noexcept {
    return std::any_of(
        memberships.begin(), memberships.end(),
        [category](const StageAssemblyMembership& membership) {
            return membership.category == category;
        });
}

std::vector<const StageResourceView*> StageWorkspaceView::by_category(
    gdspaces::StageResourceCategory category) const {
    std::vector<const StageResourceView*> result;
    for (const auto& resource : resources) {
        if (resource.has_category(category)) {
            result.push_back(&resource);
        }
    }
    return result;
}

StageWorkspaceView build_workspace_view(
    const StageAssemblyWorkspace& assembly,
    const integration::ProjectWorkspace* project) {
    StageWorkspaceView view;
    if (!assembly.valid()) {
        return view;
    }

    view.identity = assembly.identity;
    view.assembly_status = assembly.status();
    view.game_readiness = assembly.game_readiness();
    view.requirements = assembly.requirements;
    view.unresolved_requirement_count = assembly.unresolved_requirement_count();

    for (const auto& diagnostic : assembly.diagnostics) {
        count_diagnostic(view, diagnostic);
    }

    view.resources.reserve(assembly.resources.size());
    for (const auto& assembled_resource : assembly.resources) {
        const auto canonical = assembled_resource.resource.id.canonical();
        const auto memberships = assembly.memberships_for(canonical);
        std::vector<StageAssemblyMembership> membership_copies;
        membership_copies.reserve(memberships.size());
        for (const auto* membership : memberships) {
            membership_copies.push_back(*membership);
        }

        StageResourceView resource{
            .resource = assembled_resource.resource,
            .memberships = std::move(membership_copies),
            .materialized = assembled_resource.materialized,
            .byte_provenance_valid =
                assembled_resource.byte_provenance.has_value() &&
                assembled_resource.byte_provenance->valid(),
        };

        const auto* session = project == nullptr
            ? nullptr
            : project->find_session(assembled_resource.resource.id);
        attach_project_state(view, resource, session);
        view.resources.push_back(std::move(resource));
    }
    return view;
}

StageWorkspaceView build_workspace_view(
    const integration::ProjectWorkspace& project,
    std::string_view resource_set_id) {
    StageWorkspaceView view;
    auto sessions = project.sessions_for_stage_resource_set(resource_set_id);
    std::sort(
        sessions.begin(), sessions.end(),
        [](const integration::ResourceWorkspaceSession* left,
           const integration::ResourceWorkspaceSession* right) {
            return left->resource().id.canonical() <
                   right->resource().id.canonical();
        });

    for (const auto* session : sessions) {
        const auto* stage = session->stage();
        if (stage == nullptr) {
            continue;
        }
        if (!view.identity.stage.valid()) {
            view.identity = StageAssemblyIdentity{
                .stage = stage->identity,
                .catalog_entry_id = std::string{resource_set_id},
                .global_catalog_row = 0U,
                .source_table_id = "legacy-project-stage-context",
                .source_row_index = 0U,
            };
            view.assembly_status = StageAssemblyStatus::partial;
            view.game_readiness = StageGameReadiness::unproven;
        }
        if (stage->identity.resource_set_key() !=
                view.identity.stage.resource_set_key() ||
            stage->identity.profile != view.identity.stage.profile) {
            return StageWorkspaceView{};
        }

        StageResourceView resource;
        resource.resource = session->resource();
        resource.materialized = session->source_payload().readable();
        resource.byte_provenance_valid =
            session->source_payload().byte_provenance.has_value() &&
            session->source_payload().byte_provenance->valid();
        resource.memberships.push_back(StageAssemblyMembership{
            .resource_id = resource.resource.id.canonical(),
            .category = stage->category,
            .role = stage->role,
            .kind = StageAssemblyMembershipKind::descriptor_root,
            .parent_resource_id = std::nullopt,
            .container_slot = std::nullopt,
        });
        attach_project_state(view, resource, session);
        view.resources.push_back(std::move(resource));
    }
    return view;
}

} // namespace dmc::rengine::stageops
