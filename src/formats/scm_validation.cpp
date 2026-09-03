#include "scm_internal.hpp"

#include "dmc_rengine/formats/scm_layout.hpp"

#include <cmath>
#include <vector>

namespace dmc::rengine::formats::scm::detail {

void validate_serialized_document(std::span<const std::byte> bytes,
                                  const std::vector<ObjectShape>& shapes,
                                  ParseResult& out) {
    const Reader r{bytes};
    const auto& h = out.document.header;
    const auto expected = build_serialized_layout(shapes, h.scene_node_count);

    for (std::size_t oi = 0; oi < out.document.objects.size(); ++oi) {
        const auto& object = out.document.objects[oi];
        const auto& layout = expected.objects[oi];
        if (object.mesh_table_offset != layout.mesh_table_offset)
            diag(out, ParseSeverity::error, "scm.mesh-table-layout-mismatch", "Object mesh table does not match canonical serialized placement.", object.record_offset + 0x08U);
        for (std::size_t mi = 0; mi < object.meshes.size(); ++mi) {
            const auto& mesh = object.meshes[mi];
            const auto& m = layout.meshes[mi];
            if (mesh.positions_offset != m.positions_offset || mesh.normals_offset != m.normals_offset ||
                mesh.uv_offset != m.uv_offset || mesh.color_flags_offset != m.color_flags_offset ||
                mesh.index_workspace_offset != m.index_workspace_offset || mesh.index_workspace_capacity != m.index_workspace_capacity)
                diag(out, ParseSeverity::error, "scm.mesh-layout-mismatch", "Mesh streams/workspace violate canonical align16 serialized layout.", mesh.record_offset);
        }
    }

    auto& scene = out.document.scene_nodes;
    scene.offset = h.scene_node_block_offset;
    if (scene.offset != expected.scene.block_offset || !r.has(scene.offset, scene_block_header_size)) {
        diag(out, ParseSeverity::error, "scm.scene-block-layout-mismatch", "Header +0x20 does not point to canonical scene-node block.", 0x20U);
        return;
    }
    r.read(scene.offset + 0x00U, scene.parent_rel); r.read(scene.offset + 0x04U, scene.order_rel);
    r.read(scene.offset + 0x08U, scene.object_binding_rel); r.read(scene.offset + 0x0CU, scene.transform_rel);
    if (scene.parent_rel != expected.scene.parent_rel || scene.order_rel != expected.scene.order_rel ||
        scene.object_binding_rel != expected.scene.object_binding_rel || scene.transform_rel != expected.scene.transform_rel)
        diag(out, ParseSeverity::error, "scm.scene-array-layout-mismatch", "Scene arrays violate canonical align4/align16 layout.", scene.offset);
    if (!r.zero(scene.offset + 0x10U, 0x10U))
        diag(out, ParseSeverity::warning, "scm.scene-header-reserved-nonzero", "Scene block +0x10..+0x1F differs from zero-filled corpus.", scene.offset + 0x10U);

    const auto n = static_cast<std::size_t>(h.scene_node_count);
    const auto transforms_size = static_cast<std::uint64_t>(n) * scene_transform_size;
    if (!r.has(scene.offset + scene.parent_rel, n) || !r.has(scene.offset + scene.order_rel, n) ||
        !r.has(scene.offset + scene.object_binding_rel, n) || !r.has(scene.offset + scene.transform_rel, transforms_size)) {
        diag(out, ParseSeverity::error, "scm.scene-arrays-out-of-bounds", "Scene arrays/transforms exceed file bounds.", scene.offset);
        return;
    }

    scene.parents.reserve(n); scene.order.reserve(n); scene.object_bindings.reserve(n); scene.transforms.reserve(n);
    std::vector<bool> order_seen(n, false);
    for (std::size_t i = 0; i < n; ++i) {
        const auto parent = static_cast<std::int8_t>(r.u8(scene.offset + scene.parent_rel + i));
        const auto order = r.u8(scene.offset + scene.order_rel + i);
        const auto binding = static_cast<std::int8_t>(r.u8(scene.offset + scene.object_binding_rel + i));
        scene.parents.push_back(parent); scene.order.push_back(order); scene.object_bindings.push_back(binding);
        if (parent < -1 || parent >= static_cast<std::int16_t>(n))
            diag(out, ParseSeverity::error, "scm.scene-parent-out-of-range", "Scene parent is not -1 or a valid node index.", scene.offset + scene.parent_rel + i);
        if (order >= n || order_seen[order])
            diag(out, ParseSeverity::error, "scm.scene-order-not-permutation", "Scene order must be a permutation of 0..N-1.", scene.offset + scene.order_rel + i);
        else order_seen[order] = true;
        if (binding < -1 || binding >= static_cast<std::int16_t>(h.object_count))
            diag(out, ParseSeverity::error, "scm.scene-object-binding-out-of-range", "Scene object binding is not -1 or a valid object index.", scene.offset + scene.object_binding_rel + i);

        SceneTransform t;
        const auto off = scene.offset + scene.transform_rel + static_cast<std::uint64_t>(i) * scene_transform_size;
        r.read(off + 0x00U, t.translation.x); r.read(off + 0x04U, t.translation.y); r.read(off + 0x08U, t.translation.z);
        r.read(off + 0x0CU, t.translation_magnitude);
        r.read(off + 0x10U, t.rotation_xyz_radians.x); r.read(off + 0x14U, t.rotation_xyz_radians.y); r.read(off + 0x18U, t.rotation_xyz_radians.z);
        r.read(off + 0x1CU, t.reserved1c);
        const auto length = std::sqrt(t.translation.x * t.translation.x + t.translation.y * t.translation.y + t.translation.z * t.translation.z);
        const auto tolerance = std::max(0.001F, length * 0.0001F);
        if (std::isfinite(length) && std::isfinite(t.translation_magnitude) && std::fabs(length - t.translation_magnitude) > tolerance)
            diag(out, ParseSeverity::warning, "scm.scene-translation-magnitude", "Scene transform +0x0C differs from translation-vector magnitude.", off + 0x0CU);
        if (t.reserved1c != 0.0F)
            diag(out, ParseSeverity::warning, "scm.scene-transform-reserved-nonzero", "Scene transform +0x1C is non-zero.", off + 0x1CU);
        scene.transforms.push_back(t);
    }

    if (expected.file_size != bytes.size())
        diag(out, ParseSeverity::error, "scm.serialized-size-mismatch", "Canonical SCM layout does not terminate exactly at EOF.", expected.file_size);
}

} // namespace dmc::rengine::formats::scm::detail
