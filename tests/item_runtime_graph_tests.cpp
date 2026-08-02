#include "dmc_rengine/evidence/json_import.hpp"
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

} // namespace

int main(int argc, char** argv) {
    using dmc::rengine::gdspaces::ResourceId;
    using dmc::rengine::gdspaces::ResourcePayload;
    using dmc::rengine::gdspaces::ResourceRef;
    using dmc::rengine::integration::ProjectEdgeKind;
    using dmc::rengine::integration::ProjectNodeKind;
    using dmc::rengine::integration::ProjectWorkspace;
    using dmc::rengine::integration::WorkspaceEventType;
    using dmc::rengine::item::RuntimeRequestStatus;
    using dmc::rengine::item::build_workspace_view;
    using dmc::rengine::item::make_inventory_limit_request;
    using dmc::rengine::item::make_slot_registration_request;

    assert(argc == 3);
    ProjectWorkspace project;
    assert(project.import_evidence_packet(load_packet(argv[1])));
    assert(project.import_evidence_packet(load_packet(argv[2])));

    const std::vector<std::byte> bytes{
        std::byte{'I'}, std::byte{'T'}, std::byte{'M'}, std::byte{0},
        std::byte{1}, std::byte{0}, std::byte{0}, std::byte{0},
        std::byte{17}, std::byte{0}, std::byte{0}, std::byte{0},
        std::byte{0}, std::byte{0}, std::byte{0}, std::byte{0},
    };
    const ResourceRef item_resource{
        .id = ResourceId{
            .source_id = "item-runtime-graph-test",
            .logical_path = "id/item017.itm",
            .container_chain = "NBZ[0]/PAC[17]",
            .offset = 4096U,
            .size = bytes.size(),
        },
        .display_name = "item017.itm",
        .format = "itm",
        .profile = "dmc3-hd",
        .synthetic_name = false,
        .container = false,
    };
    assert(project.create_session(ResourcePayload{
        .resource = item_resource,
        .bytes = bytes,
        .diagnostics = {},
    }));
    assert(project.enable_working_copy(item_resource.id));

    const auto view = build_workspace_view(project, item_resource.id);
    assert(view.valid());
    constexpr auto artifact = "dmc3-hdc-exe-e454272e";

    const auto slot_request = make_slot_registration_request(
        view, 50U, artifact);
    assert(slot_request.status ==
        RuntimeRequestStatus::ready_for_patch_planning);
    assert(project.register_item_runtime_request(slot_request));
    assert(!project.register_item_runtime_request(slot_request));

    const auto inventory_request = make_inventory_limit_request(
        view, 127U, artifact);
    assert(inventory_request.status ==
        RuntimeRequestStatus::research_required);
    assert(project.register_item_runtime_request(inventory_request));

    const auto rejected = make_slot_registration_request(
        view, 49U, artifact);
    assert(rejected.status == RuntimeRequestStatus::rejected);
    assert(!project.register_item_runtime_request(rejected));

    assert(project.graph().nodes(ProjectNodeKind::runtime_request).size() == 2U);
    const auto slot_node = "runtime-request:" + slot_request.id;
    const auto inventory_node = "runtime-request:" + inventory_request.id;
    assert(project.graph().find(slot_node) != nullptr);
    assert(project.graph().find(inventory_node) != nullptr);

    bool item_edge = false;
    bool artifact_edge = false;
    bool producer_edge = false;
    bool consumer_edge = false;
    bool evidence_edge = false;
    bool evidence_validator = false;
    bool build_validator = false;
    for (const auto& edge : project.graph().all_edges()) {
        if (edge.from != slot_node) {
            continue;
        }
        if (edge.to == "resource:" + item_resource.id.canonical() &&
            edge.kind == ProjectEdgeKind::derived_from) {
            item_edge = true;
        }
        if (edge.to == "artifact:dmc3-hdc-exe-e454272e" &&
            edge.kind == ProjectEdgeKind::requests_change) {
            artifact_edge = true;
        }
        if (edge.to == "tool:item-editor" &&
            edge.kind == ProjectEdgeKind::produced_by) {
            producer_edge = true;
        }
        if (edge.to == "tool:exe-editor" &&
            edge.kind == ProjectEdgeKind::delivered_to) {
            consumer_edge = true;
        }
        if (edge.to == "evidence:ev-dmc3-item-slot50-guard" &&
            edge.kind == ProjectEdgeKind::evidence_for) {
            evidence_edge = true;
        }
        if (edge.to == "tool:evidence-registry" &&
            edge.kind == ProjectEdgeKind::validates) {
            evidence_validator = true;
        }
        if (edge.to == "tool:build-test-lab" &&
            edge.kind == ProjectEdgeKind::validates) {
            build_validator = true;
        }
    }
    assert(item_edge);
    assert(artifact_edge);
    assert(producer_edge);
    assert(consumer_edge);
    assert(evidence_edge);
    assert(evidence_validator);
    assert(build_validator);

    const auto* session = project.find_session(item_resource.id);
    assert(session != nullptr);
    assert(session->events().by_type(
        WorkspaceEventType::runtime_change_requested).size() == 2U);
    assert(session->events().by_type(
        WorkspaceEventType::validation_requested).size() == 2U);
    assert(session->working_copy() != nullptr);
    assert(session->working_copy()->revision() == 0U);
    assert(!session->working_copy()->dirty());

    assert(project.graph().find(
        "runtime-request:" + rejected.id) == nullptr);
    return 0;
}
