#include "dmc_rengine/evidence/record.hpp"
#include "dmc_rengine/formats/hits.hpp"
#include "dmc_rengine/formats/hits_binary.hpp"
#include "dmc_rengine/integration/project_workspace.hpp"

#include "hits_test_fixture.hpp"

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <span>
#include <string>
#include <string_view>
#include <vector>

namespace {

[[nodiscard]] dmc::rengine::gdspaces::ResourceRef resource(
    std::string path,
    std::string format,
    std::uint64_t offset,
    std::uint64_t size,
    bool container = false) {
    return dmc::rengine::gdspaces::ResourceRef{
        .id = dmc::rengine::gdspaces::ResourceId{
            .source_id = "project-workspace-test",
            .logical_path = std::move(path),
            .container_chain = container ? "NBZ[0]" : "NBZ[0]/PAC[4]",
            .offset = offset,
            .size = size,
        },
        .display_name = "resource",
        .format = std::move(format),
        .profile = "dmc3-hd",
        .synthetic_name = false,
        .container = container,
    };
}

[[nodiscard]] bool has_project_edge(
    const dmc::rengine::integration::ProjectGraph& graph,
    std::string_view from,
    std::string_view to,
    dmc::rengine::integration::ProjectEdgeKind kind) {
    return std::any_of(
        graph.all_edges().begin(), graph.all_edges().end(),
        [from, to, kind](const auto& edge) {
            return edge.from == from && edge.to == to && edge.kind == kind;
        });
}

} // namespace

int main() {
    using dmc::rengine::evidence::Confidence;
    using dmc::rengine::evidence::EvidenceRecord;
    using dmc::rengine::formats::hits::RecordScanner;
    using dmc::rengine::formats::hits::build_binary_document;
    using dmc::rengine::gdspaces::EditOperation;
    using dmc::rengine::gdspaces::ResourcePayload;
    using dmc::rengine::gdspaces::ResourceRelation;
    using dmc::rengine::gdspaces::StageBundle;
    using dmc::rengine::gdspaces::StageIdentity;
    using dmc::rengine::gdspaces::StageMember;
    using dmc::rengine::gdspaces::StageResourceCategory;
    using dmc::rengine::gdspaces::ToolTarget;
    using dmc::rengine::integration::ProjectEdgeKind;
    using dmc::rengine::integration::ProjectNodeKind;
    using dmc::rengine::integration::ProjectWorkspace;
    using dmc::rengine::integration::WorkspaceContext;
    using dmc::rengine::integration::WorkspaceStatus;

    const std::vector<std::byte> pac_bytes{
        std::byte{'P'}, std::byte{'A'}, std::byte{'C'}, std::byte{0},
        std::byte{0}, std::byte{0}, std::byte{0}, std::byte{0}};
    const auto hits_bytes =
        dmc::rengine::tests::hits_fixture::make_minimal_hits();

    const auto parent = resource(
        "room/st001cfg.pac", "pac", 4096U, pac_bytes.size(), true);
    const auto hits = resource(
        "room/st001cfg_006.hits", "hits", 8192U, hits_bytes.size());

    ProjectWorkspace project;
    assert(project.add_evidence(EvidenceRecord{
        .id = "ev-hits-record-layout",
        .claim_id = "claim-hits-record-layout",
        .title = "HITS triangle-plane layout",
        .summary = "Synthetic ProjectWorkspace integration evidence.",
        .confidence = Confidence::confirmed,
        .locations = {},
        .tags = {"hits", "synthetic-test"},
        .supersedes = {},
    }));

    const WorkspaceContext stage_context{
        .stage_context = true,
        .menu_context = false,
        .evidence_context = true,
    };
    assert(project.create_session(ResourcePayload{
        .resource = parent,
        .bytes = pac_bytes,
        .diagnostics = {},
    }, stage_context));
    assert(project.create_session(ResourcePayload{
        .resource = hits,
        .bytes = hits_bytes,
        .diagnostics = {},
    }, stage_context));
    assert(project.session_count() == 2U);

    assert(project.connect_resources(
        parent.id, hits.id, ResourceRelation::contains, "PAC member 6: HITS"));

    const auto scan = RecordScanner::scan(
        std::span<const std::byte>{hits_bytes});
    assert(scan.ok());
    assert(scan.triangles.size() == 1U);
    assert(scan.cells.size() == 1U);
    assert(project.add_parser_diagnostics(hits.id, scan.diagnostics));

    auto document = build_binary_document(
        hits, std::span<const std::byte>{hits_bytes}, scan);
    assert(document.has_value());
    assert(project.attach_binary_document(hits.id, std::move(*document)));
    assert(project.link_evidence_claim(
        hits.id, "claim-hits-record-layout") == 1U);

    StageBundle stage(StageIdentity{
        .profile = "dmc3-hd",
        .stage_id = "st001",
        .display_name = "Stage 001",
        .exe_evidence_id = "ev-dmc3-stage-resource-table",
    });
    assert(stage.add(StageMember{
        .category = StageResourceCategory::collision,
        .resource = hits,
        .role = "hits-source-1-member-6",
    }));
    assert(project.attach_stage_bundle(stage) == 1U);

    assert(!project.enable_working_copy(parent.id));
    assert(project.enable_working_copy(hits.id));
    const auto original = hits_bytes[0x06U];
    assert(project.apply_edit(
        hits.id,
        EditOperation{
            .id = "project-edit",
            .base_revision = 0U,
            .offset = 0x06U,
            .expected = {original},
            .replacement = {std::byte{3}},
            .description = "Edit the local HITS working copy only.",
        },
        ToolTarget::stage_ops).applied);

    const auto* hits_session = project.find_session(hits.id);
    assert(hits_session != nullptr);
    assert(hits_session->status() == WorkspaceStatus::editable_dirty);
    assert(hits_session->source_payload().bytes == hits_bytes);
    assert(hits_session->working_copy() != nullptr);
    assert(hits_session->working_copy()->bytes()[0x06U] == std::byte{3});

    const auto parent_node = "resource:" + parent.id.canonical();
    const auto hits_node = "resource:" + hits.id.canonical();
    assert(project.graph().find(parent_node) != nullptr);
    assert(project.graph().find(hits_node) != nullptr);
    assert(project.graph().find("stage:dmc3-hd:st001") != nullptr);
    assert(project.graph().find("evidence:ev-hits-record-layout") != nullptr);
    assert(project.graph().nodes(ProjectNodeKind::working_copy).size() == 1U);
    assert(has_project_edge(
        project.graph(), parent_node, hits_node, ProjectEdgeKind::contains));

    assert(project.undo_last_edit(hits.id, ToolTarget::stage_ops));
    hits_session = project.find_session(hits.id);
    assert(hits_session != nullptr);
    assert(hits_session->status() == WorkspaceStatus::editable_clean);
    assert(hits_session->working_copy()->bytes() == hits_bytes);
    return 0;
}
