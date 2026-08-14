#include "dmc_rengine/bridges/dmc3/recovered_stage_links.hpp"

#include <algorithm>
#include <cassert>
#include <string>

int main() {
    using namespace dmc::rengine;

    stageops::StageDomainWorkspace domains;
    domains.identity = stageops::StageAssemblyIdentity{
        .stage = gdspaces::StageIdentity{
            .profile = "dmc3-hd",
            .stage_id = "dmc3-recovered-bridge-fixture",
            .display_name = "DMC3 recovered bridge fixture",
            .exe_evidence_id = "dmc3-door-parser-contract",
            .resource_set_id = "dmc3-recovered-bridge-fixture",
            .semantic_stage_id = {},
            .numeric_stage_id = 1U,
        },
        .catalog_entry_id = "dmc3-recovered-bridge-fixture",
        .global_catalog_row = 0U,
        .source_table_id = "fixture-stage-table",
        .source_row_index = 0U,
    };
    domains.source_stage_revision = 7U;

    const auto make_object = [](std::string id, stageops::StageDomainKind kind) {
        return stageops::StageDomainObject{
            .id = std::move(id),
            .kind = kind,
            .resource_id = "fixture-resource-id",
            .parser_id = "formats.stage-txt-lexer",
            .byte_source = integration::ParsedByteSource::immutable_source,
            .byte_revision = 0U,
            .current_for_active_bytes = true,
            .attributes = {},
        };
    };

    domains.objects.push_back(make_object(
        "stage-domain:door-token:fixture:offset/10",
        stageops::StageDomainKind::door_token));
    domains.objects.push_back(make_object(
        "stage-domain:box-in-token:fixture:offset/15",
        stageops::StageDomainKind::box_in_token));
    domains.objects.push_back(make_object(
        "stage-domain:stage-set-value-token:fixture:offset/5",
        stageops::StageDomainKind::stage_set_value_token));
    assert(domains.valid());

    const auto report =
        bridges::dmc3::RecoveredStageLinkProvider::build(domains);
    assert(report.valid());
    assert(report.door_marker_count == 1U);
    assert(report.box_in_marker_count == 1U);
    assert(report.links.size() == 2U);

    for (const auto& link : report.links) {
        assert(link.valid());
        assert(link.kind ==
            stageops::StageRuntimeLinkKind::parsed_by_runtime_consumer);
        assert(link.authority == stageops::StageRuntimeLinkAuthority::
            disassembly_complete_corpus_pending);
        assert(link.runtime.id == "dmc3/runtime/stage/door-txt-parser");
        assert(link.runtime.source_tree_path ==
            "recovered-game/runtime/stage/door_parser_contract.cpp");
        assert(link.runtime.symbol == "door_txt_parser@0x1401A9DE0");
        assert(link.runtime.executable_artifact_id ==
            "dmc3-hdc-exe-e454272e");
        assert(link.runtime.evidence_ids.size() == 4U);
        assert(std::find(
            link.runtime.evidence_ids.begin(),
            link.runtime.evidence_ids.end(),
            "ev-dmc3-door-parser-logical-function") !=
            link.runtime.evidence_ids.end());
    }

    const auto* door_link = &report.links[0];
    const auto* box_in_link = &report.links[1];
    if (door_link->domain_object_id.find("door-token") == std::string::npos) {
        std::swap(door_link, box_in_link);
    }
    assert(door_link->domain_object_id.find("door-token") != std::string::npos);
    assert(door_link->claim_id ==
        "claim-dmc3-door-parser-logical-function");
    assert(box_in_link->domain_object_id.find("box-in-token") !=
        std::string::npos);
    assert(box_in_link->claim_id == "claim-dmc3-door-box-grammar");

    // No lexical StageSet marker may inherit Door parser semantics merely by
    // coexisting in the same TXT resource/domain workspace.
    assert(std::none_of(
        report.links.begin(), report.links.end(),
        [](const stageops::StageRuntimeLink& link) {
            return link.domain_object_id.find("stage-set-value-token") !=
                std::string::npos;
        }));

    const auto workspace = stageops::StageRuntimeLinkBuilder::build(
        domains, report.links);
    assert(workspace.valid());
    assert(workspace.source_stage_revision == 7U);
    assert(workspace.links.size() == 2U);

    return 0;
}
