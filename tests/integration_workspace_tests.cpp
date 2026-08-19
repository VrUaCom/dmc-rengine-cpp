#include "dmc_rengine/evidence/registry.hpp"
#include "dmc_rengine/formats/hits.hpp"
#include "dmc_rengine/formats/hits_binary.hpp"
#include "dmc_rengine/formats/pac.hpp"
#include "dmc_rengine/formats/pnst.hpp"
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
#include <string_view>
#include <vector>

namespace {

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
using dmc::rengine::integration::IntegrationMaturity;
using dmc::rengine::integration::ResourceWorkspaceSession;
using dmc::rengine::integration::ResourceWritePolicy;
using dmc::rengine::integration::ToolCapability;
using dmc::rengine::integration::ToolRegistry;
using dmc::rengine::integration::ToolRouteRole;
using dmc::rengine::integration::WorkspaceContext;
using dmc::rengine::integration::WorkspaceEventType;
using dmc::rengine::integration::WorkspaceStatus;

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

[[nodiscard]] std::vector<std::byte> hits_bytes() {
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

[[nodiscard]] std::vector<std::byte> relative_slot_bytes(
    std::string_view magic) {
    assert(magic.size() == 4U);
    std::vector<std::byte> bytes(16U, std::byte{0});
    for (std::size_t index = 0U; index < 4U; ++index) {
        bytes[index] = static_cast<std::byte>(magic[index]);
    }
    write_u32(bytes, 4U, 1U);
    write_u32(bytes, 8U, 12U);
    bytes[12U] = std::byte{'D'};
    bytes[13U] = std::byte{'D'};
    bytes[14U] = std::byte{'S'};
    bytes[15U] = std::byte{' '};
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
            .container_chain = "NBZ[0]/PAC[3]",
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
    return std::any_of(workspace.routes().begin(), workspace.routes().end(),
        [target, role](const auto& route) {
            return route.target == target &&
                (!role.has_value() || route.role == *role);
        });
}

[[nodiscard]] ResourceWorkspaceSession make_relative_slot_workspace(
    const ToolRegistry& tools,
    const FormatIntegrationRegistry& formats,
    std::string path,
    std::string format,
    std::vector<std::byte> bytes) {
    const auto size = static_cast<std::uint64_t>(bytes.size());
    return ResourceWorkspaceSession(
        ResourcePayload{
            .resource = resource(std::move(path), std::move(format), size, true),
            .bytes = std::move(bytes),
            .diagnostics = {},
        },
        tools,
        formats,
        WorkspaceContext{.stage_context = true, .evidence_context = true});
}

void test_registries() {
    const ToolRegistry tools;
    const auto* archive = tools.find(ToolTarget::gdspaces);
    assert(archive != nullptr);
    assert(archive->supports(ToolCapability::browse_resource));
    const auto* binary = tools.find(ToolTarget::binary_inspector);
    assert(binary != nullptr);
    assert(binary->supports(ToolCapability::inspect_binary));

    const FormatIntegrationRegistry formats;
    const auto* hits = formats.find("HITS");
    assert(hits != nullptr);
    assert(hits->parser_id == "formats.hits-record-scanner");
    assert(hits->binary_adapter);
    assert(hits->stage_category == StageResourceCategory::collision);
    assert(hits->write_policy == ResourceWritePolicy::working_copy_only);
    assert(!hits->parser_validation_required);

    const auto* pac = formats.find("PAC");
    assert(pac != nullptr);
    assert(pac->valid());
    assert(pac->maturity == IntegrationMaturity::structural);
    assert(pac->parser_id == "dmc3-pac-structural-v1");
    assert(pac->write_policy == ResourceWritePolicy::working_copy_only);
    assert(pac->parser_validation_required);
    assert(pac->allows_writer_mode("layout-preserving-packed"));

    const auto* pnst = formats.find("PNST");
    assert(pnst != nullptr);
    assert(pnst->valid());
    assert(pnst->parser_id == "dmc3-pnst-structural-v1");
    assert(pnst->parser_validation_required);
    assert(pnst->allows_writer_mode("layout-preserving-packed"));

    const auto* nbz = formats.find("NBZ");
    assert(nbz != nullptr);
    assert(nbz->valid());
    assert(nbz->maturity == IntegrationMaturity::structural);
    assert(nbz->parser_id.empty());
    assert(nbz->source_adapter_id == "gdspaces.nbz-zip-source-v1");
    assert(nbz->write_policy == ResourceWritePolicy::read_only);
    assert(nbz->allows_writer_mode("store-overlay-nbz"));

    const auto* afs = formats.find("AFS");
    assert(afs != nullptr);
    assert(afs->maturity == IntegrationMaturity::recognized);
    assert(afs->parser_id.empty());
    assert(afs->source_adapter_id.empty());
}

void test_hits_workspace() {
    using dmc::rengine::formats::hits::RecordScanner;
    using dmc::rengine::formats::hits::build_binary_document;

    const ToolRegistry tools;
    const FormatIntegrationRegistry formats;
    const auto bytes = hits_bytes();
    const auto ref = resource("room/st001_003.ukn", "hits", bytes.size());
    ResourceWorkspaceSession workspace(
        ResourcePayload{.resource = ref, .bytes = bytes, .diagnostics = {}},
        tools,
        formats,
        WorkspaceContext{
            .stage_context = true,
            .menu_context = false,
            .evidence_context = true,
        });
    assert(workspace.valid());
    assert(workspace.status() == WorkspaceStatus::read_only);
    assert(has_route(workspace, ToolTarget::stage_ops, ToolRouteRole::primary));
    assert(has_route(workspace, ToolTarget::gdspaces));
    assert(has_route(workspace, ToolTarget::binary_inspector));

    const auto scan = RecordScanner::scan(bytes);
    assert(scan.ok());
    assert(scan.triangles.size() == 1U);
    assert(workspace.add_parser_diagnostics(scan.diagnostics));
    auto document = build_binary_document(ref, bytes, scan);
    assert(document.has_value());
    assert(workspace.attach_binary_document(std::move(*document)));
    assert(workspace.binary_document() != nullptr);
    assert(workspace.binary_document()->find_region("hits-header") != nullptr);
    assert(workspace.binary_document()->find_region(
        "hits-triangle-00000050") != nullptr);

    StageBundle bundle(StageIdentity{
        .profile = "dmc3-hd",
        .stage_id = "st001",
        .display_name = "Stage 001",
        .exe_evidence_id = "ev-dmc3-stage-resource-table",
    });
    assert(bundle.add(StageMember{
        .category = StageResourceCategory::collision,
        .resource = ref,
        .role = "stage-collision-source-0",
    }));
    assert(workspace.attach_stage_bundle(bundle));
    assert(workspace.stage() != nullptr);

    assert(workspace.enable_working_copy());
    assert(workspace.status() == WorkspaceStatus::editable_clean);
    const auto edit = workspace.apply_edit(
        EditOperation{
            .id = "edit-local-hits-byte",
            .base_revision = 0U,
            .offset = 0x54U,
            .expected = {std::byte{0}},
            .replacement = {std::byte{0x7F}},
            .description = "Modify local HITS working-copy bytes.",
        },
        ToolTarget::stage_ops);
    assert(edit.applied);
    assert(workspace.status() == WorkspaceStatus::editable_dirty);
    assert(workspace.source_payload().bytes[0x54U] == std::byte{0});
    assert(workspace.working_copy()->bytes()[0x54U] == std::byte{0x7F});
    assert(workspace.undo_last_edit(ToolTarget::stage_ops));
    assert(workspace.status() == WorkspaceStatus::editable_clean);
    assert(workspace.events().by_type(
        WorkspaceEventType::binary_document_attached).size() == 1U);
}

void test_pac_pnst_parser_gate() {
    const ToolRegistry tools;
    const FormatIntegrationRegistry formats;
    const auto pac_bytes = relative_slot_bytes(std::string_view{"PAC\0", 4U});
    assert(dmc::rengine::formats::PacParser::parse(pac_bytes).ok());

    auto no_receipt = make_relative_slot_workspace(
        tools, formats, "room/test.pac", "pac", pac_bytes);
    assert(no_receipt.valid());
    assert(has_route(no_receipt, ToolTarget::gdspaces));
    assert(!no_receipt.enable_working_copy());
    assert(no_receipt.working_copy() == nullptr);

    auto wrong_parser = make_relative_slot_workspace(
        tools, formats, "room/test.pac", "pac", pac_bytes);
    assert(!wrong_parser.record_parser_completed(
        "formats.synthetic-slot-container", true, ToolTarget::gdspaces));
    assert(wrong_parser.parser_validation() == nullptr);
    assert(!wrong_parser.enable_working_copy());

    auto not_recognized = make_relative_slot_workspace(
        tools, formats, "room/test.pac", "pac", pac_bytes);
    assert(not_recognized.record_parser_completed(
        "dmc3-pac-structural-v1", false, ToolTarget::gdspaces));
    assert(not_recognized.parser_validation() != nullptr);
    assert(!not_recognized.parser_validation()->recognized);
    assert(!not_recognized.enable_working_copy());

    auto validated = make_relative_slot_workspace(
        tools, formats, "room/test.pac", "pac", pac_bytes);
    assert(validated.record_parser_completed(
        "dmc3-pac-structural-v1", true, ToolTarget::gdspaces));
    assert(validated.parser_validation() != nullptr);
    assert(validated.parser_validation()->valid());
    assert(validated.parser_validation()->recognized);
    assert(!validated.parser_validation()->error_diagnostics);
    assert(validated.enable_working_copy());
    assert(validated.status() == WorkspaceStatus::editable_clean);

    auto later_error = make_relative_slot_workspace(
        tools, formats, "room/test.pac", "pac", pac_bytes);
    assert(later_error.record_parser_completed(
        "dmc3-pac-structural-v1", true, ToolTarget::gdspaces));
    const std::vector<dmc::rengine::formats::ParseDiagnostic> parser_errors{
        dmc::rengine::formats::ParseDiagnostic{
            .severity = dmc::rengine::formats::ParseSeverity::error,
            .code = "pac.synthetic-error",
            .message = "Synthetic parser error after completion.",
            .offset = 12U,
        },
    };
    assert(later_error.add_parser_diagnostics(parser_errors));
    assert(later_error.parser_validation()->error_diagnostics);
    assert(!later_error.enable_working_copy());

    const auto pnst_bytes = relative_slot_bytes("PNST");
    assert(dmc::rengine::formats::PnstParser::parse(pnst_bytes).ok());
    auto pnst = make_relative_slot_workspace(
        tools, formats, "room/test-pnst.pac", "pnst", pnst_bytes);
    assert(!pnst.record_parser_completed(
        "dmc3-pac-structural-v1", true, ToolTarget::gdspaces));
    assert(pnst.record_parser_completed(
        "dmc3-pnst-structural-v1", true, ToolTarget::gdspaces));
    assert(pnst.enable_working_copy());

    const std::vector<std::byte> malformed{
        std::byte{'P'}, std::byte{'A'}, std::byte{'C'}, std::byte{0}};
    assert(!dmc::rengine::formats::PacParser::parse(malformed).ok());
    auto malformed_workspace = make_relative_slot_workspace(
        tools, formats, "room/malformed.pac", "pac", malformed);
    assert(malformed_workspace.record_parser_completed(
        "dmc3-pac-structural-v1", false, ToolTarget::gdspaces));
    assert(!malformed_workspace.enable_working_copy());
}

} // namespace

int main() {
    test_registries();
    test_hits_workspace();
    test_pac_pnst_parser_gate();
    return 0;
}
