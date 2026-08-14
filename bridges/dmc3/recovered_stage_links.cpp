#include "dmc_rengine/bridges/dmc3/recovered_stage_links.hpp"

#include "runtime/stage/door_parser_contract.hpp"

#include <string>
#include <string_view>
#include <utility>

namespace dmc::rengine::bridges::dmc3 {
namespace {

[[nodiscard]] std::vector<std::string> evidence_ids() {
    const auto& contract =
        dmc::recovered::dmc3::runtime::stage::door_parser_contract();
    std::vector<std::string> result;
    result.reserve(contract.evidence_ids.size());
    for (const auto id : contract.evidence_ids) {
        result.emplace_back(id);
    }
    return result;
}

[[nodiscard]] stageops::RecoveredRuntimeIdentity runtime_identity() {
    return stageops::RecoveredRuntimeIdentity{
        .id = "dmc3/runtime/stage/door-txt-parser",
        .source_tree_path =
            "recovered-game/runtime/stage/door_parser_contract.cpp",
        .symbol = "door_txt_parser@0x1401A9DE0",
        .executable_artifact_id = "dmc3-hdc-exe-e454272e",
        .evidence_ids = evidence_ids(),
    };
}

void append_links(
    RecoveredStageLinkReport& report,
    const stageops::StageDomainWorkspace& domains,
    stageops::StageDomainKind kind,
    std::string_view role,
    std::string_view claim_id,
    std::size_t& marker_count) {
    const auto markers = domains.by_kind(kind);
    marker_count = markers.size();
    for (const auto* marker : markers) {
        report.links.push_back(stageops::StageRuntimeLink{
            .id = "dmc3/door-parser/" + std::string{role} + ":" + marker->id,
            .domain_object_id = marker->id,
            .runtime = runtime_identity(),
            .kind = stageops::StageRuntimeLinkKind::parsed_by_runtime_consumer,
            .authority = stageops::StageRuntimeLinkAuthority::
                disassembly_complete_corpus_pending,
            .claim_id = std::string{claim_id},
        });
    }
}

} // namespace

RecoveredStageLinkReport RecoveredStageLinkProvider::build(
    const stageops::StageDomainWorkspace& domains) {
    RecoveredStageLinkReport report;
    if (!domains.valid()) {
        return report;
    }

    append_links(
        report,
        domains,
        stageops::StageDomainKind::door_token,
        "door-token",
        "claim-dmc3-door-parser-logical-function",
        report.door_marker_count);
    append_links(
        report,
        domains,
        stageops::StageDomainKind::box_in_token,
        "box-in-token",
        "claim-dmc3-door-box-grammar",
        report.box_in_marker_count);

    return report;
}

stageops::StageSceneRefreshResult RecoveredStageSceneController::refresh(
    stageops::StageOperationsSession& session) {
    return stageops::StageSceneController::refresh_with_provider(
        session,
        [](const stageops::StageDomainWorkspace& domains) {
            auto report = RecoveredStageLinkProvider::build(domains);
            return std::move(report.links);
        });
}

} // namespace dmc::rengine::bridges::dmc3
