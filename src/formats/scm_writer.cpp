#include "dmc_rengine/formats/scm_writer.hpp"

#include "dmc_rengine/formats/scm_layout.hpp"
#include "scm_internal.hpp"

#include <algorithm>
#include <array>
#include <bit>
#include <cmath>
#include <cstring>
#include <limits>
#include <optional>
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

[[nodiscard]] bool same_vec3_bits(
    const Vec3f& lhs,
    const Vec3f& rhs) noexcept {
    return same_float_bits(lhs.x, rhs.x) &&
           same_float_bits(lhs.y, rhs.y) &&
           same_float_bits(lhs.z, rhs.z);
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

[[nodiscard]] bool mark_source_span(
    std::vector<std::uint8_t>& handled,
    std::uint64_t offset,
    std::uint64_t size) noexcept {
    if (offset > handled.size() ||
        size > handled.size() - static_cast<std::size_t>(offset)) {
        return false;
    }
    std::fill(
        handled.begin() + static_cast<std::ptrdiff_t>(offset),
        handled.begin() + static_cast<std::ptrdiff_t>(offset + size),
        std::uint8_t{1});
    return true;
}

[[nodiscard]] std::optional<std::uint64_t>
first_nonzero_unmodeled_source_byte(const Document& source_document) {
    const auto& source = source_document.source_bytes;
    std::vector<std::uint8_t> handled(source.size(), std::uint8_t{0});

    const auto mark = [&](std::uint64_t offset, std::uint64_t size) {
        return mark_source_span(handled, offset, size);
    };

    if (!mark(0U, header_size)) {
        return 0U;
    }

    for (const auto& object : source_document.objects) {
        if (!mark(object.record_offset, object_record_size)) {
            return object.record_offset;
        }
        for (const auto& mesh : object.meshes) {
            const auto vertex_count =
                static_cast<std::uint64_t>(mesh.vertex_count);
            if (!mark(mesh.record_offset, mesh_record_size) ||
                !mark(mesh.positions_offset, vertex_count * 12U) ||
                !mark(mesh.normals_offset, vertex_count * 12U) ||
                !mark(mesh.uv_offset, vertex_count * 4U) ||
                !mark(mesh.color_flags_offset, vertex_count * 4U) ||
                // Index workspace is an explicitly regenerated domain in
                // canonical rebuild mode. Its source bytes are therefore not
                // unknown evidence that must be transplanted across reflow.
                !mark(
                    mesh.index_workspace_offset,
                    mesh.index_workspace_capacity)) {
                return mesh.record_offset;
            }
        }
    }

    const auto& scene = source_document.scene_nodes;
    const auto node_count = static_cast<std::uint64_t>(
        scene.transform_by_node_index.size());
    if (!mark(scene.offset, scene_block_header_size) ||
        !mark(scene.offset + scene.parent_rel, node_count) ||
        !mark(scene.offset + scene.order_rel, node_count) ||
        !mark(scene.offset + scene.object_binding_rel, node_count) ||
        !mark(
            scene.offset + scene.transform_rel,
            node_count * scene_transform_size)) {
        return scene.offset;
    }

    for (std::size_t index = 0U; index < source.size(); ++index) {
        if (handled[index] == 0U && source[index] != std::byte{0}) {
            return static_cast<std::uint64_t>(index);
        }
    }
    return std::nullopt;
}

[[nodiscard]] bool validate_source_bound_authority(
    const Document& document,
    const Document& source_document,
    WriteResult& out) {
    const auto reject_unknown = [&](std::uint64_t offset, const char* field) {
        add_diag(
            out,
            ParseSeverity::error,
            "scm.writer-source-bound-undecoded-field-mutated",
            std::string{"Source-bound SCM authoring cannot mutate undecoded "}
                + field +
                ". Preserve the source value until a dedicated evidence-backed "
                "authoring contract exists.",
            offset);
        return false;
    };
    const auto reject_derived = [&](std::uint64_t offset, const char* field) {
        add_diag(
            out,
            ParseSeverity::error,
            "scm.writer-source-bound-derived-field-mutated",
            std::string{"Source-bound SCM authoring cannot directly mutate "}
                + field +
                ". Change its authoritative typed inputs so the writer can "
                "derive the serialized value.",
            offset);
        return false;
    };
    const auto reject_structural = [&](std::uint64_t offset, const char* field) {
        add_diag(
            out,
            ParseSeverity::error,
            "scm.writer-source-bound-structural-field-mutated",
            std::string{"Source-bound SCM authoring cannot mutate physical "}
                + field +
                ". Source offsets are immutable provenance metadata; canonical "
                "reflow offsets are owned by the layout planner.",
            offset);
        return false;
    };
    const auto reject_invalid = [&](std::uint64_t offset, const char* field) {
        add_diag(
            out,
            ParseSeverity::error,
            "scm.writer-source-bound-invalid-authored-value",
            std::string{"Source-bound SCM authoring rejected invalid changed "}
                + field + ".",
            offset);
        return false;
    };

    const auto& header = document.header;
    const auto& source_header = source_document.header;
    if (header.reserved08 != source_header.reserved08) {
        return reject_unknown(0x08U, "header +0x08");
    }
    if (header.reserved13 != source_header.reserved13) {
        return reject_unknown(0x13U, "header +0x13");
    }
    if (header.reserved18 != source_header.reserved18) {
        return reject_unknown(0x18U, "header +0x18");
    }
    if (header.reserved28 != source_header.reserved28) {
        return reject_unknown(0x28U, "header +0x28");
    }
    if (header.reserved30 != source_header.reserved30) {
        return reject_unknown(0x30U, "header +0x30");
    }
    if (header.reserved38 != source_header.reserved38) {
        return reject_unknown(0x38U, "header +0x38");
    }
    if (header.scene_node_block_offset !=
        source_header.scene_node_block_offset) {
        return reject_structural(0x20U, "scene-node block offset");
    }
    if (header.object_count != source_header.object_count) {
        return reject_derived(0x10U, "header object count");
    }
    if (header.scene_node_count != source_header.scene_node_count) {
        return reject_derived(0x11U, "header scene-node count");
    }

    constexpr std::uint32_t mutable_object_flags =
        object_flag_nearest_texture_filter;

    for (std::size_t object_index = 0U;
         object_index < document.objects.size();
         ++object_index) {
        const auto& object = document.objects[object_index];
        if (object_index < source_document.objects.size()) {
            const auto& source_object = source_document.objects[object_index];
            if (object.record_offset != source_object.record_offset) {
                return reject_structural(
                    source_object.record_offset,
                    "object record offset");
            }
            if (object.mesh_table_offset != source_object.mesh_table_offset) {
                return reject_structural(
                    source_object.record_offset + 0x08U,
                    "object mesh-table offset");
            }
            if (object.mesh_count != source_object.mesh_count) {
                return reject_derived(
                    source_object.record_offset + 0x00U,
                    "object mesh count");
            }
            if (object.total_vertex_count !=
                source_object.total_vertex_count) {
                return reject_derived(
                    source_object.record_offset + 0x02U,
                    "object total vertex count");
            }
            if (object.reserved04 != source_object.reserved04) {
                return reject_unknown(
                    source_object.record_offset + 0x04U,
                    "object +0x04");
            }
            if (((object.flags ^ source_object.flags) &
                 ~mutable_object_flags) != 0U) {
                return reject_unknown(
                    source_object.record_offset + 0x10U,
                    "object flag bits outside confirmed mutable mask 0x00004000");
            }
            if (object.reserved14_2f != source_object.reserved14_2f) {
                return reject_unknown(
                    source_object.record_offset + 0x14U,
                    "object +0x14..+0x2F");
            }
            if (!geometry_or_center_changed(document, object) &&
                !same_float_bits(
                    object.bounding_radius,
                    source_object.bounding_radius)) {
                return reject_derived(
                    source_object.record_offset + 0x3CU,
                    "object bounding radius");
            }
        } else {
            if (object.reserved04 != 0U) {
                return reject_unknown(0U, "new object +0x04");
            }
            if ((object.flags & ~mutable_object_flags) != 0U) {
                return reject_unknown(
                    0U,
                    "new object flag bits outside confirmed mutable mask 0x00004000");
            }
            if (!std::all_of(
                    object.reserved14_2f.begin(),
                    object.reserved14_2f.end(),
                    [](std::byte value) { return value == std::byte{0}; })) {
                return reject_unknown(0U, "new object +0x14..+0x2F");
            }
        }

        for (std::size_t mesh_index = 0U;
             mesh_index < object.meshes.size();
             ++mesh_index) {
            const auto& mesh = object.meshes[mesh_index];
            const Mesh* source_mesh = nullptr;
            if (object_index < source_document.objects.size()) {
                const auto& source_object =
                    source_document.objects[object_index];
                if (mesh_index < source_object.meshes.size()) {
                    source_mesh = &source_object.meshes[mesh_index];
                }
            }

            if (source_mesh != nullptr) {
                if (mesh.record_offset != source_mesh->record_offset) {
                    return reject_structural(
                        source_mesh->record_offset,
                        "mesh record offset");
                }
                if (mesh.positions_offset != source_mesh->positions_offset) {
                    return reject_structural(
                        source_mesh->record_offset + 0x10U,
                        "mesh position-stream offset");
                }
                if (mesh.normals_offset != source_mesh->normals_offset) {
                    return reject_structural(
                        source_mesh->record_offset + 0x18U,
                        "mesh normal-stream offset");
                }
                if (mesh.uv_offset != source_mesh->uv_offset) {
                    return reject_structural(
                        source_mesh->record_offset + 0x20U,
                        "mesh UV-stream offset");
                }
                if (mesh.color_flags_offset !=
                    source_mesh->color_flags_offset) {
                    return reject_structural(
                        source_mesh->record_offset + 0x38U,
                        "mesh color/topology-stream offset");
                }
                if (mesh.index_workspace_relative_offset !=
                    source_mesh->index_workspace_relative_offset ||
                    mesh.index_workspace_offset !=
                    source_mesh->index_workspace_offset ||
                    mesh.index_workspace_capacity !=
                    source_mesh->index_workspace_capacity) {
                    return reject_structural(
                        source_mesh->record_offset + 0x40U,
                        "mesh index-workspace location/capacity");
                }
                if (mesh.vertex_count != source_mesh->vertex_count) {
                    return reject_derived(
                        source_mesh->record_offset + 0x00U,
                        "mesh vertex count");
                }
                if (mesh.continuation_span !=
                    source_mesh->continuation_span) {
                    return reject_derived(
                        source_mesh->record_offset + 0x28U,
                        "mesh continuation span");
                }
                if (mesh.reserved0c != source_mesh->reserved0c) {
                    return reject_unknown(
                        source_mesh->record_offset + 0x0CU,
                        "mesh +0x0C");
                }
                if (mesh.reserved30 != source_mesh->reserved30) {
                    return reject_unknown(
                        source_mesh->record_offset + 0x30U,
                        "mesh +0x30");
                }
                if (mesh.generated_index_count !=
                    source_mesh->generated_index_count) {
                    return reject_unknown(
                        source_mesh->record_offset + 0x48U,
                        "runtime-generated mesh index count +0x48");
                }
                if (mesh.reserved4c != source_mesh->reserved4c) {
                    return reject_unknown(
                        source_mesh->record_offset + 0x4CU,
                        "mesh +0x4C");
                }

                const bool clamp_changed =
                    mesh.gs_clamp_region_repeat.min_u !=
                        source_mesh->gs_clamp_region_repeat.min_u ||
                    mesh.gs_clamp_region_repeat.max_u !=
                        source_mesh->gs_clamp_region_repeat.max_u ||
                    mesh.gs_clamp_region_repeat.min_v !=
                        source_mesh->gs_clamp_region_repeat.min_v ||
                    mesh.gs_clamp_region_repeat.max_v !=
                        source_mesh->gs_clamp_region_repeat.max_v;
                if (clamp_changed &&
                    !legacy_gs_clamp_fields_fit_register(
                        mesh.gs_clamp_region_repeat)) {
                    return reject_invalid(
                        source_mesh->record_offset + 0x04U,
                        "GS CLAMP REGION_REPEAT value");
                }

                const auto common_vertices = std::min(
                    mesh.colors_topology.size(),
                    source_mesh->colors_topology.size());
                for (std::size_t vertex_index = 0U;
                     vertex_index < common_vertices;
                     ++vertex_index) {
                    const auto before =
                        source_mesh->colors_topology[vertex_index]
                            .topology_flags;
                    const auto after =
                        mesh.colors_topology[vertex_index].topology_flags;
                    if (((before ^ after) & ~triangle_break_bit) != 0U) {
                        return reject_unknown(
                            source_mesh->color_flags_offset +
                                static_cast<std::uint64_t>(vertex_index) * 4U +
                                3U,
                            "topology bits outside confirmed 0x02 break bit");
                    }
                }
                for (std::size_t vertex_index = common_vertices;
                     vertex_index < mesh.colors_topology.size();
                     ++vertex_index) {
                    if ((mesh.colors_topology[vertex_index].topology_flags &
                         ~triangle_break_bit) != 0U) {
                        return reject_unknown(
                            source_mesh->color_flags_offset +
                                static_cast<std::uint64_t>(vertex_index) * 4U +
                                3U,
                            "new topology bits outside confirmed 0x02 break bit");
                    }
                }

                const auto common_normals = std::min(
                    mesh.normals.size(), source_mesh->normals.size());
                for (std::size_t vertex_index = 0U;
                     vertex_index < common_normals;
                     ++vertex_index) {
                    if (!same_vec3_bits(
                            mesh.normals[vertex_index],
                            source_mesh->normals[vertex_index]) &&
                        !finite_vec3(mesh.normals[vertex_index])) {
                        return reject_invalid(
                            source_mesh->normals_offset +
                                static_cast<std::uint64_t>(vertex_index) * 12U,
                            "normal vector");
                    }
                }
                for (std::size_t vertex_index = common_normals;
                     vertex_index < mesh.normals.size();
                     ++vertex_index) {
                    if (!finite_vec3(mesh.normals[vertex_index])) {
                        return reject_invalid(
                            source_mesh->normals_offset +
                                static_cast<std::uint64_t>(vertex_index) * 12U,
                            "new normal vector");
                    }
                }
            } else {
                if (mesh.reserved0c != 0U ||
                    mesh.reserved30 != 0U ||
                    mesh.generated_index_count != 0U ||
                    mesh.reserved4c != 0U) {
                    return reject_unknown(
                        0U,
                        "reserved/runtime fields of a source-bound newly added mesh");
                }
                if (!legacy_gs_clamp_fields_fit_register(
                        mesh.gs_clamp_region_repeat)) {
                    return reject_invalid(
                        0U,
                        "new GS CLAMP REGION_REPEAT value");
                }
                for (const auto& value : mesh.colors_topology) {
                    if ((value.topology_flags & ~triangle_break_bit) != 0U) {
                        return reject_unknown(
                            0U,
                            "new topology bits outside confirmed 0x02 break bit");
                    }
                }
                for (const auto& normal : mesh.normals) {
                    if (!finite_vec3(normal)) {
                        return reject_invalid(0U, "new normal vector");
                    }
                }
            }
        }
    }

    const auto& scene = document.scene_nodes;
    const auto& source_scene = source_document.scene_nodes;
    if (scene.offset != source_scene.offset) {
        return reject_structural(source_scene.offset, "scene-block offset");
    }
    if (scene.parent_rel != source_scene.parent_rel) {
        return reject_structural(
            source_scene.offset + 0x00U,
            "scene parent-array relative offset");
    }
    if (scene.order_rel != source_scene.order_rel) {
        return reject_structural(
            source_scene.offset + 0x04U,
            "scene order-array relative offset");
    }
    if (scene.object_binding_rel != source_scene.object_binding_rel) {
        return reject_structural(
            source_scene.offset + 0x08U,
            "scene object-binding relative offset");
    }
    if (scene.transform_rel != source_scene.transform_rel) {
        return reject_structural(
            source_scene.offset + 0x0CU,
            "scene transform-array relative offset");
    }
    if (scene.reserved10_1f != source_scene.reserved10_1f) {
        return reject_unknown(
            source_scene.offset + 0x10U,
            "scene header +0x10..+0x1F");
    }

    for (std::size_t index = 0U;
         index < scene.transform_by_node_index.size();
         ++index) {
        const auto& transform = scene.transform_by_node_index[index];
        if (index < source_scene.transform_by_node_index.size()) {
            const auto& source_transform =
                source_scene.transform_by_node_index[index];
            const auto offset =
                source_scene.offset + source_scene.transform_rel +
                static_cast<std::uint64_t>(index) * scene_transform_size;
            if (!same_float_bits(
                    transform.reserved1c,
                    source_transform.reserved1c)) {
                return reject_unknown(offset + 0x1CU, "transform +0x1C");
            }
            const bool translation_authored =
                !same_vec3_bits(
                    transform.translation,
                    source_transform.translation);
            const bool rotation_authored =
                !same_vec3_bits(
                    transform.rotation_xyz_radians,
                    source_transform.rotation_xyz_radians);
            if (translation_authored &&
                !finite_vec3(transform.translation)) {
                return reject_invalid(offset + 0x00U, "translation vector");
            }
            if (rotation_authored &&
                !finite_vec3(transform.rotation_xyz_radians)) {
                return reject_invalid(offset + 0x10U, "rotation vector");
            }
            if (!translation_authored &&
                !same_float_bits(
                    transform.translation_magnitude,
                    source_transform.translation_magnitude)) {
                return reject_derived(
                    offset + 0x0CU,
                    "translation magnitude");
            }
        } else {
            if (!same_float_bits(transform.reserved1c, 0.0F)) {
                return reject_unknown(0U, "new transform +0x1C");
            }
            if (!finite_vec3(transform.translation)) {
                return reject_invalid(0U, "new translation vector");
            }
            if (!finite_vec3(transform.rotation_xyz_radians)) {
                return reject_invalid(0U, "new rotation vector");
            }
        }
    }

    return true;
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
                !write_value(bytes, mesh_offset + 0x20U, mesh_layout.uv_offset) ||
                !write_value(bytes, mesh_offset + 0x28U, continuation) ||
                !write_value(bytes, mesh_offset + 0x30U, mesh.reserved30) ||
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
                !write_value(bytes, mesh_offset + 0x4CU, mesh.reserved4c) ||
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

    if (!document.source_bytes.empty()) {
        const auto source_reparsed = Parser::parse(
            std::span<const std::byte>{document.source_bytes});
        if (!source_reparsed.ok()) {
            add_diag(
                out,
                ParseSeverity::error,
                "scm.writer-source-bound-source-reparse-failed",
                "Source-bound SCM authoring requires the retained source image "
                "to pass the canonical parser before preserved-field authority "
                "can be checked.");
            return out;
        }
        if (!validate_source_bound_authority(
                document,
                source_reparsed.document,
                out)) {
            return out;
        }
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
            if (!document.source_bytes.empty()) {
                const auto source_reparsed = Parser::parse(
                    std::span<const std::byte>{document.source_bytes});
                if (!source_reparsed.ok()) {
                    add_diag(
                        out,
                        ParseSeverity::error,
                        "scm.writer-canonical-reflow-source-reparse-failed",
                        "Canonical reflow requires the retained SCM source "
                        "image to pass the canonical parser before source-bound "
                        "unknown-byte loss can be assessed.");
                    return out;
                }
                const auto first_unknown =
                    first_nonzero_unmodeled_source_byte(
                        source_reparsed.document);
                if (first_unknown.has_value()) {
                    add_diag(
                        out,
                        ParseSeverity::error,
                        "scm.writer-canonical-reflow-unmodeled-nonzero-source",
                        "Canonical reflow would discard a non-zero source byte "
                        "outside typed/raw-preserved fields and the explicitly "
                        "regeneratable index workspace. Preserve-layout mode "
                        "must be used until this byte region has a mapped "
                        "reflow policy.",
                        *first_unknown);
                    return out;
                }
            }
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
