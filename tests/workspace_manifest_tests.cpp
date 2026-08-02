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
            std::bit_cast<std::uint32_t>(static_cast<float>(index)));
    }
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
            .logical_path = "room/st001cfg_006.hits",
            .container_chain = "NBZ[0]/PAC[4]",
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
        .role = "room-collision-hits",
    }));
    assert(workspace.attach_stage_bundle(stage));
    assert(workspace.enable_working_copy());
    assert(workspace.apply_edit(
        EditOperation{
            .id = "manifest-edit",
            .base_revision = 0U,
            .offset = 6U,
            .expected = {std::byte{0}},
            .replacement = {std::byte{1}},
            .description = "Create a dirty working-copy state for the manifest.",
        },
        ToolTarget::stage_ops).applied);
    assert(workspace.request_validation(
        ToolTarget::stage_ops,
        "manifest-edit",
        "Validate the synthetic HITS edit."));
    assert(workspace.record_manifest_exported(
        ToolTarget::gdspaces,
        "workspace-manifest-test"));

    const auto json = workspace_manifest_json(workspace);
    assert(!json.empty());
    const auto parsed = Parser::parse(json);
    assert(parsed.ok());

    const auto* root = parsed.value->as_object();
    assert(root != nullptr);
    const auto* schema = member(*root, "schema_version");
    assert(schema != nullptr);
    assert(schema->as_u64() != nullptr);
    assert(*schema->as_u64() == 1U);

    const auto* status = member(*root, "workspace_status");
    assert(status != nullptr);
    assert(status->as_string() != nullptr);
    assert(*status->as_string() == "editable-dirty");

    const auto* routes = member(*root, "tool_routes");
    assert(routes != nullptr);
    assert(routes->as_array() != nullptr);
    assert(routes->as_array()->size() >= 7U);

    const auto* format = member(*root, "format_integration");
    assert(format != nullptr);
    const auto* format_object = format->as_object();
    assert(format_object != nullptr);
    const auto* policy = member(*format_object, "write_policy");
    assert(policy != nullptr);
    assert(policy->as_string() != nullptr);
    assert(*policy->as_string() == "working-copy-only");

    const auto* binary = member(*root, "binary_document");
    assert(binary != nullptr);
    const auto* binary_object = binary->as_object();
    assert(binary_object != nullptr);
    const auto* fields = member(*binary_object, "field_count");
    assert(fields != nullptr);
    assert(fields->as_u64() != nullptr);
    assert(*fields->as_u64() == 16U);

    const auto* working_copy = member(*root, "working_copy");
    assert(working_copy != nullptr);
    const auto* working_object = working_copy->as_object();
    assert(working_object != nullptr);
    const auto* dirty = member(*working_object, "dirty");
    assert(dirty != nullptr);
    assert(dirty->as_bool() != nullptr);
    assert(*dirty->as_bool());

    const auto* events = member(*root, "events");
    assert(events != nullptr);
    const auto* event_array = events->as_array();
    assert(event_array != nullptr);
    assert(event_array->size() == workspace.events().size());
    assert(event_array->size() >= 8U);

    const auto* first_event = (*event_array)[0].as_object();
    assert(first_event != nullptr);
    const auto* first_sequence = member(*first_event, "sequence");
    assert(first_sequence != nullptr);
    assert(first_sequence->as_u64() != nullptr);
    assert(*first_sequence->as_u64() == 1U);
    const auto* first_type = member(*first_event, "type");
    assert(first_type != nullptr);
    assert(first_type->as_string() != nullptr);
    assert(*first_type->as_string() == "workspace-created");

    const auto* final_event = event_array->back().as_object();
    assert(final_event != nullptr);
    const auto* final_type = member(*final_event, "type");
    assert(final_type != nullptr);
    assert(final_type->as_string() != nullptr);
    assert(*final_type->as_string() == "manifest-exported");

    const auto second = workspace_manifest_json(workspace);
    assert(second == json);
    return 0;
}
