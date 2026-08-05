#include "dmc_rengine/core/json.hpp"
#include "dmc_rengine/evidence/registry.hpp"
#include "dmc_rengine/formats/hits.hpp"
#include "dmc_rengine/formats/hits_binary.hpp"
#include "dmc_rengine/integration/format_registry.hpp"
#include "dmc_rengine/integration/resource_workspace.hpp"
#include "dmc_rengine/integration/tool_registry.hpp"
#include "dmc_rengine/integration/workspace_manifest.hpp"

#include <bit>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

namespace {

void write_u32(std::vector<std::byte>& bytes, std::size_t offset, std::uint32_t value) {
    for (std::size_t index = 0; index < 4U; ++index) {
        bytes[offset + index] = static_cast<std::byte>(
            (value >> static_cast<unsigned>(index * 8U)) & 0xFFU);
    }
}

void write_i32(std::vector<std::byte>& bytes, std::size_t offset, std::int32_t value) {
    write_u32(bytes, offset, static_cast<std::uint32_t>(value));
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

[[nodiscard]] std::vector<std::byte> make_hits() {
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

[[nodiscard]] const dmc::rengine::core::json::Value* member(
    const dmc::rengine::core::json::Value::Object& object,
    std::string_view name) {
    const auto iterator = object.find(name);
    return iterator == object.end() ? nullptr : &iterator->second;
}

} // namespace

int main() {
    using dmc::rengine::core::json::Parser;
    using dmc::rengine::evidence::Confidence;
    using dmc::rengine::evidence::EvidenceRecord;
    using dmc::rengine::evidence::EvidenceRegistry;
    using dmc::rengine::formats::hits::RecordScanner;
    using dmc::rengine::formats::hits::build_binary_document;
    using dmc::rengine::gdspaces::EditOperation;
    using dmc::rengine::gdspaces::ResourceId;
    using dmc::rengine::gdspaces::ResourcePayload;
    using dmc::rengine::gdspaces::ResourceRef;
    using dmc::rengine::gdspaces::StageBundle;
    using dmc::rengine::gdspaces::StageIdentity;
    using dmc::rengine::gdspaces::StageMember;
    using dmc::rengine::gdspaces::StageResourceCategory;
    using dmc::rengine::gdspaces::ToolTarget;
    using dmc::rengine::integration::FormatIntegrationRegistry;
    using dmc::rengine::integration::ResourceWorkspaceSession;
    using dmc::rengine::integration::ToolRegistry;
    using dmc::rengine::integration::WorkspaceContext;
    using dmc::rengine::integration::workspace_manifest_json;

    const auto bytes = make_hits();
    const ResourceRef ref{
        .id = ResourceId{
            .source_id = "manifest-test",
            .logical_path = "room/st001_003.ukn",
            .container_chain = "NBZ[0]/PAC[3]",
            .offset = 4096U,
            .size = bytes.size(),
        },
        .display_name = "collision.hits",
        .format = "hits",
        .profile = "dmc3-hd",
        .synthetic_name = false,
        .container = false,
    };

    const ToolRegistry tools;
    const FormatIntegrationRegistry formats;
    ResourceWorkspaceSession workspace(
        ResourcePayload{.resource = ref, .bytes = bytes, .diagnostics = {}},
        tools,
        formats,
        WorkspaceContext{
            .stage_context = true,
            .menu_context = false,
            .evidence_context = true,
        });

    const auto scan = RecordScanner::scan(bytes);
    assert(scan.ok());
    assert(workspace.add_parser_diagnostics(scan.diagnostics));
    auto document = build_binary_document(ref, bytes, scan);
    assert(document.has_value());
    assert(document->fields().size() > 16U);
    assert(workspace.attach_binary_document(std::move(*document)));

    EvidenceRegistry evidence;
    assert(evidence.add(EvidenceRecord{
        .id = "ev-hits-layout",
        .claim_id = "claim-hits-layout",
        .title = "HITS layout",
        .summary = "Synthetic workspace-manifest test evidence.",
        .confidence = Confidence::confirmed,
        .locations = {},
        .tags = {"hits"},
        .supersedes = {},
    }));
    assert(workspace.link_evidence_claim(evidence, "claim-hits-layout") == 1U);

    StageBundle stage(StageIdentity{
        .profile = "dmc3-hd",
        .stage_id = "st001",
        .display_name = "Stage 001",
        .exe_evidence_id = "ev-dmc3-stage-resource-table",
    });
    assert(stage.add(StageMember{
        .category = StageResourceCategory::collision,
        .resource = ref,
        .role = "stage-collision-source-0",
    }));
    assert(workspace.attach_stage_bundle(stage));
    assert(workspace.enable_working_copy());
    assert(workspace.apply_edit(
        EditOperation{
            .id = "manifest-edit",
            .base_revision = 0U,
            .offset = 0x54U,
            .expected = {std::byte{0}},
            .replacement = {std::byte{1}},
            .description = "Create a dirty working-copy state for the manifest.",
        },
        ToolTarget::stage_ops).applied);
    assert(workspace.request_validation(
        ToolTarget::stage_ops, "manifest-edit", "Validate synthetic HITS edit."));
    assert(workspace.record_manifest_exported(
        ToolTarget::gdspaces, "workspace-manifest-test"));

    const auto json = workspace_manifest_json(workspace);
    const auto parsed = Parser::parse(json);
    assert(parsed.ok());
    const auto* root = parsed.value->as_object();
    assert(root != nullptr);

    const auto* status = member(*root, "workspace_status");
    assert(status != nullptr && status->as_string() != nullptr);
    assert(*status->as_string() == "editable-dirty");

    const auto* binary = member(*root, "binary_document");
    assert(binary != nullptr && binary->as_object() != nullptr);
    const auto* fields = member(*binary->as_object(), "field_count");
    assert(fields != nullptr && fields->as_u64() != nullptr);
    assert(*fields->as_u64() > 16U);

    const auto* events = member(*root, "events");
    assert(events != nullptr && events->as_array() != nullptr);
    assert(events->as_array()->size() == workspace.events().size());
    assert(workspace_manifest_json(workspace) == json);
    return 0;
}
