#pragma once

#include "dmc_rengine/core/sha256.hpp"
#include "dmc_rengine/formats/scm.hpp"
#include "dmc_rengine/formats/scm_layout.hpp"
#include "dmc_rengine/formats/scm_version.hpp"

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <iterator>
#include <map>
#include <set>
#include <span>
#include <string>
#include <string_view>
#include <system_error>
#include <vector>

namespace dmc::rengine::cli {
namespace scm_occurrence_detail {

// Where an SCM sits inside a container, and what identifies it.
struct Occurrence final {
    std::string container;
    std::uint64_t offset{};
    std::uint64_t size{};
    std::string sha256;
    float version{};
    std::uint32_t resource_code{};
    std::uint16_t family_class{};
    std::uint16_t model_set{};
    std::uint16_t sub_index{};
    std::uint8_t lighting_reference_node{};
    std::uint8_t scene_node_count{};
    std::size_t objects{};
    std::size_t meshes{};
    std::uint64_t vertices{};

    // The preservation domains, measured rather than assumed. A legacy
    // revision reviving any of these would be the single most important thing
    // a scan of new containers could find, so each is counted per occurrence
    // instead of being folded into one boolean.
    bool header_preservation_nonzero{false};
    bool scene_preservation_nonzero{false};
    std::size_t object_preservation_nonzero{};
    std::size_t mesh_preservation_nonzero{};
    std::size_t transform_preservation_nonzero{};
    std::size_t gs_clamp_nonzero_meshes{};
    std::size_t serialized_generated_count_nonzero{};
    std::uint8_t topology_flag_union{};
};

[[nodiscard]] inline std::vector<std::byte> read_file(
    const std::filesystem::path& path) {
    std::ifstream input(path, std::ios::binary);
    if (!input) return {};
    const std::string raw(
        (std::istreambuf_iterator<char>(input)),
        std::istreambuf_iterator<char>());
    std::vector<std::byte> bytes(raw.size());
    for (std::size_t index = 0U; index < raw.size(); ++index) {
        bytes[index] =
            static_cast<std::byte>(static_cast<unsigned char>(raw[index]));
    }
    return bytes;
}

[[nodiscard]] inline bool any_nonzero(std::span<const std::byte> bytes) {
    return std::any_of(bytes.begin(), bytes.end(), [](std::byte value) {
        return value != std::byte{0};
    });
}

/**
 * Every structurally valid SCM inside one container, at any offset.
 *
 * Matching `SCM ` is not the test and never was: the magic appears inside
 * unrelated payloads and a text table can carry it by accident. What decides is
 * the reader — but the reader refuses a span that does not end exactly where
 * the canonical layout ends, which is a condition no payload embedded in a
 * container can meet when handed the rest of the file.
 *
 * So the scan is two passes. The first learns the shape and computes the
 * serialized extent from the canonical layout authority; the second parses
 * exactly that extent and keeps the occurrence only if the reader accepts it.
 * The extent is also what makes the digest meaningful — hashing to end-of-file
 * would give every occurrence in a container a different hash and make the
 * deduplication below report nothing.
 */
[[nodiscard]] inline std::vector<Occurrence> scan_container(
    const std::filesystem::path& path) {
    namespace scm = formats::scm;

    const auto bytes = read_file(path);
    std::vector<Occurrence> found;
    if (bytes.size() < scm::header_size) return found;

    for (std::size_t at = 0U; at + scm::header_size <= bytes.size(); at += 16U) {
        if (!(bytes[at] == std::byte{'S'} && bytes[at + 1U] == std::byte{'C'} &&
              bytes[at + 2U] == std::byte{'M'} &&
              bytes[at + 3U] == std::byte{' '})) {
            continue;
        }
        const auto tail =
            std::span<const std::byte>{bytes.data() + at, bytes.size() - at};

        const auto probe = scm::Parser::parse(tail);
        if (probe.document.objects.empty()) continue;

        std::vector<scm::ObjectShape> shapes;
        shapes.reserve(probe.document.objects.size());
        for (const auto& object : probe.document.objects) {
            scm::ObjectShape shape;
            shape.mesh_vertex_counts.reserve(object.meshes.size());
            for (const auto& mesh : object.meshes) {
                shape.mesh_vertex_counts.push_back(mesh.vertex_count);
            }
            shapes.push_back(std::move(shape));
        }
        const auto layout = scm::build_serialized_layout(
            std::span<const scm::ObjectShape>{shapes},
            probe.document.header.scene_node_count);
        if (layout.file_size == 0U || layout.file_size > tail.size()) continue;

        const auto exact = std::span<const std::byte>{
            bytes.data() + at, static_cast<std::size_t>(layout.file_size)};
        const auto parsed = scm::Parser::parse(exact);
        if (!parsed.ok()) continue;

        Occurrence occurrence;
        occurrence.container = path.filename().string();
        occurrence.offset = at;
        occurrence.size = layout.file_size;
        occurrence.sha256 = core::Sha256::compute(exact).hex();

        const auto& header = parsed.document.header;
        occurrence.version = header.version;
        occurrence.resource_code = header.resource_code.raw;
        occurrence.family_class = header.resource_code.family_class;
        occurrence.model_set = header.resource_code.model_set;
        occurrence.sub_index = header.resource_code.sub_index;
        occurrence.lighting_reference_node = header.reserved13;
        occurrence.scene_node_count = header.scene_node_count;
        occurrence.objects = parsed.document.objects.size();
        occurrence.header_preservation_nonzero =
            header.reserved08 != 0U || header.reserved18 != 0U ||
            header.reserved28 != 0U || header.reserved30 != 0U ||
            header.reserved38 != 0U;
        occurrence.scene_preservation_nonzero =
            any_nonzero(parsed.document.scene_nodes.reserved10_1f);

        for (const auto& object : parsed.document.objects) {
            occurrence.meshes += object.meshes.size();
            if (object.reserved04 != 0U ||
                any_nonzero(object.reserved14_2f)) {
                ++occurrence.object_preservation_nonzero;
            }
            for (const auto& mesh : object.meshes) {
                occurrence.vertices += mesh.vertex_count;
                if (mesh.reserved0c != 0U || mesh.reserved30 != 0U ||
                    mesh.reserved4c != 0U) {
                    ++occurrence.mesh_preservation_nonzero;
                }
                if (mesh.generated_index_count != 0U) {
                    ++occurrence.serialized_generated_count_nonzero;
                }
                const auto& clamp = mesh.gs_clamp_region_repeat;
                if (clamp.min_u != 0U || clamp.max_u != 0U ||
                    clamp.min_v != 0U || clamp.max_v != 0U) {
                    ++occurrence.gs_clamp_nonzero_meshes;
                }
                occurrence.topology_flag_union = static_cast<std::uint8_t>(
                    occurrence.topology_flag_union |
                    mesh.observed_topology_flag_mask);
            }
        }
        for (const auto& transform : parsed.document.scene_nodes.transform_by_node_index) {
            if (transform.reserved1c != 0.0F) {
                ++occurrence.transform_preservation_nonzero;
            }
        }

        found.push_back(std::move(occurrence));
    }
    return found;
}

[[nodiscard]] inline std::string json_escape(std::string_view value) {
    std::string out;
    for (const char character : value) {
        if (character == '"' || character == '\\') {
            out.push_back('\\');
            out.push_back(character);
        } else {
            out.push_back(character);
        }
    }
    return out;
}

[[nodiscard]] inline std::string version_text(float version) {
    char buffer[16];
    std::snprintf(buffer, sizeof(buffer), "%.2f", static_cast<double>(version));
    return std::string{buffer};
}

} // namespace scm_occurrence_detail

/**
 * `census-scm-occurrences <container>... [--json <report>]`
 *
 * Reports every structurally valid SCM inside the given containers, and — the
 * part a per-file corpus sweep cannot answer — how many of them are the same
 * payload. A stage set repeats scene models across its containers, so an
 * occurrence count and a payload count are different numbers, and a statement
 * like "16 files at version 0.90" means something quite different depending on
 * which one it is. The digest over the computed serialized extent is what
 * separates them.
 */
inline int try_run_scm_occurrence_census_command(int argc, char** argv) {
    using namespace scm_occurrence_detail;

    if (argc <= 1 ||
        std::string_view{argv[1]} != "census-scm-occurrences") {
        return -1;
    }
    if (argc < 3) {
        std::cerr << "usage: census-scm-occurrences <container>... "
                     "[--json <report.json>]\n";
        return 1;
    }

    std::vector<std::filesystem::path> containers;
    std::optional<std::filesystem::path> json_path;
    for (int index = 2; index < argc; ++index) {
        const std::string_view argument{argv[index]};
        if (argument == "--json") {
            if (index + 1 >= argc) {
                std::cerr << "census-scm-occurrences: --json needs a path\n";
                return 1;
            }
            json_path = std::filesystem::path{argv[index + 1]};
            ++index;
            continue;
        }
        containers.emplace_back(argument);
    }
    if (containers.empty()) {
        std::cerr << "census-scm-occurrences: no containers given\n";
        return 1;
    }

    std::vector<Occurrence> all;
    for (const auto& container : containers) {
        std::error_code error;
        if (!std::filesystem::is_regular_file(container, error) || error) {
            std::cerr << "census-scm-occurrences: not a readable file: "
                      << container.string() << '\n';
            return 2;
        }
        auto found = scan_container(container);
        std::cout << container.filename().string() << ": " << found.size()
                  << " structurally valid SCM occurrence(s)\n";
        for (const auto& occurrence : found) {
            std::cout << "  +0x" << std::hex << occurrence.offset << std::dec
                      << "  " << occurrence.size << " bytes"
                      << "  v" << version_text(occurrence.version)
                      << "  code=" << occurrence.resource_code
                      << " (class " << occurrence.family_class
                      << "/set " << occurrence.model_set
                      << "/sub " << occurrence.sub_index << ")"
                      << "  light-node=" << unsigned{occurrence.lighting_reference_node}
                      << "/" << unsigned{occurrence.scene_node_count}
                      << "  " << occurrence.sha256.substr(0U, 16U) << '\n';
        }
        all.insert(all.end(), found.begin(), found.end());
    }

    // Deduplication. `unique` counts distinct payloads; `repeated` counts the
    // occurrences beyond the first of each.
    std::map<std::string, std::size_t> by_digest;
    for (const auto& occurrence : all) ++by_digest[occurrence.sha256];

    std::map<std::string, std::size_t> version_occurrences;
    std::map<std::string, std::set<std::string>> version_payloads;
    std::map<std::uint16_t, std::size_t> family_occurrences;
    std::uint64_t vertices = 0U;
    std::uint8_t topology_union = 0U;
    std::size_t preservation_revivals = 0U;
    std::size_t lighting_nonzero = 0U;
    std::size_t lighting_out_of_range = 0U;
    for (const auto& occurrence : all) {
        const auto version = version_text(occurrence.version);
        ++version_occurrences[version];
        version_payloads[version].insert(occurrence.sha256);
        ++family_occurrences[occurrence.family_class];
        vertices += occurrence.vertices;
        topology_union = static_cast<std::uint8_t>(
            topology_union | occurrence.topology_flag_union);
        if (occurrence.header_preservation_nonzero ||
            occurrence.scene_preservation_nonzero ||
            occurrence.object_preservation_nonzero != 0U ||
            occurrence.mesh_preservation_nonzero != 0U ||
            occurrence.transform_preservation_nonzero != 0U ||
            occurrence.gs_clamp_nonzero_meshes != 0U ||
            occurrence.serialized_generated_count_nonzero != 0U) {
            ++preservation_revivals;
        }
        if (occurrence.lighting_reference_node != 0U) ++lighting_nonzero;
        if (occurrence.scene_node_count != 0U &&
            occurrence.lighting_reference_node >= occurrence.scene_node_count) {
            ++lighting_out_of_range;
        }
    }

    std::size_t repeated = 0U;
    for (const auto& [digest, count] : by_digest) {
        static_cast<void>(digest);
        if (count > 1U) repeated += count - 1U;
    }

    std::cout << "\ntotal: " << all.size() << " occurrence(s), "
              << by_digest.size() << " unique payload(s), " << repeated
              << " repeat(s)\n";
    std::cout << "versions (occurrences / unique payloads):\n";
    for (const auto& [version, count] : version_occurrences) {
        std::cout << "  " << version << "  x" << count << " / "
                  << version_payloads[version].size()
                  << (formats::scm::is_corpus_confirmed_structural_version(
                          std::stof(version))
                          ? ""
                          : "   <- OUTSIDE THE RECORDED VERSION DOMAIN")
                  << '\n';
    }
    std::cout << "resource-code family classes:\n";
    for (const auto& [family, count] : family_occurrences) {
        std::cout << "  class " << family << "  x" << count << '\n';
    }
    std::cout << "vertices: " << vertices
              << "\ntopology flag union: 0x" << std::hex
              << unsigned{topology_union} << std::dec
              << "\npreservation domains revived: " << preservation_revivals
              << " occurrence(s)"
              << "\nlighting reference node non-zero: " << lighting_nonzero
              << " occurrence(s), out of range: " << lighting_out_of_range
              << '\n';

    if (json_path.has_value()) {
        std::ofstream out(*json_path);
        if (!out) {
            std::cerr << "census-scm-occurrences: cannot write "
                      << json_path->string() << '\n';
            return 4;
        }
        out << "{\n  \"occurrences\": " << all.size()
            << ",\n  \"uniquePayloads\": " << by_digest.size()
            << ",\n  \"repeats\": " << repeated
            << ",\n  \"vertices\": " << vertices
            << ",\n  \"topologyFlagUnion\": " << unsigned{topology_union}
            << ",\n  \"preservationRevivals\": " << preservation_revivals
            << ",\n  \"lightingReferenceNonZero\": " << lighting_nonzero
            << ",\n  \"lightingReferenceOutOfRange\": " << lighting_out_of_range
            << ",\n  \"items\": [\n";
        for (std::size_t index = 0U; index < all.size(); ++index) {
            const auto& occurrence = all[index];
            out << "    {\"container\": \""
                << json_escape(occurrence.container)
                << "\", \"offset\": " << occurrence.offset
                << ", \"size\": " << occurrence.size
                << ", \"sha256\": \"" << occurrence.sha256
                << "\", \"version\": \"" << version_text(occurrence.version)
                << "\", \"resourceCode\": " << occurrence.resource_code
                << ", \"familyClass\": " << occurrence.family_class
                << ", \"modelSet\": " << occurrence.model_set
                << ", \"subIndex\": " << occurrence.sub_index
                << ", \"lightingReferenceNode\": "
                << unsigned{occurrence.lighting_reference_node}
                << ", \"sceneNodeCount\": "
                << unsigned{occurrence.scene_node_count}
                << ", \"objects\": " << occurrence.objects
                << ", \"meshes\": " << occurrence.meshes
                << ", \"vertices\": " << occurrence.vertices << "}"
                << (index + 1U == all.size() ? "\n" : ",\n");
        }
        out << "  ]\n}\n";
    }
    return 0;
}

} // namespace dmc::rengine::cli
