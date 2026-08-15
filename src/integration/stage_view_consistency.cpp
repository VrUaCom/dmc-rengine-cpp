#include "dmc_rengine/integration/stage_view_consistency.hpp"

#include <algorithm>
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

[[nodiscard]] bool same_identity(
    const stageops::StageAssemblyIdentity& left,
    const stageops::StageAssemblyIdentity& right) noexcept {
    return left.catalog_entry_id == right.catalog_entry_id &&
        left.global_catalog_row == right.global_catalog_row &&
        left.source_table_id == right.source_table_id &&
        left.source_row_index == right.source_row_index &&
        left.stage.profile == right.stage.profile &&
        left.stage.resource_set_key() == right.stage.resource_set_key() &&
        left.stage.numeric_stage_id == right.stage.numeric_stage_id &&
        left.stage.semantic_stage_id == right.stage.semantic_stage_id &&
        left.stage.exe_evidence_id == right.stage.exe_evidence_id;
}

[[nodiscard]] bool has_matching_modviz_membership(
    const modviz::SceneResourceView& visual,
    const stageops::StageAssemblyMembership& membership) {
    return std::any_of(
        visual.memberships.begin(), visual.memberships.end(),
        [&membership](const modviz::SceneMembershipView& candidate) {
            return candidate.stage_category == membership.category &&
                candidate.role == membership.role &&
                candidate.source_kind == membership.kind &&
                candidate.parent_resource_id == membership.parent_resource_id &&
                candidate.container_slot == membership.container_slot;
        });
}

[[nodiscard]] bool has_matching_stageops_membership(
    const stageops::StageResourceView& stage_resource,
    const modviz::SceneMembershipView& membership) {
    return std::any_of(
        stage_resource.memberships.begin(), stage_resource.memberships.end(),
        [&membership](const stageops::StageAssemblyMembership& candidate) {
            return candidate.category == membership.stage_category &&
                candidate.role == membership.role &&
                candidate.kind == membership.source_kind &&
                candidate.parent_resource_id == membership.parent_resource_id &&
                candidate.container_slot == membership.container_slot;
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
            "The Stage Ops workspace projection is invalid.");
        return report;
    }
    if (!modviz.valid()) {
        add_issue(
            report,
            "stage-view.modviz-invalid",
            {},
            "The ModViz scene projection is invalid.");
        return report;
    }
    if (!same_identity(stage_ops.identity, modviz.identity)) {
        add_issue(
            report,
            "stage-view.identity-mismatch",
            {},
            "ModViz does not reference the exact Stage Ops assembly identity.");
        return report;
    }
    if (stage_ops.assembly_status != modviz.assembly_status ||
        stage_ops.game_readiness != modviz.game_readiness ||
        stage_ops.unresolved_requirement_count !=
            modviz.unresolved_requirement_count) {
        add_issue(
            report,
            "stage-view.assembly-state-mismatch",
            {},
            "ModViz does not expose the same Stage Ops assembly/readiness state.");
    }

    std::map<std::string, const stageops::StageResourceView*, std::less<>>
        stage_resources;
    for (const auto& resource : stage_ops.resources) {
        const auto canonical = resource.resource.id.canonical();
        if (!stage_resources.emplace(canonical, &resource).second) {
            add_issue(
                report,
                "stage-view.stage-ops-duplicate-resource",
                canonical,
                "Stage Ops projection contains a duplicate canonical ResourceId.");
        }
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
                "ModViz exposes a resource absent from the Stage Ops aggregate projection.");
            continue;
        }
        const auto& stage_resource = *stage_iterator->second;

        if (stage_resource.materialized != resource.materialized) {
            add_issue(
                report,
                "stage-view.materialization-mismatch",
                canonical,
                "ModViz and Stage Ops disagree about product materialization state.");
        }
        if (stage_resource.project_session_attached !=
            resource.project_session_attached) {
            add_issue(
                report,
                "stage-view.project-session-mismatch",
                canonical,
                "ModViz and Stage Ops disagree about shared ProjectWorkspace session availability.");
        }
        if (stage_resource.dirty != resource.dirty ||
            stage_resource.working_copy_revision !=
                resource.working_copy_revision) {
            add_issue(
                report,
                "stage-view.revision-mismatch",
                canonical,
                "ModViz and Stage Ops disagree about WorkingCopy state.");
        }
        if (stage_resource.binary_document != resource.binary_document ||
            stage_resource.binary_coverage_bytes !=
                resource.binary_coverage_bytes) {
            add_issue(
                report,
                "stage-view.binary-context-mismatch",
                canonical,
                "ModViz and Stage Ops disagree about Binary Inspector context.");
        }
        if (stage_resource.evidence_record_count !=
            resource.evidence_record_count) {
            add_issue(
                report,
                "stage-view.evidence-count-mismatch",
                canonical,
                "ModViz and Stage Ops disagree about linked evidence count.");
        }

        for (const auto& membership : resource.memberships) {
            if (!has_matching_stageops_membership(stage_resource, membership)) {
                add_issue(
                    report,
                    "stage-view.modviz-membership-not-in-stage-ops",
                    canonical,
                    "ModViz invented or altered a scene membership absent from Stage Ops.");
            }
        }
    }

    // Every visual Stage Ops membership must be represented by ModViz. Nonvisual
    // memberships intentionally remain Stage Ops/Semantic Graph concerns.
    for (const auto& [canonical, stage_resource] : stage_resources) {
        const auto visual = visual_resources.find(canonical);
        for (const auto& membership : stage_resource->memberships) {
            if (!visual_category(membership.category)) {
                continue;
            }
            if (visual == visual_resources.end() ||
                !has_matching_modviz_membership(*visual->second, membership)) {
                add_issue(
                    report,
                    "stage-view.visual-membership-missing-in-modviz",
                    canonical,
                    "A visual Stage Ops membership is absent from the ModViz projection.");
            }
        }
    }
    return report;
}

} // namespace dmc::rengine::integration
