#pragma once

#include "scm_authoring_commands.hpp"

#include <cmath>
#include <cstdint>
#include <limits>
#include <optional>

namespace dmc::rengine::cli {
namespace scm_uv_authoring_detail {

[[nodiscard]] inline std::optional<std::int16_t> encode_component_guard(
    float value) noexcept {
    if (!std::isfinite(value)) return std::nullopt;
    constexpr double scale = 4096.0;
    constexpr double minimum =
        static_cast<double>(std::numeric_limits<std::int16_t>::min());
    constexpr double maximum =
        static_cast<double>(std::numeric_limits<std::int16_t>::max());
    const auto scaled = static_cast<double>(value) * scale;
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

[[nodiscard]] inline bool patch_i16_le(
    std::vector<std::byte>& bytes,
    std::size_t offset,
    std::int16_t value) noexcept {
    if (offset > bytes.size() || 2U > bytes.size() - offset) return false;
    const auto raw = static_cast<std::uint16_t>(value);
    bytes[offset + 0U] = std::byte{static_cast<unsigned char>(raw & 0xFFU)};
    bytes[offset + 1U] =
        std::byte{static_cast<unsigned char>((raw >> 8U) & 0xFFU)};
    return true;
}

[[nodiscard]] inline bool same_uv(
    const formats::scm::SerializedUv& lhs,
    const formats::scm::SerializedUv& rhs) noexcept {
    return lhs.u == rhs.u && lhs.v == rhs.v;
}

} // namespace scm_uv_authoring_detail

inline void print_scm_uv_authoring_help() {
    std::cout
        << "  scm-set-vertex-uv <input.scm> <object-index> <mesh-index> <vertex-index> <u> <v> <output.scm>\n"
        << "                       Bounded SCM UV authoring at signed int16 / 4096 with exact 4-byte guard\n";
}

inline int try_run_scm_uv_authoring_command(int argc, char** argv) {
    using formats::scm::Parser;
    using formats::scm::SerializedUv;
    using formats::scm::WriteMode;
    using formats::scm::Writer;
    using formats::scm::set_uv;

    if (argc <= 1 || std::string_view{argv[1]} != "scm-set-vertex-uv") {
        return -1;
    }
    if (argc != 9) {
        std::cerr
            << "usage: scm-set-vertex-uv <input.scm> <object-index> <mesh-index> <vertex-index> <u> <v> <output.scm>\n";
        return 1;
    }

    std::uint64_t object_index_raw = 0U;
    std::uint64_t mesh_index_raw = 0U;
    std::uint64_t vertex_index_raw = 0U;
    float u = 0.0F;
    float v = 0.0F;
    if (!scm_authoring_detail::parse_u64(argv[3], object_index_raw) ||
        !scm_authoring_detail::parse_u64(argv[4], mesh_index_raw) ||
        !scm_authoring_detail::parse_u64(argv[5], vertex_index_raw) ||
        object_index_raw > std::numeric_limits<std::size_t>::max() ||
        mesh_index_raw > std::numeric_limits<std::size_t>::max() ||
        vertex_index_raw > std::numeric_limits<std::size_t>::max() ||
        !scm_authoring_detail::parse_float(argv[6], u) ||
        !scm_authoring_detail::parse_float(argv[7], v)) {
        std::cerr << "scm-set-vertex-uv: invalid index or non-finite UV\n";
        return 2;
    }

    const auto encoded_u = scm_uv_authoring_detail::encode_component_guard(u);
    const auto encoded_v = scm_uv_authoring_detail::encode_component_guard(v);
    if (!encoded_u.has_value() || !encoded_v.has_value()) {
        std::cerr
            << "scm-set-vertex-uv: UV cannot be represented as signed int16 / 4096\n";
        return 3;
    }
    const SerializedUv expected_uv{*encoded_u, *encoded_v};

    const std::filesystem::path input{argv[2]};
    const std::filesystem::path output{argv[8]};
    std::vector<std::byte> source;
    if (!scm_authoring_detail::read_file(input, source)) {
        std::cerr << "scm-set-vertex-uv: cannot read input\n";
        return 4;
    }

    const auto parsed = Parser::parse(std::span<const std::byte>{source});
    if (!parsed.ok()) {
        std::cerr << "scm-set-vertex-uv: canonical parse failed\n";
        return 5;
    }

    const auto object_index = static_cast<std::size_t>(object_index_raw);
    const auto mesh_index = static_cast<std::size_t>(mesh_index_raw);
    const auto vertex_index = static_cast<std::size_t>(vertex_index_raw);
    if (object_index >= parsed.document.objects.size()) {
        std::cerr << "scm-set-vertex-uv: object index out of range\n";
        return 6;
    }
    const auto& source_object = parsed.document.objects[object_index];
    if (mesh_index >= source_object.meshes.size()) {
        std::cerr << "scm-set-vertex-uv: mesh index out of range\n";
        return 6;
    }
    const auto& source_mesh = source_object.meshes[mesh_index];
    if (vertex_index >= source_mesh.uvs.size()) {
        std::cerr << "scm-set-vertex-uv: vertex index out of range\n";
        return 6;
    }

    const auto source_uv = source_mesh.uvs[vertex_index];
    if (scm_uv_authoring_detail::same_uv(source_uv, expected_uv)) {
        std::cerr
            << "scm-set-vertex-uv: requested UV quantizes to the existing serialized value\n";
        return 7;
    }

    const auto uv_offset_u64 =
        source_mesh.uv_offset + static_cast<std::uint64_t>(vertex_index) * 4U;
    if (uv_offset_u64 > std::numeric_limits<std::size_t>::max()) {
        std::cerr << "scm-set-vertex-uv: serialized offset overflow\n";
        return 8;
    }
    const auto uv_offset = static_cast<std::size_t>(uv_offset_u64);

    auto document = parsed.document;
    const auto edit = set_uv(
        document, object_index, mesh_index, vertex_index, u, v);
    if (!edit.ok() || !edit.changed) {
        std::cerr << "scm-set-vertex-uv: typed edit rejected\n";
        return 9;
    }
    const auto& typed_uv =
        document.objects[object_index].meshes[mesh_index].uvs[vertex_index];
    if (!scm_uv_authoring_detail::same_uv(typed_uv, expected_uv)) {
        std::cerr
            << "scm-set-vertex-uv: typed encoder disagrees with independent encoding guard\n";
        return 10;
    }

    const auto written = Writer::write(document, WriteMode::preserve_layout);
    if (!written.ok() || !written.wrote || !written.reparse_ok) {
        std::cerr
            << "scm-set-vertex-uv: preserve-layout writer rejected output\n";
        return 11;
    }
    if (written.bytes.size() != source.size()) {
        std::cerr << "scm-set-vertex-uv: preserve-layout size changed\n";
        return 12;
    }

    auto expected = source;
    if (!scm_uv_authoring_detail::patch_i16_le(
            expected, uv_offset + 0U, expected_uv.u) ||
        !scm_uv_authoring_detail::patch_i16_le(
            expected, uv_offset + 2U, expected_uv.v)) {
        std::cerr
            << "scm-set-vertex-uv: expected serialized span is out of bounds\n";
        return 13;
    }
    if (written.bytes != expected) {
        std::cerr
            << "scm-set-vertex-uv: exact-image guard rejected writer output\n";
        return 14;
    }

    const auto diffs = scm_authoring_detail::changed_offsets(
        std::span<const std::byte>{source},
        std::span<const std::byte>{written.bytes});
    if (diffs.empty()) {
        std::cerr << "scm-set-vertex-uv: authored image has no byte diff\n";
        return 14;
    }
    for (const auto offset : diffs) {
        if (offset < uv_offset || offset >= uv_offset + 4U) {
            std::cerr
                << "scm-set-vertex-uv: exact-span guard rejected unexpected output diff\n";
            return 14;
        }
    }

    const auto reparsed = Parser::parse(
        std::span<const std::byte>{written.bytes});
    if (!reparsed.ok() || object_index >= reparsed.document.objects.size() ||
        mesh_index >= reparsed.document.objects[object_index].meshes.size() ||
        vertex_index >= reparsed.document.objects[object_index]
                            .meshes[mesh_index]
                            .uvs.size()) {
        std::cerr << "scm-set-vertex-uv: canonical output reparse failed\n";
        return 15;
    }
    const auto& reparsed_uv =
        reparsed.document.objects[object_index]
            .meshes[mesh_index]
            .uvs[vertex_index];
    if (!scm_uv_authoring_detail::same_uv(reparsed_uv, expected_uv)) {
        std::cerr << "scm-set-vertex-uv: canonical output reparse mismatch\n";
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
                                .uvs.size()) {
            return false;
        }
        return scm_uv_authoring_detail::same_uv(
            staged_parse.document.objects[object_index]
                .meshes[mesh_index]
                .uvs[vertex_index],
            expected_uv);
    };

    const auto publication = core::publish_bytes_no_replace(
        output,
        std::span<const std::byte>{written.bytes},
        validator,
        ".dmc-rengine-scm-uv.staging");
    if (!publication.ok()) {
        std::cerr << "scm-set-vertex-uv: output publication failed ("
                  << core::to_string(publication.status) << ")";
        if (!publication.detail.empty()) {
            std::cerr << ": " << publication.detail;
        }
        std::cerr << '\n';
        return 17;
    }

    std::cout
        << "SCM_VERTEX_UV_EDIT_OK"
        << " object=" << object_index
        << " mesh=" << mesh_index
        << " vertex=" << vertex_index
        << " uvOffset=" << uv_offset_u64
        << " oldRawU=" << source_uv.u
        << " oldRawV=" << source_uv.v
        << " newRawU=" << expected_uv.u
        << " newRawV=" << expected_uv.v
        << " scale=4096"
        << " changedBytes=" << diffs.size()
        << " sourceSize=" << source.size()
        << " outputSize=" << written.bytes.size()
        << " reparse=PASS"
        << " exactImageGuard=PASS"
        << " encoderCrossCheck=PASS"
        << " publication=NO_REPLACE_PASS\n";
    return 0;
}

} // namespace dmc::rengine::cli
