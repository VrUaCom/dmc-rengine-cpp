#pragma once

#include "dmc_rengine/formats/scm_edit.hpp"
#include "dmc_rengine/formats/scm_topology.hpp"

#include <cstddef>
#include <cstdint>
#include <limits>

namespace dmc::rengine::formats::scm {

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
