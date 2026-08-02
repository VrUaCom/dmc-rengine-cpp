#include "dmc_rengine/evidence/json.hpp"
#include "dmc_rengine/evidence/json_import.hpp"

#include <cassert>
#include <string>

namespace {

[[nodiscard]] dmc::rengine::evidence::EvidencePacket make_packet() {
    return dmc::rengine::evidence::EvidencePacket{
        .schema_version = 1,
        .id = "packet-test",
        .title = "Evidence import test",
        .project = "DMC Rengine",
        .artifacts = {dmc::rengine::evidence::ArtifactIdentity{
            .id = "artifact-test",
            .role = "synthetic-fixture",
            .sha256 =
                "ba7816bf8f01cfea414140de5dae2223"
                "b00361a396177a9cb410ff61f20015ad",
            .size = 3,
        }},
        .records = {dmc::rengine::evidence::EvidenceRecord{
            .id = "ev-test",
            .claim_id = "claim-test",
            .title = "Synthetic claim",
            .summary = "Synthetic summary.",
            .confidence = dmc::rengine::evidence::Confidence::confirmed,
            .locations = {dmc::rengine::evidence::EvidenceLocation{
                .artifact_id = "artifact-test",
                .file_offset = 0,
                .size = 3,
                .rva = std::nullopt,
                .va = std::nullopt,
                .symbol = "fixture",
                .note = "No proprietary bytes.",
            }},
            .tags = {"synthetic", "test"},
            .supersedes = {"ev-old-test"},
        }},
    };
}

} // namespace

int main() {
    using dmc::rengine::evidence::EvidenceImportLimits;
    using dmc::rengine::evidence::evidence_packet_from_json;
    using dmc::rengine::evidence::to_json;

    const auto source = make_packet();
    assert(source.valid());
    const auto json = to_json(source);
    assert(!json.empty());

    const auto imported = evidence_packet_from_json(json);
    assert(imported.ok());
    assert(imported.packet->id == source.id);
    assert(imported.packet->artifacts == source.artifacts);
    assert(imported.packet->records == source.records);
    assert(to_json(*imported.packet) == json);

    const auto unknown_field = evidence_packet_from_json(R"({
        "schema_version":1,
        "id":"p",
        "title":"t",
        "project":"DMC Rengine",
        "artifacts":[],
        "records":[],
        "extra":true
    })");
    assert(!unknown_field.ok());
    assert(!unknown_field.diagnostics.empty());
    assert(unknown_field.diagnostics[0].path == "$.extra");

    const auto missing_artifact = evidence_packet_from_json(R"({
        "schema_version":1,
        "id":"p",
        "title":"t",
        "project":"DMC Rengine",
        "artifacts":[],
        "records":[{
            "id":"e",
            "claim_id":"c",
            "title":"t",
            "summary":"s",
            "confidence":"confirmed",
            "tags":[],
            "supersedes":[],
            "locations":[{"artifact_id":"missing"}]
        }]
    })");
    assert(!missing_artifact.ok());

    const auto invalid_confidence = evidence_packet_from_json(R"({
        "schema_version":1,
        "id":"p",
        "title":"t",
        "project":"DMC Rengine",
        "artifacts":[],
        "records":[{
            "id":"e",
            "claim_id":"c",
            "title":"t",
            "summary":"s",
            "confidence":"certain",
            "tags":[],
            "supersedes":[],
            "locations":[]
        }]
    })");
    assert(!invalid_confidence.ok());

    const auto duplicate_json_key = evidence_packet_from_json(
        R"({"schema_version":1,"schema_version":1})");
    assert(!duplicate_json_key.ok());
    assert(duplicate_json_key.diagnostics[0].path.starts_with("$@"));

    EvidenceImportLimits limited;
    limited.max_artifacts = 0U;
    const auto limited_result = evidence_packet_from_json(json, limited);
    assert(!limited_result.ok());

    const auto uppercase_hash = evidence_packet_from_json(R"({
        "schema_version":1,
        "id":"p",
        "title":"t",
        "project":"DMC Rengine",
        "artifacts":[{
            "id":"a",
            "role":"test",
            "sha256":"BA7816BF8F01CFEA414140DE5DAE2223B00361A396177A9CB410FF61F20015AD",
            "size":3
        }],
        "records":[]
    })");
    assert(uppercase_hash.ok());
    assert(uppercase_hash.packet->artifacts[0].sha256 ==
        "ba7816bf8f01cfea414140de5dae2223"
        "b00361a396177a9cb410ff61f20015ad");

    return 0;
}
