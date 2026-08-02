#include "dmc_rengine/core/json.hpp"
#include "dmc_rengine/evidence/record.hpp"
#include "dmc_rengine/formats/hits.hpp"
#include "dmc_rengine/formats/hits_binary.hpp"
#include "dmc_rengine/integration/project_workspace.hpp"
#include "dmc_rengine/integration/stage_workspace_manifest.hpp"

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
            std::bit_cast<std::uint32_t>(static_cast<float>(index + 1U)));
    }
    return bytes;
}

[[nodiscard]] dmc::rengine::gdspaces::ResourceRef resource(
    std::string path,
    std::string format,
    std::uint64_t offset,
    std::uint64_t size) {
    return dmc::rengine::gdspaces::ResourceRef{
        .id = dmc::rengine::gdspaces::ResourceId{
            .source_id = "stage-manifest-test",
            .logical_path = std::move(path),
            .container_chain = "NBZ[0]/PAC[4]",
            .offset = offset,
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
    using dmc::rengine::core::json::Parser;
    using dmc::rengine::evidence::Confidence;
    using dmc::rengine::evidence::EvidenceRecord;
    using dmc::rengine::formats::hits::RecordScanner;
    using dmc::rengine::formats::hits::build_binary_document;
    using dmc::rengine::gdspaces::EditOperation;
    using dmc::rengine::gdspaces::ResourcePayload;
    using dmc::rengine::gdspaces::StageBundle;
    using dmc::rengine::gdspaces::StageIdentity;
    using dmc::rengine::gdspaces::StageMember;
    using dmc::rengine::gdspaces::StageResourceCategory;
    using dmc::rengine::gdspaces::ToolTarget;
    using dmc::rengine::integration::ProjectWorkspace;
    using dmc::rengine::integration::WorkspaceContext;
    using dmc::rengine::integration::stage_workspace_manifest_json;

    const auto hits_bytes = make_hits();
    const std::vector<std::byte> txt_bytes{
        std::byte{'#'}, std::byte{'S'}, std::byte{'E'}, std::byte{'T'},
        std::byte{' '}, std::byte{'S'}, std::byte{'T'}, std::byte{'A'},
        std::byte{'Y'}};
    const auto hits = resource(
        "room/st001cfg_006.hits", "hits", 4096U, hits_bytes.size());
    const auto txt = resource(
        "room/st001cfg_004.txt", "txt", 8192U, txt_bytes.size());

    ProjectWorkspace project;
    assert(project.add_evidence(EvidenceRecord{
        .id = "ev-hits-layout",
        .claim_id = "claim-hits-layout",
        .title = "HITS layout",
        .summary = "Synthetic Stage Workspace Manifest evidence.",
        .confidence = Confidence::confirmed,
        .locations = {},
        .tags = {"hits"},
        .supersedes = {},
    }));
    assert(project.add_evidence(EvidenceRecord{
        .id = "ev-dmc3-stageset-token-classifier",
        .claim_id = "claim-dmc3-stageset-token-classifier",
        .title = "StageSet token classifier",
        .summary = "Synthetic Stage Workspace Manifest evidence.",
        .confidence = Confidence::confirmed,
        .locations = {},
        .tags = {"txt"},
        .supersedes = {},
    }));

    const WorkspaceContext context{
        .stage_context = true,
        .menu_context = false,
        .evidence_context = true,
    };
    assert(project.create_session(ResourcePayload{
        .resource = hits,
        .bytes = hits_bytes,
        .diagnostics = {},
    }, context));
    assert(project.create_session(ResourcePayload{
        .resource = txt,
        .bytes = txt_bytes,
        .diagnostics = {},
    }, context));

    const auto scan = RecordScanner::scan(
        std::span<const std::byte>{hits_bytes});
    assert(scan.ok());
    auto document = build_binary_document(
        hits,
        std::span<const std::byte>{hits_bytes},
        scan);
    assert(document.has_value());
    assert(project.attach_binary_document(hits.id, std::move(*document)));
    assert(project.link_evidence_claim(hits.id, "claim-hits-layout") == 1U);
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

    assert(project.enable_working_copy(hits.id));
    assert(project.apply_edit(
        hits.id,
        EditOperation{
            .id = "stage-manifest-edit",
            .base_revision = 0U,
            .offset = 6U,
            .expected = {std::byte{0}},
            .replacement = {std::byte{4}},
            .description = "Create dirty stage resource state.",
        },
        ToolTarget::stage_ops).applied);
    assert(project.request_validation(
        hits.id,
        ToolTarget::stage_ops,
        "stage-manifest-edit",
        "Validate stage resource edit."));

    const auto json = stage_workspace_manifest_json(project, "st001");
    assert(!json.empty());
    const auto parsed = Parser::parse(json);
    assert(parsed.ok());
    const auto* root = parsed.value->as_object();
    assert(root != nullptr);

    const auto* stage_value = member(*root, "stage");
    assert(stage_value != nullptr);
    const auto* stage_object = stage_value->as_object();
    assert(stage_object != nullptr);
    const auto* stage_id = member(*stage_object, "stage_id");
    assert(stage_id != nullptr);
    assert(stage_id->as_string() != nullptr);
    assert(*stage_id->as_string() == "st001");

    const auto* summary_value = member(*root, "summary");
    assert(summary_value != nullptr);
    const auto* summary = summary_value->as_object();
    assert(summary != nullptr);
    const auto* count = member(*summary, "resource_count");
    assert(count != nullptr);
    assert(count->as_u64() != nullptr);
    assert(*count->as_u64() == 2U);
    const auto* dirty = member(*summary, "dirty_resource_count");
    assert(dirty != nullptr);
    assert(dirty->as_u64() != nullptr);
    assert(*dirty->as_u64() == 1U);
    const auto* binary_count = member(*summary, "binary_document_count");
    assert(binary_count != nullptr);
    assert(binary_count->as_u64() != nullptr);
    assert(*binary_count->as_u64() == 1U);
    const auto* validation_count = member(*summary, "validation_request_count");
    assert(validation_count != nullptr);
    assert(validation_count->as_u64() != nullptr);
    assert(*validation_count->as_u64() == 1U);
    const auto* stage_ops = member(*summary, "stage_ops_route_consistent");
    assert(stage_ops != nullptr);
    assert(stage_ops->as_bool() != nullptr);
    assert(*stage_ops->as_bool());
    const auto* modviz = member(*summary, "modviz_scene_route_consistent");
    assert(modviz != nullptr);
    assert(modviz->as_bool() != nullptr);
    assert(*modviz->as_bool());

    const auto* categories_value = member(*root, "category_counts");
    assert(categories_value != nullptr);
    const auto* categories = categories_value->as_object();
    assert(categories != nullptr);
    assert(member(*categories, "collision") != nullptr);
    assert(member(*categories, "scripts") != nullptr);

    const auto* resources_value = member(*root, "resources");
    assert(resources_value != nullptr);
    const auto* resources = resources_value->as_array();
    assert(resources != nullptr);
    assert(resources->size() == 2U);

    const auto second = stage_workspace_manifest_json(project, "st001");
    assert(second == json);
    assert(stage_workspace_manifest_json(project, "st999").empty());
    return 0;
}
