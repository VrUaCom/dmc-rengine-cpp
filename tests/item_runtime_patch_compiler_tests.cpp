#include "dmc_rengine/core/json.hpp"
#include "dmc_rengine/core/sha256.hpp"
#include "dmc_rengine/evidence/packet.hpp"
#include "dmc_rengine/integration/project_workspace.hpp"
#include "dmc_rengine/integration/resource_analyzer.hpp"
#include "dmc_rengine/item/runtime_request.hpp"
#include "dmc_rengine/patch/item_runtime_compiler.hpp"
#include "dmc_rengine/patch/item_runtime_patch_manifest.hpp"
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
    const dmc::rengine::patch::ItemRuntimePatchCompilationResult& result,
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
    using dmc::rengine::patch::item_runtime_patch_plan_manifest_json;
    using dmc::rengine::validation::build_item_runtime_plan;

    const auto pe_bytes = make_pe();
    const auto original_pe_bytes = pe_bytes;
    const auto hash = Sha256::compute(std::span<const std::byte>{pe_bytes}).hex();
    const auto pe = resource(
        "patch-compiler-pe", "synthetic.exe", "pe", pe_bytes.size());
    const std::vector<std::byte> itm_bytes{
        std::byte{'I'}, std::byte{'T'}, std::byte{'M'}, std::byte{0},
        std::byte{1}, std::byte{0}, std::byte{0}, std::byte{0},
        std::byte{50}, std::byte{0}, std::byte{0}, std::byte{0},
        std::byte{0}, std::byte{0}, std::byte{0}, std::byte{0},
    };
    const auto itm = resource(
        "patch-compiler-itm", "item050.itm", "itm", itm_bytes.size());

    const EvidencePacket packet{
        .schema_version = 1U,
        .id = "packet-patch-compiler-test",
        .title = "Patch compiler test packet",
        .project = "DMC Rengine",
        .artifacts = {
            ArtifactIdentity{
                .id = "artifact-patch-compiler-pe",
                .role = "synthetic-executable",
                .sha256 = hash,
                .size = pe_bytes.size(),
            },
        },
        .records = {
            EvidenceRecord{
                .id = "ev-patch-compiler-slot",
                .claim_id = "claim-patch-compiler-slot",
                .title = "Synthetic slot patch location",
                .summary = "Patch-grade evidence for compiler testing.",
                .confidence = Confidence::confirmed,
                .locations = {
                    EvidenceLocation{
                        .artifact_id = "artifact-patch-compiler-pe",
                        .file_offset = 0x200U,
                        .size = 4U,
                        .rva = 0x1000U,
                        .va = 0x140001000ULL,
                        .symbol = "synthetic_slot",
                        .note = "Synthetic fixture.",
                    },
                },
                .tags = {"patch", "synthetic"},
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
        .id = "patch-compiler-ready-request",
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
        .target_artifact_id = "artifact-patch-compiler-pe",
        .requested_value = 50U,
        .evidence_record_ids = {"ev-patch-compiler-slot"},
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
        .evidence_record_id = "ev-patch-compiler-slot",
        .expected = {
            std::byte{0}, std::byte{0}, std::byte{0}, std::byte{0}},
        .replacement = {
            std::byte{0}, std::byte{1}, std::byte{8}, std::byte{19}},
        .description = "Register synthetic item slot 50.",
    };
    const auto compiled_result = ItemRuntimePatchCompiler::compile(
        project, request, validation_plan, input);
    assert(compiled_result.ok());
    assert(compiled_result.compiled.has_value());
    const auto& compiled = *compiled_result.compiled;
    assert(compiled.valid());
    assert(compiled.metadata.file_offset == 0x200U);
    assert(compiled.metadata.rva == 0x1000U);
    assert(compiled.metadata.va == 0x140001000ULL);
    assert(compiled.metadata.size == 4U);
    assert(compiled.metadata.expected_hex == "00000000");
    assert(compiled.metadata.replacement_hex == "00010813");
    assert(compiled.guarded_plan.patches().size() == 1U);
    assert(compiled.guarded_plan.patches()[0].offset == 0x200U);

    const auto* executable_session = project.find_session(pe.id);
    assert(executable_session != nullptr);
    assert(executable_session->source_payload().bytes == original_pe_bytes);
    assert(project.register_compiled_item_runtime_patch_plan(compiled));
    assert(!project.register_compiled_item_runtime_patch_plan(compiled));
    assert(executable_session->source_payload().bytes == original_pe_bytes);

    assert(project.graph().nodes(ProjectNodeKind::patch_plan).size() == 1U);
    assert(project.graph().find(compiled.metadata.id) != nullptr);
    bool validation_edge = false;
    bool target_edge = false;
    bool artifact_edge = false;
    bool evidence_edge = false;
    bool producer_edge = false;
    bool delivery_edge = false;
    for (const auto& edge : project.graph().all_edges()) {
        if (edge.from != compiled.metadata.id) {
            continue;
        }
        if (edge.to == validation_plan.id &&
            edge.kind == ProjectEdgeKind::compiled_from) {
            validation_edge = true;
        }
        if (edge.to == "resource:" + pe.id.canonical() &&
            edge.kind == ProjectEdgeKind::targets) {
            target_edge = true;
        }
        if (edge.to == "artifact:artifact-patch-compiler-pe" &&
            edge.kind == ProjectEdgeKind::requests_change) {
            artifact_edge = true;
        }
        if (edge.to == "evidence:ev-patch-compiler-slot" &&
            edge.kind == ProjectEdgeKind::evidence_for) {
            evidence_edge = true;
        }
        if (edge.to == "tool:exe-editor" &&
            edge.kind == ProjectEdgeKind::produced_by) {
            producer_edge = true;
        }
        if (edge.to == "tool:build-test-lab" &&
            edge.kind == ProjectEdgeKind::delivered_to) {
            delivery_edge = true;
        }
    }
    assert(validation_edge);
    assert(target_edge);
    assert(artifact_edge);
    assert(evidence_edge);
    assert(producer_edge);
    assert(delivery_edge);

    const auto* item_session = project.find_session(itm.id);
    assert(item_session != nullptr);
    assert(item_session->events().by_type(
        WorkspaceEventType::patch_plan_compiled).size() == 1U);

    const auto manifest = item_runtime_patch_plan_manifest_json(
        project, compiled);
    assert(!manifest.empty());
    assert(manifest == item_runtime_patch_plan_manifest_json(
        project, compiled));
    const auto parsed = Parser::parse(manifest);
    assert(parsed.ok());
    const auto* root = parsed.value->as_object();
    assert(root != nullptr);
    const auto* patch_plan_value = member(*root, "patch_plan");
    assert(patch_plan_value != nullptr);
    const auto* patch_plan = patch_plan_value->as_object();
    assert(patch_plan != nullptr);
    const auto* applied = member(*patch_plan, "applied");
    const auto* source_modified = member(*patch_plan, "source_modified");
    assert(applied != nullptr && applied->as_bool() != nullptr);
    assert(source_modified != nullptr && source_modified->as_bool() != nullptr);
    assert(!*applied->as_bool());
    assert(!*source_modified->as_bool());
    const auto* safety_value = member(*root, "safety");
    assert(safety_value != nullptr);
    const auto* safety = safety_value->as_object();
    assert(safety != nullptr);
    const auto* original_write = member(
        *safety, "original_file_write_performed");
    assert(original_write != nullptr && original_write->as_bool() != nullptr);
    assert(!*original_write->as_bool());

    auto mismatch_input = input;
    mismatch_input.expected[0] = std::byte{1};
    const auto mismatch = ItemRuntimePatchCompiler::compile(
        project, request, validation_plan, mismatch_input);
    assert(!mismatch.ok());
    assert(has_diagnostic(
        mismatch, "patch-compiler.expected-bytes-mismatch"));

    auto undeclared_input = input;
    undeclared_input.evidence_record_id = "ev-not-declared";
    const auto undeclared = ItemRuntimePatchCompiler::compile(
        project, request, validation_plan, undeclared_input);
    assert(!undeclared.ok());
    assert(has_diagnostic(
        undeclared, "patch-compiler.evidence-not-declared"));

    auto no_op_input = input;
    no_op_input.replacement = no_op_input.expected;
    const auto no_op = ItemRuntimePatchCompiler::compile(
        project, request, validation_plan, no_op_input);
    assert(!no_op.ok());
    assert(has_diagnostic(
        no_op, "patch-compiler.no-op-replacement"));

    auto invalid_size_input = input;
    invalid_size_input.replacement.pop_back();
    const auto invalid_size = ItemRuntimePatchCompiler::compile(
        project, request, validation_plan, invalid_size_input);
    assert(!invalid_size.ok());
    assert(has_diagnostic(
        invalid_size, "patch-compiler.input-invalid"));

    auto research_request = request;
    research_request.id = "patch-compiler-research-request";
    research_request.status = RuntimeRequestStatus::research_required;
    assert(project.register_item_runtime_request(research_request));
    const auto research_plan = build_item_runtime_plan(
        project, research_request);
    assert(project.register_item_runtime_validation_plan(
        research_plan, research_request));
    const auto research_compile = ItemRuntimePatchCompiler::compile(
        project, research_request, research_plan, input);
    assert(!research_compile.ok());
    assert(has_diagnostic(
        research_compile, "patch-compiler.request-not-ready"));

    const auto applied_copy = compiled.guarded_plan.apply(original_pe_bytes);
    assert(applied_copy.applied);
    assert(applied_copy.output != original_pe_bytes);
    assert(original_pe_bytes == executable_session->source_payload().bytes);

    return 0;
}
