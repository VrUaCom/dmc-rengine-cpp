#pragma once

#include "dmc_rengine/formats/scm.hpp"
#include "dmc_rengine/formats/scm_runtime_flags.hpp"
#include "dmc_rengine/formats/scm_topology.hpp"
#include "dmc_rengine/formats/scm_writer.hpp"
#include "dmc_rengine/gdspaces/local_directory_source.hpp"
#include "dmc_rengine/gdspaces/source_registry.hpp"

#include <algorithm>
#include <cctype>
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <memory>
#include <optional>
#include <sstream>
#include <span>
#include <string>
#include <string_view>
#include <vector>

namespace dmc::rengine::cli {
namespace scm_corpus_detail {

using formats::ParseDiagnostic;
using formats::scm::Parser;
using formats::scm::WriteMode;
using formats::scm::Writer;
using gdspaces::LocalDirectorySource;
using gdspaces::ResourceRef;
using gdspaces::SourceRegistry;

struct FileResult final {
    std::string path;
    std::uint64_t size{};
    std::size_t objects{};
    std::size_t meshes{};
    std::size_t nodes{};
    std::uint64_t vertices{};

    // Reverse-reader census. These fields are intentionally raw/operational:
    // they identify new corpus deltas without promoting semantics that are not
    // evidenced by the canonical executable.
    std::uint32_t source_flag_union{};
    std::uint32_t preserved_undecoded_source_flag_union{};
    std::uint8_t topology_flag_union{};
    std::uint8_t topology_unconfirmed_flag_union{};
    std::uint8_t texture_slot_count{};
    std::uint32_t resource_code_raw{};
    std::uint8_t resource_family_class{};
    std::uint16_t resource_model_set{};
    std::uint8_t resource_sub_index{};
    std::size_t gs_clamp_nonzero_meshes{};
    std::size_t object_preservation_nonzero_records{};
    std::size_t mesh_preservation_nonzero_records{};
    std::size_t transform_preservation_nonzero_records{};
    std::size_t serialized_generated_index_count_nonzero_meshes{};
    bool header_preservation_nonzero{false};
    bool scene_preservation_nonzero{false};

    bool read_ok{false};
    bool parse_ok{false};
    bool preserve_write_ok{false};
    bool preserve_bit_identical{false};
    bool canonical_write_ok{false};
    bool canonical_reparse_ok{false};
    bool canonical_bit_identical{false};
    std::optional<std::size_t> first_mismatch_offset;
    std::size_t diagnostic_count{};
};

[[nodiscard]] inline std::string lower_copy(std::string value) {
    std::transform(
        value.begin(), value.end(), value.begin(),
        [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    return value;
}

[[nodiscard]] inline bool is_scm_path(std::string_view path) {
    return lower_copy(std::filesystem::path{path}.extension().string()) == ".scm";
}

[[nodiscard]] inline std::optional<std::size_t> first_mismatch(
    std::span<const std::byte> source,
    std::span<const std::byte> rebuilt) noexcept {
    const auto common = std::min(source.size(), rebuilt.size());
    for (std::size_t index = 0U; index < common; ++index) {
        if (source[index] != rebuilt[index]) return index;
    }
    if (source.size() != rebuilt.size()) return common;
    return std::nullopt;
}

[[nodiscard]] inline std::string json_escape(std::string_view value) {
    std::ostringstream out;
    for (const unsigned char c : value) {
        switch (c) {
        case '"': out << "\\\""; break;
        case '\\': out << "\\\\"; break;
        case '\b': out << "\\b"; break;
        case '\f': out << "\\f"; break;
        case '\n': out << "\\n"; break;
        case '\r': out << "\\r"; break;
        case '\t': out << "\\t"; break;
        default:
            if (c < 0x20U) {
                out << "\\u" << std::hex << std::setw(4) << std::setfill('0')
                    << static_cast<unsigned>(c) << std::dec << std::setfill(' ');
            } else {
                out << static_cast<char>(c);
            }
            break;
        }
    }
    return out.str();
}

[[nodiscard]] inline std::string hex_text(
    std::uint64_t value,
    std::size_t width) {
    std::ostringstream out;
    out << "0x" << std::uppercase << std::hex << std::setfill('0')
        << std::setw(static_cast<int>(width)) << value;
    return out.str();
}

template <std::size_t N>
[[nodiscard]] inline bool any_nonzero(
    const std::array<std::byte, N>& bytes) noexcept {
    return std::any_of(
        bytes.begin(), bytes.end(),
        [](std::byte value) { return value != std::byte{0}; });
}

[[nodiscard]] inline FileResult verify_one(
    const SourceRegistry& registry,
    const ResourceRef& resource) {
    FileResult result;
    result.path = resource.id.logical_path;

    const auto payload = registry.read(resource.id);
    if (!payload.has_value() || !payload->readable()) return result;

    result.read_ok = true;
    result.size = static_cast<std::uint64_t>(payload->bytes.size());
    const auto source = std::span<const std::byte>{payload->bytes};

    const auto parsed = Parser::parse(source);
    result.parse_ok = parsed.ok();
    result.diagnostic_count += parsed.diagnostics.size();
    if (!parsed.ok()) return result;

    const auto& header = parsed.document.header;
    result.objects = parsed.document.objects.size();
    result.nodes = parsed.document.scene_nodes.transform_by_node_index.size();
    result.texture_slot_count = header.texture_slot_count;
    result.resource_code_raw = header.resource_code.raw;
    result.resource_family_class = header.resource_code.family_class;
    result.resource_model_set = header.resource_code.model_set;
    result.resource_sub_index = header.resource_code.sub_index;
    result.header_preservation_nonzero =
        header.reserved08 != 0U || header.reserved13 != 0U ||
        header.reserved18 != 0U || header.reserved28 != 0U ||
        header.reserved30 != 0U || header.reserved38 != 0U;
    result.scene_preservation_nonzero =
        any_nonzero(parsed.document.scene_nodes.reserved10_1f);

    for (const auto& object : parsed.document.objects) {
        result.source_flag_union |= object.flags;
        result.preserved_undecoded_source_flag_union |=
            formats::scm::runtime::project(object.flags)
                .preserved_undecoded_source_bits;

        if (object.reserved04 != 0U || any_nonzero(object.reserved14_2f)) {
            ++result.object_preservation_nonzero_records;
        }

        result.meshes += object.meshes.size();
        for (const auto& mesh : object.meshes) {
            result.vertices += mesh.positions.size();
            result.topology_flag_union |= mesh.observed_topology_flag_mask;
            result.topology_unconfirmed_flag_union |= static_cast<std::uint8_t>(
                mesh.observed_topology_flag_mask &
                static_cast<std::uint8_t>(~formats::scm::triangle_break_bit));

            const auto& clamp = mesh.gs_clamp_region_repeat;
            if (clamp.min_u != 0U || clamp.max_u != 0U ||
                clamp.min_v != 0U || clamp.max_v != 0U) {
                ++result.gs_clamp_nonzero_meshes;
            }
            if (mesh.reserved0c != 0U || mesh.reserved30 != 0U ||
                mesh.reserved4c != 0U) {
                ++result.mesh_preservation_nonzero_records;
            }
            if (mesh.generated_index_count != 0U) {
                ++result.serialized_generated_index_count_nonzero_meshes;
            }
        }
    }

    for (const auto& transform :
         parsed.document.scene_nodes.transform_by_node_index) {
        if (transform.reserved1c != 0.0F) {
            ++result.transform_preservation_nonzero_records;
        }
    }

    const auto preserved = Writer::write(parsed.document, WriteMode::preserve_layout);
    result.preserve_write_ok = preserved.ok();
    result.preserve_bit_identical = preserved.bit_identical_to_source;
    result.diagnostic_count += preserved.diagnostics.size();

    const auto rebuilt = Writer::write(parsed.document, WriteMode::canonical_rebuild);
    result.canonical_write_ok = rebuilt.wrote;
    result.canonical_reparse_ok = rebuilt.reparse_ok;
    result.canonical_bit_identical = rebuilt.bit_identical_to_source;
    result.diagnostic_count += rebuilt.diagnostics.size();
    if (rebuilt.wrote) {
        result.first_mismatch_offset = first_mismatch(
            source, std::span<const std::byte>{rebuilt.bytes});
    }

    return result;
}

[[nodiscard]] inline std::string build_json(
    const std::filesystem::path& root,
    const std::vector<FileResult>& results) {
    std::size_t parse_ok = 0U;
    std::size_t preserve_identical = 0U;
    std::size_t canonical_ok = 0U;
    std::size_t canonical_identical = 0U;
    std::uint32_t source_flag_union = 0U;
    std::uint32_t preserved_undecoded_source_flag_union = 0U;
    std::uint8_t topology_flag_union = 0U;
    std::uint8_t topology_unconfirmed_flag_union = 0U;
    std::size_t gs_clamp_nonzero_meshes = 0U;
    std::size_t files_with_nonzero_preservation = 0U;

    for (const auto& result : results) {
        if (result.parse_ok) ++parse_ok;
        if (result.preserve_bit_identical) ++preserve_identical;
        if (result.canonical_write_ok && result.canonical_reparse_ok) ++canonical_ok;
        if (result.canonical_bit_identical) ++canonical_identical;
        source_flag_union |= result.source_flag_union;
        preserved_undecoded_source_flag_union |=
            result.preserved_undecoded_source_flag_union;
        topology_flag_union |= result.topology_flag_union;
        topology_unconfirmed_flag_union |= result.topology_unconfirmed_flag_union;
        gs_clamp_nonzero_meshes += result.gs_clamp_nonzero_meshes;
        if (result.header_preservation_nonzero ||
            result.scene_preservation_nonzero ||
            result.object_preservation_nonzero_records != 0U ||
            result.mesh_preservation_nonzero_records != 0U ||
            result.transform_preservation_nonzero_records != 0U ||
            result.serialized_generated_index_count_nonzero_meshes != 0U) {
            ++files_with_nonzero_preservation;
        }
    }

    const auto percent = [&](std::size_t value) {
        return results.empty()
            ? 0.0
            : 100.0 * static_cast<double>(value) /
                  static_cast<double>(results.size());
    };

    std::ostringstream out;
    out << "{\n"
        << "  \"schema\": \"dmc-rengine.scm-corpus-report.v1\",\n"
        << "  \"root\": \"" << json_escape(root.generic_string()) << "\",\n"
        << "  \"summary\": {\n"
        << "    \"files\": " << results.size() << ",\n"
        << "    \"parseOk\": " << parse_ok << ",\n"
        << "    \"preserveBitIdentical\": " << preserve_identical << ",\n"
        << "    \"canonicalWriteAndReparseOk\": " << canonical_ok << ",\n"
        << "    \"canonicalBitIdentical\": " << canonical_identical << ",\n"
        << "    \"canonicalBitIdenticalPercent\": "
        << std::fixed << std::setprecision(3) << percent(canonical_identical) << ",\n"
        << "    \"sourceFlagUnion\": \"" << hex_text(source_flag_union, 8U) << "\",\n"
        << "    \"preservedUndecodedSourceFlagUnion\": \""
        << hex_text(preserved_undecoded_source_flag_union, 8U) << "\",\n"
        << "    \"topologyFlagUnion\": \""
        << hex_text(topology_flag_union, 2U) << "\",\n"
        << "    \"topologyUnconfirmedFlagUnion\": \""
        << hex_text(topology_unconfirmed_flag_union, 2U) << "\",\n"
        << "    \"gsClampNonzeroMeshes\": " << gs_clamp_nonzero_meshes << ",\n"
        << "    \"filesWithNonzeroPreservationDomains\": "
        << files_with_nonzero_preservation << "\n"
        << "  },\n"
        << "  \"files\": [\n";

    for (std::size_t index = 0U; index < results.size(); ++index) {
        const auto& result = results[index];
        out << "    {\n"
            << "      \"path\": \"" << json_escape(result.path) << "\",\n"
            << "      \"size\": " << result.size << ",\n"
            << "      \"objects\": " << result.objects << ",\n"
            << "      \"meshes\": " << result.meshes << ",\n"
            << "      \"nodes\": " << result.nodes << ",\n"
            << "      \"vertices\": " << result.vertices << ",\n"
            << "      \"textureSlotCount\": "
            << static_cast<unsigned>(result.texture_slot_count) << ",\n"
            << "      \"resourceCodeRaw\": " << result.resource_code_raw << ",\n"
            << "      \"resourceFamilyClass\": "
            << static_cast<unsigned>(result.resource_family_class) << ",\n"
            << "      \"resourceModelSet\": " << result.resource_model_set << ",\n"
            << "      \"resourceSubIndex\": "
            << static_cast<unsigned>(result.resource_sub_index) << ",\n"
            << "      \"sourceFlagUnion\": \""
            << hex_text(result.source_flag_union, 8U) << "\",\n"
            << "      \"preservedUndecodedSourceFlagUnion\": \""
            << hex_text(result.preserved_undecoded_source_flag_union, 8U) << "\",\n"
            << "      \"topologyFlagUnion\": \""
            << hex_text(result.topology_flag_union, 2U) << "\",\n"
            << "      \"topologyUnconfirmedFlagUnion\": \""
            << hex_text(result.topology_unconfirmed_flag_union, 2U) << "\",\n"
            << "      \"gsClampNonzeroMeshes\": "
            << result.gs_clamp_nonzero_meshes << ",\n"
            << "      \"headerPreservationNonzero\": "
            << (result.header_preservation_nonzero ? "true" : "false") << ",\n"
            << "      \"scenePreservationNonzero\": "
            << (result.scene_preservation_nonzero ? "true" : "false") << ",\n"
            << "      \"objectPreservationNonzeroRecords\": "
            << result.object_preservation_nonzero_records << ",\n"
            << "      \"meshPreservationNonzeroRecords\": "
            << result.mesh_preservation_nonzero_records << ",\n"
            << "      \"transformPreservationNonzeroRecords\": "
            << result.transform_preservation_nonzero_records << ",\n"
            << "      \"serializedGeneratedIndexCountNonzeroMeshes\": "
            << result.serialized_generated_index_count_nonzero_meshes << ",\n"
            << "      \"readOk\": " << (result.read_ok ? "true" : "false") << ",\n"
            << "      \"parseOk\": " << (result.parse_ok ? "true" : "false") << ",\n"
            << "      \"preserveWriteOk\": " << (result.preserve_write_ok ? "true" : "false") << ",\n"
            << "      \"preserveBitIdentical\": " << (result.preserve_bit_identical ? "true" : "false") << ",\n"
            << "      \"canonicalWriteOk\": " << (result.canonical_write_ok ? "true" : "false") << ",\n"
            << "      \"canonicalReparseOk\": " << (result.canonical_reparse_ok ? "true" : "false") << ",\n"
            << "      \"canonicalBitIdentical\": " << (result.canonical_bit_identical ? "true" : "false") << ",\n"
            << "      \"firstMismatchOffset\": ";
        if (result.first_mismatch_offset.has_value()) {
            out << *result.first_mismatch_offset;
        } else {
            out << "null";
        }
        out << ",\n"
            << "      \"diagnosticCount\": " << result.diagnostic_count << "\n"
            << "    }";
        if (index + 1U != results.size()) out << ',';
        out << '\n';
    }
    out << "  ]\n}\n";
    return out.str();
}

} // namespace scm_corpus_detail

inline void print_scm_corpus_help() {
    std::cout
        << "  verify-scm-corpus <directory> [--json <report.json>]\n"
        << "                             Parse/write/reparse every .scm through GDSpaces\n"
        << "                             and emit reverse-reader field/unknown-domain census\n";
}

inline int try_run_scm_corpus_command(int argc, char** argv) {
    if (argc <= 1 || std::string_view{argv[1]} != "verify-scm-corpus") {
        return -1;
    }
    if (argc < 3) {
        std::cerr << "verify-scm-corpus: missing directory\n";
        return 1;
    }

    const std::filesystem::path root{argv[2]};
    std::optional<std::filesystem::path> json_path;
    if (argc == 5 && std::string_view{argv[3]} == "--json") {
        json_path = std::filesystem::path{argv[4]};
    } else if (argc != 3) {
        std::cerr
            << "usage: verify-scm-corpus <directory> [--json <report.json>]\n";
        return 1;
    }

    std::error_code error;
    if (!std::filesystem::is_directory(root, error) || error) {
        std::cerr << "verify-scm-corpus: not a readable directory: "
                  << root.string() << '\n';
        return 2;
    }

    scm_corpus_detail::SourceRegistry registry;
    if (!registry.mount(std::make_unique<scm_corpus_detail::LocalDirectorySource>(
            "scm-corpus", root, true))) {
        std::cerr << "verify-scm-corpus: failed to mount corpus directory\n";
        return 3;
    }

    auto resources = registry.enumerate_all();
    resources.erase(
        std::remove_if(
            resources.begin(), resources.end(),
            [](const scm_corpus_detail::ResourceRef& resource) {
                return !scm_corpus_detail::is_scm_path(resource.id.logical_path);
            }),
        resources.end());
    std::sort(
        resources.begin(), resources.end(),
        [](const scm_corpus_detail::ResourceRef& lhs,
           const scm_corpus_detail::ResourceRef& rhs) {
            return lhs.id.logical_path < rhs.id.logical_path;
        });

    std::vector<scm_corpus_detail::FileResult> results;
    results.reserve(resources.size());
    for (const auto& resource : resources) {
        const auto result = scm_corpus_detail::verify_one(registry, resource);
        std::cout
            << (result.canonical_bit_identical
                    ? "BIT_IDENTICAL"
                    : result.canonical_write_ok && result.canonical_reparse_ok
                        ? "SEMANTIC_OR_DIFF"
                        : "FAIL")
            << "  " << result.path
            << " size=" << result.size
            << " objects=" << result.objects
            << " meshes=" << result.meshes
            << " nodes=" << result.nodes
            << " vertices=" << result.vertices
            << " srcFlags=" << scm_corpus_detail::hex_text(result.source_flag_union, 8U)
            << " topo=" << scm_corpus_detail::hex_text(result.topology_flag_union, 2U);
        if (result.preserved_undecoded_source_flag_union != 0U) {
            std::cout << " undecodedFlags="
                      << scm_corpus_detail::hex_text(
                             result.preserved_undecoded_source_flag_union, 8U);
        }
        if (result.gs_clamp_nonzero_meshes != 0U) {
            std::cout << " gsClampNonzero=" << result.gs_clamp_nonzero_meshes;
        }
        if (result.first_mismatch_offset.has_value()) {
            std::cout << " firstMismatch=0x" << std::hex
                      << *result.first_mismatch_offset << std::dec;
        }
        std::cout << " diagnostics=" << result.diagnostic_count << '\n';
        results.push_back(result);
    }

    const auto json = scm_corpus_detail::build_json(root, results);
    if (json_path.has_value()) {
        std::ofstream stream(*json_path, std::ios::binary | std::ios::trunc);
        if (!stream) {
            std::cerr << "verify-scm-corpus: cannot write report: "
                      << json_path->string() << '\n';
            return 4;
        }
        stream << json;
        if (!stream) {
            std::cerr << "verify-scm-corpus: failed while writing report\n";
            return 4;
        }
    } else {
        std::cout << json;
    }

    const auto all_good = std::all_of(
        results.begin(), results.end(),
        [](const scm_corpus_detail::FileResult& result) {
            return result.parse_ok && result.preserve_bit_identical &&
                   result.canonical_write_ok && result.canonical_reparse_ok &&
                   result.canonical_bit_identical;
        });
    return !results.empty() && all_good ? 0 : 5;
}

} // namespace dmc::rengine::cli
