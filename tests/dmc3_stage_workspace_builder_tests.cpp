#include "dmc_rengine/evidence/packet.hpp"
#include "dmc_rengine/integration/stage_workspace_manifest.hpp"
#include "dmc_rengine/profiles/dmc3/stage_workspace_builder.hpp"

#include <bit>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <span>
#include <string>
#include <vector>

namespace {

void write_u32(
    std::vector<std::byte>& bytes,
    std::size_t offset,
    std::uint32_t value) {
    for (std::size_t index = 0; index < 4U; ++index) {
        bytes[offset + index] = static_cast<std::byte>(
            (value >> static_cast<unsigned>(index * 8U)) & 0xFFU);
    }
}

[[nodiscard]] std::vector<std::byte> hits_bytes() {
    std::vector<std::byte> bytes(64U, std::byte{0});
    bytes[0] = std::byte{'H'};
    bytes[1] = std::byte{'I'};
    bytes[2] = std::byte{'T'};
    bytes[3] = std::byte{'S'};
    bytes[4] = std::byte{'$'};
    write_u32(bytes, 8U, dmc::rengine::formats::hits::record_marker);
    for (std::size_t index = 0;
         index < dmc::rengine::formats::hits::value_count;
         ++index) {
        write_u32(
            bytes,
            12U + index * 4U,
            std::bit_cast<std::uint32_t>(static_cast<float>(index + 10U)));
    }
    return bytes;
}

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

    constexpr auto hash =
        "0123456789abcdef0123456789abcdef0123456789abcdef0123456789abcdef";
    return EvidencePacket{
        .schema_version = 1U,
        .id = "packet-dmc3-stage-builder-test",
        .title = "Synthetic DMC3 stage builder packet",
        .project = "DMC Rengine",
        .artifacts = {
            ArtifactIdentity{
                .id = "artifact-stage-builder-exe",
                .role = "synthetic-executable",
                .sha256 = hash,
                .size = 4096U,
            },
        },
        .records = {
            EvidenceRecord{
                .id = "ev-dmc3-stage-resource-table",
                .claim_id = "claim-dmc3-stage-resource-table",
                .title = "DMC3 stage resource table",
                .summary = "Synthetic evidence using the canonical record ID.",
                .confidence = Confidence::confirmed,
                .locations = {
                    EvidenceLocation{
                        .artifact_id = "artifact-stage-builder-exe",
                        .file_offset = 0x5C30A8U,
                        .size = 440U,
                        .rva = 0x5C4AA8U,
                        .va = 0x1405C4AA8ULL,
                        .symbol = "stage_resource_table",
                        .note = "Synthetic packet for integration testing.",
                    },
                },
                .tags = {"stage", "table"},
                .supersedes = {},
            },
            EvidenceRecord{
                .id = "ev-dmc3-stageset-token-classifier",
                .claim_id = "claim-dmc3-stageset-token-classifier",
                .title = "StageSet token classifier",
                .summary = "Synthetic evidence using the canonical TXT claim ID.",
                .confidence = Confidence::confirmed,
                .locations = {
                    EvidenceLocation{
                        .artifact_id = "artifact-stage-builder-exe",
                        .file_offset = 0x246680U,
                        .size = 64U,
                        .rva = 0x246680U,
                        .va = 0x140246680ULL,
                        .symbol = "stageset_token_classifier",
                        .note = "Synthetic packet for integration testing.",
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
        payload("room/st001cfg_006.hits", "hits", 0x5000U, hits_bytes()),
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
    using dmc::rengine::profiles::dmc3::st001_stage_plan;

    const auto packet = stage_packet();
    auto result = StageWorkspaceBuilder::build(
        st001_stage_plan(), complete_payloads(), &packet);
    assert(result.complete());
    assert(result.match.complete());
    assert(result.stage.has_value());
    assert(result.stage->size() == 6U);
    assert(result.project.session_count() == 6U);
    assert(result.project.sessions_for_stage("st001").size() == 6U);
    assert(result.project.packets().size() == 1U);
    assert(result.project.evidence().size() == 2U);
    assert(result.project.graph().nodes(ProjectNodeKind::stage).size() == 1U);
    assert(result.project.graph().nodes(ProjectNodeKind::resource).size() == 6U);

    const auto collision = result.stage->members(
        StageResourceCategory::collision);
    assert(collision.size() == 1U);
    assert(collision[0]->resource.format == "hits");
    const auto scripts = result.stage->members(
        StageResourceCategory::scripts);
    assert(scripts.size() == 1U);
    assert(scripts[0]->resource.format == "txt");
    const auto unknown = result.stage->members(
        StageResourceCategory::unknown);
    assert(unknown.size() == 4U);

    const auto* hits_session = result.project.find_session(
        collision[0]->resource.id);
    assert(hits_session != nullptr);
    assert(hits_session->binary_document() != nullptr);
    assert(hits_session->stage() != nullptr);

    const auto* txt_session = result.project.find_session(
        scripts[0]->resource.id);
    assert(txt_session != nullptr);
    assert(txt_session->evidence_record_ids().size() == 1U);
    assert(txt_session->evidence_record_ids()[0] ==
        "ev-dmc3-stageset-token-classifier");

    for (const auto& match : result.match.matches) {
        assert(match.status ==
            dmc::rengine::profiles::dmc3::StageResourceMatchStatus::unique);
        assert(match.candidates.size() == 1U);
        const auto* session = result.project.find_session(
            match.candidates.front().id);
        assert(session != nullptr);
        assert(std::find(
            session->evidence_record_ids().begin(),
            session->evidence_record_ids().end(),
            "ev-dmc3-stage-resource-table") !=
            session->evidence_record_ids().end());
    }

    const auto stage_json = stage_workspace_manifest_json(
        result.project, "st001");
    assert(!stage_json.empty());

    auto incomplete = complete_payloads();
    incomplete.erase(incomplete.begin() + 3);
    auto incomplete_result = StageWorkspaceBuilder::build(
        st001_stage_plan(), std::move(incomplete), &packet);
    assert(!incomplete_result.complete());
    assert(!incomplete_result.match.complete());
    assert(std::any_of(
        incomplete_result.diagnostics.begin(),
        incomplete_result.diagnostics.end(),
        [](const auto& diagnostic) {
            return diagnostic.code == "dmc3-stage-resource-missing";
        }));
    return 0;
}
