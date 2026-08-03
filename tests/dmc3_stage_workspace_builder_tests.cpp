#include "dmc_rengine/integration/stage_workspace_manifest.hpp"
#include "dmc_rengine/profiles/dmc3/stage_workspace_builder.hpp"

#include "hits_test_fixture.hpp"

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

namespace {

[[nodiscard]] dmc::rengine::gdspaces::ResourcePayload payload(
    std::string path,
    std::string format,
    std::uint64_t offset,
    std::vector<std::byte> bytes) {
    return dmc::rengine::gdspaces::ResourcePayload{
        .resource = dmc::rengine::gdspaces::ResourceRef{
            .id = dmc::rengine::gdspaces::ResourceId{
                .source_id = "dmc3-stage-builder-test",
                .logical_path = std::move(path),
                .container_chain = "NBZ[0]",
                .offset = offset,
                .size = bytes.size(),
            },
            .display_name = "resource",
            .format = std::move(format),
            .profile = "dmc3-hd",
            .synthetic_name = false,
            .container = false,
        },
        .bytes = std::move(bytes),
        .diagnostics = {},
    };
}

[[nodiscard]] dmc::rengine::evidence::EvidencePacket stage_packet() {
    using dmc::rengine::evidence::ArtifactIdentity;
    using dmc::rengine::evidence::Confidence;
    using dmc::rengine::evidence::EvidenceLocation;
    using dmc::rengine::evidence::EvidencePacket;
    using dmc::rengine::evidence::EvidenceRecord;

    return EvidencePacket{
        .schema_version = 1U,
        .id = "packet-dmc3-stage-builder-test",
        .title = "Synthetic DMC3 stage builder packet",
        .project = "DMC Rengine",
        .artifacts = {
            ArtifactIdentity{
                .id = "artifact-stage-builder-exe",
                .role = "synthetic-executable",
                .sha256 =
                    "0123456789abcdef0123456789abcdef0123456789abcdef0123456789abcdef",
                .size = 0x10000U,
            },
        },
        .records = {
            EvidenceRecord{
                .id = "ev-dmc3-stage-resource-table",
                .claim_id = "claim-dmc3-stage-resource-table",
                .title = "DMC3 stage resource table",
                .summary = "Synthetic stage table evidence for integration testing.",
                .confidence = Confidence::confirmed,
                .locations = {
                    EvidenceLocation{
                        .artifact_id = "artifact-stage-builder-exe",
                        .file_offset = 0x100U,
                        .size = 16U,
                        .rva = std::nullopt,
                        .va = std::nullopt,
                        .symbol = "stage_resource_table",
                        .note = "Synthetic fixture.",
                    },
                },
                .tags = {"stage", "table"},
                .supersedes = {},
            },
            EvidenceRecord{
                .id = "ev-dmc3-stageset-token-classifier",
                .claim_id = "claim-dmc3-stageset-token-classifier",
                .title = "StageSet token classifier",
                .summary = "Synthetic StageSet classifier evidence.",
                .confidence = Confidence::confirmed,
                .locations = {
                    EvidenceLocation{
                        .artifact_id = "artifact-stage-builder-exe",
                        .file_offset = 0x200U,
                        .size = 16U,
                        .rva = std::nullopt,
                        .va = std::nullopt,
                        .symbol = "stageset_token_classifier",
                        .note = "Synthetic fixture.",
                    },
                },
                .tags = {"txt", "stage"},
                .supersedes = {},
            },
        },
    };
}

[[nodiscard]] std::vector<dmc::rengine::gdspaces::ResourcePayload>
complete_payloads() {
    return {
        payload("scr/st001.pac", "pac", 0x1000U,
                {std::byte{'P'}, std::byte{'A'}, std::byte{'C'}, std::byte{0}}),
        payload("room/st001cfg.pac", "pac", 0x2000U,
                {std::byte{'P'}, std::byte{'A'}, std::byte{'C'}, std::byte{0}}),
        payload("room/st001_effect.pac", "pac", 0x3000U,
                {std::byte{'P'}, std::byte{'A'}, std::byte{'C'}, std::byte{0}}),
        payload("se/snd_r001.pac", "pac", 0x4000U,
                {std::byte{'P'}, std::byte{'A'}, std::byte{'C'}, std::byte{0}}),
        payload(
            "room/st001cfg_006.hits",
            "hits",
            0x5000U,
            dmc::rengine::tests::hits_fixture::make_minimal_hits()),
        payload("room/st001cfg_004.txt", "txt", 0x6000U,
                {std::byte{'#'}, std::byte{'S'}, std::byte{'E'},
                 std::byte{'T'}, std::byte{' '}, std::byte{'S'},
                 std::byte{'T'}, std::byte{'A'}, std::byte{'Y'}}),
    };
}

} // namespace

int main() {
    using dmc::rengine::gdspaces::StageResourceCategory;
    using dmc::rengine::integration::ProjectNodeKind;
    using dmc::rengine::integration::stage_workspace_manifest_json;
    using dmc::rengine::profiles::dmc3::StageWorkspaceBuilder;
    using dmc::rengine::profiles::dmc3::phase12_st001_resource_plan;

    const auto packet = stage_packet();
    auto result = StageWorkspaceBuilder::build(
        phase12_st001_resource_plan(), complete_payloads(), &packet);
    assert(result.complete());
    assert(result.match.complete());
    assert(result.match.roles.size() == 4U);
    assert(result.stage.has_value());
    assert(result.stage->size() == 6U);
    assert(result.project.session_count() == 6U);
    assert(result.project.sessions_for_stage("st001").size() == 6U);
    assert(result.project.graph().nodes(ProjectNodeKind::stage).size() == 1U);
    assert(result.project.graph().nodes(ProjectNodeKind::resource).size() == 6U);

    const auto collision = result.stage->members(
        StageResourceCategory::collision);
    assert(collision.size() == 1U);
    const auto* hits_session = result.project.find_session(
        collision.front()->resource.id);
    assert(hits_session != nullptr);
    assert(hits_session->binary_document() != nullptr);
    assert(hits_session->stage() != nullptr);

    const auto scripts = result.stage->members(StageResourceCategory::scripts);
    const auto txt = std::find_if(
        scripts.begin(), scripts.end(),
        [](const auto* member) {
            return member->resource.format == "txt";
        });
    assert(txt != scripts.end());
    const auto* txt_session = result.project.find_session((*txt)->resource.id);
    assert(txt_session != nullptr);
    assert(std::find(
        txt_session->evidence_record_ids().begin(),
        txt_session->evidence_record_ids().end(),
        "ev-dmc3-stageset-token-classifier") !=
        txt_session->evidence_record_ids().end());

    for (const auto& role : result.match.roles) {
        assert(role.matches.size() == 1U);
        const auto* session = result.project.find_session(
            role.matches.front().id);
        assert(session != nullptr);
        assert(std::find(
            session->evidence_record_ids().begin(),
            session->evidence_record_ids().end(),
            "ev-dmc3-stage-resource-table") !=
            session->evidence_record_ids().end());
    }

    assert(!stage_workspace_manifest_json(result.project, "st001").empty());

    auto incomplete = complete_payloads();
    incomplete.erase(incomplete.begin() + 3);
    auto incomplete_result = StageWorkspaceBuilder::build(
        phase12_st001_resource_plan(), std::move(incomplete), &packet);
    assert(!incomplete_result.complete());
    assert(std::any_of(
        incomplete_result.diagnostics.begin(),
        incomplete_result.diagnostics.end(),
        [](const auto& diagnostic) {
            return diagnostic.code == "dmc3.stage.resource_missing";
        }));
    return 0;
}
