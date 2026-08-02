#include "dmc_rengine/evidence/packet.hpp"
#include "dmc_rengine/integration/project_workspace.hpp"

#include <cassert>
#include <string>

namespace {

[[nodiscard]] dmc::rengine::evidence::EvidencePacket make_packet(
    std::string packet_id,
    std::string artifact_hash,
    std::string record_summary = "Confirmed synthetic record.") {
    using dmc::rengine::evidence::ArtifactIdentity;
    using dmc::rengine::evidence::Confidence;
    using dmc::rengine::evidence::EvidenceLocation;
    using dmc::rengine::evidence::EvidencePacket;
    using dmc::rengine::evidence::EvidenceRecord;

    return EvidencePacket{
        .schema_version = 1U,
        .id = std::move(packet_id),
        .title = "Synthetic DMC Rengine Evidence Packet",
        .project = "DMC Rengine",
        .artifacts = {
            ArtifactIdentity{
                .id = "artifact-dmc3-test",
                .role = "synthetic-executable",
                .sha256 = std::move(artifact_hash),
                .size = 4096U,
            },
        },
        .records = {
            EvidenceRecord{
                .id = "ev-synthetic-stage-table",
                .claim_id = "claim-synthetic-stage-table",
                .title = "Synthetic stage table",
                .summary = std::move(record_summary),
                .confidence = Confidence::confirmed,
                .locations = {
                    EvidenceLocation{
                        .artifact_id = "artifact-dmc3-test",
                        .file_offset = 128U,
                        .size = 64U,
                        .rva = 256U,
                        .va = 0x140000100ULL,
                        .symbol = "stage_table",
                        .note = "Synthetic location only.",
                    },
                },
                .tags = {"stage", "synthetic-test"},
                .supersedes = {},
            },
        },
    };
}

} // namespace

int main() {
    using dmc::rengine::integration::ProjectEdgeKind;
    using dmc::rengine::integration::ProjectNodeKind;
    using dmc::rengine::integration::ProjectWorkspace;

    constexpr auto lower_hash =
        "0123456789abcdef0123456789abcdef0123456789abcdef0123456789abcdef";
    constexpr auto upper_hash =
        "0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF";
    constexpr auto conflicting_hash =
        "ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff";

    ProjectWorkspace project;
    const auto first = make_packet("packet-one", upper_hash);
    assert(project.import_evidence_packet(first));
    assert(project.artifacts().size() == 1U);
    assert(project.evidence().size() == 1U);
    assert(project.packets().size() == 1U);
    assert(project.artifacts().find("artifact-dmc3-test") != nullptr);
    assert(project.artifacts().find("artifact-dmc3-test")->sha256 == lower_hash);
    assert(project.artifacts().by_sha256(upper_hash).size() == 1U);
    assert(project.artifacts().by_role("synthetic-executable").size() == 1U);
    assert(project.packets().find("packet-one") != nullptr);
    assert(project.packets().find("packet-one")->artifact_ids.size() == 1U);
    assert(project.packets().find("packet-one")->record_ids.size() == 1U);

    assert(project.graph().find("evidence-packet:packet-one") != nullptr);
    assert(project.graph().find("artifact:artifact-dmc3-test") != nullptr);
    assert(project.graph().find("evidence:ev-synthetic-stage-table") != nullptr);
    assert(project.graph().nodes(ProjectNodeKind::evidence_packet).size() == 1U);
    assert(project.graph().nodes(ProjectNodeKind::artifact).size() == 1U);
    assert(project.graph().nodes(ProjectNodeKind::evidence_record).size() == 1U);

    bool packet_artifact_edge = false;
    bool packet_record_edge = false;
    bool record_artifact_edge = false;
    for (const auto& edge : project.graph().all_edges()) {
        if (edge.from == "evidence-packet:packet-one" &&
            edge.to == "artifact:artifact-dmc3-test" &&
            edge.kind == ProjectEdgeKind::declares) {
            packet_artifact_edge = true;
        }
        if (edge.from == "evidence-packet:packet-one" &&
            edge.to == "evidence:ev-synthetic-stage-table" &&
            edge.kind == ProjectEdgeKind::declares) {
            packet_record_edge = true;
        }
        if (edge.from == "evidence:ev-synthetic-stage-table" &&
            edge.to == "artifact:artifact-dmc3-test" &&
            edge.kind == ProjectEdgeKind::references_artifact) {
            record_artifact_edge = true;
        }
    }
    assert(packet_artifact_edge);
    assert(packet_record_edge);
    assert(record_artifact_edge);

    assert(!project.import_evidence_packet(first));

    const auto second = make_packet("packet-two", lower_hash);
    assert(project.import_evidence_packet(second));
    assert(project.artifacts().size() == 1U);
    assert(project.evidence().size() == 1U);
    assert(project.packets().size() == 2U);
    assert(project.graph().nodes(ProjectNodeKind::evidence_packet).size() == 2U);

    const auto artifact_conflict = make_packet(
        "packet-artifact-conflict", conflicting_hash);
    assert(!project.import_evidence_packet(artifact_conflict));
    assert(project.packets().size() == 2U);

    const auto record_conflict = make_packet(
        "packet-record-conflict", lower_hash,
        "Conflicting summary for the same EvidenceRecord ID.");
    assert(!project.import_evidence_packet(record_conflict));
    assert(project.packets().size() == 2U);

    return 0;
}
