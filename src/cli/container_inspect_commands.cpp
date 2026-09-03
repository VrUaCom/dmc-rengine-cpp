#include "container_inspect_commands.hpp"

#include "dmc_rengine/core/no_replace_publication.hpp"
#include "dmc_rengine/core/sha256.hpp"
#include "dmc_rengine/gdspaces/classifier.hpp"
#include "dmc_rengine/gdspaces/container_tree_expander.hpp"
#include "dmc_rengine/gdspaces/nbz_zip_source.hpp"
#include "dmc_rengine/formats/scm.hpp"
#include "dmc_rengine/formats/scm_layout.hpp"
#include "dmc_rengine/gdspaces/resource_payload.hpp"
#include "dmc_rengine/profiles/dmc3/container_parsers.hpp"

#include <charconv>
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <vector>

namespace dmc::rengine::cli {
namespace {

using gdspaces::ContainerTreeExpander;
using gdspaces::ContainerTreeExpansion;
using gdspaces::Diagnostic;
using gdspaces::DiagnosticSeverity;
using gdspaces::NbzZipSource;
using gdspaces::ResourceClassifier;
using gdspaces::ResourceId;
using gdspaces::ResourcePayload;
using gdspaces::ResourceRef;

// SafeProductValidation: a locally supplied container is untrusted input, so
// the reader keeps its own budget instead of trusting the file's own extent.
constexpr std::uint64_t k_max_container_bytes = 512ULL * 1024ULL * 1024ULL;

[[nodiscard]] std::optional<std::vector<std::byte>> read_file(
    const std::filesystem::path& path,
    std::string& error) {
    std::error_code code;
    const auto size = std::filesystem::file_size(path, code);
    if (code) {
        error = "unable to size the input file";
        return std::nullopt;
    }
    if (size > k_max_container_bytes) {
        error = "the input file exceeds the container inspection budget";
        return std::nullopt;
    }

    std::ifstream stream(path, std::ios::binary);
    if (!stream) {
        error = "unable to open the input file";
        return std::nullopt;
    }
    std::vector<std::byte> bytes(static_cast<std::size_t>(size));
    if (!bytes.empty()) {
        stream.read(
            reinterpret_cast<char*>(bytes.data()),
            static_cast<std::streamsize>(bytes.size()));
        if (!stream.good() &&
            stream.gcount() != static_cast<std::streamsize>(bytes.size())) {
            error = "unable to read the complete input file";
            return std::nullopt;
        }
    }
    return bytes;
}

[[nodiscard]] std::string digest_of(std::span<const std::byte> bytes) {
    return core::Sha256::compute(bytes).hex();
}

[[nodiscard]] ResourcePayload make_root(
    const std::filesystem::path& path,
    std::vector<std::byte> bytes) {
    const auto logical = path.filename().string();
    const auto classification = ResourceClassifier::classify(
        logical, std::span<const std::byte>{bytes});

    ResourcePayload payload;
    payload.resource = ResourceRef{
        .id = ResourceId{
            .source_id = "container-inspect",
            .logical_path = logical,
            .container_chain = {},
            .offset = 0U,
            .size = static_cast<std::uint64_t>(bytes.size()),
        },
        .display_name = logical,
        .format = classification.format,
        .profile = std::string(to_string(classification.profile)),
        .synthetic_name = false,
        .container = classification.container,
    };
    payload.bytes = std::move(bytes);
    return payload;
}

// A relative-slot container stores only slot offsets, so a slot's size is
// derived from the distance to the next distinct offset. Any inter-slot or
// trailing alignment padding is therefore absorbed into the preceding slot.
//
// When a child is a format that can state its own serialized extent, that
// extent is the resource's real length and the difference is padding. This is
// reported, not applied: the physical slot span is still the slot's identity,
// and narrowing it is a separate decision that reaches ResourceId, provenance
// and the writer path.
//
// Only SCM can answer today, because formats::scm::build_serialized_layout
// reconstructs the canonical layout exactly. Note that scm_validation already
// requires a SCM image to terminate at that size, so a padded slot fails SCM
// validation outright rather than merely carrying extra bytes.
[[nodiscard]] std::optional<std::uint64_t> intrinsic_extent(
    std::string_view format,
    std::span<const std::byte> bytes) {
    if (format != "scm") {
        return std::nullopt;
    }
    const auto parsed = formats::scm::Parser::parse(bytes);
    if (!parsed.recognized) {
        return std::nullopt;
    }

    std::vector<formats::scm::ObjectShape> shapes;
    shapes.reserve(parsed.document.objects.size());
    for (const auto& object : parsed.document.objects) {
        formats::scm::ObjectShape shape;
        shape.mesh_vertex_counts.reserve(object.meshes.size());
        for (const auto& mesh : object.meshes) {
            shape.mesh_vertex_counts.push_back(mesh.vertex_count);
        }
        shapes.push_back(std::move(shape));
    }

    const auto layout = formats::scm::build_serialized_layout(
        shapes, parsed.document.header.scene_node_count);
    return layout.file_size;
}

void print_diagnostics(const std::vector<Diagnostic>& diagnostics) {
    for (const auto& diagnostic : diagnostics) {
        const auto* severity =
            diagnostic.severity == DiagnosticSeverity::error ? "error"
            : diagnostic.severity == DiagnosticSeverity::warning ? "warning"
                                                                 : "info";
        std::cout << "  [" << severity << "] " << diagnostic.code << ": "
                  << diagnostic.message << '\n';
    }
}

// ---------------------------------------------------------------------------
// list-container
// ---------------------------------------------------------------------------

void print_nbz_members(const NbzZipSource& source) {
    std::cout << "NBZ archive members\n";
    if (const auto& receipt = source.index_receipt(); receipt.has_value()) {
        std::cout << "  archive size          : " << receipt->archive_size
                  << '\n'
                  << "  declared entry count  : "
                  << receipt->declared_entry_count << '\n'
                  << "  walked entry count    : " << receipt->walked_entry_count
                  << '\n'
                  << "  central offset matches: "
                  << (receipt->central_offset_matches ? "yes" : "no") << '\n';
    }
    print_diagnostics(source.diagnostics());

    const auto refs = source.enumerate();
    std::cout << "  members               : " << refs.size() << "\n\n";
    for (const auto& ref : refs) {
        std::cout << "  [" << std::setw(8) << ref.format << "] "
                  << std::setw(10) << ref.id.size << "  "
                  << ref.id.logical_path << '\n';
    }
}

void print_expansion_tree(const ContainerTreeExpansion& expansion) {
    std::cout << "  expanded containers   : "
              << expansion.expanded_container_count << '\n'
              << "  parser invocations    : "
              << expansion.parser_invocation_count << '\n'
              << "  parse cache hits      : " << expansion.parse_cache_hits
              << '\n'
              << "  parsed bytes          : " << expansion.parsed_container_bytes
              << '\n'
              << "  fully expanded        : "
              << (expansion.fully_expanded ? "yes" : "no") << '\n';
    print_diagnostics(expansion.diagnostics);
    std::cout << '\n';

    for (const auto& container : expansion.expansions) {
        std::cout << container.parser_format << "  "
                  << container.parent.id.logical_path << "  ("
                  << container.children.size() << " slots)\n";
        print_diagnostics(container.diagnostics);
        for (const auto& child : container.children) {
            const auto& entry = child.entry;
            std::cout << "  slot " << std::setw(4) << entry.slot_index << "  ";
            if (!entry.populated) {
                std::cout << "(empty)\n";
                continue;
            }
            std::cout << "off=" << std::setw(10) << entry.offset
                      << "  size=" << std::setw(10) << entry.size
                      << "  [" << std::setw(8) << child.payload.resource.format
                      << "]";
            if (!child.payload.bytes.empty()) {
                std::cout << "  sha256="
                          << digest_of(std::span<const std::byte>{
                                 child.payload.bytes})
                                 .substr(0U, 16U);
            }
            const auto exact = intrinsic_extent(
                child.payload.resource.format,
                std::span<const std::byte>{child.payload.bytes});
            if (exact.has_value() && *exact != entry.size) {
                const auto padding = entry.size > *exact
                    ? entry.size - *exact
                    : 0U;
                std::cout << "  intrinsic=" << *exact;
                if (padding != 0U) {
                    std::cout << " padding=" << padding;
                } else {
                    std::cout << " OVER-RUNS SLOT";
                }
            }
            std::cout << '\n';
        }
        std::cout << '\n';
    }
}

[[nodiscard]] int run_list_container(const std::filesystem::path& path) {
    std::error_code code;
    if (!std::filesystem::is_regular_file(path, code) || code) {
        std::cerr << "list-container: input is not a readable regular file\n";
        return 1;
    }

    std::string error;
    auto bytes = read_file(path, error);
    if (!bytes.has_value()) {
        std::cerr << "list-container: " << error << '\n';
        return 1;
    }

    const auto digest = digest_of(std::span<const std::byte>{*bytes});
    auto root = make_root(path, std::move(*bytes));

    std::cout << "Container: " << path.generic_string() << '\n'
              << "  size                  : " << root.bytes.size() << '\n'
              << "  sha256                : " << digest << '\n'
              << "  format                : " << root.resource.format << '\n'
              << "  container             : "
              << (root.resource.container ? "yes" : "no") << "\n\n";

    // An NBZ is a source, not a relative-slot container, so it needs the ZIP
    // reader rather than the container parser registry.
    if (root.resource.format == "nbz") {
        NbzZipSource source("container-inspect", path);
        if (!source.valid()) {
            std::cerr << "list-container: the NBZ index is not usable\n";
            print_diagnostics(source.diagnostics());
            return 1;
        }
        print_nbz_members(source);
        return 0;
    }

    const auto registry = profiles::dmc3::make_container_parser_registry();
    const auto expansion = ContainerTreeExpander::expand(root, registry);
    print_expansion_tree(expansion);
    return expansion.complete() ? 0 : 2;
}

// ---------------------------------------------------------------------------
// extract-slot
// ---------------------------------------------------------------------------

[[nodiscard]] std::optional<std::vector<std::uint32_t>> parse_slot_path(
    std::string_view text) {
    std::vector<std::uint32_t> path;
    std::size_t begin = 0U;
    while (begin <= text.size()) {
        auto end = text.find('/', begin);
        if (end == std::string_view::npos) {
            end = text.size();
        }
        const auto piece = text.substr(begin, end - begin);
        if (piece.empty()) {
            return std::nullopt;
        }
        std::uint32_t value = 0U;
        const auto* first = piece.data();
        const auto* last = piece.data() + piece.size();
        const auto parsed = std::from_chars(first, last, value);
        if (parsed.ec != std::errc{} || parsed.ptr != last) {
            return std::nullopt;
        }
        path.push_back(value);
        if (end == text.size()) {
            break;
        }
        begin = end + 1U;
    }
    return path.empty() ? std::nullopt : std::optional{path};
}

[[nodiscard]] int run_extract_slot(
    const std::filesystem::path& container_path,
    std::string_view slot_path_text,
    const std::filesystem::path& output_path) {
    const auto slot_path = parse_slot_path(slot_path_text);
    if (!slot_path.has_value()) {
        std::cerr << "extract-slot: slot path must be one or more unsigned "
                     "decimal indices separated by '/'\n";
        return 1;
    }

    std::error_code code;
    if (!std::filesystem::is_regular_file(container_path, code) || code) {
        std::cerr << "extract-slot: input is not a readable regular file\n";
        return 1;
    }

    std::string error;
    auto bytes = read_file(container_path, error);
    if (!bytes.has_value()) {
        std::cerr << "extract-slot: " << error << '\n';
        return 1;
    }

    auto payload = make_root(container_path, std::move(*bytes));
    const auto registry = profiles::dmc3::make_container_parser_registry();

    std::string chain;
    for (std::size_t depth = 0U; depth < slot_path->size(); ++depth) {
        const auto slot = (*slot_path)[depth];
        chain += (chain.empty() ? "" : "/") + std::to_string(slot);

        const auto* parser = registry.select(
            std::span<const std::byte>{payload.bytes},
            payload.resource.id.logical_path);
        if (parser == nullptr) {
            std::cerr << "extract-slot: no registered container parser "
                         "recognizes the resource at "
                      << (depth == 0U ? std::string{"the container root"}
                                      : chain)
                      << '\n';
            return 1;
        }

        const auto parsed = parser->parse(
            std::span<const std::byte>{payload.bytes},
            payload.resource.id.logical_path);
        const auto expansion = gdspaces::ContainerExpander::expand(payload, parsed);
        if (!expansion.usable()) {
            std::cerr << "extract-slot: the container at " << chain
                      << " did not expand cleanly\n";
            print_diagnostics(expansion.diagnostics);
            return 1;
        }
        if (slot >= expansion.children.size()) {
            std::cerr << "extract-slot: slot " << slot << " is out of range at "
                      << chain << " (" << expansion.children.size()
                      << " slots)\n";
            return 1;
        }

        const auto& child = expansion.children[slot];
        if (!child.entry.populated) {
            std::cerr << "extract-slot: slot " << chain << " is empty\n";
            return 1;
        }
        payload = child.payload;
    }

    const auto digest = digest_of(std::span<const std::byte>{payload.bytes});
    std::cout << "Extracted slot " << chain << '\n'
              << "  size                  : " << payload.bytes.size() << '\n'
              << "  sha256                : " << digest << '\n'
              << "  format                : " << payload.resource.format << '\n';

    // Extraction publishes through the shared no-replace path so it can never
    // overwrite an existing artifact. That path requires a real parent
    // directory, and a bare relative filename has none, so the destination is
    // resolved against the working directory first.
    std::error_code absolute_error;
    auto destination = std::filesystem::absolute(output_path, absolute_error);
    if (absolute_error) {
        std::cerr << "extract-slot: unable to resolve the output path\n";
        return 1;
    }
    destination = destination.lexically_normal();

    const auto published = core::publish_bytes_no_replace(
        destination,
        std::span<const std::byte>{payload.bytes},
        {},
        ".dmc-rengine-extract-slot.staging");
    if (published.status != core::NoReplacePublicationStatus::success) {
        std::cerr << "extract-slot: publication failed ("
                  << core::to_string(published.status) << "): "
                  << published.detail << '\n';
        return 1;
    }

    std::cout << "  written               : " << destination.generic_string()
              << '\n';
    return 0;
}

} // namespace

void print_container_inspect_help() {
    std::cout
        << "  list-container <file>     Expand and list an NBZ, PAC or PNST "
           "container without a retail runtime\n"
        << "  extract-slot <container> <slot/path> <output-file>\n"
        << "                            Extract one populated PAC/PNST slot "
           "by physical slot path\n";
}

int try_run_container_inspect_command(int argc, char** argv) {
    if (argc < 2) {
        return -1;
    }
    const std::string_view command{argv[1]};

    if (command == "list-container") {
        if (argc != 3) {
            std::cerr << "list-container: expected <file>\n";
            return 1;
        }
        return run_list_container(std::filesystem::path{argv[2]});
    }

    if (command == "extract-slot") {
        if (argc != 5) {
            std::cerr << "extract-slot: expected <container> <slot/path> "
                         "<output-file>\n";
            return 1;
        }
        return run_extract_slot(
            std::filesystem::path{argv[2]},
            std::string_view{argv[3]},
            std::filesystem::path{argv[4]});
    }

    return -1;
}

} // namespace dmc::rengine::cli
