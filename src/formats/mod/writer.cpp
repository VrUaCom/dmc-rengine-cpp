#include "dmc_rengine/formats/mod/writer.hpp"

#include "dmc_rengine/core/sha256.hpp"

#include <algorithm>
#include <bit>
#include <cstdint>
#include <span>
#include <string>
#include <utility>

namespace dmc::rengine::formats::mod::writer {
namespace {

[[nodiscard]] bool same_float(float left, float right) noexcept {
    return std::bit_cast<std::uint32_t>(left) ==
        std::bit_cast<std::uint32_t>(right);
}

template <typename Vec3>
[[nodiscard]] bool same_vec3(const Vec3& left, const Vec3& right) noexcept {
    return same_float(left.x, right.x) &&
        same_float(left.y, right.y) &&
        same_float(left.z, right.z);
}

[[nodiscard]] bool same_header(const Header& left, const Header& right) noexcept {
    return same_float(left.version, right.version) &&
        left.outer_record_count == right.outer_record_count &&
        left.transform_domain_count == right.transform_domain_count &&
        left.texture_slot_count == right.texture_slot_count &&
        left.runtime_mode_byte == right.runtime_mode_byte &&
        left.runtime_metadata_u32 == right.runtime_metadata_u32 &&
        left.document_offset == right.document_offset;
}

[[nodiscard]] bool same_mesh(const InnerMesh& left, const InnerMesh& right) noexcept {
    if (left.record_offset != right.record_offset ||
        left.element_count != right.element_count ||
        left.texture_slot != right.texture_slot ||
        left.gs_clamp_region_repeat.min_u != right.gs_clamp_region_repeat.min_u ||
        left.gs_clamp_region_repeat.max_u != right.gs_clamp_region_repeat.max_u ||
        left.gs_clamp_region_repeat.min_v != right.gs_clamp_region_repeat.min_v ||
        left.gs_clamp_region_repeat.max_v != right.gs_clamp_region_repeat.max_v ||
        left.positions_offset != right.positions_offset ||
        left.normals_offset != right.normals_offset ||
        left.uv_offset != right.uv_offset ||
        left.blend_indices_offset != right.blend_indices_offset ||
        left.control_offset != right.control_offset ||
        left.reserved38 != right.reserved38 ||
        left.generated_workspace_relative_offset !=
            right.generated_workspace_relative_offset ||
        left.generated_topology_count != right.generated_topology_count ||
        left.reserved4c != right.reserved4c ||
        left.generated_workspace_offset != right.generated_workspace_offset ||
        left.positions.size() != right.positions.size() ||
        left.normals.size() != right.normals.size() ||
        left.uvs.size() != right.uvs.size() ||
        left.blend_indices.size() != right.blend_indices.size() ||
        left.control_words != right.control_words) {
        return false;
    }

    for (std::size_t index = 0U; index < left.positions.size(); ++index) {
        if (!same_vec3(left.positions[index], right.positions[index]) ||
            !same_vec3(left.normals[index], right.normals[index])) {
            return false;
        }
    }
    for (std::size_t index = 0U; index < left.uvs.size(); ++index) {
        if (left.uvs[index].u != right.uvs[index].u ||
            left.uvs[index].v != right.uvs[index].v) {
            return false;
        }
    }
    for (std::size_t index = 0U; index < left.blend_indices.size(); ++index) {
        if (left.blend_indices[index].lanes !=
            right.blend_indices[index].lanes) {
            return false;
        }
    }
    return true;
}

[[nodiscard]] bool same_outer(
    const OuterModel& left,
    const OuterModel& right) noexcept {
    if (left.record_offset != right.record_offset ||
        left.inner_record_count != right.inner_record_count ||
        left.alpha_control != right.alpha_control ||
        left.aggregate_element_count != right.aggregate_element_count ||
        left.inner_table_offset != right.inner_table_offset ||
        left.source_flags != right.source_flags ||
        !same_vec3(left.bounding_center, right.bounding_center) ||
        !same_float(left.bounding_radius, right.bounding_radius) ||
        left.meshes.size() != right.meshes.size()) {
        return false;
    }
    for (std::size_t index = 0U; index < left.meshes.size(); ++index) {
        if (!same_mesh(left.meshes[index], right.meshes[index])) {
            return false;
        }
    }
    return true;
}

[[nodiscard]] bool same_local_transform(
    const transform_domain::LocalTransformRecord& left,
    const transform_domain::LocalTransformRecord& right) noexcept {
    return left.record_offset == right.record_offset &&
        same_vec3(left.translation, right.translation) &&
        same_float(left.translation_magnitude, right.translation_magnitude) &&
        same_vec3(left.rotation_xyz_radians, right.rotation_xyz_radians) &&
        same_float(left.raw_1c, right.raw_1c);
}

[[nodiscard]] bool same_transform_domain(
    const transform_domain::ParseResult& left,
    const transform_domain::ParseResult& right) noexcept {
    if (left.recognized != right.recognized ||
        left.raw_domain_count != right.raw_domain_count ||
        left.document_offset != right.document_offset ||
        left.parent_relative_offset != right.parent_relative_offset ||
        left.order_relative_offset != right.order_relative_offset ||
        left.adapter_relative_offset != right.adapter_relative_offset ||
        left.transform_relative_offset != right.transform_relative_offset ||
        left.reference_table != right.reference_table ||
        left.permutation_table != right.permutation_table ||
        left.adapter_table != right.adapter_table ||
        left.local_transform_records_by_node_index.size() !=
            right.local_transform_records_by_node_index.size()) {
        return false;
    }

    for (std::size_t index = 0U;
         index < left.local_transform_records_by_node_index.size(); ++index) {
        if (!same_local_transform(
                left.local_transform_records_by_node_index[index],
                right.local_transform_records_by_node_index[index])) {
            return false;
        }
    }
    return true;
}

[[nodiscard]] bool same_serialized_projection(
    const Document& left,
    const Document& right) noexcept {
    if (!same_header(left.header, right.header) ||
        left.outer_models.size() != right.outer_models.size() ||
        !same_transform_domain(left.transform_domain, right.transform_domain)) {
        return false;
    }
    for (std::size_t index = 0U; index < left.outer_models.size(); ++index) {
        if (!same_outer(left.outer_models[index], right.outer_models[index])) {
            return false;
        }
    }
    return true;
}

[[nodiscard]] RebuildResult failure(RebuildStatus status, std::string detail) {
    return RebuildResult{
        .status = status,
        .bytes = {},
        .receipt = std::nullopt,
        .detail = std::move(detail),
    };
}

[[nodiscard]] std::string sha256_of(std::span<const std::byte> bytes) {
    return core::Sha256::compute(bytes).hex();
}

[[nodiscard]] std::uint64_t mesh_count_of(const Document& document) noexcept {
    std::uint64_t count{};
    for (const auto& outer : document.outer_models) {
        count += static_cast<std::uint64_t>(outer.meshes.size());
    }
    return count;
}

} // namespace

bool NoEditRebuildReceipt::valid() const noexcept {
    return source_sha256.size() == 64U && output_sha256.size() == 64U &&
        source_sha256 == output_sha256 && byte_count != 0U &&
        source_image_matches_document && typed_ir_matches_source &&
        output_reparse_ok && byte_identical;
}

RebuildResult PreserveSourceWriter::rebuild_no_edit(
    const std::span<const std::byte> immutable_source,
    const Document& document) {
    if (immutable_source.empty() || document.source_bytes.empty()) {
        return failure(
            RebuildStatus::missing_source_bytes,
            "PreserveSourceNoEdit requires both immutable source bytes and the parser-retained source image.");
    }

    if (immutable_source.size() != document.source_bytes.size() ||
        !std::equal(
            immutable_source.begin(), immutable_source.end(),
            document.source_bytes.begin())) {
        return failure(
            RebuildStatus::source_image_mismatch,
            "Document::source_bytes no longer matches the caller-supplied immutable MOD source image.");
    }

    const auto source_parsed = Parser::parse(immutable_source);
    if (!source_parsed.ok()) {
        return failure(
            RebuildStatus::source_parse_failed,
            "Immutable MOD source does not pass the canonical parser and cannot be used as writer layout authority.");
    }

    if (!same_serialized_projection(document, source_parsed.document)) {
        return failure(
            RebuildStatus::typed_ir_diverged,
            "Serializable typed MOD state differs from the immutable source image; no-edit emission refuses to discard that divergence.");
    }

    std::vector<std::byte> emitted(
        immutable_source.begin(), immutable_source.end());
    const auto output_span = std::span<const std::byte>{
        emitted.data(), emitted.size()};
    const auto output_parsed = Parser::parse(output_span);
    if (!output_parsed.ok()) {
        return failure(
            RebuildStatus::output_parse_failed,
            "Byte-identical MOD output failed canonical reopen/reparse validation.");
    }

    const bool byte_identical =
        emitted.size() == immutable_source.size() &&
        std::equal(emitted.begin(), emitted.end(), immutable_source.begin());
    if (!byte_identical) {
        return failure(
            RebuildStatus::byte_parity_failed,
            "PreserveSourceNoEdit output is not byte-identical to the immutable source image.");
    }

    NoEditRebuildReceipt receipt{
        .source_sha256 = sha256_of(immutable_source),
        .output_sha256 = sha256_of(output_span),
        .byte_count = static_cast<std::uint64_t>(emitted.size()),
        .mesh_count = mesh_count_of(document),
        .outer_record_count = document.header.outer_record_count,
        .transform_domain_count = document.header.transform_domain_count,
        .source_image_matches_document = true,
        .typed_ir_matches_source = true,
        .output_reparse_ok = true,
        .byte_identical = true,
    };
    if (!receipt.valid()) {
        return failure(
            RebuildStatus::byte_parity_failed,
            "MOD no-edit rebuild receipt failed internal validation.");
    }

    return RebuildResult{
        .status = RebuildStatus::ok,
        .bytes = std::move(emitted),
        .receipt = std::move(receipt),
        .detail = {},
    };
}

} // namespace dmc::rengine::formats::mod::writer
