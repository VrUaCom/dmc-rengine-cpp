#include "dmc_rengine/validation/stage_scene_receipt.hpp"

#include <algorithm>
#include <string>
#include <utility>

namespace dmc::rengine::validation {

bool StageSceneValidationReceipt::valid() const noexcept {
    return !receipt_id.empty() &&
        !profile.empty() &&
        !catalog_entry_id.empty() &&
        !source_table_id.empty() &&
        assembly_status != stageops::StageAssemblyStatus::invalid;
}

StageSceneValidationReceipt StageSceneValidationReceiptBuilder::build(
    std::string receipt_id,
    const stageops::StageOperationsSession& session,
    const stageops::StageSceneSnapshot& snapshot) {
    StageSceneValidationReceipt receipt;
    if (receipt_id.empty() || !session.valid() || !snapshot.valid() ||
        snapshot.stage_revision() != session.stage_revision()) {
        return receipt;
    }

    const auto& identity = session.assembly().identity;
    const auto operations = session.snapshot();
    const auto& domains = snapshot.domains();
    receipt.receipt_id = std::move(receipt_id);
    receipt.profile = identity.stage.profile;
    receipt.catalog_entry_id = identity.catalog_entry_id;
    receipt.source_table_id = identity.source_table_id;
    receipt.global_catalog_row = identity.global_catalog_row;
    receipt.source_row_index = identity.source_row_index;
    receipt.numeric_stage_id = identity.stage.numeric_stage_id;
    receipt.semantic_stage_id = identity.stage.semantic_stage_id;
    receipt.executable_evidence_id = identity.stage.exe_evidence_id;
    receipt.stage_revision = operations.stage_revision;
    receipt.assembly_status = operations.assembly_status;
    receipt.game_readiness = operations.game_readiness;
    receipt.derived_state_stale = operations.derived_state_stale;
    receipt.scene_current_for_active_bytes =
        snapshot.current_for_active_bytes();
    receipt.requirement_count = session.assembly().requirements.size();
    receipt.unresolved_requirement_count =
        session.assembly().unresolved_requirement_count();
    receipt.resource_count = session.assembly().resources.size();
    receipt.materialized_resource_count = static_cast<std::size_t>(
        std::count_if(
            session.assembly().resources.begin(),
            session.assembly().resources.end(),
            [](const stageops::StageAssemblyResource& resource) {
                return resource.materialized;
            }));
    receipt.dirty_resource_count = operations.dirty_resource_count;
    receipt.missing_project_session_count =
        operations.missing_project_session_count;
    receipt.domain_object_count = domains.objects.size();
    receipt.stale_typed_result_count = domains.stale_typed_result_count;

    for (const auto& object : domains.objects) {
        ++receipt.domain_counts[
            std::string{stageops::to_string(object.kind)}];
    }
    return receipt;
}

} // namespace dmc::rengine::validation
