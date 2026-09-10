#pragma once

#include "scm_authoring_commands.hpp"

#include <algorithm>
#include <bit>
#include <cmath>
#include <cstdint>
#include <limits>

namespace dmc::rengine::cli {
namespace scm_vertex_authoring_detail {

[[nodiscard]] inline bool same_float_bits(float lhs, float rhs) noexcept {
    return std::bit_cast<std::uint32_t>(lhs) ==
           std::bit_cast<std::uint32_t>(rhs);
}

[[nodiscard]] inline bool same_vec3_bits(
    const formats::scm::Vec3f& lhs,
    const formats::scm::Vec3f& rhs) noexcept {
    return same_float_bits(lhs.x, rhs.x) &&
           same_float_bits(lhs.y, rhs.y) &&
           same_float_bits(lhs.z, rhs.z);
}

[[nodiscard]] inline bool derive_bounding_radius(
    const formats::scm::Object& object,
    float& radius) noexcept {
    const auto finite_vec3 = [](const formats::scm::Vec3f& value) {
        return std::isfinite(value.x) && std::isfinite(value.y) &&
               std::isfinite(value.z);
    };
    if (!finite_vec3(object.bounding_center)) return false;

    radius = 0.0F;
    for (const auto& mesh : object.meshes) {
        for (const auto& position : mesh.positions) {
            if (!finite_vec3(position)) return false;
            const auto dx = position.x - object.bounding_center.x;
            const auto dy = position.y - object.bounding_center.y;
            const auto dz = position.z - object.bounding_center.z;
            radius = std::max(
                radius,
                std::sqrt(dx * dx + dy * dy + dz * dz));
        }
    }
    return true;
}

[[nodiscard]] inline bool patch_f32_le(
    std::vector<std::byte>& bytes,
    std::size_t offset,
    float value) noexcept {
    if (offset > bytes.size() || 4U > bytes.size() - offset) return false;
    const auto raw = std::bit_cast<std::uint32_t>(value);
    bytes[offset + 0U] =
        std::byte{static_cast<unsigned char>(raw & 0xFFU)};
    bytes[offset + 1U] =
        std::byte{static_cast<unsigned char>((raw >> 8U) & 0xFFU)};
    bytes[offset + 2U] =
        std::byte{static_cast<unsigned char>((raw >> 16U) & 0xFFU)};
    bytes[offset + 3U] =
        std::byte{static_cast<unsigned char>((raw >> 24U) & 0xFFU)};
    return true;
}

} // namespace scm_vertex_authoring_detail

inline void print_scm_vertex_authoring_help() {
    std::cout
        << "  scm-set-vertex-position <input.scm> <object-index> <mesh-index> <vertex-index> <x> <y> <z> <output.scm>\n"
        << "                             Bounded preserve-layout SCM vertex authoring with source-radius and exact-image guards\n";
}

inline int try_run_scm_vertex_authoring_command(int argc, char** argv) {
    using formats::scm::Parser;
    using formats::scm::Vec3f;
    using formats::scm::WriteMode;
    using formats::scm::Writer;
    using formats::scm::set_vertex_position;

    if (argc <= 1 ||
        std::string_view{argv[1]} != "scm-set-vertex-position") {
        return -1;
    }
    if (argc != 10) {
        std::cerr
            << "usage: scm-set-vertex-position <input.scm> <object-index> <mesh-index> <vertex-index> <x> <y> <z> <output.scm>\n";
        return 1;
    }

    std::uint64_t object_index_raw = 0U;
    std::uint64_t mesh_index_raw = 0U;
    std::uint64_t vertex_index_raw = 0U;
    float x = 0.0F;
    float y = 0.0F;
    float z = 0.0F;
    if (!scm_authoring_detail::parse_u64(argv[3], object_index_raw) ||
        !scm_authoring_detail::parse_u64(argv[4], mesh_index_raw) ||
        !scm_authoring_detail::parse_u64(argv[5], vertex_index_raw) ||
        object_index_raw > std::numeric_limits<std::size_t>::max() ||
        mesh_index_raw > std::numeric_limits<std::size_t>::max() ||
        vertex_index_raw > std::numeric_limits<std::size_t>::max() ||
        !scm_authoring_detail::parse_float(argv[6], x) ||
        !scm_authoring_detail::parse_float(argv[7], y) ||
        !scm_authoring_detail::parse_float(argv[8], z)) {
        std::cerr
            << "scm-set-vertex-position: invalid index or non-finite position\n";
        return 2;
    }

    const std::filesystem::path input{argv[2]};
    const std::filesystem::path output{argv[9]};
    std::vector<std::byte> source;
    if (!scm_authoring_detail::read_file(input, source)) {
        std::cerr << "scm-set-vertex-position: cannot read input\n";
        return 3;
    }

    const auto parsed = Parser::parse(std::span<const std::byte>{source});
    if (!parsed.ok()) {
        std::cerr << "scm-set-vertex-position: canonical parse failed\n";
        return 4;
    }

    const auto object_index = static_cast<std::size_t>(object_index_raw);
    const auto mesh_index = static_cast<std::size_t>(mesh_index_raw);
    const auto vertex_index = static_cast<std::size_t>(vertex_index_raw);
    if (object_index >= parsed.document.objects.size()) {
        std::cerr << "scm-set-vertex-position: object index out of range\n";
        return 5;
    }
    const auto& source_object = parsed.document.objects[object_index];
    if (mesh_index >= source_object.meshes.size()) {
        std::cerr << "scm-set-vertex-position: mesh index out of range\n";
        return 5;
    }
    const auto& source_mesh = source_object.meshes[mesh_index];
    if (vertex_index >= source_mesh.positions.size()) {
        std::cerr << "scm-set-vertex-position: vertex index out of range\n";
        return 5;
    }

    const Vec3f source_position = source_mesh.positions[vertex_index];
    const Vec3f source_center = source_object.bounding_center;
    const float source_radius = source_object.bounding_radius;

    float reconstructed_source_radius = 0.0F;
    if (!scm_vertex_authoring_detail::derive_bounding_radius(
            source_object, reconstructed_source_radius) ||
        !scm_vertex_authoring_detail::same_float_bits(
            reconstructed_source_radius, source_radius)) {
        std::cerr
            << "scm-set-vertex-position: source bounding_radius is not bit-exact under the current reconstruction policy; authoring fails closed\n";
        return 6;
    }

    if (source_position.x == x && source_position.y == y &&
        source_position.z == z) {
        std::cerr
            << "scm-set-vertex-position: requested position is already present\n";
        return 7;
    }

    const auto position_offset_u64 =
        source_mesh.positions_offset +
        static_cast<std::uint64_t>(vertex_index) * 12U;
    const auto radius_offset_u64 = source_object.record_offset + 0x3CU;
    if (position_offset_u64 > std::numeric_limits<std::size_t>::max() ||
        radius_offset_u64 > std::numeric_limits<std::size_t>::max()) {
        std::cerr << "scm-set-vertex-position: serialized offset overflow\n";
        return 8;
    }
    const auto position_offset =
        static_cast<std::size_t>(position_offset_u64);
    const auto radius_offset = static_cast<std::size_t>(radius_offset_u64);

    auto document = parsed.document;
    const Vec3f new_position{x, y, z};
    const auto edit = set_vertex_position(
        document,
        object_index,
        mesh_index,
        vertex_index,
        new_position);
    if (!edit.ok() || !edit.changed) {
        std::cerr << "scm-set-vertex-position: typed edit rejected\n";
        return 9;
    }

    float expected_radius = 0.0F;
    if (!scm_vertex_authoring_detail::derive_bounding_radius(
            document.objects[object_index], expected_radius)) {
        std::cerr
            << "scm-set-vertex-position: cannot derive finite bounding radius\n";
        return 10;
    }

    const auto written = Writer::write(document, WriteMode::preserve_layout);
    if (!written.ok() || !written.wrote || !written.reparse_ok) {
        std::cerr
            << "scm-set-vertex-position: preserve-layout writer rejected output\n";
        return 11;
    }
    if (written.bytes.size() != source.size()) {
        std::cerr << "scm-set-vertex-position: preserve-layout size changed\n";
        return 12;
    }

    auto expected = source;
    if (!scm_vertex_authoring_detail::patch_f32_le(
            expected, position_offset + 0U, x) ||
        !scm_vertex_authoring_detail::patch_f32_le(
            expected, position_offset + 4U, y) ||
        !scm_vertex_authoring_detail::patch_f32_le(
            expected, position_offset + 8U, z) ||
        !scm_vertex_authoring_detail::patch_f32_le(
            expected, radius_offset, expected_radius)) {
        std::cerr
            << "scm-set-vertex-position: expected serialized span is out of bounds\n";
        return 13;
    }
    if (written.bytes != expected) {
        std::cerr
            << "scm-set-vertex-position: exact-image guard rejected writer output\n";
        return 14;
    }

    const auto diffs = scm_authoring_detail::changed_offsets(
        std::span<const std::byte>{source},
        std::span<const std::byte>{written.bytes});
    bool position_byte_changed = false;
    bool unexpected_diff = diffs.empty();
    for (const auto offset : diffs) {
        const bool in_position =
            offset >= position_offset && offset < position_offset + 12U;
        const bool in_radius =
            offset >= radius_offset && offset < radius_offset + 4U;
        if (!in_position && !in_radius) {
            unexpected_diff = true;
            break;
        }
        if (in_position) position_byte_changed = true;
    }
    if (unexpected_diff || !position_byte_changed) {
        std::cerr
            << "scm-set-vertex-position: exact-span guard rejected unexpected output diff\n";
        return 14;
    }

    const auto reparsed = Parser::parse(
        std::span<const std::byte>{written.bytes});
    if (!reparsed.ok() || object_index >= reparsed.document.objects.size() ||
        mesh_index >= reparsed.document.objects[object_index].meshes.size() ||
        vertex_index >= reparsed.document.objects[object_index]
                            .meshes[mesh_index]
                            .positions.size()) {
        std::cerr
            << "scm-set-vertex-position: canonical output reparse failed\n";
        return 15;
    }
    const auto& reparsed_object = reparsed.document.objects[object_index];
    const auto& reparsed_position =
        reparsed_object.meshes[mesh_index].positions[vertex_index];
    if (!scm_vertex_authoring_detail::same_vec3_bits(
            reparsed_position, new_position) ||
        !scm_vertex_authoring_detail::same_vec3_bits(
            reparsed_object.bounding_center, source_center) ||
        !scm_vertex_authoring_detail::same_float_bits(
            reparsed_object.bounding_radius, expected_radius)) {
        std::cerr
            << "scm-set-vertex-position: canonical output reparse mismatch\n";
        return 16;
    }

    const auto validator = [&](const std::filesystem::path& staged_path) {
        std::vector<std::byte> staged;
        if (!scm_authoring_detail::read_file(staged_path, staged) ||
            staged != expected) {
            return false;
        }
        const auto staged_parse = Parser::parse(
            std::span<const std::byte>{staged});
        if (!staged_parse.ok() ||
            object_index >= staged_parse.document.objects.size() ||
            mesh_index >= staged_parse.document.objects[object_index].meshes.size() ||
            vertex_index >= staged_parse.document.objects[object_index]
                                .meshes[mesh_index]
                                .positions.size()) {
            return false;
        }
        const auto& staged_object = staged_parse.document.objects[object_index];
        const auto& staged_position =
            staged_object.meshes[mesh_index].positions[vertex_index];
        return scm_vertex_authoring_detail::same_vec3_bits(
                   staged_position, new_position) &&
               scm_vertex_authoring_detail::same_vec3_bits(
                   staged_object.bounding_center, source_center) &&
               scm_vertex_authoring_detail::same_float_bits(
                   staged_object.bounding_radius, expected_radius);
    };

    const auto publication = core::publish_bytes_no_replace(
        output,
        std::span<const std::byte>{written.bytes},
        validator,
        ".dmc-rengine-scm-vertex.staging");
    if (!publication.ok()) {
        std::cerr
            << "scm-set-vertex-position: output publication failed ("
            << core::to_string(publication.status) << ")";
        if (!publication.detail.empty()) {
            std::cerr << ": " << publication.detail;
        }
        std::cerr << '\n';
        return 17;
    }

    std::cout
        << "SCM_VERTEX_POSITION_EDIT_OK"
        << " object=" << object_index
        << " mesh=" << mesh_index
        << " vertex=" << vertex_index
        << " positionOffset=" << position_offset_u64
        << " radiusOffset=" << radius_offset_u64
        << " oldX=" << source_position.x
        << " oldY=" << source_position.y
        << " oldZ=" << source_position.z
        << " newX=" << x
        << " newY=" << y
        << " newZ=" << z
        << " oldRadius=" << source_radius
        << " newRadius=" << expected_radius
        << " changedBytes=" << diffs.size()
        << " sourceSize=" << source.size()
        << " outputSize=" << written.bytes.size()
        << " sourceRadiusReconstruction=BIT_EXACT_PASS"
        << " reparse=PASS"
        << " exactImageGuard=PASS"
        << " publication=NO_REPLACE_PASS\n";
    return 0;
}

} // namespace dmc::rengine::cli
