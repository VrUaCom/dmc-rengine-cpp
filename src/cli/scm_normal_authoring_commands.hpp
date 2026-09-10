#pragma once

#include "scm_authoring_commands.hpp"

#include <bit>
#include <cstdint>
#include <limits>

namespace dmc::rengine::cli {
namespace scm_normal_authoring_detail {

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

[[nodiscard]] inline bool patch_f32_le(
    std::vector<std::byte>& bytes,
    std::size_t offset,
    float value) noexcept {
    if (offset > bytes.size() || 4U > bytes.size() - offset) return false;
    const auto raw = std::bit_cast<std::uint32_t>(value);
    bytes[offset + 0U] = std::byte{static_cast<unsigned char>(raw & 0xFFU)};
    bytes[offset + 1U] =
        std::byte{static_cast<unsigned char>((raw >> 8U) & 0xFFU)};
    bytes[offset + 2U] =
        std::byte{static_cast<unsigned char>((raw >> 16U) & 0xFFU)};
    bytes[offset + 3U] =
        std::byte{static_cast<unsigned char>((raw >> 24U) & 0xFFU)};
    return true;
}

} // namespace scm_normal_authoring_detail

inline void print_scm_normal_authoring_help() {
    std::cout
        << "  scm-set-vertex-normal <input.scm> <object-index> <mesh-index> <vertex-index> <x> <y> <z> <output.scm>\n"
        << "                           Bounded preserve-layout SCM normal authoring; preserves authored components exactly\n";
}

inline int try_run_scm_normal_authoring_command(int argc, char** argv) {
    using formats::scm::Parser;
    using formats::scm::Vec3f;
    using formats::scm::WriteMode;
    using formats::scm::Writer;
    using formats::scm::set_vertex_normal;

    if (argc <= 1 || std::string_view{argv[1]} != "scm-set-vertex-normal") {
        return -1;
    }
    if (argc != 10) {
        std::cerr
            << "usage: scm-set-vertex-normal <input.scm> <object-index> <mesh-index> <vertex-index> <x> <y> <z> <output.scm>\n";
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
        std::cerr << "scm-set-vertex-normal: invalid index or non-finite normal\n";
        return 2;
    }

    const std::filesystem::path input{argv[2]};
    const std::filesystem::path output{argv[9]};
    std::vector<std::byte> source;
    if (!scm_authoring_detail::read_file(input, source)) {
        std::cerr << "scm-set-vertex-normal: cannot read input\n";
        return 3;
    }

    const auto parsed = Parser::parse(std::span<const std::byte>{source});
    if (!parsed.ok()) {
        std::cerr << "scm-set-vertex-normal: canonical parse failed\n";
        return 4;
    }

    const auto object_index = static_cast<std::size_t>(object_index_raw);
    const auto mesh_index = static_cast<std::size_t>(mesh_index_raw);
    const auto vertex_index = static_cast<std::size_t>(vertex_index_raw);
    if (object_index >= parsed.document.objects.size()) {
        std::cerr << "scm-set-vertex-normal: object index out of range\n";
        return 5;
    }
    const auto& source_object = parsed.document.objects[object_index];
    if (mesh_index >= source_object.meshes.size()) {
        std::cerr << "scm-set-vertex-normal: mesh index out of range\n";
        return 5;
    }
    const auto& source_mesh = source_object.meshes[mesh_index];
    if (vertex_index >= source_mesh.normals.size()) {
        std::cerr << "scm-set-vertex-normal: vertex index out of range\n";
        return 5;
    }

    const Vec3f source_normal = source_mesh.normals[vertex_index];
    const Vec3f authored_normal{x, y, z};
    if (scm_normal_authoring_detail::same_vec3_bits(
            source_normal, authored_normal)) {
        std::cerr << "scm-set-vertex-normal: requested normal is already present\n";
        return 6;
    }

    const auto normal_offset_u64 =
        source_mesh.normals_offset +
        static_cast<std::uint64_t>(vertex_index) * 12U;
    if (normal_offset_u64 > std::numeric_limits<std::size_t>::max()) {
        std::cerr << "scm-set-vertex-normal: serialized offset overflow\n";
        return 7;
    }
    const auto normal_offset = static_cast<std::size_t>(normal_offset_u64);

    auto document = parsed.document;
    const auto edit = set_vertex_normal(
        document, object_index, mesh_index, vertex_index, authored_normal);
    if (!edit.ok() || !edit.changed) {
        std::cerr << "scm-set-vertex-normal: typed edit rejected\n";
        return 8;
    }

    const auto written = Writer::write(document, WriteMode::preserve_layout);
    if (!written.ok() || !written.wrote || !written.reparse_ok) {
        std::cerr
            << "scm-set-vertex-normal: preserve-layout writer rejected output\n";
        return 9;
    }
    if (written.bytes.size() != source.size()) {
        std::cerr << "scm-set-vertex-normal: preserve-layout size changed\n";
        return 10;
    }

    auto expected = source;
    if (!scm_normal_authoring_detail::patch_f32_le(
            expected, normal_offset + 0U, x) ||
        !scm_normal_authoring_detail::patch_f32_le(
            expected, normal_offset + 4U, y) ||
        !scm_normal_authoring_detail::patch_f32_le(
            expected, normal_offset + 8U, z)) {
        std::cerr
            << "scm-set-vertex-normal: expected serialized span is out of bounds\n";
        return 11;
    }
    if (written.bytes != expected) {
        std::cerr
            << "scm-set-vertex-normal: exact-image guard rejected writer output\n";
        return 12;
    }

    const auto diffs = scm_authoring_detail::changed_offsets(
        std::span<const std::byte>{source},
        std::span<const std::byte>{written.bytes});
    if (diffs.empty()) {
        std::cerr << "scm-set-vertex-normal: authored image has no byte diff\n";
        return 12;
    }
    for (const auto offset : diffs) {
        if (offset < normal_offset || offset >= normal_offset + 12U) {
            std::cerr
                << "scm-set-vertex-normal: exact-span guard rejected unexpected output diff\n";
            return 12;
        }
    }

    const auto reparsed = Parser::parse(
        std::span<const std::byte>{written.bytes});
    if (!reparsed.ok() || object_index >= reparsed.document.objects.size() ||
        mesh_index >= reparsed.document.objects[object_index].meshes.size() ||
        vertex_index >= reparsed.document.objects[object_index]
                            .meshes[mesh_index]
                            .normals.size()) {
        std::cerr << "scm-set-vertex-normal: canonical output reparse failed\n";
        return 13;
    }
    const auto& reparsed_normal =
        reparsed.document.objects[object_index]
            .meshes[mesh_index]
            .normals[vertex_index];
    if (!scm_normal_authoring_detail::same_vec3_bits(
            reparsed_normal, authored_normal)) {
        std::cerr << "scm-set-vertex-normal: canonical output reparse mismatch\n";
        return 14;
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
                                .normals.size()) {
            return false;
        }
        return scm_normal_authoring_detail::same_vec3_bits(
            staged_parse.document.objects[object_index]
                .meshes[mesh_index]
                .normals[vertex_index],
            authored_normal);
    };

    const auto publication = core::publish_bytes_no_replace(
        output,
        std::span<const std::byte>{written.bytes},
        validator,
        ".dmc-rengine-scm-normal.staging");
    if (!publication.ok()) {
        std::cerr << "scm-set-vertex-normal: output publication failed ("
                  << core::to_string(publication.status) << ")";
        if (!publication.detail.empty()) {
            std::cerr << ": " << publication.detail;
        }
        std::cerr << '\n';
        return 15;
    }

    std::cout
        << "SCM_VERTEX_NORMAL_EDIT_OK"
        << " object=" << object_index
        << " mesh=" << mesh_index
        << " vertex=" << vertex_index
        << " normalOffset=" << normal_offset_u64
        << " oldX=" << source_normal.x
        << " oldY=" << source_normal.y
        << " oldZ=" << source_normal.z
        << " newX=" << x
        << " newY=" << y
        << " newZ=" << z
        << " changedBytes=" << diffs.size()
        << " sourceSize=" << source.size()
        << " outputSize=" << written.bytes.size()
        << " policy=PRESERVE_AUTHORED_COMPONENTS"
        << " reparse=PASS"
        << " exactImageGuard=PASS"
        << " publication=NO_REPLACE_PASS\n";
    return 0;
}

} // namespace dmc::rengine::cli
