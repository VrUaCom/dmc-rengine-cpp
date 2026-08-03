#include "dmc_rengine/core/json.hpp"
#include "dmc_rengine/core/sha256.hpp"
#include "dmc_rengine/evidence/packet.hpp"
#include "dmc_rengine/integration/project_workspace.hpp"
#include "dmc_rengine/integration/resource_analyzer.hpp"
#include "dmc_rengine/item/runtime_request.hpp"
#include "dmc_rengine/patch/copy_execution_manifest.hpp"
#include "dmc_rengine/patch/copy_executor.hpp"
#include "dmc_rengine/patch/item_runtime_compiler.hpp"
#include "dmc_rengine/validation/item_runtime_plan.hpp"

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

[[nodiscard]] bool has_diagnostic(
    const dmc::rengine::patch::PatchCopyExecutionResult& result,
    std::string_view code) {
    return std::any_of(
        result.diagnostics.begin(), result.diagnostics.end(),
        [code](const auto& diagnostic) {
            return diagnostic.code == code;
        });
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
    using dmc::rengine::patch::ItemRuntimePatchCompiler;
    using dmc::rengine::patch::ItemRuntimePatchInput;
    using dmc::rengine::patch::PatchCopyExecutor;
    using dmc::rengine::patch::copy_execution_manifest_json;
    using dmc::rengine::validation::build_item_runtime_plan;

    const auto pe_bytes = make_pe();
    const auto original_pe_bytes = pe_bytes;
    const auto source_hash = Sha256::compute(
        std::span<const std::byte>{pe_bytes}).hex();
    const auto pe = resource(
        "copy-executor-pe", "synthetic.exe", "pe", pe_bytes.size());
    const std::vector<std::byte> itm_bytes{
        std::byte{'I'}, std::byte{'T'}, std::byte{'M'}, std::byte{0},
        std::byte{1}, std::byte{0}, std::byte{0}, std::byte{0},
        std::byte{50}, std::byte{0}, std::byte{0}, std::byte{0},
        std::byte{0}, std::byte{0}, std::byte{0}, std::byte{0},
    };
    const auto itm = resource(
        "copy-executor-itm", "item050.itm", "itm", itm_bytes.size());

    const EvidencePacket packet{
        .schema_version = 1U,
        .id = "packet-copy-executor-test",
        .title = "Copy executor test packet",
        .project = "DMC Rengine",
        .artifacts = {
            ArtifactIdentity{
                .id = "artifact-copy-executor-pe",
                .role = "synthetic-executable",
                .sha256 = source_hash,
                .size = pe_bytes.size(),
            },
        },
        .records = {
            EvidenceRecord{
                .id = "ev-copy-executor-slot",
                .claim_id = "claim-copy-executor-slot",
                .title = "Synthetic copied patch location",
                .summary = "Patch-grade evidence for copy execution testing.",
                .confidence = Confidence::confirmed,
                .locations = {
                    EvidenceLocation{
                        .artifact_id = "artifact-copy-executor-pe",
                        .file_offset = 0x200U,
                        .size = 4U,
                        .rva = 0x1000U,
                        .va = 0x140001000ULL,
                        .symbol = "synthetic_slot",
                        .note = "Synthetic fixture.",
                    },
                },
                .tags = {"patch", "copy", "synthetic"},
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
        .id = "copy-executor-ready-request",
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
        .target_artifact_id = "artifact-copy-executor-pe",
        .requested_value = 50U,
        .evidence_record_ids = {"ev-copy-executor-slot"},
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
    assert(project.register_item_runtime_request(request));
    const auto validation_plan = build_item_runtime_plan(project, request);
    assert(validation_plan.ready_for_execution());
    assert(project.register_item_runtime_validation_plan(
        validation_plan, request));

    const ItemRuntimePatchInput input{
        .evidence_record_id = "ev-copy-executor-slot",
        .expected = {
            std::byte{0}, std::byte{0}, std::byte{0}, std::byte{0}},
        .replacement = {
            std::byte{0}, std::byte{1}, std::byte{8}, std::byte{19}},
        .description = "Register synthetic copied item slot 50.",
    };
    const auto compiled_result = ItemRuntimePatchCompiler::compile(
        project, request, validation_plan, input);
    assert(compiled_result.ok());
    assert(compiled_result.compiled.has_value());
    const auto& compiled = *compiled_result.compiled;

    const auto before_registration = PatchCopyExecutor::execute(
        project, compiled);
    assert(!before_registration.ok());
    assert(has_diagnostic(
        before_registration,
        "copy-executor.patch-plan-not-registered"));

    assert(project.register_compiled_item_runtime_patch_plan(compiled));
    const auto execution = PatchCopyExecutor::execute(project, compiled);
    assert(execution.ok());
    assert(execution.artifact.has_value());
    const auto& copy = *execution.artifact;
    assert(copy.valid());
    assert(copy.source_sha256 == source_hash);
    assert(copy.output_sha256 != source_hash);
    assert(copy.output_bytes.size() == original_pe_bytes.size());
    assert(copy.output_bytes != original_pe_bytes);
    assert(copy.output_bytes[0x200U] == std::byte{0});
    assert(copy.output_bytes[0x201U] == std::byte{1});
    assert(copy.output_bytes[0x202U] == std::byte{8});
    assert(copy.output_bytes[0x203U] == std::byte{19});

    const auto rollback_result = copy.rollback_plan.apply(
        std::span<const std::byte>{copy.output_bytes});
    assert(rollback_result.applied);
    assert(rollback_result.output == original_pe_bytes);
    assert(Sha256::compute(
        std::span<const std::byte>{rollback_result.output}).hex() == source_hash);

    const auto* source_session = project.find_session(pe.id);
    assert(source_session != nullptr);
    assert(source_session->source_payload().bytes == original_pe_bytes);
    assert(project.register_patch_copy_artifact(copy));
    assert(!project.register_patch_copy_artifact(copy));
    assert(source_session->source_payload().bytes == original_pe_bytes);

    assert(project.graph().nodes(ProjectNodeKind::patch_execution).size() == 1U);
    assert(project.graph().nodes(ProjectNodeKind::rollback_plan).size() == 1U);
    assert(project.graph().find(copy.id) != nullptr);
    assert(project.graph().find(copy.rollback_plan_id) != nullptr);

    bool execution_edge = false;
    bool source_edge = false;
    bool rollback_edge = false;
    bool rollback_back_edge = false;
    for (const auto& edge : project.graph().all_edges()) {
        if (edge.from == copy.id && edge.to == copy.patch_plan_id &&
            edge.kind == ProjectEdgeKind::executed_from) {
            execution_edge = true;
        }
        if (edge.from == copy.id &&
            edge.to == "resource:" + pe.id.canonical() &&
            edge.kind == ProjectEdgeKind::targets) {
            source_edge = true;
        }
        if (edge.from == copy.id && edge.to == copy.rollback_plan_id &&
            edge.kind == ProjectEdgeKind::produces) {
            rollback_edge = true;
        }
        if (edge.from == copy.rollback_plan_id && edge.to == copy.id &&
            edge.kind == ProjectEdgeKind::rollback_for) {
            rollback_back_edge = true;
        }
    }
    assert(execution_edge);
    assert(source_edge);
    assert(rollback_edge);
    assert(rollback_back_edge);
    assert(source_session->events().by_type(
        WorkspaceEventType::patch_copy_executed).size() == 1U);

    const auto manifest = copy_execution_manifest_json(project, copy);
    assert(!manifest.empty());
    assert(manifest == copy_execution_manifest_json(project, copy));
    const auto parsed = Parser::parse(manifest);
    assert(parsed.ok());
    const auto* root = parsed.value->as_object();
    assert(root != nullptr);
    const auto* execution_value = member(*root, "execution");
    assert(execution_value != nullptr);
    const auto* execution_object = execution_value->as_object();
    assert(execution_object != nullptr);
    const auto* original_write = member(
        *execution_object, "original_file_write_performed");
    const auto* source_modified = member(
        *execution_object, "immutable_source_modified");
    assert(original_write != nullptr && original_write->as_bool() != nullptr);
    assert(source_modified != nullptr && source_modified->as_bool() != nullptr);
    assert(!*original_write->as_bool());
    assert(!*source_modified->as_bool());
    const auto* output_value = member(*root, "copied_output");
    assert(output_value != nullptr);
    const auto* output_object = output_value->as_object();
    assert(output_object != nullptr);
    const auto* path = member(*output_object, "filesystem_path");
    assert(path != nullptr && path->is_null());
    const auto* rollback_value = member(*root, "rollback");
    assert(rollback_value != nullptr);
    const auto* rollback_object = rollback_value->as_object();
    assert(rollback_object != nullptr);
    const auto* verified = member(*rollback_object, "verified_exact_restore");
    assert(verified != nullptr && verified->as_bool() != nullptr);
    assert(*verified->as_bool());

    auto tampered = copy;
    tampered.output_sha256 = source_hash;
    assert(!tampered.valid());
    assert(!project.register_patch_copy_artifact(tampered));

    return 0;
}
