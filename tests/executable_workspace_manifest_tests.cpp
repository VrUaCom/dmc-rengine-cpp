#include "dmc_rengine/core/json.hpp"
#include "dmc_rengine/core/sha256.hpp"
#include "dmc_rengine/evidence/packet.hpp"
#include "dmc_rengine/integration/executable_workspace_manifest.hpp"
#include "dmc_rengine/integration/project_workspace.hpp"
#include "dmc_rengine/integration/resource_analyzer.hpp"

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <span>
#include <string_view>
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

void write_u64(std::vector<std::byte>& bytes, std::size_t offset, std::uint64_t value) {
    for (std::size_t index = 0; index < 8U; ++index) {
        bytes[offset + index] = static_cast<std::byte>(
            (value >> static_cast<unsigned>(index * 8U)) & 0xFFU);
    }
}

[[nodiscard]] std::vector<std::byte> make_pe() {
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

[[nodiscard]] const dmc::rengine::core::json::Value* member(
    const dmc::rengine::core::json::Value::Object& object,
    std::string_view name) {
    const auto iterator = object.find(name);
    return iterator == object.end() ? nullptr : &iterator->second;
}

} // namespace

int main() {
    using dmc::rengine::core::Sha256;
    using dmc::rengine::core::json::Parser;
    using dmc::rengine::evidence::ArtifactIdentity;
    using dmc::rengine::evidence::Confidence;
    using dmc::rengine::evidence::EvidenceLocation;
    using dmc::rengine::evidence::EvidencePacket;
    using dmc::rengine::evidence::EvidenceRecord;
    using dmc::rengine::gdspaces::ResourceId;
    using dmc::rengine::gdspaces::ResourcePayload;
    using dmc::rengine::gdspaces::ResourceRef;
    using dmc::rengine::gdspaces::ToolTarget;
    using dmc::rengine::integration::ProjectWorkspace;
    using dmc::rengine::integration::ResourceAnalyzer;
    using dmc::rengine::integration::executable_workspace_manifest_json;

    const auto bytes = make_pe();
    const auto hash = Sha256::compute(std::span<const std::byte>{bytes}).hex();
    const ResourceRef resource{
        .id = ResourceId{
            .source_id = "exe-manifest-test",
            .logical_path = "synthetic.exe",
            .container_chain = {},
            .offset = 0U,
            .size = bytes.size(),
        },
        .display_name = "synthetic.exe",
        .format = "pe",
        .profile = "dmc3-hd",
        .synthetic_name = false,
        .container = false,
    };

    const EvidencePacket packet{
        .schema_version = 1U,
        .id = "packet-synthetic-exe-manifest",
        .title = "Synthetic executable manifest packet",
        .project = "DMC Rengine",
        .artifacts = {
            ArtifactIdentity{
                .id = "artifact-synthetic-exe-manifest",
                .role = "synthetic-executable",
                .sha256 = hash,
                .size = bytes.size(),
            },
        },
        .records = {
            EvidenceRecord{
                .id = "ev-synthetic-exe-manifest",
                .claim_id = "claim-synthetic-exe-manifest",
                .title = "Synthetic executable identity",
                .summary = "Evidence linked by exact SHA-256 for manifest testing.",
                .confidence = Confidence::confirmed,
                .locations = {
                    EvidenceLocation{
                        .artifact_id = "artifact-synthetic-exe-manifest",
                        .file_offset = 0U,
                        .size = bytes.size(),
                        .rva = 0U,
                        .va = std::nullopt,
                        .symbol = "synthetic_executable",
                        .note = "Synthetic fixture only.",
                    },
                },
                .tags = {"pe", "manifest", "synthetic-test"},
                .supersedes = {},
            },
        },
    };

    ProjectWorkspace project;
    assert(project.import_evidence_packet(packet));
    assert(project.create_session(ResourcePayload{
        .resource = resource,
        .bytes = bytes,
        .diagnostics = {},
    }));
    const auto report = ResourceAnalyzer::analyze(project, resource.id);
    assert(report.ok());
    assert(project.request_validation(
        resource.id,
        ToolTarget::exe_editor,
        "synthetic-pe-analysis",
        "Validate the synthetic PE analysis manifest."));

    const auto json = executable_workspace_manifest_json(project, resource.id);
    assert(!json.empty());
    const auto parsed = Parser::parse(json);
    assert(parsed.ok());
    const auto* root = parsed.value->as_object();
    assert(root != nullptr);

    const auto* pe_value = member(*root, "pe");
    assert(pe_value != nullptr);
    const auto* pe = pe_value->as_object();
    assert(pe != nullptr);
    const auto* sha = member(*pe, "sha256");
    assert(sha != nullptr && sha->as_string() != nullptr);
    assert(*sha->as_string() == hash);
    const auto* sections = member(*pe, "section_count");
    assert(sections != nullptr && sections->as_u64() != nullptr);
    assert(*sections->as_u64() == 1U);

    const auto* target_value = member(*root, "known_target");
    assert(target_value != nullptr);
    const auto* target = target_value->as_object();
    assert(target != nullptr);
    const auto* hash_match = member(*target, "hash_match");
    assert(hash_match != nullptr && hash_match->as_bool() != nullptr);
    assert(!*hash_match->as_bool());

    const auto* artifacts_value = member(*root, "artifact_matches");
    assert(artifacts_value != nullptr);
    const auto* artifacts = artifacts_value->as_array();
    assert(artifacts != nullptr && artifacts->size() == 1U);

    const auto* evidence_value = member(*root, "evidence_record_ids");
    assert(evidence_value != nullptr);
    const auto* evidence = evidence_value->as_array();
    assert(evidence != nullptr && evidence->size() == 1U);
    assert((*evidence)[0].as_string() != nullptr);
    assert(*(*evidence)[0].as_string() == "ev-synthetic-exe-manifest");

    const auto* sections_value = member(*root, "sections");
    assert(sections_value != nullptr);
    const auto* section_array = sections_value->as_array();
    assert(section_array != nullptr && section_array->size() == 1U);

    const auto* workspace_value = member(*root, "workspace");
    assert(workspace_value != nullptr);
    const auto* workspace = workspace_value->as_object();
    assert(workspace != nullptr);
    const auto* validation = member(*workspace, "validation_request_count");
    assert(validation != nullptr && validation->as_u64() != nullptr);
    assert(*validation->as_u64() == 1U);

    assert(executable_workspace_manifest_json(project, ResourceId{}).empty());
    assert(executable_workspace_manifest_json(project, resource.id) == json);
    return 0;
}
