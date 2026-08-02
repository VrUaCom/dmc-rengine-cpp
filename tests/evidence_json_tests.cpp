#include "dmc_rengine/evidence/json.hpp"

#include <cassert>
#include <string>

int main() {
    dmc::rengine::evidence::EvidencePacket packet{
        .schema_version = 1,
        .id = "packet-synthetic-pe",
        .title = "Synthetic PE evidence",
        .project = "DMC Rengine",
        .artifacts = {dmc::rengine::evidence::ArtifactIdentity{
            .id = "synthetic-pe",
            .role = "test-fixture",
            .sha256 =
                "00000000000000000000000000000000"
                "00000000000000000000000000000000",
            .size = 1024,
        }},
        .records = {dmc::rengine::evidence::EvidenceRecord{
            .id = "ev-synthetic-entry",
            .claim_id = "claim-synthetic-entry",
            .title = "Synthetic entry point",
            .summary = "The fixture uses an entry point RVA of 0x1000.\nSynthetic only.",
            .confidence = dmc::rengine::evidence::Confidence::confirmed,
            .locations = {dmc::rengine::evidence::EvidenceLocation{
                .artifact_id = "synthetic-pe",
                .file_offset = 128,
                .size = 4,
                .rva = 4096,
                .va = 5368713216ULL,
                .symbol = "entry\"point",
                .note = "Generated fixture",
            }},
            .tags = {"pe", "synthetic"},
            .supersedes = {},
        }},
    };

    assert(packet.valid());
    const auto json = dmc::rengine::evidence::to_json(packet);
    assert(!json.empty());
    assert(json.find("\"schema_version\": 1") != std::string::npos);
    assert(json.find("\"confidence\": \"confirmed\"") != std::string::npos);
    assert(json.find("entry\\\"point") != std::string::npos);
    assert(json.find("Synthetic only.\\n") != std::string::npos);

    packet.records[0].locations[0].artifact_id = "missing-artifact";
    assert(!packet.valid());
    assert(dmc::rengine::evidence::to_json(packet).empty());

    return 0;
}
