#include "dmc_rengine/core/sha256.hpp"
#include "dmc_rengine/evidence/packet.hpp"
#include "dmc_rengine/integration/project_workspace.hpp"
#include "dmc_rengine/integration/resource_analyzer.hpp"

#include <bit>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <span>
#include <string>
#include <vector>

namespace {

void write_u16(
    std::vector<std::byte>& bytes,
    std::size_t offset,
    std::uint16_t value) {
    bytes[offset] = static_cast<std::byte>(value & 0xFFU);
    bytes[offset + 1U] = static_cast<std::byte>((value >> 8U) & 0xFFU);
}

void write_u32(
    std::vector<std::byte>& bytes,
    std::size_t offset,
    std::uint32_t value) {
    for (std::size_t index = 0; index < 4U; ++index) {
        bytes[offset + index] = static_cast<std::byte>(
            (value >> static_cast<unsigned>(index * 8U)) & 0xFFU);
    }
}

void write_u64(
    std::vector<std::byte>& bytes,
    std::size_t offset,
    std::uint64_t value) {
    for (std::size_t index = 0; index < 8U; ++index) {
        bytes[offset + index] = static_cast<std::byte>(
            (value >> static_cast<unsigned>(index * 8U)) & 0xFFU);
    }
}

[[nodiscard]] std::vector<std::byte> synthetic_pe() {
    std::vector<std::byte> bytes(0x400U, std::byte{0});
    bytes[0] = std::byte{'M'};
    bytes[1] = std::byte{'Z'};
    write_u32(bytes, 0x3CU, 0x80U);
    write_u32(bytes, 0x80U, 0x00004550U);
    write_u16(bytes, 0x84U, 0x8664U);
    write_u16(bytes, 0x86U, 1U);
    write_u16(bytes, 0x94U, 0x00F0U);

    constexpr std::size_t optional = 0x98U;
    write_u16(bytes, optional, 0x020BU);
    write_u32(bytes, optional + 16U, 0x1000U);
    write_u64(bytes, optional + 24U, 0x140000000ULL);
    write_u32(bytes, optional + 56U, 0x2000U);
    write_u32(bytes, optional + 60U, 0x0200U);
    write_u16(bytes, optional + 68U, 2U);

    constexpr std::size_t section = 0x188U;
    bytes[section] = std::byte{'.'};
    bytes[section + 1U] = std::byte{'t'};
    bytes[section + 2U] = std::byte{'e'};
    bytes[section + 3U] = std::byte{'x'};
    bytes[section + 4U] = std::byte{'t'};
    write_u32(bytes, section + 8U, 0x0100U);
    write_u32(bytes, section + 12U, 0x1000U);
    write_u32(bytes, section + 16U, 0x0200U);
    write_u32(bytes, section + 20U, 0x0200U);
    write_u32(bytes, section + 36U, 0x60000020U);
    return bytes;
}

[[nodiscard]] std::vector<std::byte> synthetic_hits() {
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
            std::bit_cast<std::uint32_t>(static_cast<float>(index)));
    }
    return bytes;
}

[[nodiscard]] dmc::rengine::gdspaces::ResourceRef resource(
    std::string path,
    std::string format,
    std::uint64_t offset,
    std::uint64_t size) {
    return dmc::rengine::gdspaces::ResourceRef{
        .id = dmc::rengine::gdspaces::ResourceId{
            .source_id = "resource-analyzer-test",
            .logical_path = std::move(path),
            .container_chain = {},
            .offset = offset,
            .size = size,
        },
        .display_name = "resource",
        .format = std::move(format),
        .profile = "dmc3-hd",
        .synthetic_name = false,
        .container = false,
    };
}

[[nodiscard]] dmc::rengine::evidence::EvidencePacket packet_for(
    std::string packet_id,
    std::string artifact_id,
    std::string sha256,
    std::string record_id,
    std::string claim_id) {
    using dmc::rengine::evidence::ArtifactIdentity;
    using dmc::rengine::evidence::Confidence;
    using dmc::rengine::evidence::EvidenceLocation;
    using dmc::rengine::evidence::EvidencePacket;
    using dmc::rengine::evidence::EvidenceRecord;

    return EvidencePacket{
        .schema_version = 1U,
        .id = std::move(packet_id),
        .title = "Synthetic analyzer packet",
        .project = "DMC Rengine",
        .artifacts = {
            ArtifactIdentity{
                .id = std::move(artifact_id),
                .role = "synthetic-executable",
                .sha256 = std::move(sha256),
                .size = 0x400U,
            },
        },
        .records = {
            EvidenceRecord{
                .id = std::move(record_id),
                .claim_id = std::move(claim_id),
                .title = "Synthetic PE evidence",
                .summary = "Evidence is scoped to one exact synthetic artifact SHA.",
                .confidence = Confidence::confirmed,
                .locations = {
                    EvidenceLocation{
                        .artifact_id = "artifact-synthetic-pe",
                        .file_offset = 0U,
                        .size = 0x200U,
                        .rva = 0U,
                        .va = std::nullopt,
                        .symbol = "synthetic_pe",
                        .note = "Synthetic fixture only.",
                    },
                },
                .tags = {"pe", "synthetic-test"},
                .supersedes = {},
            },
        },
    };
}

} // namespace

int main() {
    using dmc::rengine::core::Sha256;
    using dmc::rengine::evidence::ArtifactIdentity;
    using dmc::rengine::evidence::Confidence;
    using dmc::rengine::evidence::EvidenceLocation;
    using dmc::rengine::evidence::EvidencePacket;
    using dmc::rengine::evidence::EvidenceRecord;
    using dmc::rengine::gdspaces::ResourcePayload;
    using dmc::rengine::integration::ProjectNodeKind;
    using dmc::rengine::integration::ProjectWorkspace;
    using dmc::rengine::integration::ResourceAnalyzer;
    using dmc::rengine::integration::WorkspaceContext;
    using dmc::rengine::integration::WorkspaceEventType;

    const auto pe_bytes = synthetic_pe();
    const auto pe_hash = Sha256::compute(
        std::span<const std::byte>{pe_bytes}).hex();
    const auto pe_ref = resource("dmc3-test.exe", "pe", 0U, pe_bytes.size());

    ProjectWorkspace project;
    auto matching_packet = packet_for(
        "packet-matching-pe",
        "artifact-synthetic-pe",
        pe_hash,
        "ev-synthetic-pe",
        "claim-synthetic-pe");
    assert(project.import_evidence_packet(matching_packet));

    const EvidencePacket nonmatching_packet{
        .schema_version = 1U,
        .id = "packet-other-pe",
        .title = "Other executable packet",
        .project = "DMC Rengine",
        .artifacts = {
            ArtifactIdentity{
                .id = "artifact-other-pe",
                .role = "other-executable",
                .sha256 =
                    "ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff",
                .size = 2048U,
            },
        },
        .records = {
            EvidenceRecord{
                .id = "ev-other-pe",
                .claim_id = "claim-other-pe",
                .title = "Other PE evidence",
                .summary = "This record must not attach to the synthetic PE session.",
                .confidence = Confidence::confirmed,
                .locations = {
                    EvidenceLocation{
                        .artifact_id = "artifact-other-pe",
                        .file_offset = 0U,
                        .size = 16U,
                        .rva = std::nullopt,
                        .va = std::nullopt,
                        .symbol = {},
                        .note = "Nonmatching artifact.",
                    },
                },
                .tags = {"negative-test"},
                .supersedes = {},
            },
        },
    };
    assert(project.import_evidence_packet(nonmatching_packet));

    assert(project.create_session(ResourcePayload{
        .resource = pe_ref,
        .bytes = pe_bytes,
        .diagnostics = {},
    }, WorkspaceContext{}));

    const auto pe_report = ResourceAnalyzer::analyze(project, pe_ref.id);
    assert(pe_report.ok());
    assert(pe_report.parser_available);
    assert(pe_report.recognized);
    assert(!pe_report.binary_document_attached);

    const auto* pe_session = project.find_session(pe_ref.id);
    assert(pe_session != nullptr);
    assert(pe_session->executable_context() != nullptr);
    assert(pe_session->executable_context()->sha256 == pe_hash);
    assert(!pe_session->executable_context()->recognized_target());
    assert(pe_session->evidence_record_ids().size() == 1U);
    assert(pe_session->evidence_record_ids()[0] == "ev-synthetic-pe");
    assert(pe_session->events().by_type(
        WorkspaceEventType::parser_completed).size() == 1U);
    assert(pe_session->events().by_type(
        WorkspaceEventType::executable_context_attached).size() == 1U);
    assert(project.graph().find(
        "executable-analysis:" + pe_ref.id.canonical()) != nullptr);
    assert(project.graph().nodes(ProjectNodeKind::executable_analysis).size() == 1U);

    bool references_matching_artifact = false;
    bool references_other_artifact = false;
    for (const auto& edge : project.graph().all_edges()) {
        if (edge.from == "executable-analysis:" + pe_ref.id.canonical() &&
            edge.to == "artifact:artifact-synthetic-pe") {
            references_matching_artifact = true;
        }
        if (edge.from == "executable-analysis:" + pe_ref.id.canonical() &&
            edge.to == "artifact:artifact-other-pe") {
            references_other_artifact = true;
        }
    }
    assert(references_matching_artifact);
    assert(!references_other_artifact);

    const auto hits_bytes = synthetic_hits();
    const auto hits_ref = resource(
        "room/st001cfg_006.hits", "hits", 4096U, hits_bytes.size());
    assert(project.create_session(ResourcePayload{
        .resource = hits_ref,
        .bytes = hits_bytes,
        .diagnostics = {},
    }, WorkspaceContext{
        .stage_context = true,
        .menu_context = false,
        .evidence_context = true,
    }));

    const auto hits_report = ResourceAnalyzer::analyze(project, hits_ref.id);
    assert(hits_report.ok());
    assert(hits_report.parser_available);
    assert(hits_report.recognized);
    assert(hits_report.binary_document_attached);
    const auto* hits_session = project.find_session(hits_ref.id);
    assert(hits_session != nullptr);
    assert(hits_session->binary_document() != nullptr);
    assert(hits_session->executable_context() == nullptr);
    assert(hits_session->events().by_type(
        WorkspaceEventType::parser_completed).size() == 1U);

    const std::vector<std::byte> pac_bytes{
        std::byte{'P'}, std::byte{'A'}, std::byte{'C'}, std::byte{0}};
    const auto pac_ref = resource("room/st001cfg.pac", "pac", 8192U, pac_bytes.size());
    assert(project.create_session(ResourcePayload{
        .resource = pac_ref,
        .bytes = pac_bytes,
        .diagnostics = {},
    }, WorkspaceContext{
        .stage_context = true,
        .menu_context = false,
        .evidence_context = true,
    }));
    const auto pac_report = ResourceAnalyzer::analyze(project, pac_ref.id);
    assert(!pac_report.ok());
    assert(!pac_report.parser_available);
    assert(!pac_report.recognized);
    assert(!pac_report.diagnostics.empty());
    return 0;
}
