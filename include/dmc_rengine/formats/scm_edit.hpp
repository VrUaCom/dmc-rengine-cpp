#pragma once

#include "dmc_rengine/formats/scm.hpp"
#include "dmc_rengine/formats/scm_render.hpp"

#include <cmath>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <optional>
#include <string>
#include <utility>
#include <vector>

namespace dmc::rengine::formats::scm {

inline constexpr float scm_uv_scale = 4096.0F;

struct EditResult final {
    bool changed{false};
    std::vector<ParseDiagnostic> diagnostics;

    [[nodiscard]] bool ok() const noexcept {
        for (const auto& diagnostic : diagnostics) {
            if (diagnostic.severity == ParseSeverity::error) return false;
        }
        return true;
    }
};

namespace edit_detail {

inline void error(
    EditResult& result,
    std::string code,
    std::string message) {
    result.diagnostics.push_back(
        {ParseSeverity::error, std::move(code), std::move(message), 0U});
}

[[nodiscard]] inline Object* object_at(
    Document& document,
    std::size_t object_index,
    EditResult& result) {
    if (object_index >= document.objects.size()) {
        error(
            result,
            "scm.edit-object-out-of-range",
            "SCM object index is outside the current document.");
        return nullptr;
    }
    return &document.objects[object_index];
}

[[nodiscard]] inline Mesh* mesh_at(
    Document& document,
    std::size_t object_index,
    std::size_t mesh_index,
    EditResult& result) {
    auto* object = object_at(document, object_index, result);
    if (object == nullptr) return nullptr;
    if (mesh_index >= object->meshes.size()) {
        error(
            result,
            "scm.edit-mesh-out-of-range",
            "SCM mesh index is outside the selected object.");
        return nullptr;
    }
    return &object->meshes[mesh_index];
}

[[nodiscard]] inline std::optional<std::int16_t> encode_uv_component(
    float value) noexcept {
    if (!std::isfinite(value)) return std::nullopt;
    const auto scaled = static_cast<double>(value) *
                        static_cast<double>(scm_uv_scale);
    const auto minimum =
        static_cast<double>(std::numeric_limits<std::int16_t>::min());
    const auto maximum =
        static_cast<double>(std::numeric_limits<std::int16_t>::max());
    if (scaled < minimum - 0.5 || scaled > maximum + 0.5) {
        return std::nullopt;
    }
    const auto rounded = std::llround(scaled);
    if (rounded < std::numeric_limits<std::int16_t>::min() ||
        rounded > std::numeric_limits<std::int16_t>::max()) {
        return std::nullopt;
    }
    return static_cast<std::int16_t>(rounded);
}

} // namespace edit_detail

[[nodiscard]] inline EditResult set_vertex_position(
    Document& document,
    std::size_t object_index,
    std::size_t mesh_index,
    std::size_t vertex_index,
    Vec3f value) {
    EditResult result;
    auto* mesh = edit_detail::mesh_at(
        document, object_index, mesh_index, result);
    if (mesh == nullptr) return result;
    if (vertex_index >= mesh->positions.size()) {
        edit_detail::error(
            result,
            "scm.edit-vertex-out-of-range",
            "SCM vertex index is outside the position stream.");
        return result;
    }
    if (!std::isfinite(value.x) || !std::isfinite(value.y) ||
        !std::isfinite(value.z)) {
        edit_detail::error(
            result,
            "scm.edit-nonfinite-position",
            "SCM vertex positions must be finite floating-point values.");
        return result;
    }
    result.changed = mesh->positions[vertex_index].x != value.x ||
                     mesh->positions[vertex_index].y != value.y ||
                     mesh->positions[vertex_index].z != value.z;
    mesh->positions[vertex_index] = value;
    return result;
}

[[nodiscard]] inline EditResult set_vertex_normal(
    Document& document,
    std::size_t object_index,
    std::size_t mesh_index,
    std::size_t vertex_index,
    Vec3f value) {
    EditResult result;
    auto* mesh = edit_detail::mesh_at(
        document, object_index, mesh_index, result);
    if (mesh == nullptr) return result;
    if (vertex_index >= mesh->normals.size()) {
        edit_detail::error(
            result,
            "scm.edit-normal-out-of-range",
            "SCM vertex index is outside the normal stream.");
        return result;
    }
    if (!std::isfinite(value.x) || !std::isfinite(value.y) ||
        !std::isfinite(value.z)) {
        edit_detail::error(
            result,
            "scm.edit-nonfinite-normal",
            "SCM normals must contain finite floating-point values.");
        return result;
    }
    result.changed = mesh->normals[vertex_index].x != value.x ||
                     mesh->normals[vertex_index].y != value.y ||
                     mesh->normals[vertex_index].z != value.z;
    mesh->normals[vertex_index] = value;
    return result;
}

[[nodiscard]] inline EditResult set_uv_raw(
    Document& document,
    std::size_t object_index,
    std::size_t mesh_index,
    std::size_t vertex_index,
    SerializedUv value) {
    EditResult result;
    auto* mesh = edit_detail::mesh_at(
        document, object_index, mesh_index, result);
    if (mesh == nullptr) return result;
    if (vertex_index >= mesh->uvs.size()) {
        edit_detail::error(
            result,
            "scm.edit-uv-out-of-range",
            "SCM vertex index is outside the UV stream.");
        return result;
    }
    result.changed = mesh->uvs[vertex_index].u != value.u ||
                     mesh->uvs[vertex_index].v != value.v;
    mesh->uvs[vertex_index] = value;
    return result;
}

[[nodiscard]] inline EditResult set_uv(
    Document& document,
    std::size_t object_index,
    std::size_t mesh_index,
    std::size_t vertex_index,
    float u,
    float v) {
    EditResult result;
    const auto encoded_u = edit_detail::encode_uv_component(u);
    const auto encoded_v = edit_detail::encode_uv_component(v);
    if (!encoded_u.has_value() || !encoded_v.has_value()) {
        edit_detail::error(
            result,
            "scm.edit-uv-not-representable",
            "UV value cannot be represented by signed int16 SCM fixed-point "
            "coordinates at scale 1/4096.");
        return result;
    }
    return set_uv_raw(
        document,
        object_index,
        mesh_index,
        vertex_index,
        SerializedUv{*encoded_u, *encoded_v});
}

[[nodiscard]] inline EditResult set_texture_slot(
    Document& document,
    std::size_t object_index,
    std::size_t mesh_index,
    std::uint16_t texture_index) {
    EditResult result;
    auto* mesh = edit_detail::mesh_at(
        document, object_index, mesh_index, result);
    if (mesh == nullptr) return result;
    if (document.header.texture_slot_count != 0U &&
        texture_index >= document.header.texture_slot_count) {
        edit_detail::error(
            result,
            "scm.edit-texture-slot-out-of-range",
            "Texture slot exceeds the SCM mirror count. Bundle authoring must "
            "synchronize this field with the external texture companion.");
        return result;
    }
    result.changed = mesh->texture_index != texture_index;
    mesh->texture_index = texture_index;
    return result;
}

[[nodiscard]] inline EditResult set_alpha_control(
    Document& document,
    std::size_t object_index,
    std::uint8_t alpha_control) {
    EditResult result;
    auto* object = edit_detail::object_at(document, object_index, result);
    if (object == nullptr) return result;
    result.changed = object->alpha_control != alpha_control;
    object->alpha_control = alpha_control;
    return result;
}

[[nodiscard]] inline EditResult set_texture_filter_nearest(
    Document& document,
    std::size_t object_index,
    bool nearest) {
    EditResult result;
    auto* object = edit_detail::object_at(document, object_index, result);
    if (object == nullptr) return result;
    const auto before = object->flags;
    if (nearest) {
        object->flags |= object_flag_nearest_texture_filter;
    } else {
        object->flags &= ~object_flag_nearest_texture_filter;
    }
    result.changed = before != object->flags;
    return result;
}

[[nodiscard]] inline EditResult set_region_repeat(
    Document& document,
    std::size_t object_index,
    std::size_t mesh_index,
    LegacyGsClampRegionRepeat value) {
    EditResult result;
    auto* mesh = edit_detail::mesh_at(
        document, object_index, mesh_index, result);
    if (mesh == nullptr) return result;
    if (!legacy_gs_clamp_fields_fit_register(value)) {
        edit_detail::error(
            result,
            "scm.edit-gs-clamp-out-of-range",
            "Safe SCM editing requires each GS CLAMP REGION_REPEAT field to "
            "fit the 10-bit hardware register width.");
        return result;
    }
    result.changed =
        mesh->gs_clamp_region_repeat.min_u != value.min_u ||
        mesh->gs_clamp_region_repeat.max_u != value.max_u ||
        mesh->gs_clamp_region_repeat.min_v != value.min_v ||
        mesh->gs_clamp_region_repeat.max_v != value.max_v;
    mesh->gs_clamp_region_repeat = value;
    return result;
}

[[nodiscard]] inline EditResult set_node_translation(
    Document& document,
    std::size_t node_index,
    Vec3f value) {
    EditResult result;
    auto& transforms = document.scene_nodes.transform_by_node_index;
    if (node_index >= transforms.size()) {
        edit_detail::error(
            result,
            "scm.edit-node-out-of-range",
            "SCM scene-node index is outside the transform table.");
        return result;
    }
    if (!std::isfinite(value.x) || !std::isfinite(value.y) ||
        !std::isfinite(value.z)) {
        edit_detail::error(
            result,
            "scm.edit-nonfinite-translation",
            "SCM scene translation must be finite.");
        return result;
    }
    auto& transform = transforms[node_index];
    result.changed = transform.translation.x != value.x ||
                     transform.translation.y != value.y ||
                     transform.translation.z != value.z;
    transform.translation = value;
    transform.translation_magnitude =
        std::sqrt(value.x * value.x + value.y * value.y + value.z * value.z);
    return result;
}

[[nodiscard]] inline EditResult set_node_rotation(
    Document& document,
    std::size_t node_index,
    Vec3f xyz_radians) {
    EditResult result;
    auto& transforms = document.scene_nodes.transform_by_node_index;
    if (node_index >= transforms.size()) {
        edit_detail::error(
            result,
            "scm.edit-node-out-of-range",
            "SCM scene-node index is outside the transform table.");
        return result;
    }
    if (!std::isfinite(xyz_radians.x) || !std::isfinite(xyz_radians.y) ||
        !std::isfinite(xyz_radians.z)) {
        edit_detail::error(
            result,
            "scm.edit-nonfinite-rotation",
            "SCM XYZ Euler rotation must contain finite radians.");
        return result;
    }
    auto& rotation = transforms[node_index].rotation_xyz_radians;
    result.changed = rotation.x != xyz_radians.x ||
                     rotation.y != xyz_radians.y ||
                     rotation.z != xyz_radians.z;
    rotation = xyz_radians;
    return result;
}

} // namespace dmc::rengine::formats::scm
