#include "dmc_rengine/evidence/record.hpp"
#include "dmc_rengine/formats/hits.hpp"
#include "dmc_rengine/formats/hits_binary.hpp"
#include "dmc_rengine/integration/project_workspace.hpp"

#include <algorithm>
#include <bit>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <span>
#include <string>
#include <string_view>
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

[[nodiscard]] std::vector<std::byte> make_hits() {
    std::vector<std::byte> bytes(64U, std::byte{0});
    bytes[0] = std::byte{'H'};
    bytes[1] = std::byte{'I'};
    bytes[2] = std::byte{'T'};
    bytes[3] = std::byte{'S'};
    bytes[4] = std::byte{'$'};
    write_u32(bytes, 8U, dmc::rengine::formats::hits::record_marker);
    for (std::size_t index = 0;
         index < dmc::rengine::formats::hits::value_count;
         ++index) {
        write_u32(
            bytes,
            12U + index * 4U,
            std::bit_cast<std::uint32_t>(5.0F + static_cast<float>(index)));
    }
    return bytes;
}

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
    using dmc::rengine::integration::WorkspaceEventType;
    using dmc::rengine::integration::WorkspaceStatus;

    const std::vector<std::byte> pac_bytes{
        std::byte{'P'}, std::byte{'A'}, std::byte{'C'}, std::byte{0},
        std::byte{0}, std::byte{0}, std::byte{0}, std::byte{0}};
    const auto hits_bytes = make_hits();
    const std::vector<std::byte> txt_bytes{
        std::byte{'#'}, std::byte{'S'}, std::byte{'E'}, std::byte{'T'},
        std::byte{' '}, std::byte{'S'}, std::byte{'T'}, std::byte{'A'},
        std::byte{'Y'}};

    const auto parent = resource(
        "room/st001cfg.pac", "pac", 4096U, pac_bytes.size(), true);
    const auto hits = resource(
        "room/st001cfg_006.hits", "hits", 8192U, hits_bytes.size());
    const auto txt = resource(
        "room/st001cfg_004.txt", "txt", 12288U, txt_bytes.size());

    ProjectWorkspace project;
    assert(project.add_evidence(EvidenceRecord{
        .id = "ev-hits-record-layout",
        .claim_id = "claim-hits-record-layout",
        .title = "HITS raw record layout",
        .summary = "Synthetic ProjectWorkspace integration evidence.",
        .confidence = Confidence::confirmed,
        .locations = {},
        .tags = {"hits", "synthetic-test"},
        .supersedes = {},
    }));
    assert(project.add_evidence(EvidenceRecord{
        .id = "ev-dmc3-stageset-token-classifier",
        .claim_id = "claim-dmc3-stageset-token-classifier",
        .title = "StageSet token classifier",
        .summary = "Synthetic registry entry matching the canonical public claim ID.",
        .confidence = Confidence::confirmed,
        .locations = {},
        .tags = {"txt", "stage"},
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
    assert(project.create_session(ResourcePayload{
        .resource = txt,
        .bytes = txt_bytes,
        .diagnostics = {},
    }, stage_context));
    assert(project.session_count() == 3U);
    assert(!project.create_session(ResourcePayload{
        .resource = hits,
        .bytes = hits_bytes,
        .diagnostics = {},
    }, stage_context));

    assert(project.connect_resources(
        parent.id, hits.id, ResourceRelation::contains, "PAC slot: HITS"));
    assert(project.connect_resources(
        parent.id, txt.id, ResourceRelation::contains, "PAC slot: TXT"));
    assert(project.resources().resource_count() == 3U);
    assert(project.resources().edge_count() == 2U);
    assert(project.resources().related(
        parent.id, ResourceRelation::contains).size() == 2U);

    const auto scan = RecordScanner::scan(
        std::span<const std::byte>{hits_bytes});
    assert(scan.ok());
    assert(project.add_parser_diagnostics(hits.id, scan.diagnostics));
    auto document = build_binary_document(
        hits,
        std::span<const std::byte>{hits_bytes},
        scan);
    assert(document.has_value());
    assert(project.attach_binary_document(hits.id, std::move(*document)));
    assert(project.link_evidence_claim(
        hits.id, "claim-hits-record-layout") == 1U);
    assert(project.link_format_evidence(txt.id) == 1U);

    StageBundle stage(StageIdentity{
        .profile = "dmc3-hd",
        .stage_id = "st001",
        .display_name = "Stage 001",
        .exe_evidence_id = "ev-dmc3-stage-resource-table",
    });
    assert(stage.add(StageMember{
        .category = StageResourceCategory::collision,
        .resource = hits,
        .role = "room-collision-hits",
    }));
    assert(stage.add(StageMember{
        .category = StageResourceCategory::scripts,
        .resource = txt,
        .role = "room-config-script",
    }));
    assert(project.attach_stage_bundle(stage) == 2U);
    assert(project.sessions_for_stage("st001").size() == 2U);

    assert(!project.enable_working_copy(parent.id));
    assert(project.enable_working_copy(hits.id));
    assert(project.apply_edit(
        hits.id,
        EditOperation{
            .id = "project-edit",
            .base_revision = 0U,
            .offset = 6U,
            .expected = {std::byte{0}},
            .replacement = {std::byte{3}},
            .description = "Edit HITS working copy through ProjectWorkspace.",
        },
        ToolTarget::stage_ops).applied);
    assert(project.request_validation(
        hits.id,
        ToolTarget::stage_ops,
        "project-edit",
        "Validate the project-level HITS edit."));
    assert(project.record_manifest_exported(
        hits.id,
        ToolTarget::gdspaces,
        "project-st001-hits-manifest"));

    const auto* hits_session = project.find_session(hits.id);
    assert(hits_session != nullptr);
    assert(hits_session->status() == WorkspaceStatus::editable_dirty);
    assert(hits_session->source_payload().bytes == hits_bytes);
    assert(hits_session->working_copy() != nullptr);
    assert(hits_session->working_copy()->bytes()[6U] == std::byte{3});
    assert(hits_session->events().by_type(
        WorkspaceEventType::edit_applied).size() == 1U);

    const auto* parent_session = project.find_session(parent.id.canonical());
    assert(parent_session != nullptr);
    assert(parent_session->status() == WorkspaceStatus::read_only);

    const auto parent_node = "resource:" + parent.id.canonical();
    const auto hits_node = "resource:" + hits.id.canonical();
    const auto txt_node = "resource:" + txt.id.canonical();
    assert(project.graph().find(parent_node) != nullptr);
    assert(project.graph().find(hits_node) != nullptr);
    assert(project.graph().find(txt_node) != nullptr);
    assert(has_project_edge(
        project.graph(), parent_node, hits_node, ProjectEdgeKind::contains));
    assert(has_project_edge(
        project.graph(), parent_node, txt_node, ProjectEdgeKind::contains));
    assert(project.graph().find("stage:dmc3-hd:st001") != nullptr);
    assert(project.graph().find("evidence:ev-hits-record-layout") != nullptr);
    assert(project.graph().find(
        "evidence:ev-dmc3-stageset-token-classifier") != nullptr);
    assert(project.graph().find(
        "manifest:project-st001-hits-manifest") != nullptr);
    assert(project.graph().nodes(ProjectNodeKind::resource).size() == 3U);
    assert(project.graph().nodes(ProjectNodeKind::stage).size() == 1U);
    assert(project.graph().nodes(ProjectNodeKind::working_copy).size() == 1U);

    assert(project.undo_last_edit(hits.id, ToolTarget::stage_ops));
    hits_session = project.find_session(hits.id);
    assert(hits_session != nullptr);
    assert(hits_session->status() == WorkspaceStatus::editable_clean);
    assert(hits_session->working_copy()->bytes() == hits_bytes);

    assert(!project.connect_resources(
        parent.id, hits.id, ResourceRelation::contains, "duplicate"));
    return 0;
}
