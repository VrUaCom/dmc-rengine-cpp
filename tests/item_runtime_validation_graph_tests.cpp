#include "dmc_rengine/core/json.hpp"
#include "dmc_rengine/core/sha256.hpp"
#include "dmc_rengine/evidence/packet.hpp"
#include "dmc_rengine/integration/project_workspace.hpp"
#include "dmc_rengine/integration/resource_analyzer.hpp"
#include "dmc_rengine/item/runtime_request.hpp"
#include "dmc_rengine/validation/item_runtime_plan.hpp"
#include "dmc_rengine/validation/item_runtime_plan_manifest.hpp"

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <span>
#include <string>
#include <string_view>
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
    using dmc::rengine::gdspaces::ResourcePayload;
    using dmc::rengine::gdspaces::ToolTarget;
    using dmc::rengine::integration::ProjectEdgeKind;
    using dmc::rengine::integration::ProjectNodeKind;
    using dmc::rengine::integration::ProjectWorkspace;
    using dmc::rengine::integration::ResourceAnalyzer;
    using dmc::rengine::integration::WorkspaceEventType;
    using dmc::rengine::item::RuntimeChangeKind;
    using dmc::rengine::item::RuntimeChangeRequest;
    using dmc::rengine::item::RuntimeRequestStatus;
    using dmc::rengine::validation::build_item_runtime_plan;
    using dmc::rengine::validation::item_runtime_plan_manifest_json;

    const auto pe_bytes = make_pe();
    const auto hash = Sha256::compute(std::span<const std::byte>{pe_bytes}).hex();
    const auto pe = resource(
        "validation-graph-pe", "synthetic.exe", "pe", pe_bytes.size());
    const std::vector<std::byte> itm_bytes{
        std::byte{'I'}, std::byte{'T'}, std::byte{'M'}, std::byte{0},
        std::byte{1}, std::byte{0}, std::byte{0}, std::byte{0},
        std::byte{50}, std::byte{0}, std::byte{0}, std::byte{0},
        std::byte{0}, std::byte{0}, std::byte{0}, std::byte{0},
    };
    const auto itm = resource(
        "validation-graph-itm", "item050.itm", "itm", itm_bytes.size());

    const EvidencePacket packet{
        .schema_version = 1U,
        .id = "packet-validation-graph-test",
        .title = "Validation graph test packet",
        .project = "DMC Rengine",
        .artifacts = {
            ArtifactIdentity{
                .id = "artifact-validation-graph-pe",
                .role = "synthetic-executable",
                .sha256 = hash,
                .size = pe_bytes.size(),
            },
        },
        .records = {
            EvidenceRecord{
                .id = "ev-validation-graph-slot",
                .claim_id = "claim-validation-graph-slot",
                .title = "Synthetic slot patch location",
                .summary = "Patch-grade evidence for validation graph testing.",
                .confidence = Confidence::confirmed,
                .locations = {
                    EvidenceLocation{
                        .artifact_id = "artifact-validation-graph-pe",
                        .file_offset = 0x200U,
                        .size = 4U,
                        .rva = 0x1000U,
                        .va = 0x140001000ULL,
                        .symbol = "synthetic_slot",
                        .note = "Synthetic fixture.",
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

    const RuntimeChangeRequest ready_request{
        .id = "validation-graph-ready-request",
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
        .target_artifact_id = "artifact-validation-graph-pe",
        .requested_value = 50U,
        .evidence_record_ids = {"ev-validation-graph-slot"},
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
    assert(project.register_item_runtime_request(ready_request));
    const auto ready_plan = build_item_runtime_plan(project, ready_request);
    assert(ready_plan.ready_for_execution());
    assert(ready_plan.requirements.size() == 13U);
    assert(project.register_item_runtime_validation_plan(
        ready_plan, ready_request));
    assert(!project.register_item_runtime_validation_plan(
        ready_plan, ready_request));

    assert(project.graph().nodes(ProjectNodeKind::validation_plan).size() == 1U);
    assert(project.graph().nodes(
        ProjectNodeKind::validation_requirement).size() == 13U);
    assert(project.graph().find(ready_plan.id) != nullptr);

    std::size_t requires_edges = 0U;
    bool request_edge = false;
    bool artifact_edge = false;
    bool executable_edge = false;
    bool producer_edge = false;
    for (const auto& edge : project.graph().all_edges()) {
        if (edge.from != ready_plan.id) {
            continue;
        }
        if (edge.kind == ProjectEdgeKind::requires) {
            ++requires_edges;
        }
        if (edge.to == "runtime-request:" + ready_request.id &&
            edge.kind == ProjectEdgeKind::derived_from) {
            request_edge = true;
        }
        if (edge.to == "artifact:artifact-validation-graph-pe" &&
            edge.kind == ProjectEdgeKind::validates) {
            artifact_edge = true;
        }
        if (edge.to == "resource:" + pe.id.canonical() &&
            edge.kind == ProjectEdgeKind::validates) {
            executable_edge = true;
        }
        if (edge.to == "tool:build-test-lab" &&
            edge.kind == ProjectEdgeKind::produced_by) {
            producer_edge = true;
        }
    }
    assert(requires_edges == 13U);
    assert(request_edge);
    assert(artifact_edge);
    assert(executable_edge);
    assert(producer_edge);

    const auto* item_session = project.find_session(itm.id);
    assert(item_session != nullptr);
    assert(item_session->events().by_type(
        WorkspaceEventType::validation_plan_created).size() == 1U);

    const auto ready_json = item_runtime_plan_manifest_json(
        project, ready_plan, ready_request);
    assert(!ready_json.empty());
    assert(ready_json == item_runtime_plan_manifest_json(
        project, ready_plan, ready_request));
    const auto parsed_ready = Parser::parse(ready_json);
    assert(parsed_ready.ok());
    const auto* ready_root = parsed_ready.value->as_object();
    assert(ready_root != nullptr);
    const auto* ready_plan_value = member(*ready_root, "plan");
    assert(ready_plan_value != nullptr);
    const auto* ready_plan_object = ready_plan_value->as_object();
    assert(ready_plan_object != nullptr);
    const auto* ready_flag = member(*ready_plan_object, "ready_for_execution");
    assert(ready_flag != nullptr && ready_flag->as_bool() != nullptr);
    assert(*ready_flag->as_bool());
    const auto* ready_requirements = member(*ready_root, "requirements");
    assert(ready_requirements != nullptr &&
        ready_requirements->as_array() != nullptr);
    assert(ready_requirements->as_array()->size() == 13U);
    const auto* ready_blockers = member(*ready_root, "blockers");
    assert(ready_blockers != nullptr && ready_blockers->as_array() != nullptr);
    assert(ready_blockers->as_array()->empty());

    auto research_request = ready_request;
    research_request.id = "validation-graph-research-request";
    research_request.status = RuntimeRequestStatus::research_required;
    assert(project.register_item_runtime_request(research_request));
    const auto research_plan = build_item_runtime_plan(
        project, research_request);
    assert(!research_plan.ready_for_execution());
    assert(research_plan.blockers.size() == 1U);
    assert(project.register_item_runtime_validation_plan(
        research_plan, research_request));

    assert(project.graph().nodes(ProjectNodeKind::validation_plan).size() == 2U);
    assert(project.graph().nodes(
        ProjectNodeKind::validation_requirement).size() == 27U);
    bool external_blocker = false;
    bool blocker_edge = false;
    for (const auto* node : project.graph().nodes(
             ProjectNodeKind::validation_requirement)) {
        const auto attribute = node->attributes.find("external_blocker");
        if (attribute != node->attributes.end() && attribute->second == "true") {
            external_blocker = true;
            for (const auto* edge : project.graph().outgoing(node->id)) {
                if (edge->to == research_plan.id &&
                    edge->kind == ProjectEdgeKind::blocks) {
                    blocker_edge = true;
                }
            }
        }
    }
    assert(external_blocker);
    assert(blocker_edge);
    assert(item_session->events().by_type(
        WorkspaceEventType::validation_plan_created).size() == 2U);

    const auto research_json = item_runtime_plan_manifest_json(
        project, research_plan, research_request);
    const auto parsed_research = Parser::parse(research_json);
    assert(parsed_research.ok());
    const auto* research_root = parsed_research.value->as_object();
    assert(research_root != nullptr);
    const auto* blockers = member(*research_root, "blockers");
    assert(blockers != nullptr && blockers->as_array() != nullptr);
    assert(blockers->as_array()->size() == 1U);

    return 0;
}
