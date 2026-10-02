#include "dmc_rengine/formats/hits.hpp"
#include "dmc_rengine/formats/scm.hpp"
#include "dmc_rengine/hits/editor.hpp"
#include "dmc_rengine/hits/scm_import.hpp"

#include <bit>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <vector>

namespace {

void write_u32(
    std::vector<std::byte>& bytes,
    std::size_t offset,
    std::uint32_t value) {
    for (std::size_t i = 0U; i < 4U; ++i) {
        bytes[offset + i] = static_cast<std::byte>(
            (value >> static_cast<unsigned>(i * 8U)) & 0xFFU);
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

[[nodiscard]] std::vector<std::byte> make_hits_fixture() {
    constexpr std::size_t pointer_table = 0x44U;
    constexpr std::size_t list_offset = 0x48U;
    constexpr std::size_t triangle_offset = 0x50U;
    constexpr std::size_t end_offset = triangle_offset + 0x38U;

    std::vector<std::byte> bytes(end_offset, std::byte{0});
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

    write_u32(bytes, triangle_offset, 0x00000001U);
    write_vec3(bytes, triangle_offset + 0x04U, 0.0F, 0.0F, 0.0F);
    write_vec3(bytes, triangle_offset + 0x10U, 1.0F, 0.0F, 0.0F);
    write_vec3(bytes, triangle_offset + 0x1CU, 0.0F, 0.0F, 1.0F);
    write_vec3(bytes, triangle_offset + 0x28U, 0.0F, -1.0F, 0.0F);
    write_f32(bytes, triangle_offset + 0x34U, 0.0F);
    return bytes;
}

[[nodiscard]] dmc::rengine::formats::scm::Document make_scm_document() {
    using namespace dmc::rengine::formats::scm;

    Document document{};
    document.objects.resize(1U);

    Mesh mesh{};
    mesh.vertex_count = 4U;
    mesh.positions = {
        Vec3f{0.0F, 0.0F, 0.0F},
        Vec3f{1.0F, 0.0F, 0.0F},
        Vec3f{0.0F, 0.0F, 1.0F},
        Vec3f{1.0F, 0.0F, 1.0F},
    };
    mesh.colors_topology = {
        ColorTopology{255U, 255U, 255U, 0U},
        ColorTopology{255U, 255U, 255U, 0U},
        ColorTopology{255U, 255U, 255U, 0U},
        ColorTopology{255U, 255U, 255U, 0U},
    };

    document.objects[0].mesh_count = 1U;
    document.objects[0].meshes.push_back(std::move(mesh));

    document.scene_nodes.parent_by_order_position = {-1};
    document.scene_nodes.node_at_order_position = {0U};
    document.scene_nodes.object_binding_by_node_index = {0};
    document.scene_nodes.transform_by_node_index.resize(1U);
    document.scene_nodes.transform_by_node_index[0].translation =
        Vec3f{15.0F, 2.0F, 20.0F};
    return document;
}

} // namespace

int main() {
    using dmc::rengine::hits::editor::CollisionPreset;
    using dmc::rengine::hits::editor::Session;
    using dmc::rengine::hits::scm_import::extract_mesh;

    const auto scm = make_scm_document();
    const auto extracted = extract_mesh(scm, 0U, 0U);
    assert(extracted.has_value());
    assert(extracted->node_index == 0U);
    assert(extracted->triangles.size() == 2U);

    const auto& first = extracted->triangles[0];
    assert(first.point_a ==
        dmc::rengine::formats::hits::Vec3{15.0F, 2.0F, 20.0F});
    assert(first.point_b ==
        dmc::rengine::formats::hits::Vec3{16.0F, 2.0F, 20.0F});
    assert(first.point_c ==
        dmc::rengine::formats::hits::Vec3{15.0F, 2.0F, 21.0F});

    const auto& second = extracted->triangles[1];
    assert(second.point_a ==
        dmc::rengine::formats::hits::Vec3{15.0F, 2.0F, 21.0F});
    assert(second.point_b ==
        dmc::rengine::formats::hits::Vec3{16.0F, 2.0F, 20.0F});
    assert(second.point_c ==
        dmc::rengine::formats::hits::Vec3{16.0F, 2.0F, 21.0F});

    assert(!extract_mesh(scm, 1U, 0U).has_value());
    assert(!extract_mesh(scm, 0U, 1U).has_value());

    const auto hits = make_hits_fixture();
    auto opened = Session::open(hits);
    assert(opened.has_value());
    auto session = std::move(*opened);

    const auto imported = session.import_scm_mesh(
        scm,
        0U,
        0U,
        CollisionPreset::green_raw_0000000a);
    assert(imported.has_value());
    assert(imported->surface_ids.size() == 2U);
    assert(session.surfaces().size() == 3U);
    assert(session.meshes().size() == 1U);
    assert(session.meshes()[0].stable_id == imported->mesh_id);
    assert(session.meshes()[0].surface_ids == imported->surface_ids);

    for (const auto id : imported->surface_ids) {
        const auto index = session.index_of(id);
        assert(index.has_value());
        assert(session.surfaces()[*index].flags == 0x0000000AU);
    }

    const auto rebuilt = session.rebuild();
    assert(rebuilt.ok());
    assert(rebuilt.header.triangle_count == 3U);
    assert(rebuilt.header.bounds_max.x >= 16.0F);
    assert(rebuilt.header.bounds_max.z >= 21.0F);

    const auto reparsed =
        dmc::rengine::formats::hits::RecordScanner::scan(rebuilt.bytes);
    assert(reparsed.ok());
    assert(reparsed.triangles.size() == 3U);
    assert(reparsed.triangles[1].flags == 0x0000000AU);
    assert(reparsed.triangles[2].flags == 0x0000000AU);

    assert(session.undo());
    assert(session.surfaces().size() == 1U);
    assert(session.meshes().empty());

    assert(session.redo());
    assert(session.surfaces().size() == 3U);
    assert(session.meshes().size() == 1U);

    return 0;
}
