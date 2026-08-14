#pragma once

#include "dmc_rengine/stageops/operations_session.hpp"
#include "dmc_rengine/stageops/scene_snapshot.hpp"

#include <cstddef>
#include <cstdint>
#include <map>
#include <optional>
#include <string>

namespace dmc::rengine::validation {

// Proprietary-byte-free validation summary for one Stage Ops scene revision.
// It records structural/operational proof metadata only; no resource bytes are
// embedded and no local absolute filesystem path is required.
struct StageSceneValidationReceipt final {
    std::string receipt_id;
    std::string profile;
    std::string catalog_entry_id;
    std::string source_table_id;
    std::uint32_t global_catalog_row{};
    std::uint32_t source_row_index{};
    std::optional<std::uint16_t> numeric_stage_id;
    std::string semantic_stage_id;
    std::string executable_evidence_id;

    std::uint64_t stage_revision{};
    stageops::StageAssemblyStatus assembly_status{
        stageops::StageAssemblyStatus::invalid};
    stageops::StageGameReadiness game_readiness{
        stageops::StageGameReadiness::unproven};
    bool derived_state_stale{false};
    bool scene_current_for_active_bytes{false};

    std::size_t requirement_count{};
    std::size_t unresolved_requirement_count{};
    std::size_t resource_count{};
    std::size_t materialized_resource_count{};
    std::size_t dirty_resource_count{};
    std::size_t missing_project_session_count{};
    std::size_t domain_object_count{};
    std::size_t stale_typed_result_count{};
    std::map<std::string, std::size_t, std::less<>> domain_counts;

    [[nodiscard]] bool valid() const noexcept;
};

class StageSceneValidationReceiptBuilder final {
public:
    [[nodiscard]] static StageSceneValidationReceipt build(
        std::string receipt_id,
        const stageops::StageOperationsSession& session,
        const stageops::StageSceneSnapshot& snapshot);
};

} // namespace dmc::rengine::validation
