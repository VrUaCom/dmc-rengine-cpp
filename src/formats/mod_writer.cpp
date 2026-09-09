#include "dmc_rengine/formats/mod_writer.hpp"

#include <bit>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <string>
#include <utility>

namespace dmc::rengine::formats::mod {
namespace {

void diag(WriteResult& out,
          std::string code,
          std::string message,
          std::uint64_t offset) {
    out.diagnostics.push_back(ParseDiagnostic{
        ParseSeverity::error,
        std::move(code),
        std::move(message),
        offset,
    });
}

[[nodiscard]] bool in_bounds(
    const std::vector<std::byte>& bytes,
    const std::uint64_t offset,
    const std::size_t length) noexcept {
    if (offset > static_cast<std::uint64_t>(bytes.size())) return false;
    const auto remaining =
        static_cast<std::uint64_t>(bytes.size()) - offset;
    return static_cast<std::uint64_t>(length) <= remaining;
}

[[nodiscard]] bool same_f32(const float lhs, const float rhs) noexcept {
    return std::bit_cast<std::uint32_t>(lhs) ==
           std::bit_cast<std::uint32_t>(rhs);
}

[[nodiscard]] bool same_vec3(const Vec3f& lhs, const Vec3f& rhs) noexcept {
    return same_f32(lhs.x, rhs.x) &&
           same_f32(lhs.y, rhs.y) &&
           same_f32(lhs.z, rhs.z);
}

[[nodiscard]] bool finite_vec3(const Vec3f& value) noexcept {
    return std::isfinite(value.x) &&
           std::isfinite(value.y) &&
           std::isfinite(value.z);
}

[[nodiscard]] bool same_transform_vec3(
    const transform_domain::Vec3f& lhs,
    const transform_domain::Vec3f& rhs) noexcept {
    return same_f32(lhs.x, rhs.x) &&
           same_f32(lhs.y, rhs.y) &&
           same_f32(lhs.z, rhs.z);
}

[[nodiscard]] bool same_skin(
    const SkinDecodeResult& lhs,
    const SkinDecodeResult& rhs) noexcept {
    if (lhs.status != rhs.status ||
        lhs.skin.influence_count != rhs.skin.influence_count ||
        lhs.skin.topology_break != rhs.skin.topology_break) {
        return false;
    }
    for (std::size_t index = 0U;
         index < lhs.skin.influences.size();
         ++index) {
        const auto& a = lhs.skin.influences[index];
        const auto& b = rhs.skin.influences[index];
        if (a.bone_index != b.bone_index ||
            a.quantized_weight != b.quantized_weight ||
            !same_f32(a.weight, b.weight)) {
            return false;
        }
    }
    return true;
}

[[nodiscard]] bool same_blend_indices(
    const BlendIndices& lhs,
    const BlendIndices& rhs) noexcept {
    return lhs.lanes == rhs.lanes;
}

[[nodiscard]] bool same_uv(
    const SerializedUv& lhs,
    const SerializedUv& rhs) noexcept {
    return lhs.u == rhs.u && lhs.v == rhs.v;
}

[[nodiscard]] bool same_transform_domain(
    const transform_domain::ParseResult& lhs,
    const transform_domain::ParseResult& rhs) noexcept {
    if (lhs.recognized != rhs.recognized ||
        lhs.raw_domain_count != rhs.raw_domain_count ||
        lhs.document_offset != rhs.document_offset ||
        lhs.parent_relative_offset != rhs.parent_relative_offset ||
        lhs.order_relative_offset != rhs.order_relative_offset ||
        lhs.adapter_relative_offset != rhs.adapter_relative_offset ||
        lhs.transform_relative_offset != rhs.transform_relative_offset ||
        lhs.reference_table != rhs.reference_table ||
        lhs.permutation_table != rhs.permutation_table ||
        lhs.adapter_table != rhs.adapter_table ||
        lhs.parent_by_order_position != rhs.parent_by_order_position ||
        lhs.node_at_order_position != rhs.node_at_order_position ||
        lhs.derived_hierarchy_candidate != rhs.derived_hierarchy_candidate ||
        lhs.permutation_is_complete != rhs.permutation_is_complete ||
        lhs.hierarchy_is_topological != rhs.hierarchy_is_topological ||
        lhs.hierarchy_candidate_is_acyclic !=
            rhs.hierarchy_candidate_is_acyclic ||
        lhs.serialized_layout_matches_core !=
            rhs.serialized_layout_matches_core ||
        lhs.transform_records_complete != rhs.transform_records_complete ||
        lhs.transform_records_finite != rhs.transform_records_finite ||
        lhs.local_transform_records_by_node_index.size() !=
            rhs.local_transform_records_by_node_index.size()) {
        return false;
    }

    for (std::size_t index = 0U;
         index < lhs.local_transform_records_by_node_index.size();
         ++index) {
        const auto& a = lhs.local_transform_records_by_node_index[index];
        const auto& b = rhs.local_transform_records_by_node_index[index];
        if (a.record_offset != b.record_offset ||
            !same_transform_vec3(a.translation, b.translation) ||
            !same_f32(a.translation_magnitude, b.translation_magnitude) ||
            !same_transform_vec3(a.rotation_xyz_radians,
                                 b.rotation_xyz_radians) ||
            !same_f32(a.raw_1c, b.raw_1c)) {
            return false;
        }
    }
    return true;
}

[[nodiscard]] bool put_u16(
    std::vector<std::byte>& bytes,
    const std::uint64_t offset,
    const std::uint16_t value) noexcept {
    if (!in_bounds(bytes, offset, 2U)) return false;
    bytes[static_cast<std::size_t>(offset + 0U)] =
        static_cast<std::byte>(value & 0xFFU);
    bytes[static_cast<std::size_t>(offset + 1U)] =
        static_cast<std::byte>((value >> 8U) & 0xFFU);
    return true;
}

[[nodiscard]] bool put_u32(
    std::vector<std::byte>& bytes,
    const std::uint64_t offset,
    const std::uint32_t value) noexcept {
    if (!in_bounds(bytes, offset, 4U)) return false;
    for (std::size_t index = 0U; index < 4U; ++index) {
        bytes[static_cast<std::size_t>(offset) + index] =
            static_cast<std::byte>((value >> (index * 8U)) & 0xFFU);
    }
    return true;
}

[[nodiscard]] bool put_f32(
    std::vector<std::byte>& bytes,
    const std::uint64_t offset,
    const float value) noexcept {
    return put_u32(bytes, offset, std::bit_cast<std::uint32_t>(value));
}

[[nodiscard]] bool validate_header_immutable(
    const Header& edited,
    const Header& baseline) noexcept {
    return same_f32(edited.version, baseline.version) &&
           edited.outer_record_count == baseline.outer_record_count &&
           edited.transform_domain_count == baseline.transform_domain_count &&
           edited.texture_slot_count == baseline.texture_slot_count &&
           edited.runtime_mode_byte == baseline.runtime_mode_byte &&
           edited.runtime_metadata_u32 == baseline.runtime_metadata_u32 &&
           edited.document_offset == baseline.document_offset;
}

[[nodiscard]] bool validate_mesh_immutable(
    const InnerMesh& edited,
    const InnerMesh& baseline) noexcept {
    if (edited.record_offset != baseline.record_offset ||
        edited.element_count != baseline.element_count ||
        edited.texture_slot != baseline.texture_slot ||
        edited.gs_clamp_region_repeat.min_u !=
            baseline.gs_clamp_region_repeat.min_u ||
        edited.gs_clamp_region_repeat.max_u !=
            baseline.gs_clamp_region_repeat.max_u ||
        edited.gs_clamp_region_repeat.min_v !=
            baseline.gs_clamp_region_repeat.min_v ||
        edited.gs_clamp_region_repeat.max_v !=
            baseline.gs_clamp_region_repeat.max_v ||
        edited.positions_offset != baseline.positions_offset ||
        edited.normals_offset != baseline.normals_offset ||
        edited.uv_offset != baseline.uv_offset ||
        edited.blend_indices_offset != baseline.blend_indices_offset ||
        edited.control_offset != baseline.control_offset ||
        edited.reserved38 != baseline.reserved38 ||
        edited.generated_workspace_relative_offset !=
            baseline.generated_workspace_relative_offset ||
        edited.generated_topology_count != baseline.generated_topology_count ||
        edited.reserved4c != baseline.reserved4c ||
        edited.generated_workspace_offset != baseline.generated_workspace_offset ||
        edited.skin_decode_failures != baseline.skin_decode_failures ||
        edited.reserved_blend_lane_nonzero !=
            baseline.reserved_blend_lane_nonzero ||
        edited.blend_indices.size() != baseline.blend_indices.size() ||
        edited.control_words != baseline.control_words ||
        edited.skin.size() != baseline.skin.size()) {
        return false;
    }

    for (std::size_t index = 0U;
         index < edited.blend_indices.size();
         ++index) {
        if (!same_blend_indices(edited.blend_indices[index],
                                baseline.blend_indices[index])) {
            return false;
        }
    }
    for (std::size_t index = 0U; index < edited.skin.size(); ++index) {
        if (!same_skin(edited.skin[index], baseline.skin[index])) {
            return false;
        }
    }
    return true;
}

} // namespace

bool WriteResult::ok() const noexcept {
    return success && reparsed.ok();
}

WriteResult Writer::write(const Document& document, const WriteMode mode) {
    WriteResult out;
    if (mode != WriteMode::preserve_layout) {
        diag(out,
             "mod.writer.unsupported-mode",
             "Only preserve-layout MOD writing is currently authorized.",
             0U);
        return out;
    }
    if (document.source_bytes.empty()) {
        diag(out,
             "mod.writer.no-source-bytes",
             "Preserve-layout MOD writing requires the original source image.",
             0U);
        return out;
    }

    const auto baseline = Parser::parse(std::span<const std::byte>(
        document.source_bytes.data(), document.source_bytes.size()));
    if (!baseline.ok()) {
        diag(out,
             "mod.writer.source-parse-failed",
             "The retained MOD source image no longer reparses cleanly.",
             0U);
        return out;
    }

    if (!validate_header_immutable(document.header,
                                   baseline.document.header)) {
        diag(out,
             "mod.writer.unsupported-header-edit",
             "Preserve-layout mode does not yet authorize MOD header edits.",
             0U);
        return out;
    }
    if (!same_transform_domain(document.transform_domain,
                               baseline.document.transform_domain)) {
        diag(out,
             "mod.writer.unsupported-transform-edit",
             "Transform-domain authoring is outside the first preserve-layout writer gate.",
             document.header.document_offset);
        return out;
    }
    if (document.outer_models.size() !=
        baseline.document.outer_models.size()) {
        diag(out,
             "mod.writer.structural-edit",
             "MOD object cardinality changed; preserve-layout mode refuses reflow.",
             header_size);
        return out;
    }

    out.bytes = document.source_bytes;

    for (std::size_t outer_index = 0U;
         outer_index < document.outer_models.size();
         ++outer_index) {
        const auto& edited_outer = document.outer_models[outer_index];
        const auto& source_outer =
            baseline.document.outer_models[outer_index];

        if (edited_outer.record_offset != source_outer.record_offset ||
            edited_outer.inner_record_count != source_outer.inner_record_count ||
            edited_outer.alpha_control != source_outer.alpha_control ||
            edited_outer.aggregate_element_count !=
                source_outer.aggregate_element_count ||
            edited_outer.inner_table_offset != source_outer.inner_table_offset ||
            edited_outer.source_flags != source_outer.source_flags ||
            edited_outer.meshes.size() != source_outer.meshes.size()) {
            diag(out,
                 "mod.writer.unsupported-object-edit",
                 "Preserve-layout mode currently permits bounding/vertex edits only; object state or structure changed.",
                 edited_outer.record_offset);
            return out;
        }
        if (!finite_vec3(edited_outer.bounding_center) ||
            !std::isfinite(edited_outer.bounding_radius)) {
            diag(out,
                 "mod.writer.non-finite-bounds",
                 "MOD bounding sphere edits must remain finite.",
                 edited_outer.record_offset +
                     model_family::ObjectCoreAbi::bounding_center_field);
            return out;
        }

        const auto bound_offset =
            edited_outer.record_offset +
            model_family::ObjectCoreAbi::bounding_center_field;
        if (!put_f32(out.bytes, bound_offset + 0U,
                     edited_outer.bounding_center.x) ||
            !put_f32(out.bytes, bound_offset + 4U,
                     edited_outer.bounding_center.y) ||
            !put_f32(out.bytes, bound_offset + 8U,
                     edited_outer.bounding_center.z) ||
            !put_f32(out.bytes,
                     edited_outer.record_offset +
                         model_family::ObjectCoreAbi::bounding_radius_field,
                     edited_outer.bounding_radius)) {
            diag(out,
                 "mod.writer.range",
                 "MOD bounding sphere write exceeds the retained source image.",
                 edited_outer.record_offset);
            return out;
        }

        for (std::size_t mesh_index = 0U;
             mesh_index < edited_outer.meshes.size();
             ++mesh_index) {
            const auto& edited_mesh = edited_outer.meshes[mesh_index];
            const auto& source_mesh = source_outer.meshes[mesh_index];
            if (!validate_mesh_immutable(edited_mesh, source_mesh)) {
                diag(out,
                     "mod.writer.unsupported-mesh-edit",
                     "Preserve-layout mode refuses material, skin, workspace, undecoded-field or mesh-structure edits in the first writer gate.",
                     edited_mesh.record_offset);
                return out;
            }
            if (edited_mesh.positions.size() != source_mesh.positions.size() ||
                edited_mesh.normals.size() != source_mesh.normals.size() ||
                edited_mesh.uvs.size() != source_mesh.uvs.size()) {
                diag(out,
                     "mod.writer.stream-size-change",
                     "MOD stream cardinality changed; preserve-layout mode refuses reflow.",
                     edited_mesh.record_offset);
                return out;
            }

            for (std::size_t element = 0U;
                 element < edited_mesh.positions.size();
                 ++element) {
                const auto& position = edited_mesh.positions[element];
                const auto& normal = edited_mesh.normals[element];
                if (!finite_vec3(position) || !finite_vec3(normal)) {
                    diag(out,
                         "mod.writer.non-finite-stream",
                         "MOD position/normal edits must remain finite.",
                         edited_mesh.record_offset);
                    return out;
                }

                const auto position_offset =
                    edited_mesh.positions_offset +
                    static_cast<std::uint64_t>(element) *
                        model_family::MeshCoreAbi::position_stride;
                const auto normal_offset =
                    edited_mesh.normals_offset +
                    static_cast<std::uint64_t>(element) *
                        model_family::MeshCoreAbi::normal_stride;
                const auto uv_offset =
                    edited_mesh.uv_offset +
                    static_cast<std::uint64_t>(element) *
                        model_family::MeshCoreAbi::uv_stride;

                if (!put_f32(out.bytes, position_offset + 0U, position.x) ||
                    !put_f32(out.bytes, position_offset + 4U, position.y) ||
                    !put_f32(out.bytes, position_offset + 8U, position.z) ||
                    !put_f32(out.bytes, normal_offset + 0U, normal.x) ||
                    !put_f32(out.bytes, normal_offset + 4U, normal.y) ||
                    !put_f32(out.bytes, normal_offset + 8U, normal.z) ||
                    !put_u16(out.bytes, uv_offset + 0U,
                             static_cast<std::uint16_t>(
                                 edited_mesh.uvs[element].u)) ||
                    !put_u16(out.bytes, uv_offset + 2U,
                             static_cast<std::uint16_t>(
                                 edited_mesh.uvs[element].v))) {
                    diag(out,
                         "mod.writer.range",
                         "MOD vertex stream write exceeds the retained source image.",
                         edited_mesh.record_offset);
                    return out;
                }
            }
        }
    }

    out.reparsed = Parser::parse(std::span<const std::byte>(
        out.bytes.data(), out.bytes.size()));
    if (!out.reparsed.ok()) {
        diag(out,
             "mod.writer.reparse-failed",
             "Serialized MOD failed the canonical parser reopen gate.",
             0U);
        return out;
    }

    out.success = true;
    return out;
}

} // namespace dmc::rengine::formats::mod
