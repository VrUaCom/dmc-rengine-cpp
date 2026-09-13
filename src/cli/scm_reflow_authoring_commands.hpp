#pragma once

#include "dmc_rengine/core/no_replace_publication.hpp"
#include "dmc_rengine/formats/scm.hpp"
#include "dmc_rengine/formats/scm_reflow_edit.hpp"
#include "dmc_rengine/formats/scm_topology.hpp"
#include "dmc_rengine/formats/scm_writer.hpp"

#include <charconv>
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <limits>
#include <span>
#include <string_view>
#include <system_error>
#include <vector>

namespace dmc::rengine::cli {
namespace scm_reflow_authoring_detail {

[[nodiscard]] inline bool parse_index(
    std::string_view text,
    std::size_t& value) noexcept {
    if (text.empty()) return false;
    std::uint64_t parsed = 0U;
    const auto result = std::from_chars(
        text.data(), text.data() + text.size(), parsed, 10);
    if (result.ec != std::errc{} || result.ptr != text.data() + text.size() ||
        parsed > std::numeric_limits<std::size_t>::max()) {
        return false;
    }
    value = static_cast<std::size_t>(parsed);
    return true;
}

[[nodiscard]] inline bool read_file(
    const std::filesystem::path& path,
    std::vector<std::byte>& bytes) {
    std::ifstream stream(path, std::ios::binary | std::ios::ate);
    if (!stream) return false;
    const auto end = stream.tellg();
    if (end < 0) return false;
    bytes.resize(static_cast<std::size_t>(end));
    stream.seekg(0, std::ios::beg);
    if (!bytes.empty()) {
        stream.read(
            reinterpret_cast<char*>(bytes.data()),
            static_cast<std::streamsize>(bytes.size()));
    }
    return static_cast<bool>(stream) || bytes.empty();
}

[[nodiscard]] inline std::size_t nondegenerate_strip_triangles(
    std::span<const std::uint8_t> flags) {
    const auto indices =
        formats::scm::generate_triangle_strip_indices(flags);
    std::size_t count = 0U;
    for (std::size_t index = 2U; index < indices.size(); ++index) {
        const auto a = indices[index - 2U];
        const auto b = indices[index - 1U];
        const auto c = indices[index];
        if (a != b && b != c && a != c) ++count;
    }
    return count;
}

[[nodiscard]] inline std::vector<std::uint8_t> topology_flags(
    const formats::scm::Mesh& mesh) {
    std::vector<std::uint8_t> result;
    result.reserve(mesh.colors_topology.size());
    for (const auto& value : mesh.colors_topology) {
        result.push_back(value.topology_flags);
    }
    return result;
}

} // namespace scm_reflow_authoring_detail

inline void print_scm_reflow_authoring_help() {
    std::cout
        << "  scm-append-break-vertex-copy <input.scm> <object-index> <mesh-index> <source-vertex-index> <output.scm>\n"
        << "                             Evidence-oriented size-changing canonical rebuild: copy one vertex and append topology break 0x02\n";
}

inline int try_run_scm_reflow_authoring_command(int argc, char** argv) {
    using namespace formats::scm;

    if (argc <= 1) return -1;
    const std::string_view command{argv[1]};
    if (command != "scm-append-break-vertex-copy") return -1;

    if (argc != 7) {
        std::cerr
            << "usage: scm-append-break-vertex-copy <input.scm> <object-index> <mesh-index> <source-vertex-index> <output.scm>\n";
        return 1;
    }

    std::size_t object_index = 0U;
    std::size_t mesh_index = 0U;
    std::size_t source_vertex_index = 0U;
    if (!scm_reflow_authoring_detail::parse_index(argv[3], object_index) ||
        !scm_reflow_authoring_detail::parse_index(argv[4], mesh_index) ||
        !scm_reflow_authoring_detail::parse_index(
            argv[5], source_vertex_index)) {
        std::cerr << "scm-append-break-vertex-copy: invalid index\n";
        return 2;
    }

    const std::filesystem::path input{argv[2]};
    const std::filesystem::path output{argv[6]};
    std::vector<std::byte> source;
    if (!scm_reflow_authoring_detail::read_file(input, source)) {
        std::cerr << "scm-append-break-vertex-copy: cannot read input\n";
        return 3;
    }

    const auto parsed = Parser::parse(std::span<const std::byte>{source});
    if (!parsed.ok()) {
        std::cerr << "scm-append-break-vertex-copy: canonical parse failed\n";
        return 4;
    }
    if (object_index >= parsed.document.objects.size() ||
        mesh_index >= parsed.document.objects[object_index].meshes.size()) {
        std::cerr << "scm-append-break-vertex-copy: object/mesh index out of range\n";
        return 5;
    }

    const auto& source_mesh =
        parsed.document.objects[object_index].meshes[mesh_index];
    if (source_vertex_index >= source_mesh.positions.size()) {
        std::cerr << "scm-append-break-vertex-copy: source vertex out of range\n";
        return 6;
    }
    const auto source_vertex_count = source_mesh.positions.size();
    const auto source_object_vertex_count = [&]() {
        std::size_t total = 0U;
        for (const auto& mesh : parsed.document.objects[object_index].meshes) {
            total += mesh.positions.size();
        }
        return total;
    }();
    const auto source_topology =
        scm_reflow_authoring_detail::topology_flags(source_mesh);
    const auto source_triangle_count =
        scm_reflow_authoring_detail::nondegenerate_strip_triangles(
            source_topology);

    const auto copied_position = source_mesh.positions[source_vertex_index];
    const auto copied_normal = source_mesh.normals[source_vertex_index];
    const auto copied_uv = source_mesh.uvs[source_vertex_index];
    const auto copied_color = source_mesh.colors_topology[source_vertex_index];

    auto document = parsed.document;
    const auto edit = append_break_vertex_copy(
        document, object_index, mesh_index, source_vertex_index);
    if (!edit.ok() || !edit.changed) {
        std::cerr << "scm-append-break-vertex-copy: typed structural edit rejected\n";
        return 7;
    }

    const auto preserve = Writer::write(document, WriteMode::preserve_layout);
    if (preserve.ok()) {
        std::cerr
            << "scm-append-break-vertex-copy: preserve-layout unexpectedly accepted size-changing edit\n";
        return 8;
    }

    const auto written = Writer::write(document, WriteMode::canonical_rebuild);
    if (!written.ok() || !written.wrote || !written.reparse_ok) {
        std::cerr
            << "scm-append-break-vertex-copy: canonical rebuild rejected output\n";
        for (const auto& diagnostic : written.diagnostics) {
            std::cerr << "  " << diagnostic.code << ": "
                      << diagnostic.message << '\n';
        }
        return 9;
    }
    if (written.bytes.size() <= source.size()) {
        std::cerr
            << "scm-append-break-vertex-copy: output did not grow\n";
        return 10;
    }

    const auto reparsed = Parser::parse(
        std::span<const std::byte>{written.bytes});
    if (!reparsed.ok() || object_index >= reparsed.document.objects.size() ||
        mesh_index >= reparsed.document.objects[object_index].meshes.size()) {
        std::cerr
            << "scm-append-break-vertex-copy: rebuilt output reparse failed\n";
        return 11;
    }

    const auto& output_object = reparsed.document.objects[object_index];
    const auto& output_mesh = output_object.meshes[mesh_index];
    if (output_mesh.positions.size() != source_vertex_count + 1U ||
        output_object.total_vertex_count != source_object_vertex_count + 1U ||
        output_mesh.vertex_count != source_vertex_count + 1U) {
        std::cerr
            << "scm-append-break-vertex-copy: derived vertex counts mismatch\n";
        return 12;
    }

    const auto& appended_position = output_mesh.positions.back();
    const auto& appended_normal = output_mesh.normals.back();
    const auto& appended_uv = output_mesh.uvs.back();
    const auto& appended_color = output_mesh.colors_topology.back();
    if (appended_position.x != copied_position.x ||
        appended_position.y != copied_position.y ||
        appended_position.z != copied_position.z ||
        appended_normal.x != copied_normal.x ||
        appended_normal.y != copied_normal.y ||
        appended_normal.z != copied_normal.z ||
        appended_uv.u != copied_uv.u || appended_uv.v != copied_uv.v ||
        appended_color.r != copied_color.r ||
        appended_color.g != copied_color.g ||
        appended_color.b != copied_color.b ||
        appended_color.topology_flags != triangle_break_bit) {
        std::cerr
            << "scm-append-break-vertex-copy: appended vertex reparse mismatch\n";
        return 13;
    }

    const auto output_topology =
        scm_reflow_authoring_detail::topology_flags(output_mesh);
    const auto output_triangle_count =
        scm_reflow_authoring_detail::nondegenerate_strip_triangles(
            output_topology);
    if (output_triangle_count != source_triangle_count) {
        std::cerr
            << "scm-append-break-vertex-copy: structural growth changed non-degenerate triangle count\n";
        return 14;
    }

    const auto validator = [&](const std::filesystem::path& staged_path) {
        std::vector<std::byte> staged;
        if (!scm_reflow_authoring_detail::read_file(staged_path, staged) ||
            staged != written.bytes) {
            return false;
        }
        const auto staged_parse = Parser::parse(
            std::span<const std::byte>{staged});
        if (!staged_parse.ok() ||
            object_index >= staged_parse.document.objects.size() ||
            mesh_index >=
                staged_parse.document.objects[object_index].meshes.size()) {
            return false;
        }
        const auto& staged_mesh =
            staged_parse.document.objects[object_index].meshes[mesh_index];
        return staged_mesh.vertex_count == source_vertex_count + 1U &&
               !staged_mesh.colors_topology.empty() &&
               staged_mesh.colors_topology.back().topology_flags ==
                   triangle_break_bit;
    };

    const auto publication = core::publish_bytes_no_replace(
        output,
        std::span<const std::byte>{written.bytes},
        validator,
        ".dmc-rengine-scm-reflow.staging");
    if (!publication.ok()) {
        std::cerr
            << "scm-append-break-vertex-copy: publication failed: "
            << to_string(publication.status) << '\n';
        return 15;
    }

    std::cout
        << "SCM_SIZE_CHANGING_REBUILD_OK\n"
        << "object=" << object_index << '\n'
        << "mesh=" << mesh_index << '\n'
        << "copiedVertex=" << source_vertex_index << '\n'
        << "sourceVertices=" << source_vertex_count << '\n'
        << "outputVertices=" << output_mesh.vertex_count << '\n'
        << "sourceObjectVertices=" << source_object_vertex_count << '\n'
        << "outputObjectVertices=" << output_object.total_vertex_count << '\n'
        << "sourceSize=" << source.size() << '\n'
        << "outputSize=" << written.bytes.size() << '\n'
        << "sizeDelta=" << (written.bytes.size() - source.size()) << '\n'
        << "trianglesBefore=" << source_triangle_count << '\n'
        << "trianglesAfter=" << output_triangle_count << '\n'
        << "appendedTopology=2\n"
        << "reparse=PASS\n"
        << "publication=NO_REPLACE_PASS\n";
    return 0;
}

} // namespace dmc::rengine::cli
