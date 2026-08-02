#include "dmc_rengine/evidence/registry.hpp"
#include "dmc_rengine/formats/hits.hpp"
#include "dmc_rengine/formats/hits_binary.hpp"
#include "dmc_rengine/integration/format_registry.hpp"
#include "dmc_rengine/integration/resource_workspace.hpp"
#include "dmc_rengine/integration/tool_registry.hpp"

#include <algorithm>
#include <bit>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <span>
#include <string>
#include <vector>

namespace {

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
using dmc::rengine::integration::ResourceWritePolicy;
using dmc::rengine::integration::ToolCapability;
using dmc::rengine::integration::ToolRegistry;
using dmc::rengine::integration::ToolRouteRole;
using dmc::rengine::integration::WorkspaceContext;
using dmc::rengine::integration::WorkspaceEventType;
using dmc::rengine::integration::WorkspaceStatus;

void write_u32(
    std::vector<std::byte>& bytes,
    std::size_t offset,
    std::uint32_t value) {
    for (std::size_t index = 0; index < 4U; ++index) {
        bytes[offset + index] = static_cast<std::byte>(
            (value >> static_cast<unsigned>(index * 8U)) & 0xFFU);
    }
}

void write_record(
    std::vector<std::byte>& bytes,
    std::size_t offset,
    float base) {
    write_u32(bytes, offset, dmc::rengine::formats::hits::record_marker);
    for (std::size_t index = 0;
         index < dmc::rengine::formats::hits::value_count;
         ++index) {
        write_u32(
            bytes,
            offset + 4U + index * 4U,
            std::bit_cast<std::uint32_t>(base + static_cast<float>(index)));
    }
}

[[nodiscard]] std::vector<std::byte> hits_bytes() {
    std::vector<std::byte> bytes(64U, std::byte{0});
    bytes[0] = std::byte{'H'};
    bytes[1] = std::byte{'I'};
    bytes[2] = std::byte{'T'};
    bytes[3] = std::byte{'S'};
    bytes[4] = std::byte{'$'};
    write_record(bytes, 8U, 1.0F);
    return bytes;
}

[[nodiscard]] ResourceRef resource(
    std::string path,
    std::string format,
    std::uint64_t size,
    bool container = false) {
    return ResourceRef{
        .id = ResourceId{
            .source_id = "synthetic-stage",
            .logical_path = std::move(path),
            .container_chain = "NBZ[0]/PAC[4]",
            .offset = 8192U,
            .size = size,
        },
        .display_name = "collision.hits",
        .format = std::move(format),
        .profile = "dmc3-hd",
        .synthetic_name = false,
        .container = container,
    };
}

[[nodiscard]] bool has_route(
    const ResourceWorkspaceSession& workspace,
    ToolTarget target,
    std::optional<ToolRouteRole> role = std::nullopt) {
    return std::any_of(
        workspace.routes().begin(), workspace.routes().end(),
        [target, role](const auto& route) {
            return route.target == target &&
                   (!role.has_value() || route.role == *role);
        });
}

void test_tool_and_format_registries() {
    const ToolRegistry tools;
    assert(tools.tools().size() == 10U);

    const auto* archive = tools.find(ToolTarget::gdspaces);
    assert(archive != nullptr);
    assert(archive->product_name == "GDSpaces");
    assert(archive->lore_name == "The Archive");
    assert(archive->supports(ToolCapability::browse_resource));
    assert(archive->supports(ToolCapability::create_working_copy));

    const auto* binary = tools.find(ToolTarget::binary_inspector);
    assert(binary != nullptr);
    assert(binary->product_name == "Binary Inspector");
    assert(binary->lore_name == "The Reliquary");
    assert(binary->supports(ToolCapability::inspect_binary));
    assert(!binary->supports(ToolCapability::create_working_copy));

    const FormatIntegrationRegistry formats;
    const auto* hits = formats.find("HITS");
    assert(hits != nullptr);
    assert(hits->parser_id == "formats.hits-record-scanner");
    assert(hits->binary_adapter);
    assert(hits->stage_category == StageResourceCategory::collision);
    assert(hits->write_policy == ResourceWritePolicy::working_copy_only);
    assert(hits->allows_working_copy());
    assert(!hits->allows_guarded_export());

    const auto* pac = formats.find("pac");
    assert(pac != nullptr);
    assert(pac->write_policy == ResourceWritePolicy::read_only);
    assert(!pac->allows_working_copy());
}

void test_hits_full_workspace_flow() {
    const ToolRegistry tools;
    const FormatIntegrationRegistry formats;
    const auto bytes = hits_bytes();
    const auto ref = resource("room/st001cfg_006.hits", "hits", bytes.size());
    const ResourcePayload payload{
        .resource = ref,
        .bytes = bytes,
        .diagnostics = {},
    };

    ResourceWorkspaceSession workspace(
        payload,
        tools,
        formats,
        WorkspaceContext{
            .stage_context = true,
            .menu_context = false,
            .evidence_context = true,
        });
    assert(workspace.valid());
    assert(workspace.status() == WorkspaceStatus::read_only);
    assert(workspace.format() != nullptr);
    assert(workspace.format()->format == "hits");
    assert(workspace.events().size() == 2U);
    assert(workspace.events().by_type(
        WorkspaceEventType::workspace_created).size() == 1U);
    assert(workspace.events().by_type(
        WorkspaceEventType::routes_resolved).size() == 1U);

    assert(has_route(
        workspace,
        ToolTarget::stage_ops,
        ToolRouteRole::primary));
    assert(has_route(workspace, ToolTarget::gdspaces));
    assert(has_route(workspace, ToolTarget::binary_inspector));
    assert(has_route(workspace, ToolTarget::modviz_scene));
    assert(has_route(workspace, ToolTarget::evidence_registry));
    assert(has_route(workspace, ToolTarget::spider_hub));
    assert(has_route(workspace, ToolTarget::build_test_lab));

    const auto scan = RecordScanner::scan(
        std::span<const std::byte>{payload.bytes});
    assert(scan.ok());
    assert(workspace.add_parser_diagnostics(scan.diagnostics));

    auto document = build_binary_document(
        ref,
        std::span<const std::byte>{payload.bytes},
        scan);
    assert(document.has_value());
    assert(workspace.attach_binary_document(std::move(*document)));
    assert(workspace.binary_document() != nullptr);
    assert(workspace.binary_document()->regions().size() == 2U);
    assert(workspace.binary_document()->fields().size() == 16U);
    assert(workspace.events().by_type(
        WorkspaceEventType::binary_document_attached).size() == 1U);

    EvidenceRegistry evidence;
    assert(evidence.add(EvidenceRecord{
        .id = "ev-hits-record-layout",
        .claim_id = "claim-hits-record-layout",
        .title = "HITS raw record layout",
        .summary = "Synthetic evidence link used to verify workspace relationships.",
        .confidence = Confidence::confirmed,
        .locations = {},
        .tags = {"hits", "record", "synthetic-test"},
        .supersedes = {},
    }));
    assert(workspace.link_evidence_claim(
        evidence, "claim-hits-record-layout") == 1U);
    assert(workspace.evidence_record_ids().size() == 1U);
    assert(workspace.evidence_record_ids()[0] == "ev-hits-record-layout");
    assert(workspace.events().by_type(
        WorkspaceEventType::evidence_record_linked).size() == 1U);

    StageBundle bundle(StageIdentity{
        .profile = "dmc3-hd",
        .stage_id = "st001",
        .display_name = "Stage 001",
        .exe_evidence_id = "ev-dmc3-stage-resource-table",
    });
    assert(bundle.add(StageMember{
        .category = StageResourceCategory::collision,
        .resource = ref,
        .role = "room-collision-hits",
    }));
    assert(workspace.attach_stage_bundle(bundle));
    assert(workspace.stage() != nullptr);
    assert(workspace.stage()->identity.stage_id == "st001");
    assert(workspace.stage()->category == StageResourceCategory::collision);
    assert(workspace.stage()->role == "room-collision-hits");
    assert(workspace.events().by_type(
        WorkspaceEventType::stage_context_attached).size() == 1U);

    assert(workspace.enable_working_copy());
    assert(workspace.status() == WorkspaceStatus::editable_clean);
    assert(workspace.working_copy() != nullptr);
    assert(workspace.working_copy()->bytes() == payload.bytes);
    assert(workspace.events().by_type(
        WorkspaceEventType::working_copy_enabled).size() == 1U);

    const auto original_source_byte = workspace.source_payload().bytes[6U];
    const auto rejected = workspace.apply_edit(
        EditOperation{
            .id = "rejected-edit",
            .base_revision = 0U,
            .offset = 6U,
            .expected = {std::byte{0}},
            .replacement = {std::byte{0x55}},
            .description = "An unrouted tool must not edit this workspace.",
        },
        ToolTarget::item_editor);
    assert(!rejected.applied);

    const auto edit = workspace.apply_edit(
        EditOperation{
            .id = "edit-unknown-padding",
            .base_revision = 0U,
            .offset = 6U,
            .expected = {std::byte{0}},
            .replacement = {std::byte{0x7F}},
            .description = "Modify unknown padding in the local working copy only.",
        },
        ToolTarget::stage_ops);
    assert(edit.applied);
    assert(workspace.status() == WorkspaceStatus::editable_dirty);
    assert(workspace.source_payload().bytes[6U] == original_source_byte);
    assert(workspace.working_copy()->bytes()[6U] == std::byte{0x7F});
    assert(!workspace.format()->allows_guarded_export());
    assert(workspace.events().by_type(
        WorkspaceEventType::edit_applied).size() == 1U);

    assert(workspace.request_validation(
        ToolTarget::stage_ops,
        "edit-unknown-padding",
        "Validate the HITS working-copy change before any future export."));
    assert(workspace.events().by_type(
        WorkspaceEventType::validation_requested).size() == 1U);
    assert(workspace.events().by_tool(
        ToolTarget::build_test_lab).size() >= 2U);

    assert(workspace.record_manifest_exported(
        ToolTarget::gdspaces,
        "workspace-manifest-st001-hits"));
    assert(workspace.events().by_type(
        WorkspaceEventType::manifest_exported).size() == 1U);

    assert(workspace.undo_last_edit(ToolTarget::stage_ops));
    assert(workspace.status() == WorkspaceStatus::editable_clean);
    assert(workspace.working_copy()->bytes() == payload.bytes);
    assert(workspace.events().by_type(
        WorkspaceEventType::edit_undone).size() == 1U);

    const auto* latest = workspace.events().latest();
    assert(latest != nullptr);
    assert(latest->type == WorkspaceEventType::edit_undone);
    assert(latest->revision == 2U);

    for (std::size_t index = 0; index < workspace.events().events().size(); ++index) {
        assert(workspace.events().events()[index].sequence == index + 1U);
        assert(workspace.events().events()[index].resource == ref.id);
        assert(workspace.events().events()[index].valid());
    }
}

void test_read_only_format_policy() {
    const ToolRegistry tools;
    const FormatIntegrationRegistry formats;
    const std::vector<std::byte> bytes{
        std::byte{'P'}, std::byte{'A'}, std::byte{'C'}, std::byte{0}};
    ResourceWorkspaceSession workspace(
        ResourcePayload{
            .resource = resource("room/st001cfg.pac", "pac", bytes.size(), true),
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

    assert(workspace.valid());
    assert(workspace.status() == WorkspaceStatus::read_only);
    assert(!workspace.enable_working_copy());
    assert(workspace.working_copy() == nullptr);
    assert(workspace.status() == WorkspaceStatus::read_only);
}

} // namespace

int main() {
    test_tool_and_format_registries();
    test_hits_full_workspace_flow();
    test_read_only_format_policy();
    return 0;
}
