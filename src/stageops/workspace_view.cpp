#include "dmc_rengine/stageops/workspace_view.hpp"

#include <algorithm>

namespace dmc::rengine::stageops {

std::vector<const StageResourceView*> StageWorkspaceView::by_category(
    gdspaces::StageResourceCategory category) const {
    std::vector<const StageResourceView*> result;
    for (const auto& resource : resources) {
        if (resource.category == category) {
            result.push_back(&resource);
        }
    }
    return result;
}

StageWorkspaceView build_workspace_view(
    const integration::ProjectWorkspace& project,
    std::string_view stage_id) {
    StageWorkspaceView view;
    auto sessions = project.sessions_for_stage(stage_id);
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
        if (!view.identity.valid()) {
            view.identity = stage->identity;
        }
        if (stage->identity.stage_id != view.identity.stage_id ||
            stage->identity.profile != view.identity.profile) {
            return StageWorkspaceView{};
        }

        const auto* binary = session->binary_document();
        const auto* working = session->working_copy();
        view.resources.push_back(StageResourceView{
            .category = stage->category,
            .role = stage->role,
            .resource = session->resource(),
            .workspace_status = session->status(),
            .binary_document = binary != nullptr,
            .binary_coverage_bytes = binary == nullptr
                ? 0U
                : binary->coverage_bytes(),
            .evidence_record_count = session->evidence_record_ids().size(),
            .diagnostic_count = session->diagnostics().size(),
            .working_copy_revision = working == nullptr
                ? 0U
                : working->revision(),
            .dirty = working != nullptr && working->dirty(),
        });

        if (working != nullptr && working->dirty()) {
            ++view.dirty_resource_count;
        }
        view.validation_request_count += session->events().by_type(
            integration::WorkspaceEventType::validation_requested).size();
        for (const auto& diagnostic : session->diagnostics()) {
            if (diagnostic.severity == gdspaces::DiagnosticSeverity::error) {
                ++view.error_count;
            } else if (
                diagnostic.severity == gdspaces::DiagnosticSeverity::warning) {
                ++view.warning_count;
            }
        }
    }
    return view;
}

} // namespace dmc::rengine::stageops
