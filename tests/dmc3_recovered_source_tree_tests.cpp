#include "dmc_rengine/core/json.hpp"
#include "dmc_rengine/gdspaces/resource_payload.hpp"
#include "dmc_rengine/gdspaces/resource_ref.hpp"
#include "dmc_rengine/integration/executable_workspace_manifest.hpp"
#include "dmc_rengine/integration/project_workspace.hpp"
#include "dmc_rengine/profiles/dmc3/recovered_source_tree.hpp"

#include <cassert>
#include <cstddef>
#include <set>
#include <string>
#include <string_view>
#include <vector>

namespace {

[[nodiscard]] const dmc::rengine::core::json::Value* member(
    const dmc::rengine::core::json::Value::Object& object,
    std::string_view name) {
    const auto iterator = object.find(name);
    return iterator == object.end() ? nullptr : &iterator->second;
}

} // namespace

int main() {
    using namespace dmc::rengine;
    using profiles::dmc3::RecoveredSourceStatus;
    using profiles::dmc3::find_recovered_source_symbol;
    using profiles::dmc3::recovered_source_children;
    using profiles::dmc3::recovered_source_tree;

    const auto& tree = recovered_source_tree();
    assert(tree.size() >= 100U);

    std::set<std::string, std::less<>> ids;
    std::size_t mapped_va_count = 0U;
    std::size_t open_count = 0U;
    for (const auto& symbol : tree) {
        assert(symbol.valid());
        assert(!symbol.summary.empty());
        assert(ids.insert(symbol.id).second);
        if (!symbol.parent_id.empty()) {
            assert(find_recovered_source_symbol(symbol.parent_id) != nullptr);
        }
        if (symbol.va.has_value()) {
            ++mapped_va_count;
            assert(*symbol.va >= 0x140000000ULL);
        }
        if (symbol.status == RecoveredSourceStatus::open) {
            ++open_count;
        }
    }
    assert(mapped_va_count >= 70U);
    assert(open_count >= 1U);

    const auto* open_resource = find_recovered_source_symbol(
        "dmc3.vfs.OpenGameResource");
    assert(open_resource != nullptr);
    assert(open_resource->va.has_value());
    assert(*open_resource->va == 0x14002FCA0ULL);

    const auto* registry = find_recovered_source_symbol("dmc3.loaded.registry");
    assert(registry != nullptr);
    assert(registry->va.has_value() && *registry->va == 0x140C99D30ULL);
    assert(registry->size.has_value() && *registry->size == 0x6618ULL);

    const auto* inflater = find_recovered_source_symbol("dmc3.zip.ZipInflaterV1");
    assert(inflater != nullptr);
    assert(inflater->size.has_value() && *inflater->size == 0x1080ULL);

    const auto* physical_open = find_recovered_source_symbol(
        "dmc3.vfs.physical-final-open");
    assert(physical_open != nullptr);
    assert(physical_open->status == RecoveredSourceStatus::open);

    const auto roots = recovered_source_children("");
    assert(roots.size() >= 6U);
    assert(!recovered_source_children("dmc3.resource-runtime.zip").empty());
    assert(!recovered_source_children("dmc3.scene.CUIDGoldOrb").empty());

    const gdspaces::ResourceRef resource{
        .id = gdspaces::ResourceId{
            .source_id = "recovered-tree-test",
            .logical_path = "dmc3.exe",
            .container_chain = {},
            .offset = 0U,
            .size = 1U,
        },
        .display_name = "dmc3.exe",
        .format = "pe",
        .profile = "dmc3-hd",
        .synthetic_name = false,
        .container = false,
    };

    integration::ProjectWorkspace project;
    assert(project.create_session(gdspaces::ResourcePayload{
        .resource = resource,
        .bytes = {std::byte{0}},
        .diagnostics = {},
    }));

    integration::ExecutableResourceContext context;
    context.image.kind = exe::PeKind::pe32_plus;
    context.image.machine = exe::PeMachine::amd64;
    context.image.image_base = 0x140000000ULL;
    context.image.section_count = 0U;
    context.sha256 =
        "e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082";
    context.known_target_id = "dmc3-hdc-phase12-canonical-target";
    context.known_target_name = "Devil May Cry 3 HD canonical Phase 12 target";
    context.known_target_hash_match = true;
    context.known_target_metadata_match = false;
    assert(project.attach_executable_context(resource.id, context));

    const auto* root_node = project.graph().find(
        "source-symbol:dmc3.resource-runtime");
    assert(root_node != nullptr);
    assert(root_node->kind == integration::ProjectNodeKind::source_symbol);

    const auto* function_node = project.graph().find(
        "source-symbol:dmc3.vfs.OpenGameResource");
    assert(function_node != nullptr);
    const auto va_attribute = function_node->attributes.find("va");
    assert(va_attribute != function_node->attributes.end());
    assert(va_attribute->second == "0x14002FCA0");

    const auto* mapping_node = project.graph().find(
        "source-binary-mapping:dmc3.vfs.OpenGameResource");
    assert(mapping_node != nullptr);
    assert(mapping_node->kind == integration::ProjectNodeKind::source_binary_mapping);

    const auto manifest = integration::executable_workspace_manifest_json(
        project, resource.id);
    const auto parsed = core::json::Parser::parse(manifest);
    assert(parsed.ok());
    const auto* root = parsed.value->as_object();
    assert(root != nullptr);
    const auto* schema = member(*root, "schema_version");
    assert(schema != nullptr && schema->as_u64() != nullptr);
    assert(*schema->as_u64() == 2U);
    const auto* recovered = member(*root, "recovered_source_tree");
    assert(recovered != nullptr && recovered->as_array() != nullptr);
    assert(recovered->as_array()->size() == tree.size());

    integration::ProjectWorkspace unknown_project;
    assert(unknown_project.create_session(gdspaces::ResourcePayload{
        .resource = resource,
        .bytes = {std::byte{0}},
        .diagnostics = {},
    }));
    auto unknown_context = context;
    unknown_context.known_target_id.clear();
    unknown_context.known_target_name.clear();
    unknown_context.known_target_hash_match = false;
    assert(unknown_project.attach_executable_context(resource.id, unknown_context));
    assert(unknown_project.graph().find(
        "source-symbol:dmc3.resource-runtime") == nullptr);

    const auto unknown_manifest = integration::executable_workspace_manifest_json(
        unknown_project, resource.id);
    const auto unknown_parsed = core::json::Parser::parse(unknown_manifest);
    assert(unknown_parsed.ok());
    const auto* unknown_root = unknown_parsed.value->as_object();
    assert(unknown_root != nullptr);
    const auto* unknown_tree = member(*unknown_root, "recovered_source_tree");
    assert(unknown_tree != nullptr && unknown_tree->as_array() != nullptr);
    assert(unknown_tree->as_array()->empty());
    return 0;
}
