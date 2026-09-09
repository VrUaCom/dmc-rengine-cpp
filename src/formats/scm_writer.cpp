#include "dmc_rengine/formats/scm_writer.hpp"

#include "dmc_rengine/formats/scm_layout.hpp"
#include "scm_internal.hpp"

#include <algorithm>
#include <array>
#include <bit>
#include <cmath>
#include <cstring>
#include <limits>
#include <type_traits>

namespace dmc::rengine::formats::scm {
namespace {

using detail::Reader;

void add_diag(
    WriteResult& out,
    ParseSeverity severity,
    std::string code,
    std::string message,
    std::uint64_t offset = 0U) {
    out.diagnostics.push_back(
        {severity, std::move(code), std::move(message), offset});
}

[[nodiscard]] bool has_error(const WriteResult& out) noexcept {
    return std::any_of(
        out.diagnostics.begin(),
        out.diagnostics.end(),
        [](const ParseDiagnostic& diagnostic) {
            return diagnostic.severity == ParseSeverity::error;
        });
}

template <class T>
[[nodiscard]] std::array<std::byte, sizeof(T)> little_endian_bytes(
    T value) noexcept {
    static_assert(std::is_trivially_copyable_v<T>);
    std::array<std::byte, sizeof(T)> bytes{};
    std::memcpy(bytes.data(), &value, sizeof(T));
    if constexpr (std::endian::native == std::endian::big &&
                  sizeof(T) > 1U) {
        std::reverse(bytes.begin(), bytes.end());
    }
    return bytes;
}

template <class T>
[[nodiscard]] bool write_value(
    std::span<std::byte> bytes,
    std::uint64_t offset,
    T value) noexcept {
    const auto encoded = little_endian_bytes(value);
    if (offset > bytes.size() ||
        encoded.size() > bytes.size() - static_cast<std::size_t>(offset)) {
        return false;
    }
    std::copy(
        encoded.begin(),
        encoded.end(),
        bytes.begin() + static_cast<std::ptrdiff_t>(offset));
    return true;
}

[[nodiscard]] bool write_raw(
    std::span<std::byte> bytes,
    std::uint64_t offset,
    std::span<const std::byte> source) noexcept {
    if (offset > bytes.size() ||
        source.size() > bytes.size() - static_cast<std::size_t>(offset)) {
        return false;
    }
    std::copy(
        source.begin(),
        source.end(),
        bytes.begin() + static_cast<std::ptrdiff_t>(offset));
    return true;
}

[[nodiscard]] bool same_float_bits(float lhs, float rhs) noexcept {
    return std::bit_cast<std::uint32_t>(lhs) ==
           std::bit_cast<std::uint32_t>(rhs);
}

[[nodiscard]] bool source_float_equals(
    const Reader& source,
    std::uint64_t offset,
    float value) noexcept {
    float original{};
    return source.read(offset, original) && same_float_bits(original, value);
}

[[nodiscard]] bool source_vec3_equals(
    const Reader& source,
    std::uint64_t offset,
    const Vec3f& value) noexcept {
    return source_float_equals(source, offset + 0U, value.x) &&
           source_float_equals(source, offset + 4U, value.y) &&
           source_float_equals(source, offset + 8U, value.z);
}

[[nodiscard]] float vector_length(const Vec3f& value) noexcept {
    return std::sqrt(
        value.x * value.x + value.y * value.y + value.z * value.z);
}

[[nodiscard]] bool finite_vec3(const Vec3f& value) noexcept {
    return std::isfinite(value.x) && std::isfinite(value.y) &&
           std::isfinite(value.z);
}

[[nodiscard]] float radius_from_center(
    const Object& object,
    bool& finite) noexcept {
    finite = finite_vec3(object.bounding_center);
    float radius = 0.0F;
    for (const auto& mesh : object.meshes) {
        for (const auto& position : mesh.positions) {
            if (!finite_vec3(position)) {
                finite = false;
                return object.bounding_radius;
            }
            const auto dx = position.x - object.bounding_center.x;
            const auto dy = position.y - object.bounding_center.y;
            const auto dz = position.z - object.bounding_center.z;
            radius = std::max(
                radius,
                std::sqrt(dx * dx + dy * dy + dz * dz));
        }
    }
    return radius;
}

[[nodiscard]] bool geometry_or_center_changed(
    const Document& document,
    const Object& object) noexcept {
    if (document.source_bytes.empty()) return true;
    const Reader source{
        std::span<const std::byte>{document.source_bytes}};
    if (!source_vec3_equals(
            source,
            object.record_offset + 0x30U,
            object.bounding_center)) {
        return true;
    }
    for (const auto& mesh : object.meshes) {
        if (mesh.positions.size() != mesh.vertex_count) return true;
        for (std::size_t index = 0U; index < mesh.positions.size(); ++index) {
            const auto offset =
                mesh.positions_offset +
                static_cast<std::uint64_t>(index) * 12U;
            if (!source_vec3_equals(source, offset, mesh.positions[index])) {
                return true;
            }
        }
    }
    return false;
}

[[nodiscard]] bool translation_changed(
    const Document& document,
    std::size_t node_index,
    const SceneTransform& transform) noexcept {
    if (document.source_bytes.empty()) return true;
    const auto& scene = document.scene_nodes;
    const auto offset =
        scene.offset + scene.transform_rel +
        static_cast<std::uint64_t>(node_index) * scene_transform_size;
    const Reader source{
        std::span<const std::byte>{document.source_bytes}};
    return !source_vec3_equals(source, offset, transform.translation);
}

[[nodiscard]] bool validate_stream_shapes(
    const Document& document,
    WriteMode mode,
    WriteResult& out,
    std::vector<ObjectShape>& shapes) {
    if (document.objects.size() >
        std::numeric_limits<std::uint8_t>::max()) {
        add_diag(
            out, ParseSeverity::error, "scm.writer-object-count-overflow",
            "SCM writer supports at most 255 objects.");
        return false;
    }

    shapes.clear();
    shapes.reserve(document.objects.size());

    for (std::size_t object_index = 0U;
         object_index < document.objects.size();
         ++object_index) {
        const auto& object = document.objects[object_index];
        if (object.meshes.size() >
            std::numeric_limits<std::uint8_t>::max()) {
            add_diag(
                out, ParseSeverity::error, "scm.writer-mesh-count-overflow",
                "SCM object contains more than 255 meshes.",
                object.record_offset);
            continue;
        }

        if (mode == WriteMode::preserve_layout &&
            object.meshes.size() != object.mesh_count) {
            add_diag(
                out, ParseSeverity::error,
                "scm.writer-preserve-layout-mesh-count-changed",
                "Preserve-layout mode cannot add or remove meshes.",
                object.record_offset);
        }

        ObjectShape shape;
        shape.mesh_vertex_counts.reserve(object.meshes.size());
        std::uint64_t total_vertices = 0U;

        for (const auto& mesh : object.meshes) {
            const auto count = mesh.positions.size();
            if (count > std::numeric_limits<std::uint16_t>::max()) {
                add_diag(
                    out, ParseSeverity::error,
                    "scm.writer-vertex-count-overflow",
                    "SCM mesh exceeds the 16-bit serialized vertex-count "
                    "domain.",
                    mesh.record_offset);
                continue;
            }
            if (mesh.normals.size() != count ||
                mesh.uvs.size() != count ||
                mesh.colors_topology.size() != count) {
                add_diag(
                    out, ParseSeverity::error,
                    "scm.writer-stream-count-mismatch",
                    "Position, normal, UV and color/topology streams must have "
                    "the same vertex count.",
                    mesh.record_offset);
                continue;
            }
            if (mode == WriteMode::preserve_layout &&
                count != mesh.vertex_count) {
                add_diag(
                    out, ParseSeverity::error,
                    "scm.writer-preserve-layout-vertex-count-changed",
                    "Preserve-layout mode cannot change mesh vertex counts.",
                    mesh.record_offset);
            }

            total_vertices += count;
            shape.mesh_vertex_counts.push_back(
                static_cast<std::uint16_t>(count));
        }

        if (total_vertices >
            std::numeric_limits<std::uint16_t>::max()) {
            add_diag(
                out, ParseSeverity::error,
                "scm.writer-object-vertex-count-overflow",
                "Object total vertex count exceeds the 16-bit serialized "
                "field.",
                object.record_offset);
        }
        if (mode == WriteMode::preserve_layout &&
            total_vertices != object.total_vertex_count) {
            add_diag(
                out, ParseSeverity::error,
                "scm.writer-preserve-layout-total-vertex-count-changed",
                "Preserve-layout mode requires the source object total vertex "
                "count to remain unchanged.",
                object.record_offset + 0x02U);
        }

        shapes.push_back(std::move(shape));
    }

    return !has_error(out);
}

[[nodiscard]] bool validate_scene_shape(
    const Document& document,
    WriteMode mode,
    WriteResult& out,
    std::uint8_t& node_count) {
    const auto& scene = document.scene_nodes;
    const auto count = scene.transform_by_node_index.size();
    if (count > std::numeric_limits<std::uint8_t>::max()) {
        add_diag(
            out, ParseSeverity::error, "scm.writer-node-count-overflow",
            "SCM scene contains more than 255 nodes.");
        return false;
    }
    if (scene.parent_by_order_position.size() != count ||
        scene.node_at_order_position.size() != count ||
        scene.object_binding_by_node_index.size() != count) {
        add_diag(
            out, ParseSeverity::error, "scm.writer-scene-array-count-mismatch",
            "All scene hierarchy arrays and transform arrays must have the "
            "same node count.",
            scene.offset);
        return false;
    }
    if (mode == WriteMode::preserve_layout &&
        count != document.header.scene_node_count) {
        add_diag(
            out, ParseSeverity::error,
            "scm.writer-preserve-layout-node-count-changed",
            "Preserve-layout mode cannot add or remove scene nodes.",
            scene.offset);
        return false;
    }
    node_count = static_cast<std::uint8_t>(count);
    return true;
}

[[nodiscard]] bool write_mesh_streams(
    std::span<std::byte> bytes,
    const Mesh& mesh,
    const MeshSerializedLayout& layout) noexcept {
    for (std::size_t index = 0U; index < mesh.positions.size(); ++index) {
        const auto p =
            layout.positions_offset +
            static_cast<std::uint64_t>(index) * 12U;
        const auto n =
            layout.normals_offset +
            static_cast<std::uint64_t>(index) * 12U;
        const auto uv =
            layout.uv_offset +
            static_cast<std::uint64_t>(index) * 4U;
        const auto color =
            layout.color_flags_offset +
            static_cast<std::uint64_t>(index) * 4U;

        if (!write_value(bytes, p + 0U, mesh.positions[index].x) ||
            !write_value(bytes, p + 4U, mesh.positions[index].y) ||
            !write_value(bytes, p + 8U, mesh.positions[index].z) ||
            !write_value(bytes, n + 0U, mesh.normals[index].x) ||
            !write_value(bytes, n + 4U, mesh.normals[index].y) ||
            !write_value(bytes, n + 8U, mesh.normals[index].z) ||
            !write_value(bytes, uv + 0U, mesh.uvs[index].u) ||
            !write_value(bytes, uv + 2U, mesh.uvs[index].v)) {
            return false;
        }

        const auto& value = mesh.colors_topology[index];
        if (!write_value(bytes, color + 0U, value.r) ||
            !write_value(bytes, color + 1U, value.g) ||
            !write_value(bytes, color + 2U, value.b) ||
            !write_value(bytes, color + 3U, value.topology_flags)) {
            return false;
        }
    }
    return true;
}

[[nodiscard]] bool write_scene(
    std::span<std::byte> bytes,
    const Document& document,
    const SceneSerializedLayout& layout) {
    const auto& scene = document.scene_nodes;
    if (!write_value(bytes, layout.block_offset + 0x00U, layout.parent_rel) ||
        !write_value(bytes, layout.block_offset + 0x04U, layout.order_rel) ||
        !write_value(
            bytes, layout.block_offset + 0x08U, layout.object_binding_rel) ||
        !write_value(
            bytes, layout.block_offset + 0x0CU, layout.transform_rel) ||
        !write_raw(
            bytes,
            layout.block_offset + 0x10U,
            std::span<const std::byte>{scene.reserved10_1f})) {
        return false;
    }

    for (std::size_t index = 0U;
         index < scene.transform_by_node_index.size();
         ++index) {
        const auto parent = static_cast<std::uint8_t>(
            scene.parent_by_order_position[index]);
        const auto binding = static_cast<std::uint8_t>(
            scene.object_binding_by_node_index[index]);
        if (!write_value(
                bytes,
                layout.block_offset + layout.parent_rel + index,
                parent) ||
            !write_value(
                bytes,
                layout.block_offset + layout.order_rel + index,
                scene.node_at_order_position[index]) ||
            !write_value(
                bytes,
                layout.block_offset + layout.object_binding_rel + index,
                binding)) {
            return false;
        }

        const auto& transform = scene.transform_by_node_index[index];
        const auto offset =
            layout.block_offset + layout.transform_rel +
            static_cast<std::uint64_t>(index) * scene_transform_size;
        float magnitude = transform.translation_magnitude;
        if (translation_changed(document, index, transform)) {
            magnitude = vector_length(transform.translation);
        }

        if (!write_value(bytes, offset + 0x00U, transform.translation.x) ||
            !write_value(bytes, offset + 0x04U, transform.translation.y) ||
            !write_value(bytes, offset + 0x08U, transform.translation.z) ||
            !write_value(bytes, offset + 0x0CU, magnitude) ||
            !write_value(
                bytes, offset + 0x10U, transform.rotation_xyz_radians.x) ||
            !write_value(
                bytes, offset + 0x14U, transform.rotation_xyz_radians.y) ||
            !write_value(
                bytes, offset + 0x18U, transform.rotation_xyz_radians.z) ||
            !write_value(bytes, offset + 0x1CU, transform.reserved1c)) {
            return false;
        }
    }

    return true;
}

[[nodiscard]] bool write_document(
    const Document& document,
    WriteMode mode,
    const SerializedLayout& layout,
    std::span<const ObjectShape> shapes,
    std::vector<std::byte>& bytes,
    WriteResult& out) {
    const bool preserve_layout = mode == WriteMode::preserve_layout;
    const auto object_count =
        static_cast<std::uint8_t>(document.objects.size());
    const auto node_count =
        static_cast<std::uint8_t>(
            document.scene_nodes.transform_by_node_index.size());

    if (!write_raw(bytes, 0U, std::span<const std::byte>{magic}) ||
        !write_value(bytes, 0x04U, document.header.version) ||
        !write_value(bytes, 0x08U, document.header.reserved08) ||
        !write_value(bytes, 0x10U, object_count) ||
        !write_value(bytes, 0x11U, node_count) ||
        !write_value(bytes, 0x12U, document.header.texture_slot_count) ||
        !write_value(bytes, 0x13U, document.header.reserved13) ||
        !write_value(bytes, 0x14U, document.header.resource_code.raw) ||
        !write_value(bytes, 0x18U, document.header.reserved18) ||
        !write_value(bytes, 0x20U, layout.scene.block_offset) ||
        !write_value(bytes, 0x28U, document.header.reserved28) ||
        !write_value(bytes, 0x30U, document.header.reserved30) ||
        !write_value(bytes, 0x38U, document.header.reserved38)) {
        add_diag(
            out, ParseSeverity::error, "scm.writer-output-out-of-bounds",
            "Failed to serialize SCM header within the planned output.");
        return false;
    }

    for (std::size_t object_index = 0U;
         object_index < document.objects.size();
         ++object_index) {
        const auto& object = document.objects[object_index];
        const auto& object_layout = layout.objects[object_index];
        const auto object_offset = object_layout.record_offset;
        const auto mesh_count =
            static_cast<std::uint8_t>(object.meshes.size());

        std::uint64_t total_vertices = 0U;
        for (const auto count : shapes[object_index].mesh_vertex_counts) {
            total_vertices += count;
        }
        const auto total_vertex_count =
            static_cast<std::uint16_t>(total_vertices);

        float radius = object.bounding_radius;
        if (geometry_or_center_changed(document, object)) {
            bool finite = true;
            radius = radius_from_center(object, finite);
            if (!finite) {
                add_diag(
                    out, ParseSeverity::error,
                    "scm.writer-nonfinite-bounds-input",
                    "Cannot recompute SCM bounding radius from non-finite "
                    "geometry or center.",
                    object_offset + 0x30U);
                return false;
            }
        }

        if (!write_value(bytes, object_offset + 0x00U, mesh_count) ||
            !write_value(
                bytes, object_offset + 0x01U, object.alpha_control) ||
            !write_value(
                bytes, object_offset + 0x02U, total_vertex_count) ||
            !write_value(
                bytes, object_offset + 0x04U, object.reserved04) ||
            !write_value(
                bytes,
                object_offset + 0x08U,
                object_layout.mesh_table_offset) ||
            !write_value(bytes, object_offset + 0x10U, object.flags) ||
            !write_raw(
                bytes,
                object_offset + 0x14U,
                std::span<const std::byte>{object.reserved14_2f}) ||
            !write_value(
                bytes, object_offset + 0x30U, object.bounding_center.x) ||
            !write_value(
                bytes, object_offset + 0x34U, object.bounding_center.y) ||
            !write_value(
                bytes, object_offset + 0x38U, object.bounding_center.z) ||
            !write_value(bytes, object_offset + 0x3CU, radius)) {
            add_diag(
                out, ParseSeverity::error, "scm.writer-output-out-of-bounds",
                "Failed to serialize SCM object within the planned output.",
                object_offset);
            return false;
        }

        for (std::size_t mesh_index = 0U;
             mesh_index < object.meshes.size();
             ++mesh_index) {
            const auto& mesh = object.meshes[mesh_index];
            const auto& mesh_layout = object_layout.meshes[mesh_index];
            const auto mesh_offset = mesh_layout.record_offset;
            const auto vertex_count =
                shapes[object_index].mesh_vertex_counts[mesh_index];
            const auto continuation =
                mesh_index + 1U < object.meshes.size()
                    ? static_cast<std::uint64_t>(mesh_record_size)
                    : 0U;
            const auto workspace_relative =
                mesh_layout.index_workspace_offset - mesh_offset;
            const auto generated_index_count =
                preserve_layout ? mesh.generated_index_count : 0U;

            if (!write_value(
                    bytes, mesh_offset + 0x00U, vertex_count) ||
                !write_value(
                    bytes, mesh_offset + 0x02U, mesh.texture_index) ||
                !write_value(
                    bytes,
                    mesh_offset + 0x04U,
                    mesh.gs_clamp_region_repeat.min_u) ||
                !write_value(
                    bytes,
                    mesh_offset + 0x06U,
                    mesh.gs_clamp_region_repeat.max_u) ||
                !write_value(
                    bytes,
                    mesh_offset + 0x08U,
                    mesh.gs_clamp_region_repeat.min_v) ||
                !write_value(
                    bytes,
                    mesh_offset + 0x0AU,
                    mesh.gs_clamp_region_repeat.max_v) ||
                !write_value(
                    bytes, mesh_offset + 0x0CU, mesh.reserved0c) ||
                !write_value(
                    bytes,
                    mesh_offset + 0x10U,
                    mesh_layout.positions_offset) ||
                !write_value(
                    bytes,
                    mesh_offset + 0x18U,
                    mesh_layout.normals_offset) ||
                !write_value(
                    bytes, mesh_offset + 0x20U, mesh_layout.uv_offset) ||
                !write_value(
                    bytes, mesh_offset + 0x28U, continuation) ||
                !write_value(
                    bytes, mesh_offset + 0x30U, mesh.reserved30) ||
                !write_value(
                    bytes,
                    mesh_offset + 0x38U,
                    mesh_layout.color_flags_offset) ||
                !write_value(
                    bytes,
                    mesh_offset + 0x40U,
                    workspace_relative) ||
                !write_value(
                    bytes,
                    mesh_offset + 0x48U,
                    generated_index_count) ||
                !write_value(
                    bytes, mesh_offset + 0x4CU, mesh.reserved4c) ||
                !write_mesh_streams(bytes, mesh, mesh_layout)) {
                add_diag(
                    out, ParseSeverity::error,
                    "scm.writer-output-out-of-bounds",
                    "Failed to serialize SCM mesh or vertex streams within "
                    "the planned output.",
                    mesh_offset);
                return false;
            }

            if (!preserve_layout &&
                mesh_layout.index_workspace_capacity >= 2U &&
                !write_value(
                    bytes,
                    mesh_layout.index_workspace_offset,
                    index_workspace_sentinel)) {
                add_diag(
                    out, ParseSeverity::error,
                    "scm.writer-output-out-of-bounds",
                    "Failed to emit SCM index-workspace regeneration "
                    "sentinel.",
                    mesh_layout.index_workspace_offset);
                return false;
            }
        }
    }

    if (!write_scene(
            bytes,
            document,
            layout.scene)) {
        add_diag(
            out, ParseSeverity::error, "scm.writer-output-out-of-bounds",
            "Failed to serialize SCM scene hierarchy within the planned "
            "output.",
            layout.scene.block_offset);
        return false;
    }

    return true;
}

[[nodiscard]] bool source_layout_matches_plan(
    const Document& document,
    const SerializedLayout& layout,
    std::span<const ObjectShape> shapes) noexcept {
    if (document.source_bytes.size() != layout.file_size ||
        document.objects.size() != layout.objects.size() ||
        document.scene_nodes.offset != layout.scene.block_offset ||
        document.scene_nodes.parent_rel != layout.scene.parent_rel ||
        document.scene_nodes.order_rel != layout.scene.order_rel ||
        document.scene_nodes.object_binding_rel !=
            layout.scene.object_binding_rel ||
        document.scene_nodes.transform_rel != layout.scene.transform_rel) {
        return false;
    }

    for (std::size_t object_index = 0U;
         object_index < document.objects.size();
         ++object_index) {
        const auto& object = document.objects[object_index];
        const auto& planned = layout.objects[object_index];
        if (object.record_offset != planned.record_offset ||
            object.mesh_table_offset != planned.mesh_table_offset ||
            object.meshes.size() != planned.meshes.size() ||
            object.meshes.size() !=
                shapes[object_index].mesh_vertex_counts.size()) {
            return false;
        }

        for (std::size_t mesh_index = 0U;
             mesh_index < object.meshes.size();
             ++mesh_index) {
            const auto& mesh = object.meshes[mesh_index];
            const auto& mesh_plan = planned.meshes[mesh_index];
            if (mesh.vertex_count !=
                    shapes[object_index].mesh_vertex_counts[mesh_index] ||
                mesh.record_offset != mesh_plan.record_offset ||
                mesh.positions_offset != mesh_plan.positions_offset ||
                mesh.normals_offset != mesh_plan.normals_offset ||
                mesh.uv_offset != mesh_plan.uv_offset ||
                mesh.color_flags_offset != mesh_plan.color_flags_offset ||
                mesh.index_workspace_offset !=
                    mesh_plan.index_workspace_offset ||
                mesh.index_workspace_capacity !=
                    mesh_plan.index_workspace_capacity) {
                return false;
            }
        }
    }
    return true;
}

[[nodiscard]] SerializedLayout preserve_layout_from_document(
    const Document& document) {
    SerializedLayout layout;
    layout.objects.reserve(document.objects.size());
    for (const auto& object : document.objects) {
        ObjectSerializedLayout object_layout;
        object_layout.record_offset = object.record_offset;
        object_layout.mesh_table_offset = object.mesh_table_offset;
        object_layout.meshes.reserve(object.meshes.size());
        for (const auto& mesh : object.meshes) {
            object_layout.meshes.push_back(MeshSerializedLayout{
                .record_offset = mesh.record_offset,
                .positions_offset = mesh.positions_offset,
                .normals_offset = mesh.normals_offset,
                .uv_offset = mesh.uv_offset,
                .color_flags_offset = mesh.color_flags_offset,
                .index_workspace_offset = mesh.index_workspace_offset,
                .index_workspace_capacity = mesh.index_workspace_capacity,
            });
        }
        layout.objects.push_back(std::move(object_layout));
    }
    layout.scene.block_offset = document.scene_nodes.offset;
    layout.scene.parent_rel = document.scene_nodes.parent_rel;
    layout.scene.order_rel = document.scene_nodes.order_rel;
    layout.scene.object_binding_rel =
        document.scene_nodes.object_binding_rel;
    layout.scene.transform_rel = document.scene_nodes.transform_rel;
    layout.file_size = document.source_bytes.size();
    return layout;
}

} // namespace

bool WriteResult::ok() const noexcept {
    return wrote && reparse_ok && !has_error(*this);
}

WriteResult Writer::write(
    const Document& document,
    WriteMode mode) {
    WriteResult out;

    std::vector<ObjectShape> shapes;
    if (!validate_stream_shapes(document, mode, out, shapes)) {
        return out;
    }

    std::uint8_t node_count{};
    if (!validate_scene_shape(document, mode, out, node_count)) {
        return out;
    }

    SerializedLayout layout;
    if (mode == WriteMode::preserve_layout) {
        if (document.source_bytes.empty()) {
            add_diag(
                out, ParseSeverity::error,
                "scm.writer-preserve-layout-source-required",
                "Preserve-layout mode requires a parsed SCM source image.");
            return out;
        }
        if (document.objects.size() != document.header.object_count) {
            add_diag(
                out, ParseSeverity::error,
                "scm.writer-preserve-layout-object-count-changed",
                "Preserve-layout mode cannot add or remove objects.");
            return out;
        }
        layout = preserve_layout_from_document(document);
        out.bytes = document.source_bytes;
    } else {
        layout = build_serialized_layout(
            std::span<const ObjectShape>{shapes},
            node_count);
        if (layout.file_size >
            std::numeric_limits<std::size_t>::max()) {
            add_diag(
                out, ParseSeverity::error,
                "scm.writer-output-size-overflow",
                "Canonical SCM rebuild exceeds addressable output size.");
            return out;
        }
        if (source_layout_matches_plan(
                document,
                layout,
                std::span<const ObjectShape>{shapes})) {
            // Source-bound unknown padding/workspace bytes are preservation
            // evidence. Reuse them only when the newly planned canonical
            // layout is exactly the same shape and offsets.
            out.bytes = document.source_bytes;
        } else {
            out.bytes.assign(
                static_cast<std::size_t>(layout.file_size),
                std::byte{0});
        }
    }

    if (!write_document(
            document,
            mode,
            layout,
            std::span<const ObjectShape>{shapes},
            out.bytes,
            out)) {
        out.bytes.clear();
        return out;
    }

    out.wrote = true;
    out.bit_identical_to_source =
        !document.source_bytes.empty() &&
        out.bytes == document.source_bytes;

    const auto reparsed =
        Parser::parse(std::span<const std::byte>{out.bytes});
    out.reparse_ok = reparsed.ok();
    out.diagnostics.insert(
        out.diagnostics.end(),
        reparsed.diagnostics.begin(),
        reparsed.diagnostics.end());

    if (!out.reparse_ok) {
        add_diag(
            out, ParseSeverity::error, "scm.writer-reparse-failed",
            "Generated SCM failed the canonical parser/validator reparse "
            "gate.");
    }

    return out;
}

} // namespace dmc::rengine::formats::scm
