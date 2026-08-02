#include "dmc_rengine/evidence/packet_range_validation.hpp"
#include "dmc_rengine/integration/project_workspace.hpp"

#include <cassert>
#include <cstdint>
#include <string>

namespace {

[[nodiscard]] dmc::rengine::evidence::EvidencePacket packet_with_range(
    std::uint64_t offset,
    std::uint64_t size) {
    using dmc::rengine::evidence::ArtifactIdentity;
    using dmc::rengine::evidence::Confidence;
    using dmc::rengine::evidence::EvidenceLocation;
    using dmc::rengine::evidence::EvidencePacket;
    using dmc::rengine::evidence::EvidenceRecord;

    return EvidencePacket{
        .schema_version = 1U,
        .id = "packet-range-" + std::to_string(offset) + '-' +
            std::to_string(size),
        .title = "Synthetic range packet",
        .project = "DMC Rengine",
        .artifacts = {
            ArtifactIdentity{
                .id = "artifact-range-test",
                .role = "synthetic",
                .sha256 =
                    "0123456789abcdef0123456789abcdef0123456789abcdef0123456789abcdef",
                .size = 100U,
            },
        },
        .records = {
            EvidenceRecord{
                .id = "ev-range-" + std::to_string(offset) + '-' +
                    std::to_string(size),
                .claim_id = "claim-range-test",
                .title = "Synthetic physical range",
                .summary = "Tests artifact-relative physical file range validation.",
                .confidence = Confidence::confirmed,
                .locations = {
                    EvidenceLocation{
                        .artifact_id = "artifact-range-test",
                        .file_offset = offset,
                        .size = size,
                        .rva = std::nullopt,
                        .va = std::nullopt,
                        .symbol = "range",
                        .note = "Synthetic fixture.",
                    },
                },
                .tags = {"range-test"},
                .supersedes = {},
            },
        },
    };
}

} // namespace

int main() {
    using dmc::rengine::evidence::validate_packet_artifact_ranges;
    using dmc::rengine::integration::ProjectWorkspace;

    const auto valid = packet_with_range(90U, 10U);
    assert(validate_packet_artifact_ranges(valid).ok());
    ProjectWorkspace valid_project;
    assert(valid_project.import_evidence_packet(valid));
    assert(valid_project.artifacts().size() == 1U);
    assert(valid_project.evidence().size() == 1U);

    const auto outside = packet_with_range(100U, 1U);
    const auto outside_validation = validate_packet_artifact_ranges(outside);
    assert(!outside_validation.ok());
    assert(outside_validation.issues.size() == 1U);
    assert(outside_validation.issues[0].code ==
        "evidence-range.file-offset-outside-artifact");
    ProjectWorkspace outside_project;
    assert(!outside_project.import_evidence_packet(outside));
    assert(outside_project.artifacts().empty());
    assert(outside_project.evidence().empty());
    assert(outside_project.packets().empty());

    const auto crossing = packet_with_range(95U, 10U);
    const auto crossing_validation = validate_packet_artifact_ranges(crossing);
    assert(!crossing_validation.ok());
    assert(crossing_validation.issues[0].code ==
        "evidence-range.file-range-outside-artifact");

    const auto zero = packet_with_range(10U, 0U);
    const auto zero_validation = validate_packet_artifact_ranges(zero);
    assert(!zero_validation.ok());
    assert(zero_validation.issues[0].code ==
        "evidence-range.zero-size");

    return 0;
}
