#include "dmc_rengine/core/sha256.hpp"
#include "dmc_rengine/evidence/packet.hpp"
#include "dmc_rengine/integration/project_workspace.hpp"
#include "dmc_rengine/integration/resource_analyzer.hpp"
#include "dmc_rengine/item/runtime_request.hpp"
#include "dmc_rengine/validation/item_runtime_plan.hpp"

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

[[nodiscard]] dmc::rengine::gdspaces::ResourceRef resource(
    std::string source,
    std::string path,
    std::string format,
    std::uint64_t size) {
    return dmc::rengine::gdspaces::ResourceRef{
        .id = dmc::rengine::gdspaces::ResourceId{
            .source_id = std::move(source),
            .logical_path = std::move(path),
            .container_chain = {},
            .offset = 0U,
            .size = size,
        },
        .display_name = "resource",
        .format = std::move(format),
        .profile = "dmc3-hd",
        .synthetic_name = false,
        .container = false,
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
    using dmc::rengine::gdspaces::ToolTarget;
    using dmc::rengine::integration::ProjectWorkspace;
    using dmc::rengine::integration::ResourceAnalyzer;
    using dmc::rengine::item::RuntimeChangeKind;
    using dmc::rengine::item::RuntimeChangeRequest;
    using dmc::rengine::item::RuntimeRequestStatus;
    using dmc::rengine::validation::RequirementKind;
    using dmc::rengine::validation::RequirementStatus;
    using dmc::rengine::validation::build_item_runtime_plan;

    const auto pe_bytes = make_pe();
    const auto hash = Sha256::compute(std::span<const std::byte>{pe_bytes}).hex();
    const auto pe = resource("validation-pe", "synthetic.exe", "pe", pe_bytes.size());
    const std::vector<std::byte> itm_bytes{
        std::byte{'I'}, std::byte{'T'}, std::byte{'M'}, std::byte{0},
        std::byte{1}, std::byte{0}, std::byte{0}, std::byte{0},
        std::byte{50}, std::byte{0}, std::byte{0}, std::byte{0},
        std::byte{0}, std::byte{0}, std::byte{0}, std::byte{0},
    };
    const auto itm = resource("validation-itm", "item050.itm", "itm", itm_bytes.size());

    const EvidencePacket packet{
        .schema_version = 1U,
        .id = "packet-validation-plan-test",
        .title = "Validation plan test packet",
        .project = "DMC Rengine",
        .artifacts = {
            ArtifactIdentity{
                .id = "artifact-validation-pe",
                .role = "synthetic-executable",
                .sha256 = hash,
                .size = pe_bytes.size(),
            },
        },
        .records = {
            EvidenceRecord{
                .id = "ev-validation-slot",
                .claim_id = "claim-validation-slot",
                .title = "Synthetic slot patch location",
                .summary = "A patch-grade synthetic PE location for Build and Test planning.",
                .confidence = Confidence::confirmed,
                .locations = {
                    EvidenceLocation{
                        .artifact_id = "artifact-validation-pe",
                        .file_offset = 0x200U,
                        .size = 4U,
                        .rva = 0x1000U,
                        .va = 0x140001000ULL,
                        .symbol = "synthetic_slot",
                        .note = "Expected bytes are supplied by the request guard contract.",
                    },
                },
                .tags = {"validation", "synthetic"},
                .supersedes = {},
            },
        },
    };

    ProjectWorkspace project;
    assert(project.import_evidence_packet(packet));
    assert(project.create_session(ResourcePayload{
        .resource = pe,
        .bytes = pe_bytes,
        .diagnostics = {},
    }));
    assert(ResourceAnalyzer::analyze(project, pe.id).ok());
    assert(project.create_session(ResourcePayload{
        .resource = itm,
        .bytes = itm_bytes,
        .diagnostics = {},
    }));
    assert(project.enable_working_copy(itm.id));

    const RuntimeChangeRequest request{
        .id = "synthetic-ready-request",
        .kind = RuntimeChangeKind::register_item_slot,
        .status = RuntimeRequestStatus::ready_for_patch_planning,
        .item_resource = itm.id,
        .item_revision = 0U,
        .producer = ToolTarget::item_editor,
        .consumer = ToolTarget::exe_editor,
        .validators = {
            ToolTarget::evidence_registry,
            ToolTarget::build_test_lab,
        },
        .target_artifact_id = "artifact-validation-pe",
        .requested_value = 50U,
        .evidence_record_ids = {"ev-validation-slot"},
        .required_guards = {
            "target-artifact-sha256",
            "expected-source-bytes",
            "patch-range-bounds",
            "patch-overlap-conflict-check",
            "atomic-application",
            "rollback-plan",
            "build-and-test-validation",
        },
        .diagnostics = {},
    };
    assert(request.valid());
    assert(project.register_item_runtime_request(request));

    const auto plan = build_item_runtime_plan(project, request);
    assert(plan.valid());
    assert(plan.ready_for_execution());
    assert(plan.blockers.empty());
    assert(plan.requirements.size() == 13U);
    const auto address_requirement = std::find_if(
        plan.requirements.begin(), plan.requirements.end(),
        [](const auto& requirement) {
            return requirement.kind ==
                RequirementKind::evidence_address_resolution;
        });
    assert(address_requirement != plan.requirements.end());
    assert(address_requirement->status == RequirementStatus::ready);

    auto research = request;
    research.id = "synthetic-research-request";
    research.status = RuntimeRequestStatus::research_required;
    assert(project.register_item_runtime_request(research));
    const auto research_plan = build_item_runtime_plan(project, research);
    assert(!research_plan.ready_for_execution());
    assert(std::find(
        research_plan.blockers.begin(),
        research_plan.blockers.end(),
        "runtime-request-research-required") != research_plan.blockers.end());

    ProjectWorkspace no_executable;
    assert(no_executable.import_evidence_packet(packet));
    assert(no_executable.create_session(ResourcePayload{
        .resource = itm,
        .bytes = itm_bytes,
        .diagnostics = {},
    }));
    assert(no_executable.enable_working_copy(itm.id));
    assert(no_executable.register_item_runtime_request(request));
    const auto blocked = build_item_runtime_plan(no_executable, request);
    assert(!blocked.ready_for_execution());
    assert(std::find(
        blocked.blockers.begin(),
        blocked.blockers.end(),
        "executable-session") != blocked.blockers.end());

    return 0;
}
