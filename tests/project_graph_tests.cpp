#include "dmc_rengine/evidence/registry.hpp"
#include "dmc_rengine/formats/hits.hpp"
#include "dmc_rengine/formats/hits_binary.hpp"
#include "dmc_rengine/integration/format_registry.hpp"
#include "dmc_rengine/integration/project_graph.hpp"
#include "dmc_rengine/integration/resource_workspace.hpp"
#include "dmc_rengine/integration/tool_registry.hpp"

#include <bit>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <span>
#include <string>
#include <vector>

namespace {

void write_u32(
    std::vector<std::byte>& bytes,
    std::size_t offset,
    std::uint32_t value) {
    for (std::size_t index = 0; index < 4U; ++index) {
        bytes[offset + index] = static_cast<std::byte>(
            (value >> static_cast<unsigned>(index * 8U)) & 0xFFU);
    }
}

void write_i32(
    std::vector<std::byte>& bytes,
    std::size_t offset,
    std::int32_t value) {
    write_u32(bytes, offset, static_cast<std::uint32_t>(value));
}

void write_f32(
    std::vector<std::byte>& bytes,
    std::size_t offset,
    float value) {
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

[[nodiscard]] bool has_edge(
    const dmc::rengine::integration::ProjectGraph& graph,
    std::string_view from,
    std::string_view to,
    dmc::rengine::integration::ProjectEdgeKind kind) {
    for (const auto& edge : graph.all_edges()) {
        if (edge.from == from && edge.to == to && edge.kind == kind) {
            return true;
        }
    }
    return false;
}

} // namespace

int main() {
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
    using dmc::rengine::integration::ProjectEdgeKind;
    using dmc::rengine::integration::ProjectGraph;
    using dmc::rengine::integration::ProjectNodeKind;
    using dmc::rengine::integration::ResourceWorkspaceSession;
    using dmc::rengine::integration::ToolRegistry;
    using dmc::rengine::integration::WorkspaceContext;

    const auto bytes = make_hits();
    const ResourceRef ref{
        .id = ResourceId{
            .source_id = "graph-test",
            .logical_path = "room/st001_003.ukn",
            .container_chain = "NBZ[0]/PAC[3]",
            .offset = 12288U,
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
        ResourcePayload{
            .resource = ref,
            .bytes = bytes,
            .diagnostics = {},
        },
        tools,
        formats,
        WorkspaceContext{
            .stage_context = true,
            .menu_context = false,
            .evidence_context = true,
        });

    const auto scan = RecordScanner::scan(
        std::span<const std::byte>{bytes});
    assert(scan.ok());
    assert(workspace.add_parser_diagnostics(scan.diagnostics));
    auto document = build_binary_document(
        ref,
        std::span<const std::byte>{bytes},
        scan);
    assert(document.has_value());
    assert(workspace.attach_binary_document(std::move(*document)));

    EvidenceRegistry evidence;
    assert(evidence.add(EvidenceRecord{
        .id = "ev-hits-layout",
        .claim_id = "claim-hits-layout",
        .title = "HITS layout",
        .summary = "Synthetic project-graph test evidence.",
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
            .id = "graph-edit",
            .base_revision = 0U,
            .offset = 0x54U,
            .expected = {std::byte{0}},
            .replacement = {std::byte{2}},
            .description = "Create a graph-visible WorkingCopy revision.",
        },
        ToolTarget::stage_ops).applied);
    assert(workspace.request_validation(
        ToolTarget::stage_ops,
        "graph-edit",
        "Validate graph-visible edit."));
    assert(workspace.record_manifest_exported(
        ToolTarget::gdspaces,
        "st001-hits-workspace"));

    ProjectGraph graph;
    assert(graph.ingest_workspace(workspace, tools));
    assert(graph.node_count() > 15U);
    assert(graph.edge_count() > 20U);

    const auto resource_id = "resource:" + ref.id.canonical();
    const auto binary_id = "binary-document:" + ref.id.canonical();
    const auto working_id = "working-copy:" + ref.id.canonical();
    assert(graph.find(resource_id) != nullptr);
    assert(graph.find(resource_id)->kind == ProjectNodeKind::resource);
    assert(graph.find("format:hits") != nullptr);
    assert(graph.find("parser:formats.hits-record-scanner") != nullptr);
    assert(graph.find("tool:gdspaces") != nullptr);
    assert(graph.find("tool:gdspaces")->label == "GDSpaces");
    assert(graph.find("tool:gdspaces")->attributes.at("lore_name") ==
        "The Archive");
    assert(graph.find("tool:binary-inspector") != nullptr);
    assert(graph.find("tool:binary-inspector")->attributes.at("lore_name") ==
        "The Reliquary");
    assert(graph.find("evidence:ev-hits-layout") != nullptr);
    assert(graph.find("stage:dmc3-hd:st001") != nullptr);
    assert(graph.find(binary_id) != nullptr);
    assert(graph.find(working_id) != nullptr);
    assert(graph.find("manifest:st001-hits-workspace") != nullptr);

    assert(has_edge(
        graph, resource_id, "format:hits", ProjectEdgeKind::classified_as));
    assert(has_edge(
        graph,
        resource_id,
        "parser:formats.hits-record-scanner",
        ProjectEdgeKind::parsed_by));
    assert(has_edge(
        graph,
        resource_id,
        "tool:stage-ops",
        ProjectEdgeKind::opens_with));
    assert(has_edge(
        graph,
        "evidence:ev-hits-layout",
        resource_id,
        ProjectEdgeKind::evidence_for));
    assert(has_edge(
        graph,
        "stage:dmc3-hd:st001",
        resource_id,
        ProjectEdgeKind::stage_member));
    assert(has_edge(
        graph, binary_id, resource_id, ProjectEdgeKind::derived_from));
    assert(has_edge(
        graph, working_id, resource_id, ProjectEdgeKind::derived_from));
    assert(has_edge(
        graph,
        "manifest:st001-hits-workspace",
        resource_id,
        ProjectEdgeKind::derived_from));
    assert(has_edge(
        graph,
        "tool:build-test-lab",
        resource_id,
        ProjectEdgeKind::validates));

    assert(graph.nodes(ProjectNodeKind::event).size() ==
        workspace.events().size());
    assert(graph.nodes(ProjectNodeKind::tool).size() >= 7U);
    assert(!graph.outgoing(resource_id).empty());
    assert(!graph.incoming(resource_id).empty());

    const auto nodes_before = graph.node_count();
    const auto edges_before = graph.edge_count();
    assert(graph.ingest_workspace(workspace, tools));
    assert(graph.node_count() == nodes_before);
    assert(graph.edge_count() == edges_before);

    assert(!graph.connect(dmc::rengine::integration::ProjectEdge{
        .from = "missing",
        .to = resource_id,
        .kind = ProjectEdgeKind::related_to,
        .label = {},
    }));
    return 0;
}
