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

void write_u16(std::vector<std::byte>& bytes, std::size_t offset, std::uint16_t value) {
    bytes[offset] = static_cast<std::byte>(value & 0xFFU);
    bytes[offset + 1U] = static_cast<std::byte>((value >> 8U) & 0xFFU);
}

void write_u32(std::vector<std::byte>& bytes, std::size_t offset, std::uint32_t value) {
    for (std::size_t index = 0; index < 4U; ++index) {
        bytes[offset + index] = static_cast<std::byte>(
            (value >> static_cast<unsigned>(index * 8U)) & 0xFFU);
    }
}

void write_i32(std::vector<std::byte>& bytes, std::size_t offset, std::int32_t value) {
    write_u32(bytes, offset, static_cast<std::uint32_t>(value));
}

void write_u64(std::vector<std::byte>& bytes, std::size_t offset, std::uint64_t value) {
    for (std::size_t index = 0; index < 8U; ++index) {
        bytes[offset + index] = static_cast<std::byte>(
            (value >> static_cast<unsigned>(index * 8U)) & 0xFFU);
    }
}

void write_f32(std::vector<std::byte>& bytes, std::size_t offset, float value) {
    write_u32(bytes, offset, std::bit_cast<std::uint32_t>(value));
}

void write_vec3(
    std::vector<std::byte>& bytes,
    std::size_t offset,
    float x,
    float y,
    float z) {
    write_f32(bytes, offset, x);
    write_f32(bytes, offset + 4U, y);
    write_f32(bytes, offset + 8U, z);
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
    constexpr std::size_t pointer_table_offset = 0x44U;
    constexpr std::size_t list_offset = 0x48U;
    constexpr std::size_t triangle_offset = 0x50U;
    constexpr std::size_t end_offset = triangle_offset + 0x38U;
    std::vector<std::byte> bytes(end_offset, std::byte{0});
    bytes[0] = std::byte{'H'};
    bytes[1] = std::byte{'I'};
    bytes[2] = std::byte{'T'};
    bytes[3] = std::byte{'S'};
    write_u32(bytes, 0x04U, static_cast<std::uint32_t>(end_offset));
    write_vec3(bytes, 0x08U, -1.0F, -1.0F, -1.0F);
    write_vec3(bytes, 0x14U, 1.0F, 1.0F, 1.0F);
    write_vec3(bytes, 0x20U, 2.0F, 2.0F, 2.0F);
    write_u32(bytes, 0x2CU, 1U);
    write_u32(bytes, 0x30U, 1U);
    write_u32(bytes, 0x34U, 1U);
    write_u32(bytes, 0x38U, 1U);
    write_u32(bytes, 0x3CU, 0x3CU);
    write_u32(bytes, 0x40U, static_cast<std::uint32_t>(triangle_offset - 8U));
    write_i32(bytes, pointer_table_offset,
              static_cast<std::int32_t>(list_offset - 8U));
    write_i32(bytes, list_offset, 0);
    write_i32(bytes, list_offset + 4U, -1);
    write_u32(bytes, triangle_offset, 0x18060001U);
    write_vec3(bytes, triangle_offset + 0x04U, 0.0F, 0.0F, 0.0F);
    write_vec3(bytes, triangle_offset + 0x10U, 1.0F, 0.0F, 0.0F);
    write_vec3(bytes, triangle_offset + 0x1CU, 0.0F, 0.0F, 1.0F);
    write_vec3(bytes, triangle_offset + 0x28U, 0.0F, 1.0F, 0.0F);
    write_f32(bytes, triangle_offset + 0x34U, 0.0F);
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
    std::string sha256) {
    using dmc::rengine::evidence::ArtifactIdentity;
    using dmc::rengine::evidence::Confidence;
    using dmc::rengine::evidence::EvidenceLocation;
    using dmc::rengine::evidence::EvidencePacket;
    using dmc::rengine::evidence::EvidenceRecord;
    return EvidencePacket{
        .schema_version = 1U,
        .id = "packet-matching-pe",
        .title = "Synthetic analyzer packet",
        .project = "DMC Rengine",
        .artifacts = {ArtifactIdentity{
            .id = "artifact-synthetic-pe",
            .role = "synthetic-executable",
            .sha256 = std::move(sha256),
            .size = 0x400U,
        }},
        .records = {EvidenceRecord{
            .id = "ev-synthetic-pe",
            .claim_id = "claim-synthetic-pe",
            .title = "Synthetic PE evidence",
            .summary = "Evidence scoped to one exact synthetic artifact SHA.",
            .confidence = Confidence::confirmed,
            .locations = {EvidenceLocation{
                .artifact_id = "artifact-synthetic-pe",
                .file_offset = 0U,
                .size = 0x200U,
                .rva = 0U,
                .va = std::nullopt,
                .symbol = "synthetic_pe",
                .note = "Synthetic fixture only.",
            }},
            .tags = {"pe", "synthetic-test"},
            .supersedes = {},
        }},
    };
}

} // namespace

int main() {
    using dmc::rengine::core::Sha256;
    using dmc::rengine::gdspaces::ResourcePayload;
    using dmc::rengine::integration::ProjectWorkspace;
    using dmc::rengine::integration::ResourceAnalyzer;
    using dmc::rengine::integration::WorkspaceContext;
    using dmc::rengine::integration::WorkspaceEventType;

    const auto pe_bytes = synthetic_pe();
    const auto pe_hash = Sha256::compute(std::span<const std::byte>{pe_bytes}).hex();
    const auto pe_ref = resource("dmc3-test.exe", "pe", 0U, pe_bytes.size());
    ProjectWorkspace project;
    assert(project.import_evidence_packet(packet_for(pe_hash)));
    assert(project.create_session(ResourcePayload{
        .resource = pe_ref,
        .bytes = pe_bytes,
        .diagnostics = {},
    }, WorkspaceContext{}));
    const auto pe_report = ResourceAnalyzer::analyze(project, pe_ref.id);
    assert(pe_report.ok());
    const auto* pe_session = project.find_session(pe_ref.id);
    assert(pe_session != nullptr);
    assert(pe_session->executable_context() != nullptr);
    assert(pe_session->executable_context()->sha256 == pe_hash);
    assert(pe_session->evidence_record_ids().size() == 1U);

    const auto hits_bytes = synthetic_hits();
    const auto hits_ref = resource(
        "room/st001_003.ukn", "hits", 4096U, hits_bytes.size());
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
    return 0;
}
