#include "dmc_rengine/core/json.hpp"
#include "dmc_rengine/evidence/json_import.hpp"
#include "dmc_rengine/integration/project_workspace.hpp"
#include "dmc_rengine/item/runtime_request.hpp"
#include "dmc_rengine/item/workspace_manifest.hpp"
#include "dmc_rengine/item/workspace_view.hpp"

#include <cassert>
#include <cstddef>
#include <fstream>
#include <span>
#include <sstream>
#include <string_view>
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

[[nodiscard]] const dmc::rengine::core::json::Value* member(
    const dmc::rengine::core::json::Value::Object& object,
    std::string_view name) {
    const auto iterator = object.find(name);
    return iterator == object.end() ? nullptr : &iterator->second;
}

} // namespace

int main(int argc, char** argv) {
    using dmc::rengine::core::json::Parser;
    using dmc::rengine::gdspaces::ResourceId;
    using dmc::rengine::gdspaces::ResourcePayload;
    using dmc::rengine::gdspaces::ResourceRef;
    using dmc::rengine::integration::ProjectWorkspace;
    using dmc::rengine::item::RuntimeChangeRequest;
    using dmc::rengine::item::build_workspace_view;
    using dmc::rengine::item::make_inventory_limit_request;
    using dmc::rengine::item::make_slot_registration_request;
    using dmc::rengine::item::workspace_manifest_json;

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
    const ResourceRef resource{
        .id = ResourceId{
            .source_id = "item-manifest-test",
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
        .resource = resource,
        .bytes = bytes,
        .diagnostics = {},
    }));
    assert(project.enable_working_copy(resource.id));

    const auto view = build_workspace_view(project, resource.id);
    assert(view.valid());
    constexpr auto artifact = "dmc3-hdc-exe-e454272e";
    const std::vector<RuntimeChangeRequest> requests{
        make_slot_registration_request(view, 50U, artifact),
        make_slot_registration_request(view, 51U, artifact),
        make_inventory_limit_request(view, 127U, artifact),
    };

    const auto json = workspace_manifest_json(
        project,
        resource.id,
        std::span<const RuntimeChangeRequest>{requests});
    assert(!json.empty());
    assert(json == workspace_manifest_json(
        project,
        resource.id,
        std::span<const RuntimeChangeRequest>{requests}));

    const auto parsed = Parser::parse(json);
    assert(parsed.ok());
    const auto* root = parsed.value->as_object();
    assert(root != nullptr);

    const auto* workspace_value = member(*root, "workspace");
    assert(workspace_value != nullptr);
    const auto* workspace = workspace_value->as_object();
    assert(workspace != nullptr);
    const auto* revision = member(*workspace, "working_copy_revision");
    assert(revision != nullptr && revision->as_u64() != nullptr);
    assert(*revision->as_u64() == 0U);
    const auto* dirty = member(*workspace, "dirty");
    assert(dirty != nullptr && dirty->as_bool() != nullptr);
    assert(!*dirty->as_bool());

    const auto* routes_value = member(*root, "routes");
    assert(routes_value != nullptr);
    const auto* routes = routes_value->as_array();
    assert(routes != nullptr && routes->size() >= 4U);

    const auto* evidence_value = member(*root, "runtime_evidence");
    assert(evidence_value != nullptr);
    const auto* evidence = evidence_value->as_array();
    assert(evidence != nullptr && evidence->size() == 3U);

    const auto* requests_value = member(*root, "runtime_requests");
    assert(requests_value != nullptr);
    const auto* request_array = requests_value->as_array();
    assert(request_array != nullptr && request_array->size() == 3U);

    bool ready = false;
    bool slot_research = false;
    bool inventory_research = false;
    for (const auto& value : *request_array) {
        const auto* request = value.as_object();
        assert(request != nullptr);
        const auto* id = member(*request, "id");
        const auto* status = member(*request, "status");
        const auto* consumer = member(*request, "consumer");
        const auto* validators = member(*request, "validators");
        const auto* guards = member(*request, "required_guards");
        assert(id != nullptr && id->as_string() != nullptr);
        assert(status != nullptr && status->as_string() != nullptr);
        assert(consumer != nullptr && consumer->as_string() != nullptr);
        assert(*consumer->as_string() == "exe-editor");
        assert(validators != nullptr && validators->as_array() != nullptr);
        assert(validators->as_array()->size() == 2U);
        assert(guards != nullptr && guards->as_array() != nullptr);
        assert(!guards->as_array()->empty());

        if (id->as_string()->find("register-item-slot-50") !=
            std::string::npos) {
            ready = *status->as_string() == "ready-for-patch-planning";
        } else if (id->as_string()->find("register-item-slot-51") !=
                   std::string::npos) {
            slot_research = *status->as_string() == "research-required";
        } else if (id->as_string()->find("set-inventory-limit-127") !=
                   std::string::npos) {
            inventory_research =
                *status->as_string() == "research-required";
        }
    }
    assert(ready);
    assert(slot_research);
    assert(inventory_research);

    assert(workspace_manifest_json(project, ResourceId{}).empty());
    return 0;
}
