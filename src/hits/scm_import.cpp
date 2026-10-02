#include "dmc_rengine/hits/scm_import.hpp"

#include "dmc_rengine/formats/scm_hierarchy.hpp"
#include "dmc_rengine/formats/scm_topology.hpp"
#include "dmc_rengine/formats/scm_transform.hpp"
#include "dmc_rengine/hits/edit.hpp"

#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <optional>
#include <vector>

namespace dmc::rengine::hits::scm_import {
namespace {

using ScmVec3 = formats::scm::Vec3f;
using HitVec3 = formats::hits::Vec3;

[[nodiscard]] HitVec3 transform_point(
    const ScmVec3& point,
    const formats::scm::Matrix4f& matrix) noexcept {
    // DMC3 uses row-vector composition. Position is [x y z 1] * M.
    return HitVec3{
        .x = point.x * matrix(0U, 0U) +
             point.y * matrix(1U, 0U) +
             point.z * matrix(2U, 0U) +
             matrix(3U, 0U),
        .y = point.x * matrix(0U, 1U) +
             point.y * matrix(1U, 1U) +
             point.z * matrix(2U, 1U) +
             matrix(3U, 1U),
        .z = point.x * matrix(0U, 2U) +
             point.y * matrix(1U, 2U) +
             point.z * matrix(2U, 2U) +
             matrix(3U, 2U),
    };
}

[[nodiscard]] std::optional<std::size_t> bound_node_for_object(
    const formats::scm::SceneNodeBlock& scene,
    std::size_t object_index) noexcept {
    std::optional<std::size_t> found;
    for (std::size_t node = 0U;
         node < scene.object_binding_by_node_index.size();
         ++node) {
        const auto binding = scene.object_binding_by_node_index[node];
        if (binding < 0) {
            continue;
        }
        if (static_cast<std::size_t>(binding) != object_index) {
            continue;
        }
        if (found) {
            return std::nullopt;
        }
        found = node;
    }
    return found;
}

} // namespace

std::optional<MeshExtraction> extract_mesh(
    const formats::scm::Document& document,
    std::size_t object_index,
    std::size_t mesh_index) noexcept {
    if (object_index >= document.objects.size()) {
        return std::nullopt;
    }
    const auto& object = document.objects[object_index];
    if (mesh_index >= object.meshes.size()) {
        return std::nullopt;
    }

    const auto node_index =
        bound_node_for_object(document.scene_nodes, object_index);
    if (!node_index) {
        return std::nullopt;
    }

    const auto world =
        formats::scm::build_world_matrices(document.scene_nodes);
    if (!world || *node_index >= world->size()) {
        return std::nullopt;
    }

    const auto& mesh = object.meshes[mesh_index];
    if (mesh.positions.size() != mesh.colors_topology.size() ||
        mesh.positions.size() != mesh.vertex_count) {
        return std::nullopt;
    }

    std::vector<std::uint8_t> topology_flags;
    topology_flags.reserve(mesh.colors_topology.size());
    for (const auto& color : mesh.colors_topology) {
        topology_flags.push_back(color.topology_flags);
    }

    const auto strip_indices =
        formats::scm::generate_triangle_strip_indices(topology_flags);
    if (strip_indices.size() < 3U) {
        return MeshExtraction{
            .object_index = object_index,
            .mesh_index = mesh_index,
            .node_index = *node_index,
            .triangles = {},
        };
    }

    MeshExtraction result{
        .object_index = object_index,
        .mesh_index = mesh_index,
        .node_index = *node_index,
        .triangles = {},
    };
    result.triangles.reserve(strip_indices.size() - 2U);

    std::size_t source_triangle_ordinal = 0U;
    for (std::size_t cursor = 2U;
         cursor < strip_indices.size();
         ++cursor, ++source_triangle_ordinal) {
        auto a = strip_indices[cursor - 2U];
        auto b = strip_indices[cursor - 1U];
        const auto c = strip_indices[cursor];

        if ((cursor & 1U) != 0U) {
            std::swap(a, b);
        }
        if (a == b || b == c || a == c) {
            continue;
        }
        if (a >= mesh.positions.size() ||
            b >= mesh.positions.size() ||
            c >= mesh.positions.size()) {
            return std::nullopt;
        }

        const auto point_a =
            transform_point(mesh.positions[a], (*world)[*node_index]);
        const auto point_b =
            transform_point(mesh.positions[b], (*world)[*node_index]);
        const auto point_c =
            transform_point(mesh.positions[c], (*world)[*node_index]);

        if (!edit::recompute_geometry(point_a, point_b, point_c)) {
            continue;
        }

        result.triangles.push_back(TriangleSeed{
            .point_a = point_a,
            .point_b = point_b,
            .point_c = point_c,
            .source_triangle_ordinal = source_triangle_ordinal,
        });
    }

    return result;
}

} // namespace dmc::rengine::hits::scm_import
