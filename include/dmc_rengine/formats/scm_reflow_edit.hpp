#pragma once

#include "dmc_rengine/formats/scm_edit.hpp"
#include "dmc_rengine/formats/scm_layout.hpp"
#include "dmc_rengine/formats/scm_topology.hpp"

#include <cstddef>
#include <cstdint>
#include <limits>
#include <span>
#include <utility>
#include <vector>

namespace dmc::rengine::formats::scm {

struct AppendBreakVertexLayoutPlan final {
    bool valid{false};
    std::size_t object_index{};
    std::size_t mesh_index{};
    std::size_t source_vertex_index{};
    std::uint16_t source_vertex_count{};
    std::uint64_t source_file_size{};
    std::uint64_t output_file_size{};

    [[nodiscard]] bool grows() const noexcept {
        return valid && output_file_size > source_file_size;
    }

    [[nodiscard]] std::uint64_t size_delta() const noexcept {
        return grows() ? output_file_size - source_file_size : 0U;
    }
};

namespace reflow_edit_detail {

[[nodiscard]] inline bool canonical_shapes(
    const Document& document,
    std::vector<ObjectShape>& shapes,
    std::uint8_t& scene_node_count) {
    const auto node_count = document.scene_nodes.transform_by_node_index.size();
    if (node_count > std::numeric_limits<std::uint8_t>::max()) return false;
    scene_node_count = static_cast<std::uint8_t>(node_count);

    shapes.clear();
    shapes.reserve(document.objects.size());
    for (const auto& object : document.objects) {
        ObjectShape shape;
        shape.mesh_vertex_counts.reserve(object.meshes.size());
        std::size_t object_vertex_total = 0U;
        for (const auto& mesh : object.meshes) {
            const auto count = mesh.positions.size();
            if (mesh.normals.size() != count || mesh.uvs.size() != count ||
                mesh.colors_topology.size() != count ||
                count > std::numeric_limits<std::uint16_t>::max()) {
                return false;
            }
            object_vertex_total += count;
            if (object_vertex_total >
                std::numeric_limits<std::uint16_t>::max()) {
                return false;
            }
            shape.mesh_vertex_counts.push_back(
                static_cast<std::uint16_t>(count));
        }
        shapes.push_back(std::move(shape));
    }
    return true;
}

} // namespace reflow_edit_detail

// Computes the canonical physical-size effect of one append operation without
// mutating the document or serializing bytes. The result is deliberately tied
// to the same build_serialized_layout() authority used by canonical_rebuild.
// A caller that needs a guarantee relative to an existing source file must
// additionally require plan.source_file_size == document.source_bytes.size().
[[nodiscard]] inline AppendBreakVertexLayoutPlan
plan_append_break_vertex_copy_layout(
    const Document& document,
    std::size_t object_index,
    std::size_t mesh_index,
    std::size_t source_vertex_index) {
    AppendBreakVertexLayoutPlan plan;
    plan.object_index = object_index;
    plan.mesh_index = mesh_index;
    plan.source_vertex_index = source_vertex_index;

    if (object_index >= document.objects.size()) return plan;
    const auto& object = document.objects[object_index];
    if (mesh_index >= object.meshes.size()) return plan;
    const auto& mesh = object.meshes[mesh_index];

    const auto count = mesh.positions.size();
    if (mesh.normals.size() != count || mesh.uvs.size() != count ||
        mesh.colors_topology.size() != count || source_vertex_index >= count ||
        count >= std::numeric_limits<std::uint16_t>::max()) {
        return plan;
    }

    std::size_t object_vertex_total = 0U;
    for (const auto& candidate : object.meshes) {
        if (candidate.positions.size() >
            std::numeric_limits<std::uint16_t>::max()) {
            return plan;
        }
        object_vertex_total += candidate.positions.size();
    }
    if (object_vertex_total >= std::numeric_limits<std::uint16_t>::max()) {
        return plan;
    }

    std::vector<ObjectShape> shapes;
    std::uint8_t scene_node_count = 0U;
    if (!reflow_edit_detail::canonical_shapes(
            document, shapes, scene_node_count)) {
        return plan;
    }

    const auto source_layout = build_serialized_layout(
        std::span<const ObjectShape>{shapes}, scene_node_count);
    ++shapes[object_index].mesh_vertex_counts[mesh_index];
    const auto output_layout = build_serialized_layout(
        std::span<const ObjectShape>{shapes}, scene_node_count);

    plan.valid = true;
    plan.source_vertex_count = static_cast<std::uint16_t>(count);
    plan.source_file_size = source_layout.file_size;
    plan.output_file_size = output_layout.file_size;
    return plan;
}

// Finds the first deterministic append target that provably increases the
// canonical SCM physical size. The copied source is the last vertex in the
// mesh, keeping the evidence edit local to the existing strip tail while the
// appended record is forced to the confirmed topology-break bit.
[[nodiscard]] inline AppendBreakVertexLayoutPlan
find_append_break_vertex_growth_target(const Document& document) {
    for (std::size_t object_index = 0U;
         object_index < document.objects.size();
         ++object_index) {
        const auto& object = document.objects[object_index];
        for (std::size_t mesh_index = 0U;
             mesh_index < object.meshes.size();
             ++mesh_index) {
            const auto& mesh = object.meshes[mesh_index];
            if (mesh.positions.empty()) continue;
            const auto plan = plan_append_break_vertex_copy_layout(
                document,
                object_index,
                mesh_index,
                mesh.positions.size() - 1U);
            if (plan.grows()) return plan;
        }
    }
    return {};
}

// Evidence-oriented structural edit used to exercise canonical SCM reflow
// without inventing a new topology semantic. The selected source vertex is
// copied into all four serialized vertex streams and the appended topology
// byte is forced to the EXE/corpus-confirmed 0x02 break bit. With no following
// vertices, the appended record cannot introduce a new non-degenerate strip
// triangle. Preserve-layout writing is expected to reject the changed stream
// count; canonical_rebuild owns all derived counts/offsets/workspace layout.
[[nodiscard]] inline EditResult append_break_vertex_copy(
    Document& document,
    std::size_t object_index,
    std::size_t mesh_index,
    std::size_t source_vertex_index) {
    EditResult result;
    auto* object = edit_detail::object_at(document, object_index, result);
    if (object == nullptr) return result;
    if (mesh_index >= object->meshes.size()) {
        edit_detail::error(
            result,
            "scm.edit-mesh-out-of-range",
            "SCM mesh index is outside the selected object.");
        return result;
    }

    auto& mesh = object->meshes[mesh_index];
    const auto count = mesh.positions.size();
    if (mesh.normals.size() != count ||
        mesh.uvs.size() != count ||
        mesh.colors_topology.size() != count) {
        edit_detail::error(
            result,
            "scm.edit-stream-count-mismatch",
            "SCM structural vertex edits require position, normal, UV and "
            "color/topology streams to have identical lengths.");
        return result;
    }
    if (source_vertex_index >= count) {
        edit_detail::error(
            result,
            "scm.edit-vertex-out-of-range",
            "SCM source vertex index is outside the selected mesh.");
        return result;
    }
    if (count >= std::numeric_limits<std::uint16_t>::max()) {
        edit_detail::error(
            result,
            "scm.edit-vertex-count-overflow",
            "SCM mesh cannot grow beyond the 16-bit serialized vertex-count "
            "domain.");
        return result;
    }

    std::size_t object_vertex_total = 0U;
    for (const auto& candidate : object->meshes) {
        object_vertex_total += candidate.positions.size();
    }
    if (object_vertex_total >= std::numeric_limits<std::uint16_t>::max()) {
        edit_detail::error(
            result,
            "scm.edit-object-vertex-count-overflow",
            "SCM object cannot grow beyond the 16-bit serialized total-vertex "
            "domain.");
        return result;
    }

    const auto position = mesh.positions[source_vertex_index];
    const auto normal = mesh.normals[source_vertex_index];
    const auto uv = mesh.uvs[source_vertex_index];
    const auto source_color = mesh.colors_topology[source_vertex_index];

    mesh.positions.push_back(position);
    mesh.normals.push_back(normal);
    mesh.uvs.push_back(uv);
    mesh.colors_topology.push_back(ColorTopology{
        source_color.r,
        source_color.g,
        source_color.b,
        triangle_break_bit,
    });

    result.changed = true;
    return result;
}

} // namespace dmc::rengine::formats::scm
