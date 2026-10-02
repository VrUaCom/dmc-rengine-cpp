#include "dmc_rengine/formats/hits.hpp"
#include "dmc_rengine/hits/editor.hpp"

#include <array>
#include <bit>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <span>
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

void write_triangle(
    std::vector<std::byte>& bytes,
    std::size_t offset,
    std::uint32_t flags,
    float origin_x) {
    write_u32(bytes, offset, flags);
    write_vec3(bytes, offset + 0x04U, origin_x, 0.0F, 0.0F);
    write_vec3(bytes, offset + 0x10U, origin_x + 1.0F, 0.0F, 0.0F);
    write_vec3(bytes, offset + 0x1CU, origin_x, 0.0F, 1.0F);
    write_vec3(bytes, offset + 0x28U, 0.0F, -1.0F, 0.0F);
    write_f32(bytes, offset + 0x34U, 0.0F);
}

[[nodiscard]] std::vector<std::byte> make_fixture() {
    constexpr std::size_t pointer_table_offset = 0x44U;
    constexpr std::size_t lists_offset = 0x4CU;
    constexpr std::size_t triangle_offset = 0x64U;
    constexpr std::size_t end_offset = triangle_offset + 2U * 0x38U;

    std::vector<std::byte> bytes(end_offset, std::byte{0});
    bytes[0] = std::byte{'H'};
    bytes[1] = std::byte{'I'};
    bytes[2] = std::byte{'T'};
    bytes[3] = std::byte{'S'};

    write_u32(bytes, 0x04U, static_cast<std::uint32_t>(end_offset));
    write_vec3(bytes, 0x08U, -10.0F, -2.0F, -10.0F);
    write_vec3(bytes, 0x14U, 10.0F, 2.0F, 10.0F);
    write_vec3(bytes, 0x20U, 10.0F, 4.0F, 20.0F);
    write_u32(bytes, 0x2CU, 2U);
    write_u32(bytes, 0x30U, 1U);
    write_u32(bytes, 0x34U, 1U);
    write_u32(bytes, 0x38U, 2U);
    write_u32(bytes, 0x3CU, 0x3CU);
    write_u32(
        bytes,
        0x40U,
        static_cast<std::uint32_t>(triangle_offset - 8U));

    write_i32(
        bytes,
        pointer_table_offset,
        static_cast<std::int32_t>(lists_offset - 8U));
    write_i32(
        bytes,
        pointer_table_offset + 4U,
        static_cast<std::int32_t>(lists_offset + 8U - 8U));

    write_i32(bytes, lists_offset, 0);
    write_i32(bytes, lists_offset + 4U, -1);
    write_i32(bytes, lists_offset + 8U, 0x38);
    write_i32(bytes, lists_offset + 12U, -1);

    write_triangle(bytes, triangle_offset, 0x18060001U, -2.0F);
    write_triangle(
        bytes,
        triangle_offset + 0x38U,
        0x00000001U,
        3.0F);
    return bytes;
}

} // namespace

int main() {
    using dmc::rengine::formats::hits::RecordScanner;
    using dmc::rengine::formats::hits::Vec3;
    using dmc::rengine::hits::editor::CollisionPreset;
    using dmc::rengine::hits::editor::Session;
    using dmc::rengine::hits::editor::collision_preset_from_flags;
    using dmc::rengine::hits::editor::collision_preset_info;

    const auto source = make_fixture();
    auto opened = Session::open(source);
    assert(opened.has_value());

    auto session = std::move(*opened);
    assert(!session.dirty());
    assert(!session.can_undo());
    assert(!session.can_redo());
    assert(session.revision() == 0U);
    assert(session.surfaces().size() == 2U);
    assert(session.surfaces()[0].stable_id == 1U);
    assert(session.surfaces()[1].stable_id == 2U);
    assert(session.meshes().empty());

    assert(collision_preset_info(
        CollisionPreset::blue_raw_00000001).raw_flags == 0x00000001U);
    assert(collision_preset_info(
        CollisionPreset::orange_raw_00000009).raw_flags == 0x00000009U);
    assert(collision_preset_info(
        CollisionPreset::green_raw_0000000a).raw_flags == 0x0000000AU);
    assert(collision_preset_info(
        CollisionPreset::red_raw_18060001).raw_flags == 0x18060001U);
    assert(collision_preset_from_flags(0x00000001U) ==
           CollisionPreset::blue_raw_00000001);
    assert(!collision_preset_from_flags(0x12345678U).has_value());

    assert(session.set_collision_preset(
        1U,
        CollisionPreset::green_raw_0000000a));
    assert(session.dirty());
    assert(session.can_undo());
    assert(session.surfaces()[0].flags == 0x0000000AU);

    assert(session.undo());
    assert(!session.dirty());
    assert(session.can_redo());
    assert(session.surfaces()[0].flags == 0x18060001U);

    assert(session.redo());
    assert(session.dirty());
    assert(session.surfaces()[0].flags == 0x0000000AU);

    const auto quad_mesh = session.add_quad(
        Vec3{-2.0F, 0.0F, -2.0F},
        Vec3{-1.0F, 0.0F, -2.0F},
        Vec3{-1.0F, 0.0F, -1.0F},
        Vec3{-2.0F, 0.0F, -1.0F},
        CollisionPreset::orange_raw_00000009);
    assert(quad_mesh.has_value());
    const auto quad_index = session.mesh_index_of(*quad_mesh);
    assert(quad_index.has_value());
    assert(session.meshes()[*quad_index].surface_ids.size() == 2U);

    const auto boundary_mesh = session.create_rectangular_boundary(
        Vec3{-4.0F, -1.0F, -4.0F},
        Vec3{4.0F, 3.0F, 4.0F},
        CollisionPreset::red_raw_18060001);
    assert(boundary_mesh.has_value());
    const auto boundary_index = session.mesh_index_of(*boundary_mesh);
    assert(boundary_index.has_value());
    assert(session.meshes()[*boundary_index].surface_ids.size() == 8U);

    const std::array<dmc::rengine::hits::editor::StableSurfaceId, 2U>
        initial_pair{1U, 2U};
    assert(session.set_collision_preset(
        initial_pair,
        CollisionPreset::orange_raw_00000009));
    assert(session.surfaces()[0].flags == 0x00000009U);
    assert(session.surfaces()[1].flags == 0x00000009U);

    const std::array<dmc::rengine::hits::editor::StableSurfaceId, 1U>
        first_only{1U};
    const std::array<dmc::rengine::hits::editor::StableSurfaceId, 1U>
        second_only{2U};
    const auto mesh_a = session.create_mesh(first_only);
    const auto mesh_b = session.create_mesh(second_only);
    assert(mesh_a.has_value());
    assert(mesh_b.has_value());
    assert(*mesh_a != *mesh_b);
    assert(session.meshes().size() == 2U);

    const std::array<dmc::rengine::hits::editor::StableMeshId, 2U>
        mesh_pair{*mesh_a, *mesh_b};
    const auto merged_mesh = session.merge_meshes(mesh_pair);
    assert(merged_mesh.has_value());
    assert(session.meshes().size() == 1U);
    assert(session.meshes()[0].surface_ids.size() == 2U);

    assert(session.set_mesh_collision_preset(
        *merged_mesh,
        CollisionPreset::blue_raw_00000001));
    assert(session.surfaces()[0].flags == 0x00000001U);
    assert(session.surfaces()[1].flags == 0x00000001U);

    assert(session.translate_mesh(
        *merged_mesh,
        Vec3{0.0F, 0.5F, 0.0F}));
    assert(session.surfaces()[0].point_a.y == 0.5F);
    assert(session.surfaces()[1].point_a.y == 0.5F);

    const auto duplicate = session.duplicate_surface(1U);
    assert(duplicate.has_value());
    assert(*duplicate == 3U);
    assert(session.surfaces().size() == 3U);
    assert(session.surfaces()[2].flags == 0x00000001U);
    assert(session.meshes().size() == 1U);
    assert(session.meshes()[0].surface_ids.size() == 3U);

    const auto added = session.add_surface(
        0x00000009U,
        Vec3{-1.0F, 0.0F, 2.0F},
        Vec3{0.0F, 0.0F, 2.0F},
        Vec3{-1.0F, 0.0F, 3.0F});
    assert(added.has_value());
    assert(*added == 4U);
    assert(session.surfaces().size() == 4U);

    const auto revision_before_invalid_add = session.revision();
    const auto invalid = session.add_surface(
        1U,
        Vec3{0.0F, 0.0F, 0.0F},
        Vec3{1.0F, 0.0F, 0.0F},
        Vec3{2.0F, 0.0F, 0.0F});
    assert(!invalid.has_value());
    assert(session.revision() == revision_before_invalid_add);
    assert(session.surfaces().size() == 4U);

    assert(session.translate_surface(
        *added,
        Vec3{30.0F, 0.0F, 0.0F}));

    const std::array<dmc::rengine::hits::editor::StableSurfaceId, 2U>
        selection{1U, 2U};
    assert(session.translate_surfaces(
        selection,
        Vec3{0.0F, 1.0F, 0.0F}));

    const auto before_failed_batch = session.surfaces()[0].point_a;
    const std::array<dmc::rengine::hits::editor::StableSurfaceId, 2U>
        invalid_selection{1U, 999U};
    assert(!session.translate_surfaces(
        invalid_selection,
        Vec3{5.0F, 0.0F, 0.0F}));
    assert(session.surfaces()[0].point_a == before_failed_batch);

    assert(session.erase_surface(*duplicate));
    assert(session.surfaces().size() == 3U);
    assert(!session.index_of(*duplicate).has_value());

    const auto rebuilt = session.rebuild();
    assert(rebuilt.ok());
    assert(rebuilt.header.triangle_count == 3U);

    const auto reparsed = RecordScanner::scan(rebuilt.bytes);
    assert(reparsed.ok());
    assert(reparsed.triangles.size() == 3U);
    assert(reparsed.triangles[0].flags == 0x00000001U);
    assert(reparsed.triangles[1].flags == 0x00000001U);
    assert(reparsed.triangles[2].flags == 0x00000009U);
    assert(reparsed.header.bounds_max.x >= 30.0F);

    assert(session.reset_to_source());
    assert(!session.dirty());
    assert(session.surfaces().size() == 2U);
    assert(session.surfaces()[0].flags == 0x18060001U);
    assert(session.meshes().empty());
    assert(session.can_undo());

    assert(session.undo());
    assert(session.dirty());
    assert(session.surfaces().size() == 3U);

    assert(session.redo());
    assert(!session.dirty());
    assert(session.surfaces().size() == 2U);

    assert(session.erase_surface(1U));
    assert(session.erase_surface(2U));
    assert(session.surfaces().empty());

    const auto empty = session.rebuild();
    assert(empty.ok());
    const auto empty_scan = RecordScanner::scan(empty.bytes);
    assert(empty_scan.ok());
    assert(empty_scan.triangles.empty());
    assert(empty_scan.header.triangle_count == 0U);

    return 0;
}
