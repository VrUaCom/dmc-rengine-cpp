#include "dmc_rengine/evidence/json_import.hpp"
#include "dmc_rengine/gdspaces/working_copy.hpp"
#include "dmc_rengine/integration/project_workspace.hpp"
#include "dmc_rengine/item/runtime_request.hpp"
#include "dmc_rengine/item/workspace_view.hpp"

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

namespace {

[[nodiscard]] dmc::rengine::evidence::EvidencePacket load_packet(
    const char* path) {
    std::ifstream input(path, std::ios::binary);
    assert(input.good());
    std::ostringstream buffer;
    buffer << input.rdbuf();
    const auto imported =
        dmc::rengine::evidence::evidence_packet_from_json(buffer.str());
    assert(imported.ok());
    assert(imported.packet.has_value());
    return *imported.packet;
}

[[nodiscard]] bool has_guard(
    const dmc::rengine::item::RuntimeChangeRequest& request,
    const std::string& guard) {
    return std::find(
        request.required_guards.begin(),
        request.required_guards.end(),
        guard) != request.required_guards.end();
}

[[nodiscard]] bool has_diagnostic(
    const dmc::rengine::item::RuntimeChangeRequest& request,
    const std::string& code) {
    return std::any_of(
        request.diagnostics.begin(), request.diagnostics.end(),
        [&code](const auto& diagnostic) {
            return diagnostic.code == code;
        });
}

} // namespace

int main(int argc, char** argv) {
    using dmc::rengine::gdspaces::EditOperation;
    using dmc::rengine::gdspaces::ResourceId;
    using dmc::rengine::gdspaces::ResourcePayload;
    using dmc::rengine::gdspaces::ResourceRef;
    using dmc::rengine::gdspaces::ToolTarget;
    using dmc::rengine::integration::ProjectNodeKind;
    using dmc::rengine::integration::ProjectWorkspace;
    using dmc::rengine::item::RuntimeEvidenceKind;
    using dmc::rengine::item::RuntimeRequestStatus;
    using dmc::rengine::item::build_workspace_view;
    using dmc::rengine::item::make_inventory_limit_request;
    using dmc::rengine::item::make_slot_registration_request;

    assert(argc == 3);
    const auto phase12 = load_packet(argv[1]);
    const auto item_packet = load_packet(argv[2]);

    ProjectWorkspace project;
    assert(project.import_evidence_packet(phase12));
    assert(project.import_evidence_packet(item_packet));
    assert(project.artifacts().size() == 1U);
    assert(project.packets().size() == 2U);
    assert(project.evidence().size() == 10U);
    assert(project.graph().nodes(ProjectNodeKind::evidence_packet).size() == 2U);
    assert(project.graph().nodes(ProjectNodeKind::artifact).size() == 1U);
    assert(project.graph().nodes(ProjectNodeKind::evidence_record).size() == 10U);

    const std::vector<std::byte> itm_bytes{
        std::byte{'I'}, std::byte{'T'}, std::byte{'M'}, std::byte{0},
        std::byte{1}, std::byte{0}, std::byte{0}, std::byte{0},
        std::byte{17}, std::byte{0}, std::byte{0}, std::byte{0},
        std::byte{0}, std::byte{0}, std::byte{0}, std::byte{0},
    };
    const ResourceRef item_resource{
        .id = ResourceId{
            .source_id = "item-workspace-test",
            .logical_path = "id/item017.itm",
            .container_chain = "NBZ[0]/PAC[17]",
            .offset = 4096U,
            .size = itm_bytes.size(),
        },
        .display_name = "item017.itm",
        .format = "itm",
        .profile = "dmc3-hd",
        .synthetic_name = false,
        .container = false,
    };
    assert(project.create_session(ResourcePayload{
        .resource = item_resource,
        .bytes = itm_bytes,
        .diagnostics = {},
    }));

    auto view = build_workspace_view(project, item_resource.id);
    assert(view.valid());
    assert(view.resource.id == item_resource.id);
    assert(view.has_route("item-editor"));
    assert(view.has_route("binary-inspector"));
    assert(view.has_route("evidence-registry"));
    assert(view.has_route("build-test-lab"));
    assert(!view.has_route("exe-editor"));
    assert(view.runtime_evidence.size() == 3U);

    const auto* slot_evidence = view.evidence(
        RuntimeEvidenceKind::registry_slot_guard);
    assert(slot_evidence != nullptr);
    assert(slot_evidence->record_id == "ev-dmc3-item-slot50-guard");
    assert(slot_evidence->location_complete);

    const auto* item_ids = view.evidence(
        RuntimeEvidenceKind::game_verified_item_ids);
    assert(item_ids != nullptr);
    assert(!item_ids->location_complete);

    const auto* inventory = view.evidence(
        RuntimeEvidenceKind::inventory_limit_patch_family);
    assert(inventory != nullptr);
    assert(!inventory->location_complete);

    constexpr auto target_artifact = "dmc3-hdc-exe-e454272e";
    const auto slot_request = make_slot_registration_request(
        view, 50U, target_artifact);
    assert(slot_request.valid());
    assert(slot_request.status ==
        RuntimeRequestStatus::ready_for_patch_planning);
    assert(slot_request.producer == ToolTarget::item_editor);
    assert(slot_request.consumer == ToolTarget::exe_editor);
    assert(slot_request.validators.size() == 2U);
    assert(slot_request.evidence_record_ids.size() == 1U);
    assert(slot_request.evidence_record_ids[0] ==
        "ev-dmc3-item-slot50-guard");
    assert(has_guard(slot_request, "target-artifact-sha256"));
    assert(has_guard(slot_request, "slot-50-source-bytes-00-01-08-13"));
    assert(has_guard(slot_request, "registry-capacity-check"));

    const auto outside_slot = make_slot_registration_request(
        view, 49U, target_artifact);
    assert(outside_slot.status == RuntimeRequestStatus::rejected);
    assert(has_diagnostic(
        outside_slot,
        "item-request.slot-outside-confirmed-extension-range"));

    const auto inventory_127 = make_inventory_limit_request(
        view, 127U, target_artifact);
    assert(inventory_127.valid());
    assert(inventory_127.status == RuntimeRequestStatus::research_required);
    assert(has_diagnostic(
        inventory_127,
        "item-request.inventory-locations-incomplete"));
    assert(has_guard(inventory_127, "imm8-1-127-direct-range"));

    const auto inventory_200 = make_inventory_limit_request(
        view, 200U, target_artifact);
    assert(inventory_200.status == RuntimeRequestStatus::research_required);
    assert(has_guard(
        inventory_200,
        "imm8-128-255-encoding-review"));

    const auto inventory_256 = make_inventory_limit_request(
        view, 256U, target_artifact);
    assert(inventory_256.status == RuntimeRequestStatus::rejected);
    assert(has_diagnostic(
        inventory_256,
        "item-request.inventory-limit-out-of-range"));

    assert(project.enable_working_copy(item_resource.id));
    assert(project.apply_edit(
        item_resource.id,
        EditOperation{
            .id = "item-y-rotation-test",
            .base_revision = 0U,
            .offset = 12U,
            .expected = {
                std::byte{0}, std::byte{0}, std::byte{0}, std::byte{0}},
            .replacement = {
                std::byte{0}, std::byte{0}, std::byte{52}, std::byte{66}},
            .description = "Synthetic Y rotation working-copy edit.",
        },
        ToolTarget::item_editor).applied);
    view = build_workspace_view(project, item_resource.id);
    assert(view.dirty);
    assert(view.working_copy_revision == 1U);

    const auto revised_request = make_slot_registration_request(
        view, 50U, target_artifact);
    assert(revised_request.item_revision == 1U);
    assert(revised_request.id.find("-r1") != std::string::npos);
    assert(project.request_validation(
        item_resource.id,
        ToolTarget::item_editor,
        revised_request.id,
        "Validate Item Editor runtime request through EXE Editor and Build & Test Lab."));

    return 0;
}
