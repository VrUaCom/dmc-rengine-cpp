#include "dmc_rengine/modviz/stage_scene_projection.hpp"

#include <algorithm>
#include <string>
#include <utility>

namespace dmc::rengine::modviz {
namespace {

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

[[nodiscard]] bool visual_domain(stageops::StageDomainKind kind) noexcept {
    switch (kind) {
    case stageops::StageDomainKind::hits_collision:
    case stageops::StageDomainKind::lighting_records:
        return true;
    case stageops::StageDomainKind::dca_records:
    case stageops::StageDomainKind::stage_script_tokens:
    case stageops::StageDomainKind::stage_set_directive_token:
    case stageops::StageDomainKind::stage_set_value_token:
    case stageops::StageDomainKind::door_token:
    case stageops::StageDomainKind::box_in_token:
    case stageops::StageDomainKind::next_room_token:
        return false;
    }
    return false;
}

} // namespace

StageSceneProjection StageSceneProjectionBuilder::build(
    const stageops::StageOperationsSession& session,
    const stageops::StageSceneSnapshot& snapshot) {
    StageSceneProjection projection;
    if (!session.valid() || !snapshot.valid() ||
        snapshot.stage_revision() != session.stage_revision() ||
        !same_identity(
            snapshot.knowledge.identity(),
            session.assembly().identity)) {
        return projection;
    }

    projection.identity = session.assembly().identity;
    projection.stage_revision = snapshot.stage_revision();
    projection.current_for_active_bytes =
        snapshot.current_for_active_bytes();
    projection.unresolved_requirement_count =
        session.assembly().unresolved_requirement_count();

    projection.resources.reserve(session.assembly().resources.size());
    for (const auto& assembled : session.assembly().resources) {
        StageSceneResourceProjection resource{
            .resource = assembled.resource,
            .memberships = {},
            .materialized = assembled.materialized,
            .project_session_present = false,
            .workspace_status = integration::WorkspaceStatus::invalid,
            .dirty = false,
            .working_copy_revision = 0U,
        };

        const auto memberships = session.assembly().memberships_for(
            assembled.resource.id.canonical());
        resource.memberships.reserve(memberships.size());
        for (const auto* membership : memberships) {
            resource.memberships.push_back(*membership);
        }

        const auto* workspace = session.project().find_session(
            assembled.resource.id);
        if (workspace != nullptr) {
            resource.project_session_present = true;
            resource.workspace_status = workspace->status();
            const auto* working = workspace->working_copy();
            resource.dirty = working != nullptr && working->dirty();
            resource.working_copy_revision = working == nullptr
                ? 0U
                : working->revision();
        }
        projection.resources.push_back(std::move(resource));
    }

    for (const auto& object : snapshot.domains().objects) {
        if (!visual_domain(object.kind)) {
            continue;
        }
        projection.visual_domains.push_back(StageSceneDomainProjection{
            .domain_object_id = object.id,
            .kind = object.kind,
            .resource_id = object.resource_id,
            .current_for_active_bytes = object.current_for_active_bytes,
        });
    }

    std::sort(
        projection.resources.begin(), projection.resources.end(),
        [](const StageSceneResourceProjection& left,
           const StageSceneResourceProjection& right) {
            return left.resource.id.canonical() < right.resource.id.canonical();
        });
    std::sort(
        projection.visual_domains.begin(), projection.visual_domains.end(),
        [](const StageSceneDomainProjection& left,
           const StageSceneDomainProjection& right) {
            return left.domain_object_id < right.domain_object_id;
        });

    return projection;
}

} // namespace dmc::rengine::modviz
