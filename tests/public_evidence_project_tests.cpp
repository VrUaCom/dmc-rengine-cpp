#include "dmc_rengine/evidence/json_import.hpp"
#include "dmc_rengine/integration/project_workspace.hpp"

#include <cassert>
#include <fstream>
#include <sstream>
#include <string>

int main(int argc, char** argv) {
    using dmc::rengine::integration::ProjectEdgeKind;
    using dmc::rengine::integration::ProjectNodeKind;
    using dmc::rengine::integration::ProjectWorkspace;

    assert(argc == 2);
    std::ifstream input(argv[1], std::ios::binary);
    assert(input.good());
    std::ostringstream buffer;
    buffer << input.rdbuf();

    const auto imported =
        dmc::rengine::evidence::evidence_packet_from_json(buffer.str());
    assert(imported.ok());
    assert(imported.packet.has_value());

    ProjectWorkspace project;
    assert(project.import_evidence_packet(*imported.packet));
    assert(project.packets().size() == 1U);
    assert(project.artifacts().size() == 1U);
    assert(project.evidence().size() == 7U);

    const auto* packet = project.packets().find(
        "dmc3-hdc-phase12-canonical-target");
    assert(packet != nullptr);
    assert(packet->artifact_ids.size() == 1U);
    assert(packet->record_ids.size() == 7U);

    const auto* artifact = project.artifacts().find(
        "dmc3-hdc-exe-e454272e");
    assert(artifact != nullptr);
    assert(artifact->sha256 ==
        "e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082");
    assert(artifact->role == "canonical-research-target");

    assert(project.evidence().find("ev-dmc3-pe-identity") != nullptr);
    assert(project.evidence().find("ev-dmc3-image-base") != nullptr);
    assert(project.evidence().find("ev-dmc3-entry-point") != nullptr);
    assert(project.evidence().find("ev-dmc3-main-export") != nullptr);
    assert(project.evidence().find("ev-dmc3-stage-resource-table") != nullptr);
    assert(project.evidence().find(
        "ev-dmc3-stageset-token-classifier") != nullptr);

    assert(project.graph().find(
        "evidence-packet:dmc3-hdc-phase12-canonical-target") != nullptr);
    assert(project.graph().find(
        "artifact:dmc3-hdc-exe-e454272e") != nullptr);
    assert(project.graph().nodes(ProjectNodeKind::evidence_packet).size() == 1U);
    assert(project.graph().nodes(ProjectNodeKind::artifact).size() == 1U);
    assert(project.graph().nodes(ProjectNodeKind::evidence_record).size() == 7U);

    std::size_t declared_records = 0U;
    std::size_t artifact_references = 0U;
    for (const auto& edge : project.graph().all_edges()) {
        if (edge.from ==
                "evidence-packet:dmc3-hdc-phase12-canonical-target" &&
            edge.kind == ProjectEdgeKind::declares &&
            edge.to.starts_with("evidence:")) {
            ++declared_records;
        }
        if (edge.kind == ProjectEdgeKind::references_artifact &&
            edge.to == "artifact:dmc3-hdc-exe-e454272e") {
            ++artifact_references;
        }
    }
    assert(declared_records == 7U);
    assert(artifact_references == 7U);

    assert(!project.import_evidence_packet(*imported.packet));
    return 0;
}
