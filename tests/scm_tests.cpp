#include "dmc_rengine/formats/scm.hpp"
#include "dmc_rengine/formats/scm_hierarchy.hpp"
#include "dmc_rengine/formats/scm_layout.hpp"
#include "dmc_rengine/formats/scm_runtime_flags.hpp"
#include "dmc_rengine/formats/scm_topology.hpp"
#include "dmc_rengine/formats/scm_transform.hpp"

#include <cassert>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <span>
#include <vector>

namespace {

template <typename T>
void put(std::vector<std::byte>& bytes, std::size_t offset, T value) {
    std::memcpy(bytes.data() + offset, &value, sizeof(T));
}

[[nodiscard]] bool near(float actual, float expected, float epsilon = 0.0001F) {
    return std::fabs(actual - expected) <= epsilon;
}

std::vector<std::byte> fixture() {
    using namespace dmc::rengine::formats::scm;
    ObjectShape shape;
    shape.mesh_vertex_counts = {3U};
    const std::vector<ObjectShape> shapes{shape};
    const auto layout = build_serialized_layout(
        std::span<const ObjectShape>{shapes}, 1U);
    std::vector<std::byte> bytes(
        static_cast<std::size_t>(layout.file_size), std::byte{0});

    bytes[0] = std::byte{'S'};
    bytes[1] = std::byte{'C'};
    bytes[2] = std::byte{'M'};
    bytes[3] = std::byte{' '};
    put<float>(bytes, 0x04U, 1.01F);
    bytes[0x10U] = std::byte{1};
    bytes[0x11U] = std::byte{1};
    bytes[0x12U] = std::byte{1};
    put<std::uint64_t>(bytes, 0x20U, layout.scene.block_offset);

    const auto& object_layout = layout.objects[0];
    const auto object_offset = static_cast<std::size_t>(object_layout.record_offset);
    bytes[object_offset] = std::byte{1};
    bytes[object_offset + 1U] = std::byte{0x80};
    put<std::uint16_t>(bytes, object_offset + 0x02U, 3U);
    put<std::uint64_t>(
        bytes, object_offset + 0x08U, object_layout.mesh_table_offset);

    const auto& mesh_layout = object_layout.meshes[0];
    const auto mesh_offset = static_cast<std::size_t>(mesh_layout.record_offset);
    put<std::uint16_t>(bytes, mesh_offset + 0x00U, 3U);
    put<std::uint16_t>(bytes, mesh_offset + 0x02U, 0U);
    put<std::uint64_t>(bytes, mesh_offset + 0x10U, mesh_layout.positions_offset);
    put<std::uint64_t>(bytes, mesh_offset + 0x18U, mesh_layout.normals_offset);
    put<std::uint64_t>(bytes, mesh_offset + 0x20U, mesh_layout.uv_offset);
    put<std::uint64_t>(bytes, mesh_offset + 0x28U, 0U);
    put<std::uint64_t>(bytes, mesh_offset + 0x38U, mesh_layout.color_flags_offset);
    put<std::uint64_t>(
        bytes,
        mesh_offset + 0x40U,
        mesh_layout.index_workspace_offset - mesh_layout.record_offset);
    put<std::uint16_t>(
        bytes,
        static_cast<std::size_t>(mesh_layout.index_workspace_offset),
        index_workspace_sentinel);

    const auto scene_offset = static_cast<std::size_t>(layout.scene.block_offset);
    put<std::uint32_t>(bytes, scene_offset + 0x00U, layout.scene.parent_rel);
    put<std::uint32_t>(bytes, scene_offset + 0x04U, layout.scene.order_rel);
    put<std::uint32_t>(bytes, scene_offset + 0x08U, layout.scene.object_binding_rel);
    put<std::uint32_t>(bytes, scene_offset + 0x0CU, layout.scene.transform_rel);
    bytes[scene_offset + layout.scene.parent_rel] = std::byte{0xFF};
    bytes[scene_offset + layout.scene.order_rel] = std::byte{0};
    bytes[scene_offset + layout.scene.object_binding_rel] = std::byte{0};
    return bytes;
}

void assert_identity(const dmc::rengine::formats::scm::Matrix4f& matrix) {
    for (std::size_t row = 0U; row < 4U; ++row) {
        for (std::size_t column = 0U; column < 4U; ++column) {
            const auto expected = row == column ? 1.0F : 0.0F;
            assert(near(matrix(row, column), expected));
        }
    }
}

} // namespace

int main() {
    using namespace dmc::rengine::formats::scm;

    static_assert(header_size == 0x40U);
    static_assert(object_record_size == 0x40U);
    static_assert(mesh_record_size == 0x50U);
    static_assert(index_workspace_capacity_bytes(3U) == 16U);
    static_assert(index_workspace_capacity_bytes(10U) == 48U);

    constexpr MeshRenderWords no_render_words{};
    static_assert(pack_mesh_render_words(no_render_words) == 0U);
    constexpr MeshRenderWords render_words{{1U, 2U, 3U, 4U}};
    static_assert(
        pack_mesh_render_words(render_words) ==
        ((1ULL << 4U) | 0x0FULL | (2ULL << 14U) |
         (3ULL << 24U) | (4ULL << 34U)));
    static_assert(mesh_descriptor_field_08(0U) == 0x60U);
    static_assert(mesh_descriptor_field_08(0x00004000U) == 0U);

    {
        constexpr auto alpha80 = project_effective_alpha_control(0x80U);
        static_assert(alpha80.runtime_control_value == 0x80U);
        static_assert(alpha80.runtime_override_code == 0U);
        static_assert(alpha80.packet_alpha_w == 128.0F / 255.0F);

        constexpr auto alpha_c5 = project_effective_alpha_control(0xC5U);
        static_assert(alpha_c5.runtime_control_value == 0xC5U);
        static_assert(alpha_c5.runtime_override_code == 0xC5U);
        static_assert(alpha_c5.packet_alpha_w == 1.0F);
    }

    ObjectShape shape;
    shape.mesh_vertex_counts = {3U, 4U};
    const std::vector<ObjectShape> shapes{shape};
    const auto layout = build_serialized_layout(
        std::span<const ObjectShape>{shapes}, 3U);
    assert(layout.objects.size() == 1U);
    assert(layout.objects[0].meshes.size() == 2U);
    assert(layout.objects[0].meshes[1].record_offset ==
        layout.objects[0].meshes[0].record_offset + 0x50U);
    assert(layout.scene.parent_rel == 0x20U);
    assert(layout.scene.order_rel == 0x24U);
    assert(layout.scene.object_binding_rel == 0x28U);
    assert(layout.scene.transform_rel == 0x30U);

    const std::vector<std::uint8_t> topology{
        0U, 0U, 0U, 0U, triangle_break_bit, 0U, 0U};
    const auto indices = generate_triangle_strip_indices(topology);
    const std::vector<std::uint16_t> expected_indices{
        0U, 1U, 2U, 3U, 3U, 3U, 3U, 4U, 5U, 6U};
    assert(indices == expected_indices);

    const auto bytes = fixture();
    const auto parsed = Parser::parse(std::span<const std::byte>{bytes});
    assert(parsed.recognized);
    assert(parsed.ok());
    assert(parsed.document.header.object_count == 1U);
    assert(parsed.document.header.scene_node_count == 1U);
    assert(parsed.document.objects.size() == 1U);
    assert(parsed.document.objects[0].alpha_control == 0x80U);
    assert(parsed.document.objects[0].meshes.size() == 1U);
    assert(parsed.document.objects[0].meshes[0].vertex_count == 3U);
    assert(parsed.document.objects[0].meshes[0].render_words.values[0] == 0U);
    assert(parsed.document.objects[0].meshes[0].observed_topology_flag_mask == 0U);
    assert(parsed.document.scene_nodes.parent_by_order_position[0] == -1);
    assert(parsed.document.scene_nodes.node_at_order_position[0] == 0U);
    assert(parsed.document.scene_nodes.object_binding_by_node_index[0] == 0);

    const auto mesh_offset = static_cast<std::size_t>(
        parsed.document.objects[0].meshes[0].record_offset);

    auto nonzero_render_words = bytes;
    put<std::uint16_t>(nonzero_render_words, mesh_offset + 0x04U, 1U);
    put<std::uint16_t>(nonzero_render_words, mesh_offset + 0x06U, 2U);
    put<std::uint16_t>(nonzero_render_words, mesh_offset + 0x08U, 3U);
    put<std::uint16_t>(nonzero_render_words, mesh_offset + 0x0AU, 4U);
    const auto render_parsed = Parser::parse(
        std::span<const std::byte>{nonzero_render_words});
    assert(render_parsed.ok());
    assert(render_parsed.document.objects[0].meshes[0].render_words.values ==
           render_words.values);

    auto bad_vertex_sum = bytes;
    put<std::uint16_t>(bad_vertex_sum, 0x42U, 4U);
    const auto bad_sum = Parser::parse(std::span<const std::byte>{bad_vertex_sum});
    assert(bad_sum.recognized);
    assert(!bad_sum.ok());

    auto bad_continuation = bytes;
    put<std::uint64_t>(bad_continuation, mesh_offset + 0x28U, 0x50U);
    const auto bad_cont = Parser::parse(
        std::span<const std::byte>{bad_continuation});
    assert(bad_cont.recognized);
    assert(!bad_cont.ok());

    std::vector<std::byte> other(header_size, std::byte{0});
    const auto unrecognized = Parser::parse(std::span<const std::byte>{other});
    assert(!unrecognized.recognized);
    assert(!unrecognized.ok());

    const std::vector<std::byte> truncated(0x10U, std::byte{0});
    const auto short_result = Parser::parse(
        std::span<const std::byte>{truncated});
    assert(!short_result.recognized);
    assert(!short_result.ok());
    assert(short_result.diagnostics[0].code == "scm.truncated-header");

    namespace runtime = dmc::rengine::formats::scm::runtime;
    static_assert(runtime::observed_corpus_source_mask == 0x003A0003U);

    {
        constexpr auto projection = runtime::project(0U);
        static_assert(projection.runtime_flags_to_set == 0U);
        static_assert(projection.helper_mode == 9U);
        static_assert(projection.helper_state_selector == 0x0005080BU);
        static_assert(projection.helper_secondary_boolean);
    }
    {
        constexpr auto projection = runtime::project(0x00020000U);
        static_assert(
            (projection.runtime_flags_to_set & runtime::runtime_flag_bit_9) != 0U);
        static_assert(projection.initialize_unit_vector);
    }
    {
        constexpr auto projection = runtime::project(0x00100001U);
        static_assert(projection.helper_mode == 1U);
        static_assert(projection.helper_state_selector == 0x0005010DU);
        static_assert(
            (projection.runtime_flags_to_set & runtime::runtime_flag_bit_8) != 0U);
    }
    {
        constexpr auto projection = runtime::project(0x04000000U);
        static_assert(projection.high_mode_present);
        static_assert(projection.high_mode_minus_one == 3U);
        static_assert(
            (projection.runtime_flags_to_set & runtime::runtime_flag_bit_15) != 0U);
    }
    {
        constexpr auto projection = runtime::project(0x00010004U);
        static_assert(projection.helper_mode == 4U);
        static_assert(projection.helper_state_selector == 0x00050007U);
        static_assert(!projection.helper_secondary_boolean);
        static_assert(
            (projection.runtime_flags_to_set & runtime::runtime_flag_bit_7) != 0U);
    }

    constexpr float half_pi = 1.57079632679489661923F;
    assert_identity(build_rotation_xyz_radians(Vec3f{}));
    {
        const auto matrix = build_rotation_xyz_radians(
            Vec3f{half_pi, 0.0F, 0.0F});
        assert(near(matrix(0U, 0U), 1.0F));
        assert(near(matrix(1U, 2U), 1.0F));
        assert(near(matrix(2U, 1U), -1.0F));
    }
    {
        const auto matrix = build_rotation_xyz_radians(
            Vec3f{0.0F, half_pi, 0.0F});
        assert(near(matrix(0U, 2U), -1.0F));
        assert(near(matrix(2U, 0U), 1.0F));
    }
    {
        const auto matrix = build_rotation_xyz_radians(
            Vec3f{0.0F, 0.0F, half_pi});
        assert(near(matrix(0U, 1U), 1.0F));
        assert(near(matrix(1U, 0U), -1.0F));
    }

    {
        SceneTransform transform{};
        transform.translation = Vec3f{3.0F, 4.0F, 12.0F};
        transform.translation_magnitude = 13.0F;
        transform.rotation_xyz_radians = Vec3f{0.2F, -0.4F, 0.7F};
        const auto local = build_local_transform(transform);
        assert(near(local(3U, 0U), 3.0F));
        assert(near(local(3U, 1U), 4.0F));
        assert(near(local(3U, 2U), 12.0F));
        assert(near(local(3U, 3U), 1.0F));

        const auto inverse = invert_dmc3_rigid_transform(local);
        assert_identity(multiply_dmc3_matrices(local, inverse));
        assert_identity(multiply_dmc3_matrices(inverse, local));
    }

    {
        SceneNodeBlock scene{};
        scene.parent_by_order_position = {-1, 0, 2};
        scene.node_at_order_position = {0U, 2U, 1U};
        scene.object_binding_by_node_index = {-1, 0, 1};
        scene.transform_by_node_index.resize(3U);
        scene.transform_by_node_index[0].translation = Vec3f{10.0F, 0.0F, 0.0F};
        scene.transform_by_node_index[1].translation = Vec3f{0.0F, 0.0F, 3.0F};
        scene.transform_by_node_index[2].translation = Vec3f{0.0F, 2.0F, 0.0F};

        const auto world = build_world_matrices(scene);
        assert(world.has_value());
        assert(near((*world)[0](3U, 0U), 10.0F));
        assert(near((*world)[2](3U, 0U), 10.0F));
        assert(near((*world)[2](3U, 1U), 2.0F));
        assert(near((*world)[1](3U, 0U), 10.0F));
        assert(near((*world)[1](3U, 1U), 2.0F));
        assert(near((*world)[1](3U, 2U), 3.0F));

        auto root_base = identity_matrix();
        root_base.values[12] = 100.0F;
        const auto shifted = build_world_matrices(scene, root_base);
        assert(shifted.has_value());
        assert(near((*shifted)[1](3U, 0U), 110.0F));

        scene.parent_by_order_position[2] = 1;
        assert(!build_world_matrices(scene).has_value());
    }

    return 0;
}
