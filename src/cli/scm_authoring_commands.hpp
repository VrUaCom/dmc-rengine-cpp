#pragma once

#include "dmc_rengine/core/no_replace_publication.hpp"
#include "dmc_rengine/formats/scm.hpp"
#include "dmc_rengine/formats/scm_edit.hpp"
#include "dmc_rengine/formats/scm_writer.hpp"

#include <charconv>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <limits>
#include <span>
#include <string>
#include <string_view>
#include <system_error>
#include <vector>

namespace dmc::rengine::cli {
namespace scm_authoring_detail {

[[nodiscard]] inline bool parse_u64(
    std::string_view text,
    std::uint64_t& value) noexcept {
    if (text.empty()) return false;
    const auto* first = text.data();
    const auto* last = text.data() + text.size();
    const auto result = std::from_chars(first, last, value, 10);
    return result.ec == std::errc{} && result.ptr == last;
}

[[nodiscard]] inline bool parse_float(
    std::string_view text,
    float& value) noexcept {
    if (text.empty()) return false;
    const auto* first = text.data();
    const auto* last = text.data() + text.size();
    const auto result = std::from_chars(
        first, last, value, std::chars_format::general);
    return result.ec == std::errc{} && result.ptr == last &&
           std::isfinite(value);
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

[[nodiscard]] inline std::vector<std::size_t> changed_offsets(
    std::span<const std::byte> before,
    std::span<const std::byte> after) {
    std::vector<std::size_t> offsets;
    const auto common = before.size() < after.size() ? before.size() : after.size();
    for (std::size_t index = 0U; index < common; ++index) {
        if (before[index] != after[index]) offsets.push_back(index);
    }
    if (before.size() != after.size()) {
        const auto larger = before.size() > after.size() ? before.size() : after.size();
        for (std::size_t index = common; index < larger; ++index) {
            offsets.push_back(index);
        }
    }
    return offsets;
}

} // namespace scm_authoring_detail

inline void print_scm_authoring_help() {
    std::cout
        << "  scm-set-alpha-control <input.scm> <object-index> <0..255> <output.scm>\n"
        << "                             Bounded preserve-layout SCM alpha authoring with exact-byte guard\n"
        << "  scm-set-node-translation <input.scm> <node-index> <x> <y> <z> <output.scm>\n"
        << "                             Bounded preserve-layout SCM translation authoring with derived-magnitude guard\n";
}

inline int try_run_scm_authoring_command(int argc, char** argv) {
    using formats::scm::Parser;
    using formats::scm::Vec3f;
    using formats::scm::WriteMode;
    using formats::scm::Writer;
    using formats::scm::scene_transform_size;
    using formats::scm::set_alpha_control;
    using formats::scm::set_node_translation;

    if (argc <= 1) {
        return -1;
    }

    const std::string_view command{argv[1]};
    if (command == "scm-set-node-translation") {
        if (argc != 8) {
            std::cerr
                << "usage: scm-set-node-translation <input.scm> <node-index> <x> <y> <z> <output.scm>\n";
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
                << "scm-set-node-translation: invalid node index or non-finite translation\n";
            return 2;
        }

        const std::filesystem::path input{argv[2]};
        const std::filesystem::path output{argv[7]};
        std::vector<std::byte> source;
        if (!scm_authoring_detail::read_file(input, source)) {
            std::cerr << "scm-set-node-translation: cannot read input\n";
            return 3;
        }

        const auto parsed = Parser::parse(std::span<const std::byte>{source});
        if (!parsed.ok()) {
            std::cerr << "scm-set-node-translation: canonical parse failed\n";
            return 4;
        }

        const auto node_index = static_cast<std::size_t>(node_index_raw);
        if (node_index >=
            parsed.document.scene_nodes.transform_by_node_index.size()) {
            std::cerr << "scm-set-node-translation: node index out of range\n";
            return 5;
        }

        auto document = parsed.document;
        const auto& source_transform =
            parsed.document.scene_nodes.transform_by_node_index[node_index];
        const Vec3f new_translation{x, y, z};
        if (source_transform.translation.x == x &&
            source_transform.translation.y == y &&
            source_transform.translation.z == z) {
            std::cerr
                << "scm-set-node-translation: requested translation is already present\n";
            return 6;
        }

        const auto transform_offset =
            document.scene_nodes.offset + document.scene_nodes.transform_rel +
            static_cast<std::uint64_t>(node_index) * scene_transform_size;
        const auto edit =
            set_node_translation(document, node_index, new_translation);
        if (!edit.ok() || !edit.changed) {
            std::cerr << "scm-set-node-translation: typed edit rejected\n";
            return 7;
        }
        const auto authored_magnitude =
            document.scene_nodes.transform_by_node_index[node_index]
                .translation_magnitude;

        const auto written = Writer::write(document, WriteMode::preserve_layout);
        if (!written.ok() || !written.wrote || !written.reparse_ok) {
            std::cerr
                << "scm-set-node-translation: preserve-layout writer rejected output\n";
            return 8;
        }
        if (written.bytes.size() != source.size()) {
            std::cerr
                << "scm-set-node-translation: preserve-layout size changed\n";
            return 9;
        }

        const auto diffs = scm_authoring_detail::changed_offsets(
            std::span<const std::byte>{source},
            std::span<const std::byte>{written.bytes});
        const auto authored_begin =
            static_cast<std::size_t>(transform_offset);
        const auto authored_end = authored_begin + 0x10U;
        bool translation_byte_changed = false;
        bool unexpected_diff = diffs.empty();
        for (const auto offset : diffs) {
            if (offset < authored_begin || offset >= authored_end) {
                unexpected_diff = true;
                break;
            }
            if (offset < authored_begin + 0x0CU) {
                translation_byte_changed = true;
            }
        }
        if (unexpected_diff || !translation_byte_changed) {
            std::cerr
                << "scm-set-node-translation: exact-span guard rejected unexpected output diff\n";
            return 10;
        }

        const auto reparsed = Parser::parse(
            std::span<const std::byte>{written.bytes});
        if (!reparsed.ok() ||
            node_index >= reparsed.document.scene_nodes.transform_by_node_index.size()) {
            std::cerr
                << "scm-set-node-translation: canonical output reparse failed\n";
            return 11;
        }
        const auto& reparsed_transform =
            reparsed.document.scene_nodes.transform_by_node_index[node_index];
        if (reparsed_transform.translation.x != x ||
            reparsed_transform.translation.y != y ||
            reparsed_transform.translation.z != z ||
            reparsed_transform.translation_magnitude != authored_magnitude ||
            reparsed_transform.rotation_xyz_radians.x !=
                source_transform.rotation_xyz_radians.x ||
            reparsed_transform.rotation_xyz_radians.y !=
                source_transform.rotation_xyz_radians.y ||
            reparsed_transform.rotation_xyz_radians.z !=
                source_transform.rotation_xyz_radians.z ||
            reparsed_transform.reserved1c != source_transform.reserved1c) {
            std::cerr
                << "scm-set-node-translation: canonical output reparse mismatch\n";
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
            return staged_transform.translation.x == x &&
                   staged_transform.translation.y == y &&
                   staged_transform.translation.z == z &&
                   staged_transform.translation_magnitude == authored_magnitude &&
                   staged_transform.reserved1c == source_transform.reserved1c;
        };

        const auto publication = core::publish_bytes_no_replace(
            output,
            std::span<const std::byte>{written.bytes},
            validator,
            ".dmc-rengine-scm-translation.staging");
        if (!publication.ok()) {
            std::cerr
                << "scm-set-node-translation: output publication failed ("
                << core::to_string(publication.status) << ")";
            if (!publication.detail.empty()) {
                std::cerr << ": " << publication.detail;
            }
            std::cerr << '\n';
            return 12;
        }

        std::cout
            << "SCM_TRANSLATION_EDIT_OK"
            << " node=" << node_index
            << " transformOffset=" << transform_offset
            << " oldX=" << source_transform.translation.x
            << " oldY=" << source_transform.translation.y
            << " oldZ=" << source_transform.translation.z
            << " newX=" << x
            << " newY=" << y
            << " newZ=" << z
            << " oldMagnitude=" << source_transform.translation_magnitude
            << " newMagnitude=" << authored_magnitude
            << " changedBytes=" << diffs.size()
            << " sourceSize=" << source.size()
            << " outputSize=" << written.bytes.size()
            << " reparse=PASS"
            << " exactSpanGuard=PASS"
            << " publication=NO_REPLACE_PASS\n";
        return 0;
    }

    if (command != "scm-set-alpha-control") {
        return -1;
    }
    if (argc != 6) {
        std::cerr
            << "usage: scm-set-alpha-control <input.scm> <object-index> <0..255> <output.scm>\n";
        return 1;
    }

    std::uint64_t object_index_raw = 0U;
    std::uint64_t alpha_raw = 0U;
    if (!scm_authoring_detail::parse_u64(argv[3], object_index_raw) ||
        !scm_authoring_detail::parse_u64(argv[4], alpha_raw) ||
        object_index_raw > std::numeric_limits<std::size_t>::max() ||
        alpha_raw > 0xFFU) {
        std::cerr << "scm-set-alpha-control: invalid object index or alpha value\n";
        return 2;
    }

    const std::filesystem::path input{argv[2]};
    const std::filesystem::path output{argv[5]};
    std::vector<std::byte> source;
    if (!scm_authoring_detail::read_file(input, source)) {
        std::cerr << "scm-set-alpha-control: cannot read input\n";
        return 3;
    }

    const auto parsed = Parser::parse(std::span<const std::byte>{source});
    if (!parsed.ok()) {
        std::cerr << "scm-set-alpha-control: canonical parse failed\n";
        return 4;
    }
    const auto object_index = static_cast<std::size_t>(object_index_raw);
    if (object_index >= parsed.document.objects.size()) {
        std::cerr << "scm-set-alpha-control: object index out of range\n";
        return 5;
    }

    auto document = parsed.document;
    const auto old_alpha = document.objects[object_index].alpha_control;
    const auto object_offset = document.objects[object_index].record_offset;
    const auto authored_offset = object_offset + 0x01U;
    const auto new_alpha = static_cast<std::uint8_t>(alpha_raw);
    if (old_alpha == new_alpha) {
        std::cerr << "scm-set-alpha-control: requested value is already present\n";
        return 6;
    }

    const auto edit = set_alpha_control(document, object_index, new_alpha);
    if (!edit.ok() || !edit.changed) {
        std::cerr << "scm-set-alpha-control: typed edit rejected\n";
        return 7;
    }

    const auto written = Writer::write(document, WriteMode::preserve_layout);
    if (!written.ok() || !written.wrote || !written.reparse_ok) {
        std::cerr << "scm-set-alpha-control: preserve-layout writer rejected output\n";
        return 8;
    }
    if (written.bytes.size() != source.size()) {
        std::cerr << "scm-set-alpha-control: preserve-layout size changed\n";
        return 9;
    }

    const auto diffs = scm_authoring_detail::changed_offsets(
        std::span<const std::byte>{source},
        std::span<const std::byte>{written.bytes});
    if (diffs.size() != 1U || diffs[0] != authored_offset) {
        std::cerr
            << "scm-set-alpha-control: exact-byte guard rejected unexpected output diff\n";
        return 10;
    }

    const auto reparsed = Parser::parse(
        std::span<const std::byte>{written.bytes});
    if (!reparsed.ok() || object_index >= reparsed.document.objects.size() ||
        reparsed.document.objects[object_index].alpha_control != new_alpha) {
        std::cerr << "scm-set-alpha-control: canonical output reparse mismatch\n";
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
        return staged_parse.ok() &&
               object_index < staged_parse.document.objects.size() &&
               staged_parse.document.objects[object_index].alpha_control == new_alpha;
    };

    const auto publication = core::publish_bytes_no_replace(
        output,
        std::span<const std::byte>{written.bytes},
        validator,
        ".dmc-rengine-scm-alpha.staging");
    if (!publication.ok()) {
        std::cerr
            << "scm-set-alpha-control: output publication failed ("
            << core::to_string(publication.status) << ")";
        if (!publication.detail.empty()) {
            std::cerr << ": " << publication.detail;
        }
        std::cerr << '\n';
        return 12;
    }

    std::cout
        << "SCM_ALPHA_EDIT_OK"
        << " object=" << object_index
        << " old=" << static_cast<unsigned>(old_alpha)
        << " new=" << static_cast<unsigned>(new_alpha)
        << " objectOffset=" << object_offset
        << " changedOffset=" << diffs[0]
        << " sourceSize=" << source.size()
        << " outputSize=" << written.bytes.size()
        << " reparse=PASS"
        << " exactByteGuard=PASS"
        << " publication=NO_REPLACE_PASS\n";
    return 0;
}

} // namespace dmc::rengine::cli
