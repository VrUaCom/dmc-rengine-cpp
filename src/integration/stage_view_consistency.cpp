#include "dmc_rengine/integration/stage_view_consistency.hpp"

#include <map>
#include <string>

namespace dmc::rengine::integration {
namespace {

[[nodiscard]] bool visual_category(
    gdspaces::StageResourceCategory category) noexcept {
    switch (category) {
    case gdspaces::StageResourceCategory::models:
    case gdspaces::StageResourceCategory::textures:
    case gdspaces::StageResourceCategory::animations:
    case gdspaces::StageResourceCategory::cameras:
    case gdspaces::StageResourceCategory::lighting:
    case gdspaces::StageResourceCategory::positions:
    case gdspaces::StageResourceCategory::effects:
    case gdspaces::StageResourceCategory::collision:
        return true;
    case gdspaces::StageResourceCategory::scripts:
    case gdspaces::StageResourceCategory::events:
    case gdspaces::StageResourceCategory::sounds:
    case gdspaces::StageResourceCategory::unknown:
        return false;
    }
    return false;
}

void add_issue(
    StageViewConsistencyReport& report,
    std::string code,
    std::string resource_id,
    std::string message) {
    report.issues.push_back(StageViewConsistencyIssue{
        .code = std::move(code),
        .resource_id = std::move(resource_id),
        .message = std::move(message),
    });
}

} // namespace

StageViewConsistencyReport validate_stage_views(
    const stageops::StageWorkspaceView& stage_ops,
    const modviz::SceneWorkspaceView& modviz) {
    StageViewConsistencyReport report;

    if (!stage_ops.valid()) {
        add_issue(
            report,
            "stage-view.stage-ops-invalid",
            {},
            "The Stage Ops workspace view is invalid.");
        return report;
    }
    if (!modviz.valid()) {
        add_issue(
            report,
            "stage-view.modviz-invalid",
            {},
            "The ModViz scene workspace view is invalid.");
        return report;
    }
    if (stage_ops.identity.stage_id != modviz.identity.stage_id ||
        stage_ops.identity.profile != modviz.identity.profile) {
        add_issue(
            report,
            "stage-view.identity-mismatch",
            {},
            "Stage Ops and ModViz do not reference the same stage identity.");
        return report;
    }

    std::map<std::string, const stageops::StageResourceView*, std::less<>>
        stage_resources;
    for (const auto& resource : stage_ops.resources) {
        stage_resources.emplace(resource.resource.id.canonical(), &resource);
    }

    std::map<std::string, const modviz::SceneResourceView*, std::less<>>
        visual_resources;
    for (const auto& resource : modviz.resources) {
        const auto canonical = resource.resource.id.canonical();
        if (!visual_resources.emplace(canonical, &resource).second) {
            add_issue(
                report,
                "stage-view.modviz-duplicate-resource",
                canonical,
                "ModViz contains the same canonical resource more than once.");
        }

        const auto stage_iterator = stage_resources.find(canonical);
        if (stage_iterator == stage_resources.end()) {
            add_issue(
                report,
                "stage-view.modviz-resource-missing-in-stage-ops",
                canonical,
                "ModViz exposes a resource absent from Stage Ops.");
            continue;
        }
        const auto& stage_resource = *stage_iterator->second;
        if (stage_resource.category != resource.stage_category) {
            add_issue(
                report,
                "stage-view.category-mismatch",
                canonical,
                "Stage Ops and ModViz assign different categories to the resource.");
        }
        if (stage_resource.role != resource.role) {
            add_issue(
                report,
                "stage-view.role-mismatch",
                canonical,
                "Stage Ops and ModViz assign different roles to the resource.");
        }
        if (stage_resource.dirty != resource.dirty ||
            stage_resource.working_copy_revision !=
                resource.working_copy_revision) {
            add_issue(
                report,
                "stage-view.revision-mismatch",
                canonical,
                "Stage Ops and ModViz disagree about WorkingCopy state.");
        }
        if (stage_resource.binary_document != resource.binary_document ||
            stage_resource.binary_coverage_bytes !=
                resource.binary_coverage_bytes) {
            add_issue(
                report,
                "stage-view.binary-context-mismatch",
                canonical,
                "Stage Ops and ModViz disagree about Binary Inspector context.");
        }
        if (stage_resource.evidence_record_count !=
            resource.evidence_record_count) {
            add_issue(
                report,
                "stage-view.evidence-count-mismatch",
                canonical,
                "Stage Ops and ModViz disagree about linked evidence count.");
        }
    }

    for (const auto& [canonical, stage_resource] : stage_resources) {
        if (visual_category(stage_resource->category) &&
            visual_resources.find(canonical) == visual_resources.end()) {
            add_issue(
                report,
                "stage-view.visual-resource-missing-in-modviz",
                canonical,
                "A visual Stage Ops resource is absent from ModViz.");
        }
    }
    return report;
}

} // namespace dmc::rengine::integration
