#pragma once

#include "dmc_rengine/binary/manifest.hpp"
#include "dmc_rengine/formats/scm.hpp"
#include "dmc_rengine/formats/scm_binary.hpp"
#include "dmc_rengine/gdspaces/local_directory_source.hpp"
#include "dmc_rengine/gdspaces/source_registry.hpp"

#include <cstdint>
#include <filesystem>
#include <iostream>
#include <limits>
#include <memory>
#include <optional>
#include <span>
#include <string>
#include <string_view>

namespace dmc::rengine::cli {
namespace scm_reader_detail {

struct Options final {
    bool json{false};
    std::optional<std::uint64_t> offset;
};

[[nodiscard]] inline std::optional<std::uint64_t> parse_offset(
    std::string_view text) {
    try {
        std::size_t consumed = 0U;
        const auto value = std::stoull(std::string{text}, &consumed, 0);
        if (consumed != text.size()) return std::nullopt;
        return static_cast<std::uint64_t>(value);
    } catch (...) {
        return std::nullopt;
    }
}

[[nodiscard]] inline std::optional<gdspaces::ResourcePayload> load(
    const std::filesystem::path& input_path) {
    std::error_code error;
    const auto absolute = std::filesystem::absolute(input_path, error);
    if (error || !std::filesystem::is_regular_file(absolute, error) || error) {
        std::cerr << "inspect-scm: not a readable file: "
                  << input_path.string() << '\n';
        return std::nullopt;
    }

    const auto raw_size = std::filesystem::file_size(absolute, error);
    if (error || raw_size > std::numeric_limits<std::uint64_t>::max()) {
        std::cerr << "inspect-scm: unsupported file size\n";
        return std::nullopt;
    }

    gdspaces::SourceRegistry registry;
    constexpr std::string_view source_id = "scm-reader-inspect";
    if (!registry.mount(std::make_unique<gdspaces::LocalDirectorySource>(
            std::string(source_id), absolute.parent_path(), false))) {
        std::cerr << "inspect-scm: failed to mount parent directory\n";
        return std::nullopt;
    }

    const gdspaces::ResourceId id{
        .source_id = std::string(source_id),
        .logical_path = absolute.filename().generic_string(),
        .container_chain = {},
        .offset = 0U,
        .size = static_cast<std::uint64_t>(raw_size),
    };
    auto payload = registry.read(id);
    if (!payload.has_value() || !payload->readable()) {
        std::cerr << "inspect-scm: GDSpaces could not read the resource\n";
        return std::nullopt;
    }
    return payload;
}

inline void print_selection(
    const binary::Document& document,
    std::uint64_t offset) {
    const auto selection = document.selection_at(offset);
    std::cout << "SCM byte selection @ 0x" << std::hex << offset << std::dec
              << '\n';

    std::cout << "Regions: " << selection.regions.size() << '\n';
    for (const auto* region : selection.regions) {
        std::cout << "  " << region->id << " | " << region->name
                  << " | " << binary::to_string(region->kind)
                  << " | evidence=" << region->evidence_id << '\n';
    }

    std::cout << "Fields: " << selection.fields.size() << '\n';
    for (const auto* field : selection.fields) {
        std::cout << "  " << field->id << " | " << field->name
                  << " | " << binary::to_string(field->kind)
                  << " | type=" << field->type_name;
        if (!field->display_value.empty()) {
            std::cout << " | value=" << field->display_value;
        }
        std::cout << " | evidence=" << field->evidence_id << '\n';
    }

    std::cout << "Owners: " << selection.owners.size() << '\n';
    for (const auto* owner : selection.owners) {
        std::cout << "  " << owner->owner_id << " | "
                  << owner->rationale << '\n';
    }

    std::cout << "Annotations: " << selection.annotations.size() << '\n';
    for (const auto* annotation : selection.annotations) {
        std::cout << "  " << annotation->id << " | " << annotation->text
                  << " | evidence=" << annotation->evidence_id;
        if (!annotation->tags.empty()) {
            std::cout << " | tags=";
            for (std::size_t i = 0U; i < annotation->tags.size(); ++i) {
                if (i != 0U) std::cout << ',';
                std::cout << annotation->tags[i];
            }
        }
        std::cout << '\n';
    }
}

inline void print_summary(
    const formats::scm::ParseResult& parsed,
    const binary::Document& document) {
    const auto& h = parsed.document.header;
    std::size_t mesh_count = 0U;
    std::uint64_t vertex_count = 0U;
    for (const auto& object : parsed.document.objects) {
        mesh_count += object.meshes.size();
        for (const auto& mesh : object.meshes) {
            vertex_count += mesh.vertex_count;
        }
    }

    std::cout
        << "SCM deep reader\n"
        << "version=" << h.version << '\n'
        << "objects=" << parsed.document.objects.size() << '\n'
        << "meshes=" << mesh_count << '\n'
        << "nodes=" << parsed.document.scene_nodes.transform_by_node_index.size()
        << '\n'
        << "vertices=" << vertex_count << '\n'
        << "textureSlots=" << static_cast<unsigned>(h.texture_slot_count) << '\n'
        << "resourceCode=" << h.resource_code.raw
        << " family=" << static_cast<unsigned>(h.resource_code.family_class)
        << " modelSet=" << h.resource_code.model_set
        << " subIndex=" << static_cast<unsigned>(h.resource_code.sub_index)
        << '\n'
        << "binaryCoverage=" << document.coverage_bytes()
        << '/' << document.byte_size() << '\n'
        << "regions=" << document.regions().size()
        << " fields=" << document.fields().size()
        << " owners=" << document.ownership().size()
        << " annotations=" << document.annotations().size() << '\n'
        << "unknownRanges=" << document.unknown_ranges().size()
        << " regionConflicts=" << document.conflicts().size()
        << " ownershipConflicts=" << document.ownership_conflicts().size()
        << '\n';

    for (const auto& annotation : document.annotations()) {
        if (std::find(
                annotation.tags.begin(), annotation.tags.end(),
                "PRESERVED_UNDECODED") == annotation.tags.end()) {
            continue;
        }
        std::cout << "  [preserve] 0x" << std::hex << annotation.range.offset
                  << "+0x" << annotation.range.size << std::dec
                  << " " << annotation.text << '\n';
    }
}

} // namespace scm_reader_detail

inline void print_scm_reader_help() {
    std::cout
        << "  inspect-scm <file> [--json | --offset <n>]\n"
        << "                             Deep read-only SCM byte/evidence inspection\n"
        << "                             --json emits the full Binary Inspector manifest\n"
        << "                             --offset accepts decimal or 0x-prefixed byte offset\n";
}

inline int try_run_scm_reader_command(int argc, char** argv) {
    if (argc <= 1 || std::string_view{argv[1]} != "inspect-scm") {
        return -1;
    }
    if (argc < 3) {
        std::cerr << "inspect-scm: missing SCM file\n";
        return 1;
    }

    scm_reader_detail::Options options;
    for (int index = 3; index < argc; ++index) {
        const std::string_view option{argv[index]};
        if (option == "--json") {
            if (options.offset.has_value()) {
                std::cerr << "inspect-scm: --json and --offset are mutually exclusive\n";
                return 1;
            }
            options.json = true;
        } else if (option == "--offset") {
            if (options.json || index + 1 >= argc) {
                std::cerr << "inspect-scm: --offset requires one value and cannot be combined with --json\n";
                return 1;
            }
            const auto parsed_offset =
                scm_reader_detail::parse_offset(argv[++index]);
            if (!parsed_offset.has_value()) {
                std::cerr << "inspect-scm: invalid byte offset\n";
                return 1;
            }
            options.offset = *parsed_offset;
        } else {
            std::cerr << "inspect-scm: unknown option: " << option << '\n';
            return 1;
        }
    }

    auto payload = scm_reader_detail::load(std::filesystem::path{argv[2]});
    if (!payload.has_value()) return 2;

    const auto bytes = std::span<const std::byte>{payload->bytes};
    const auto parsed = formats::scm::Parser::parse(bytes);
    if (!parsed.recognized) {
        std::cerr << "inspect-scm: resource is not recognized as SCM\n";
        return 3;
    }
    if (!parsed.ok()) {
        std::cerr << "inspect-scm: SCM parse failed\n";
        for (const auto& diagnostic : parsed.diagnostics) {
            std::cerr << "  " << diagnostic.code << ": "
                      << diagnostic.message << " @0x" << std::hex
                      << diagnostic.offset << std::dec << '\n';
        }
        return 4;
    }

    auto document = formats::scm::build_binary_document(
        payload->resource, bytes, parsed);
    if (!document.has_value()) {
        std::cerr << "inspect-scm: failed to build SCM binary map\n";
        return 5;
    }

    if (options.json) {
        std::cout << binary::manifest_json(*document);
        return 0;
    }
    if (options.offset.has_value()) {
        if (*options.offset >= document->byte_size()) {
            std::cerr << "inspect-scm: byte offset is outside the file\n";
            return 6;
        }
        scm_reader_detail::print_selection(*document, *options.offset);
        return 0;
    }

    scm_reader_detail::print_summary(parsed, *document);
    return 0;
}

} // namespace dmc::rengine::cli
