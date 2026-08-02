#include "dmc_rengine/core/sha256.hpp"
#include "dmc_rengine/evidence/packet.hpp"
#include "dmc_rengine/exe/evidence_address_resolver.hpp"
#include "dmc_rengine/integration/executable_evidence.hpp"
#include "dmc_rengine/integration/project_workspace.hpp"
#include "dmc_rengine/integration/resource_analyzer.hpp"

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

[[nodiscard]] dmc::rengine::evidence::EvidencePacket make_packet(
    std::string hash,
    std::uint64_t size) {
    using dmc::rengine::evidence::ArtifactIdentity;
    using dmc::rengine::evidence::Confidence;
    using dmc::rengine::evidence::EvidenceLocation;
    using dmc::rengine::evidence::EvidencePacket;
    using dmc::rengine::evidence::EvidenceRecord;

    return EvidencePacket{
        .schema_version = 1U,
        .id = "packet-address-resolver-test",
        .title = "Address resolver test packet",
        .project = "DMC Rengine",
        .artifacts = {
            ArtifactIdentity{
                .id = "artifact-address-resolver-test",
                .role = "synthetic-executable",
                .sha256 = std::move(hash),
                .size = size,
            },
        },
        .records = {
            EvidenceRecord{
                .id = "ev-address-resolver-good",
                .claim_id = "claim-address-resolver-good",
                .title = "Consistent PE location",
                .summary = "A synthetic location with matching file offset, RVA, and VA.",
                .confidence = Confidence::confirmed,
                .locations = {
                    EvidenceLocation{
                        .artifact_id = "artifact-address-resolver-test",
                        .file_offset = 0x200U,
                        .size = 4U,
                        .rva = 0x1000U,
                        .va = 0x140001000ULL,
                        .symbol = "synthetic_entry",
                        .note = "Synthetic fixture.",
                    },
                },
                .tags = {"pe", "address-resolution"},
                .supersedes = {},
            },
            EvidenceRecord{
                .id = "ev-address-resolver-conflict",
                .claim_id = "claim-address-resolver-conflict",
                .title = "Conflicting PE location",
                .summary = "A synthetic location with an intentionally conflicting VA.",
                .confidence = Confidence::candidate,
                .locations = {
                    EvidenceLocation{
                        .artifact_id = "artifact-address-resolver-test",
                        .file_offset = 0x200U,
                        .size = 4U,
                        .rva = 0x1000U,
                        .va = 0x140001004ULL,
                        .symbol = "conflict",
                        .note = "Negative fixture.",
                    },
                },
                .tags = {"negative-test"},
                .supersedes = {},
            },
        },
    };
}

} // namespace

int main() {
    using dmc::rengine::core::Sha256;
    using dmc::rengine::evidence::ArtifactIdentity;
    using dmc::rengine::gdspaces::ResourceId;
    using dmc::rengine::gdspaces::ResourcePayload;
    using dmc::rengine::gdspaces::ResourceRef;
    using dmc::rengine::integration::ProjectWorkspace;
    using dmc::rengine::integration::ResourceAnalyzer;
    using dmc::rengine::integration::resolve_executable_evidence;

    const auto bytes = make_pe();
    const auto hash = Sha256::compute(std::span<const std::byte>{bytes}).hex();
    const auto packet = make_packet(hash, bytes.size());
    const ResourceRef resource{
        .id = ResourceId{
            .source_id = "address-resolver-test",
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

    ProjectWorkspace project;
    assert(project.import_evidence_packet(packet));
    assert(project.create_session(ResourcePayload{
        .resource = resource,
        .bytes = bytes,
        .diagnostics = {},
    }));
    assert(ResourceAnalyzer::analyze(project, resource.id).ok());

    const auto good = resolve_executable_evidence(
        project, resource.id, "ev-address-resolver-good");
    assert(good.ok());
    assert(good.locations.size() == 1U);
    assert(good.locations[0].file_offset == 0x200U);
    assert(good.locations[0].rva == 0x1000U);
    assert(good.locations[0].va == 0x140001000ULL);
    assert(good.locations[0].size == 4U);

    const auto conflict = resolve_executable_evidence(
        project, resource.id, "ev-address-resolver-conflict");
    assert(!conflict.ok());
    assert(conflict.locations.empty());
    assert(!conflict.diagnostics.empty());
    assert(conflict.diagnostics[0].code ==
        "evidence-address.address-conflict");

    const auto missing = resolve_executable_evidence(
        project, resource.id, "ev-does-not-exist");
    assert(!missing.ok());
    assert(missing.diagnostics[0].code ==
        "executable-evidence.record-missing");

    auto wrong_artifact = packet.artifacts[0];
    wrong_artifact.sha256 =
        "ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff";
    const auto* executable = project.find_session(resource.id)->executable_context();
    assert(executable != nullptr);
    const auto mismatch = dmc::rengine::exe::resolve_evidence_location(
        packet.records[0].locations[0],
        wrong_artifact,
        *executable,
        bytes.size());
    assert(!mismatch.ok());
    assert(mismatch.diagnostics[0].code ==
        "evidence-address.sha256-mismatch");

    return 0;
}
