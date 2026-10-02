#include "dmc_rengine/gdspaces/open_router.hpp"
#include "dmc_rengine/hits/gdspaces_tool.hpp"
#include "dmc_rengine/hits/standalone_session.hpp"
#include "dmc_rengine/integration/tool_registry.hpp"

#include <algorithm>
#include <bit>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <string>
#include <vector>

namespace {

void write_u32(
    std::vector<std::byte>& bytes,
    std::size_t offset,
    std::uint32_t value) {
    for (std::size_t index = 0U; index < 4U; ++index) {
        bytes[offset + index] = static_cast<std::byte>(
            (value >> static_cast<unsigned>(index * 8U)) & 0xFFU);
    }
}

void write_i32(
    std::vector<std::byte>& bytes,
    std::size_t offset,
    std::int32_t value) {
    write_u32(bytes, offset, static_cast<std::uint32_t>(value));
}

void write_f32(
    std::vector<std::byte>& bytes,
    std::size_t offset,
    float value) {
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

[[nodiscard]] std::vector<std::byte> make_hits() {
    constexpr std::size_t pointer_table = 0x44U;
    constexpr std::size_t list_offset = 0x48U;
    constexpr std::size_t triangle_offset = 0x50U;
    constexpr std::size_t end_offset = triangle_offset + 0x38U;
    constexpr std::size_t file_size = 0x90U;

    std::vector<std::byte> bytes(file_size, std::byte{0});
    bytes[0] = std::byte{'H'};
    bytes[1] = std::byte{'I'};
    bytes[2] = std::byte{'T'};
    bytes[3] = std::byte{'S'};
    write_u32(bytes, 0x04U, static_cast<std::uint32_t>(end_offset));
    write_vec3(bytes, 0x08U, -10.0F, -10.0F, -10.0F);
    write_vec3(bytes, 0x14U, 10.0F, 10.0F, 10.0F);
    write_vec3(bytes, 0x20U, 20.0F, 20.0F, 20.0F);
    write_u32(bytes, 0x2CU, 1U);
    write_u32(bytes, 0x30U, 1U);
    write_u32(bytes, 0x34U, 1U);
    write_u32(bytes, 0x38U, 1U);
    write_u32(bytes, 0x3CU, 0x3CU);
    write_u32(
        bytes,
        0x40U,
        static_cast<std::uint32_t>(triangle_offset - 8U));
    write_i32(
        bytes,
        pointer_table,
        static_cast<std::int32_t>(list_offset - 8U));
    write_i32(bytes, list_offset, 0);
    write_i32(bytes, list_offset + 4U, -1);

    write_u32(bytes, triangle_offset, 0x18060001U);
    write_vec3(bytes, triangle_offset + 0x04U, 0.0F, 0.0F, 0.0F);
    write_vec3(bytes, triangle_offset + 0x10U, 1.0F, 0.0F, 0.0F);
    write_vec3(bytes, triangle_offset + 0x1CU, 0.0F, 0.0F, 1.0F);
    write_vec3(bytes, triangle_offset + 0x28U, 0.0F, -1.0F, 0.0F);
    write_f32(bytes, triangle_offset + 0x34U, 0.0F);
    return bytes;
}

[[nodiscard]] dmc::rengine::gdspaces::ResourcePayload make_payload() {
    namespace gdspaces = dmc::rengine::gdspaces;
    auto bytes = make_hits();

    return gdspaces::ResourcePayload{
        .resource = gdspaces::ResourceRef{
            .id = gdspaces::ResourceId{
                .source_id = "hits-product-split-test",
                .logical_path = "GData.afs/st000.pac/slot_0003.hits",
                .container_chain = "nbz[0]/pac[3]",
                .offset = 0x40U,
                .size = static_cast<std::uint64_t>(bytes.size()),
            },
            .display_name = "slot_0003.hits",
            .format = "hits",
            .profile = "dmc3-hd",
            .synthetic_name = true,
            .container = false,
        },
        .bytes = std::move(bytes),
        .diagnostics = {},
        .byte_provenance = std::nullopt,
        .name_evidence = {},
        .enclosing_container_name_evidence = {},
        .semantic_evidence = {},
    };
}

[[nodiscard]] bool has_route(
    const std::vector<dmc::rengine::integration::ToolRoute>& routes,
    dmc::rengine::gdspaces::ToolTarget target,
    dmc::rengine::integration::ToolRouteRole role) {
    return std::any_of(
        routes.begin(),
        routes.end(),
        [target, role](const auto& route) {
            return route.target == target && route.role == role;
        });
}

} // namespace

int main() {
    namespace gdspaces = dmc::rengine::gdspaces;
    namespace editor = dmc::rengine::hits::editor;
    namespace embedded = dmc::rengine::hits::gdspaces_tool;
    namespace standalone = dmc::rengine::hits::standalone;
    namespace integration = dmc::rengine::integration;

    const auto source = make_hits();

    auto standalone_session = standalone::Session::open(source);
    assert(standalone_session.has_value());
    assert(!standalone_session->dirty());

    const auto standalone_unchanged = standalone_session->save();
    assert(standalone_unchanged.ok());
    assert(standalone_unchanged.status == standalone::SaveStatus::no_changes);
    assert(standalone_unchanged.bytes == source);

    assert(standalone_session->editor().set_collision_preset(
        1U,
        editor::CollisionPreset::green_raw_0000000a));
    const auto standalone_saved = standalone_session->save();
    assert(standalone_saved.ok());
    assert(standalone_saved.status == standalone::SaveStatus::ok);
    assert(standalone_saved.bytes != source);

    const auto payload = make_payload();
    const auto source_id = payload.resource.id;
    const auto source_profile = payload.resource.profile;

    auto embedded_session = embedded::Session::open(payload);
    assert(embedded_session.has_value());
    assert(!embedded_session->dirty());

    assert(embedded_session->editor().set_collision_preset(
        1U,
        editor::CollisionPreset::orange_raw_00000009));
    const auto committed = embedded_session->commit();
    assert(committed.ok());
    assert(committed.status == embedded::CommitStatus::ok);
    assert(committed.payload.resource.id.source_id == source_id.source_id);
    assert(committed.payload.resource.id.logical_path == source_id.logical_path);
    assert(committed.payload.resource.id.container_chain == source_id.container_chain);
    assert(committed.payload.resource.id.offset == source_id.offset);
    assert(committed.payload.resource.id.size ==
        static_cast<std::uint64_t>(committed.payload.bytes.size()));
    assert(committed.payload.resource.profile == source_profile);

    const gdspaces::OpenRouter router;
    const gdspaces::OpenRequest direct_request{
        .resource = payload.resource,
        .preferred_target = std::nullopt,
        .stage_context = false,
        .menu_context = false,
        .evidence_context = true,
    };
    assert(router.route(direct_request) == gdspaces::ToolTarget::hits_editor);

    auto stage_request = direct_request;
    stage_request.stage_context = true;
    assert(router.route(stage_request) == gdspaces::ToolTarget::hits_editor);

    const integration::ToolRegistry tools;
    const auto stage_routes =
        tools.routes_for(payload.resource, true, false, true);
    assert(has_route(
        stage_routes,
        gdspaces::ToolTarget::hits_editor,
        integration::ToolRouteRole::primary));
    assert(has_route(
        stage_routes,
        gdspaces::ToolTarget::stage_ops,
        integration::ToolRouteRole::companion));
    assert(has_route(
        stage_routes,
        gdspaces::ToolTarget::modviz_scene,
        integration::ToolRouteRole::companion));

    const auto* descriptor = tools.find(gdspaces::ToolTarget::hits_editor);
    assert(descriptor != nullptr);
    assert(descriptor->supports(integration::ToolCapability::hits_editing));

    return 0;
}
