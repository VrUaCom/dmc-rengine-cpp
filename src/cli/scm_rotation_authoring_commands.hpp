#pragma once

#include "scm_authoring_commands.hpp"

namespace dmc::rengine::cli {

inline void print_scm_rotation_authoring_help() {
    std::cout
        << "  scm-set-node-rotation <input.scm> <node-index> <x-rad> <y-rad> <z-rad> <output.scm>\n"
        << "                             Bounded preserve-layout SCM rotation authoring with exact-span guard\n";
}

inline int try_run_scm_rotation_authoring_command(int argc, char** argv) {
    using formats::scm::Parser;
    using formats::scm::Vec3f;
    using formats::scm::WriteMode;
    using formats::scm::Writer;
    using formats::scm::scene_transform_size;
    using formats::scm::set_node_rotation;

    if (argc <= 1 || std::string_view{argv[1]} != "scm-set-node-rotation") {
        return -1;
    }
    if (argc != 8) {
        std::cerr
            << "usage: scm-set-node-rotation <input.scm> <node-index> <x-rad> <y-rad> <z-rad> <output.scm>\n";
        return 1;
    }

    std::uint64_t node_index_raw = 0U;
    float x = 0.0F;
    float y = 0.0F;
    float z = 0.0F;
    if (!scm_authoring_detail::parse_u64(argv[3], node_index_raw) ||
        node_index_raw > std::numeric_limits<std::size_t>::max() ||
        !scm_authoring_detail::parse_float(argv[4], x) ||
        !scm_authoring_detail::parse_float(argv[5], y) ||
        !scm_authoring_detail::parse_float(argv[6], z)) {
        std::cerr
            << "scm-set-node-rotation: invalid node index or non-finite rotation\n";
        return 2;
    }

    const std::filesystem::path input{argv[2]};
    const std::filesystem::path output{argv[7]};
    std::vector<std::byte> source;
    if (!scm_authoring_detail::read_file(input, source)) {
        std::cerr << "scm-set-node-rotation: cannot read input\n";
        return 3;
    }

    const auto parsed = Parser::parse(std::span<const std::byte>{source});
    if (!parsed.ok()) {
        std::cerr << "scm-set-node-rotation: canonical parse failed\n";
        return 4;
    }

    const auto node_index = static_cast<std::size_t>(node_index_raw);
    if (node_index >=
        parsed.document.scene_nodes.transform_by_node_index.size()) {
        std::cerr << "scm-set-node-rotation: node index out of range\n";
        return 5;
    }

    auto document = parsed.document;
    const auto source_transform =
        parsed.document.scene_nodes.transform_by_node_index[node_index];
    if (source_transform.rotation_xyz_radians.x == x &&
        source_transform.rotation_xyz_radians.y == y &&
        source_transform.rotation_xyz_radians.z == z) {
        std::cerr
            << "scm-set-node-rotation: requested rotation is already present\n";
        return 6;
    }

    const auto transform_offset =
        document.scene_nodes.offset + document.scene_nodes.transform_rel +
        static_cast<std::uint64_t>(node_index) * scene_transform_size;
    const Vec3f new_rotation{x, y, z};
    const auto edit = set_node_rotation(document, node_index, new_rotation);
    if (!edit.ok() || !edit.changed) {
        std::cerr << "scm-set-node-rotation: typed edit rejected\n";
        return 7;
    }

    const auto written = Writer::write(document, WriteMode::preserve_layout);
    if (!written.ok() || !written.wrote || !written.reparse_ok) {
        std::cerr
            << "scm-set-node-rotation: preserve-layout writer rejected output\n";
        return 8;
    }
    if (written.bytes.size() != source.size()) {
        std::cerr
            << "scm-set-node-rotation: preserve-layout size changed\n";
        return 9;
    }

    const auto diffs = scm_authoring_detail::changed_offsets(
        std::span<const std::byte>{source},
        std::span<const std::byte>{written.bytes});
    const auto authored_begin =
        static_cast<std::size_t>(transform_offset + 0x10U);
    const auto authored_end =
        static_cast<std::size_t>(transform_offset + 0x1CU);
    bool unexpected_diff = diffs.empty();
    for (const auto offset : diffs) {
        if (offset < authored_begin || offset >= authored_end) {
            unexpected_diff = true;
            break;
        }
    }
    if (unexpected_diff) {
        std::cerr
            << "scm-set-node-rotation: exact-span guard rejected unexpected output diff\n";
        return 10;
    }

    const auto reparsed = Parser::parse(
        std::span<const std::byte>{written.bytes});
    if (!reparsed.ok() ||
        node_index >= reparsed.document.scene_nodes.transform_by_node_index.size()) {
        std::cerr
            << "scm-set-node-rotation: canonical output reparse failed\n";
        return 11;
    }
    const auto& reparsed_transform =
        reparsed.document.scene_nodes.transform_by_node_index[node_index];
    if (reparsed_transform.rotation_xyz_radians.x != x ||
        reparsed_transform.rotation_xyz_radians.y != y ||
        reparsed_transform.rotation_xyz_radians.z != z ||
        reparsed_transform.translation.x != source_transform.translation.x ||
        reparsed_transform.translation.y != source_transform.translation.y ||
        reparsed_transform.translation.z != source_transform.translation.z ||
        reparsed_transform.translation_magnitude !=
            source_transform.translation_magnitude ||
        reparsed_transform.reserved1c != source_transform.reserved1c) {
        std::cerr
            << "scm-set-node-rotation: canonical output reparse mismatch\n";
        return 11;
    }

    const auto validator = [&](const std::filesystem::path& staged_path) {
        std::vector<std::byte> staged;
        if (!scm_authoring_detail::read_file(staged_path, staged) ||
            staged != written.bytes) {
            return false;
        }
        const auto staged_parse = Parser::parse(
            std::span<const std::byte>{staged});
        if (!staged_parse.ok() ||
            node_index >=
                staged_parse.document.scene_nodes.transform_by_node_index.size()) {
            return false;
        }
        const auto& staged_transform =
            staged_parse.document.scene_nodes.transform_by_node_index[node_index];
        return staged_transform.rotation_xyz_radians.x == x &&
               staged_transform.rotation_xyz_radians.y == y &&
               staged_transform.rotation_xyz_radians.z == z &&
               staged_transform.translation.x == source_transform.translation.x &&
               staged_transform.translation.y == source_transform.translation.y &&
               staged_transform.translation.z == source_transform.translation.z &&
               staged_transform.translation_magnitude ==
                   source_transform.translation_magnitude &&
               staged_transform.reserved1c == source_transform.reserved1c;
    };

    const auto publication = core::publish_bytes_no_replace(
        output,
        std::span<const std::byte>{written.bytes},
        validator,
        ".dmc-rengine-scm-rotation.staging");
    if (!publication.ok()) {
        std::cerr
            << "scm-set-node-rotation: output publication failed ("
            << core::to_string(publication.status) << ")";
        if (!publication.detail.empty()) {
            std::cerr << ": " << publication.detail;
        }
        std::cerr << '\n';
        return 12;
    }

    std::cout
        << "SCM_ROTATION_EDIT_OK"
        << " node=" << node_index
        << " transformOffset=" << transform_offset
        << " oldX=" << source_transform.rotation_xyz_radians.x
        << " oldY=" << source_transform.rotation_xyz_radians.y
        << " oldZ=" << source_transform.rotation_xyz_radians.z
        << " newX=" << x
        << " newY=" << y
        << " newZ=" << z
        << " changedBytes=" << diffs.size()
        << " sourceSize=" << source.size()
        << " outputSize=" << written.bytes.size()
        << " reparse=PASS"
        << " exactSpanGuard=PASS"
        << " publication=NO_REPLACE_PASS\n";
    return 0;
}

} // namespace dmc::rengine::cli
