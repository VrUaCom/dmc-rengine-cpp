#include "dmc_rengine/formats/scm.hpp"

#include "dmc_rengine/formats/scm_layout.hpp"
#include "dmc_rengine/formats/scm_topology.hpp"
#include "scm_internal.hpp"

#include <algorithm>
#include <cmath>

namespace dmc::rengine::formats::scm {
namespace {
using detail::Reader;
using detail::diag;

[[nodiscard]] std::uint64_t stream_size(std::uint16_t vertices, std::uint64_t stride) noexcept {
    return static_cast<std::uint64_t>(vertices) * stride;
}
}

bool ParseResult::ok() const noexcept {
    return recognized && !detail::has_error(*this);
}

ParseResult Parser::parse(std::span<const std::byte> bytes) {
    ParseResult out;
    const Reader r{bytes};
    if (bytes.size() < header_size) {
        diag(out, ParseSeverity::error, "scm.truncated-header",
             "SCM candidate is shorter than the confirmed 0x40-byte header.", bytes.size());
        return out;
    }
    if (!std::equal(magic.begin(), magic.end(), bytes.begin())) return out;
    out.recognized = true;

    auto& h = out.document.header;
    r.read(0x04U, h.version); r.read(0x08U, h.reserved08);
    r.read(0x10U, h.object_count); r.read(0x11U, h.scene_node_count);
    r.read(0x12U, h.texture_slot_count); r.read(0x13U, h.reserved13);
    r.read(0x14U, h.unresolved_id14); r.read(0x18U, h.reserved18);
    r.read(0x20U, h.scene_node_block_offset); r.read(0x28U, h.reserved28);
    r.read(0x30U, h.reserved30); r.read(0x38U, h.reserved38);
    if (std::fabs(h.version - 1.01F) > 0.0001F)
        diag(out, ParseSeverity::warning, "scm.unconfirmed-version", "Confirmed DMC3-HD SCM corpus uses version 1.01.", 0x04U);
    if (h.reserved08 || h.reserved13 || h.reserved18 || h.reserved28 || h.reserved30 || h.reserved38)
        diag(out, ParseSeverity::warning, "scm.header-reserved-nonzero", "Reserved SCM header fields differ from the confirmed zero-filled corpus.", 0x08U);

    const auto object_bytes = static_cast<std::uint64_t>(h.object_count) * object_record_size;
    if (!r.has(header_size, object_bytes)) {
        diag(out, ParseSeverity::error, "scm.truncated-object-table", "SCM object table exceeds file bounds.", header_size);
        return out;
    }

    std::vector<ObjectShape> shapes;
    shapes.reserve(h.object_count);
    out.document.objects.reserve(h.object_count);
    for (std::size_t oi = 0; oi < h.object_count; ++oi) {
        Object object;
        object.record_offset = header_size + static_cast<std::uint64_t>(oi) * object_record_size;
        r.read(object.record_offset + 0x00U, object.mesh_count); r.read(object.record_offset + 0x01U, object.unresolved01);
        r.read(object.record_offset + 0x02U, object.total_vertex_count); r.read(object.record_offset + 0x04U, object.reserved04);
        r.read(object.record_offset + 0x08U, object.mesh_table_offset); r.read(object.record_offset + 0x10U, object.flags);
        r.read(object.record_offset + 0x30U, object.bounding_center.x); r.read(object.record_offset + 0x34U, object.bounding_center.y);
        r.read(object.record_offset + 0x38U, object.bounding_center.z); r.read(object.record_offset + 0x3CU, object.bounding_radius);
        if (object.reserved04 || !r.zero(object.record_offset + 0x14U, 0x1CU))
            diag(out, ParseSeverity::warning, "scm.object-reserved-nonzero", "Reserved SCM object bytes differ from the confirmed corpus.", object.record_offset + 0x04U);

        const auto mesh_bytes = static_cast<std::uint64_t>(object.mesh_count) * mesh_record_size;
        if (!r.has(object.mesh_table_offset, mesh_bytes)) {
            diag(out, ParseSeverity::error, "scm.mesh-table-out-of-bounds", "Object mesh table exceeds file bounds.", object.mesh_table_offset);
            return out;
        }

        ObjectShape shape;
        shape.mesh_vertex_counts.reserve(object.mesh_count);
        object.meshes.reserve(object.mesh_count);
        std::uint64_t vertex_sum = 0U;
        for (std::size_t mi = 0; mi < object.mesh_count; ++mi) {
            Mesh mesh;
            mesh.record_offset = object.mesh_table_offset + static_cast<std::uint64_t>(mi) * mesh_record_size;
            r.read(mesh.record_offset + 0x00U, mesh.vertex_count);
            r.read(mesh.record_offset + 0x02U, mesh.texture_index);
            r.read(mesh.record_offset + 0x04U, mesh.render_words.values[0]);
            r.read(mesh.record_offset + 0x06U, mesh.render_words.values[1]);
            r.read(mesh.record_offset + 0x08U, mesh.render_words.values[2]);
            r.read(mesh.record_offset + 0x0AU, mesh.render_words.values[3]);
            r.read(mesh.record_offset + 0x0CU, mesh.reserved0c);
            r.read(mesh.record_offset + 0x10U, mesh.positions_offset); r.read(mesh.record_offset + 0x18U, mesh.normals_offset);
            r.read(mesh.record_offset + 0x20U, mesh.uv_offset); r.read(mesh.record_offset + 0x28U, mesh.continuation_span);
            r.read(mesh.record_offset + 0x30U, mesh.reserved30); r.read(mesh.record_offset + 0x38U, mesh.color_flags_offset);
            r.read(mesh.record_offset + 0x40U, mesh.index_workspace_relative_offset); r.read(mesh.record_offset + 0x48U, mesh.generated_index_count);
            r.read(mesh.record_offset + 0x4CU, mesh.reserved4c);
            mesh.index_workspace_offset = mesh.record_offset + mesh.index_workspace_relative_offset;
            mesh.index_workspace_capacity = index_workspace_capacity_bytes(mesh.vertex_count);

            const auto continuation = mi + 1U < object.mesh_count ? mesh_record_size : 0U;
            if (mesh.continuation_span != continuation)
                diag(out, ParseSeverity::error, "scm.mesh-continuation-mismatch", "Mesh +0x28 must be 0x50 for non-final meshes and 0 for the final mesh.", mesh.record_offset + 0x28U);
            if (h.texture_slot_count != 0U && mesh.texture_index >= h.texture_slot_count)
                diag(out, ParseSeverity::error, "scm.texture-index-out-of-range", "Mesh texture index exceeds header texture-slot count.", mesh.record_offset + 0x02U);
            if (mesh.reserved0c || mesh.reserved30 || mesh.generated_index_count || mesh.reserved4c)
                diag(out, ParseSeverity::warning, "scm.mesh-reserved-nonzero", "Serialized reserved/runtime mesh fields differ from confirmed corpus.", mesh.record_offset + 0x0CU);

            const std::array ranges{
                std::pair{mesh.positions_offset, stream_size(mesh.vertex_count, 12U)},
                std::pair{mesh.normals_offset, stream_size(mesh.vertex_count, 12U)},
                std::pair{mesh.uv_offset, stream_size(mesh.vertex_count, 4U)},
                std::pair{mesh.color_flags_offset, stream_size(mesh.vertex_count, 4U)},
                std::pair{mesh.index_workspace_offset, mesh.index_workspace_capacity}};
            if (std::any_of(ranges.begin(), ranges.end(), [&](const auto& range) { return !r.has(range.first, range.second); }))
                diag(out, ParseSeverity::error, "scm.mesh-stream-out-of-bounds", "Mesh stream or index workspace exceeds file bounds.", mesh.record_offset);

            if (r.has(mesh.color_flags_offset, stream_size(mesh.vertex_count, 4U))) {
                for (std::size_t vi = 0; vi < mesh.vertex_count; ++vi)
                    mesh.observed_topology_flag_mask |= r.u8(mesh.color_flags_offset + vi * 4U + 3U);
                if ((mesh.observed_topology_flag_mask & ~triangle_break_bit) != 0U)
                    diag(out, ParseSeverity::warning, "scm.unconfirmed-topology-bits", "Topology byte uses bits outside canonical 0x02 break bit.", mesh.color_flags_offset + 3U);
            }
            if (mesh.index_workspace_capacity >= 2U && r.has(mesh.index_workspace_offset, 2U)) {
                std::uint16_t sentinel{}; r.read(mesh.index_workspace_offset, sentinel);
                if (sentinel != index_workspace_sentinel)
                    diag(out, ParseSeverity::warning, "scm.index-workspace-sentinel", "Serialized index workspace does not begin with 0x1212.", mesh.index_workspace_offset);
            }

            vertex_sum += mesh.vertex_count;
            shape.mesh_vertex_counts.push_back(mesh.vertex_count);
            object.meshes.push_back(mesh);
        }
        if (vertex_sum != object.total_vertex_count)
            diag(out, ParseSeverity::error, "scm.object-vertex-count-mismatch", "Object +0x02 must equal the sum of child mesh vertex counts.", object.record_offset + 0x02U);
        shapes.push_back(std::move(shape));
        out.document.objects.push_back(std::move(object));
    }

    if (!detail::has_error(out)) detail::validate_serialized_document(bytes, shapes, out);
    return out;
}

} // namespace dmc::rengine::formats::scm
