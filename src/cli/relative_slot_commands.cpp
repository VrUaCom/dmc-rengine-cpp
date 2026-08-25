#include "relative_slot_commands.hpp"

#include "dmc_rengine/core/no_replace_publication.hpp"
#include "dmc_rengine/core/sha256.hpp"
#include "dmc_rengine/gdspaces/container_expander.hpp"
#include "dmc_rengine/gdspaces/local_directory_source.hpp"
#include "dmc_rengine/gdspaces/source_registry.hpp"
#include "dmc_rengine/profiles/dmc3/container_parsers.hpp"
#include "dmc_rengine/profiles/dmc3/relative_slot_packed_reflow_writer.hpp"
#include "dmc_rengine/profiles/dmc3/relative_slot_path_reflow_writer.hpp"

#include <charconv>
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <iostream>
#include <limits>
#include <memory>
#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <vector>

namespace dmc::rengine::cli {
namespace {

namespace core = dmc::rengine::core;
namespace gdspaces = dmc::rengine::gdspaces;
namespace dmc3 = dmc::rengine::profiles::dmc3;

[[nodiscard]] std::optional<unsigned int> parse_slot_index(
    std::string_view text) noexcept {
    if (text.empty()) {
        return std::nullopt;
    }
    unsigned long long value{};
    const auto* first = text.data();
    const auto* last = text.data() + text.size();
    const auto result = std::from_chars(first, last, value, 10);
    if (result.ec != std::errc{} || result.ptr != last ||
        value > std::numeric_limits<unsigned int>::max()) {
        return std::nullopt;
    }
    return static_cast<unsigned int>(value);
}

[[nodiscard]] std::optional<std::vector<unsigned int>> parse_slot_path(
    std::string_view text) {
    if (text.empty()) {
        return std::nullopt;
    }
    std::vector<unsigned int> path;
    std::size_t begin = 0U;
    while (begin < text.size()) {
        const auto separator = text.find('/', begin);
        const auto end = separator == std::string_view::npos ? text.size() : separator;
        const auto component = text.substr(begin, end - begin);
        const auto slot = parse_slot_index(component);
        if (!slot.has_value()) {
            return std::nullopt;
        }
        path.push_back(*slot);
        if (separator == std::string_view::npos) {
            break;
        }
        begin = separator + 1U;
        if (begin == text.size()) {
            return std::nullopt;
        }
    }
    return path.empty() ? std::nullopt : std::optional<std::vector<unsigned int>>{std::move(path)};
}

[[nodiscard]] std::filesystem::path normalized_absolute(
    const std::filesystem::path& path) {
    std::error_code error;
    auto absolute = std::filesystem::absolute(path, error);
    if (error) {
        return path.lexically_normal();
    }
    auto canonical = std::filesystem::weakly_canonical(absolute, error);
    return error ? absolute.lexically_normal() : canonical;
}

[[nodiscard]] std::optional<gdspaces::ResourcePayload> read_local_resource(
    const std::filesystem::path& path,
    std::string_view source_id) {
    std::error_code error;
    const auto absolute = std::filesystem::absolute(path, error);
    if (error || !std::filesystem::is_regular_file(absolute, error) || error) {
        return std::nullopt;
    }
    const auto size = std::filesystem::file_size(absolute, error);
    if (error) {
        return std::nullopt;
    }

    gdspaces::SourceRegistry registry;
    if (!registry.mount(std::make_unique<gdspaces::LocalDirectorySource>(
            std::string{source_id}, absolute.parent_path(), false))) {
        return std::nullopt;
    }
    const gdspaces::ResourceId id{
        .source_id = std::string{source_id},
        .logical_path = absolute.filename().generic_string(),
        .container_chain = {},
        .offset = 0U,
        .size = size,
    };
    return registry.read(id);
}

[[nodiscard]] std::string sha256_of(std::span<const std::byte> bytes) {
    return core::Sha256::compute(bytes).hex();
}

[[nodiscard]] bool ensure_output_parent(const std::filesystem::path& output_file) {
    std::error_code error;
    const auto output_parent = output_file.parent_path();
    if (!output_parent.empty()) {
        std::filesystem::create_directories(output_parent, error);
    }
    return !error;
}

[[nodiscard]] bool validate_relative_slot_file(
    const std::filesystem::path& staged_path,
    std::string_view expected_format,
    std::uint32_t expected_slots) {
    auto staged = read_local_resource(staged_path, "relative-slot-staged-validation");
    if (!staged.has_value() || !staged->readable()) {
        return false;
    }
    const auto parser_registry = dmc3::make_container_parser_registry();
    const auto reparsed = parser_registry.parse(
        std::span<const std::byte>{staged->bytes.data(), staged->bytes.size()},
        staged_path.filename().generic_string());
    return reparsed.ok() && reparsed.document.format == expected_format &&
        reparsed.document.declared_slot_count == expected_slots;
}

} // namespace

int run_rebuild_relative_slot(
    const std::filesystem::path& parent_file,
    unsigned int slot_index,
    const std::filesystem::path& replacement_file,
    const std::filesystem::path& output_file) {
    if (parent_file.empty() || replacement_file.empty() || output_file.empty() ||
        normalized_absolute(parent_file) == normalized_absolute(output_file)) {
        std::cerr
            << "rebuild-relative-slot: source container and output must be distinct paths\n";
        return 2;
    }

    auto parent = read_local_resource(parent_file, "relative-slot-parent");
    if (!parent.has_value() || !parent->readable()) {
        std::cerr << "rebuild-relative-slot: parent container is not readable\n";
        return 3;
    }

    const auto registry = dmc3::make_container_parser_registry();
    const auto parsed = registry.parse(
        std::span<const std::byte>{parent->bytes.data(), parent->bytes.size()},
        parent->resource.id.logical_path);
    if (!parsed.ok() ||
        (parsed.document.format != "PAC" && parsed.document.format != "PNST")) {
        std::cerr
            << "rebuild-relative-slot: parent is not a canonical PAC/PNST relative-slot container\n";
        return 4;
    }

    const auto expansion = gdspaces::ContainerExpander::expand(*parent, parsed);
    if (!expansion.usable() || slot_index >= expansion.children.size()) {
        std::cerr << "rebuild-relative-slot: slot index is outside the parsed topology\n";
        return 5;
    }
    const auto& child = expansion.children[slot_index];
    if (!child.entry.populated || !child.payload.readable()) {
        std::cerr << "rebuild-relative-slot: target slot is empty or unreadable\n";
        return 5;
    }

    auto replacement = read_local_resource(
        replacement_file, "relative-slot-replacement");
    if (!replacement.has_value() || !replacement->readable()) {
        std::cerr << "rebuild-relative-slot: replacement file is not readable\n";
        return 6;
    }

    dmc3::AuthoredChildImage authored{
        .resource = child.payload.resource.id,
        .source_sha256 = sha256_of(std::span<const std::byte>{
            child.payload.bytes.data(), child.payload.bytes.size()}),
        .output_sha256 = sha256_of(std::span<const std::byte>{
            replacement->bytes.data(), replacement->bytes.size()}),
        .revision = 1U,
        .writer_mode = "cli-complete-child-image",
        .bytes = std::move(replacement->bytes),
    };
    const std::vector<dmc3::AuthoredChildImage> authored_children{
        std::move(authored)};
    const auto rebuilt = dmc3::RelativeSlotPackedReflowWriter::rebuild(
        *parent, expansion, authored_children);
    if (!rebuilt.ok()) {
        std::cerr
            << "rebuild-relative-slot: canonical packed reflow failed ("
            << dmc3::to_string(rebuilt.status) << ")";
        if (!rebuilt.detail.empty()) {
            std::cerr << ": " << rebuilt.detail;
        }
        std::cerr << '\n';
        return 7;
    }

    if (!ensure_output_parent(output_file)) {
        std::cerr << "rebuild-relative-slot: cannot create output directory\n";
        return 8;
    }

    const auto expected_format = rebuilt.receipt->output_topology.format;
    const auto expected_slots = rebuilt.receipt->output_topology.declared_slot_count;
    const auto validator = [expected_format, expected_slots](
                               const std::filesystem::path& staged_path) {
        return validate_relative_slot_file(staged_path, expected_format, expected_slots);
    };

    const auto publication = core::publish_bytes_no_replace(
        output_file,
        std::span<const std::byte>{rebuilt.bytes.data(), rebuilt.bytes.size()},
        validator,
        ".dmc-rengine-relative-slot.staging");
    if (!publication.ok()) {
        std::cerr
            << "rebuild-relative-slot: output publication failed ("
            << core::to_string(publication.status) << ")";
        if (!publication.detail.empty()) {
            std::cerr << ": " << publication.detail;
        }
        std::cerr << '\n';
        return 9;
    }

    std::cout
        << "Relative-slot rebuild: VERIFIED\n"
        << "Format: " << rebuilt.receipt->output_topology.format << '\n'
        << "Target slot: " << slot_index << '\n'
        << "Source SHA-256: " << rebuilt.receipt->source_sha256 << '\n'
        << "Output SHA-256: " << rebuilt.receipt->output_sha256 << '\n'
        << "Source bytes: " << rebuilt.receipt->source_topology.container_size << '\n'
        << "Output bytes: " << rebuilt.receipt->output_topology.container_size << '\n'
        << "Output: " << output_file.string() << '\n';
    return 0;
}

int run_rebuild_relative_slot_path(
    const std::filesystem::path& parent_file,
    std::span<const unsigned int> slot_path,
    const std::filesystem::path& replacement_file,
    const std::filesystem::path& output_file) {
    if (parent_file.empty() || replacement_file.empty() || output_file.empty() ||
        slot_path.empty() ||
        normalized_absolute(parent_file) == normalized_absolute(output_file)) {
        std::cerr
            << "rebuild-relative-slot-path: source/output must be distinct and slot path must be non-empty\n";
        return 2;
    }

    auto parent = read_local_resource(parent_file, "relative-slot-path-parent");
    auto replacement = read_local_resource(
        replacement_file, "relative-slot-path-replacement");
    if (!parent.has_value() || !parent->readable()) {
        std::cerr << "rebuild-relative-slot-path: parent container is not readable\n";
        return 3;
    }
    if (!replacement.has_value() || !replacement->readable() ||
        replacement->bytes.empty()) {
        std::cerr << "rebuild-relative-slot-path: replacement file is not readable or empty\n";
        return 4;
    }

    const auto rebuilt = dmc3::RelativeSlotPathReflowWriter::rebuild(
        *parent,
        slot_path,
        std::span<const std::byte>{replacement->bytes.data(), replacement->bytes.size()});
    if (!rebuilt.ok()) {
        std::cerr
            << "rebuild-relative-slot-path: canonical nested reflow failed ("
            << dmc3::to_string(rebuilt.status) << ")";
        if (!rebuilt.detail.empty()) {
            std::cerr << ": " << rebuilt.detail;
        }
        std::cerr << '\n';
        return 5;
    }

    if (!ensure_output_parent(output_file)) {
        std::cerr << "rebuild-relative-slot-path: cannot create output directory\n";
        return 6;
    }

    const auto expected_format = rebuilt.receipt->levels.front().reflow.output_topology.format;
    const auto expected_slots = rebuilt.receipt->levels.front().reflow.output_topology.declared_slot_count;
    const auto validator = [expected_format, expected_slots](
                               const std::filesystem::path& staged_path) {
        return validate_relative_slot_file(staged_path, expected_format, expected_slots);
    };
    const auto publication = core::publish_bytes_no_replace(
        output_file,
        std::span<const std::byte>{rebuilt.bytes.data(), rebuilt.bytes.size()},
        validator,
        ".dmc-rengine-relative-slot-path.staging");
    if (!publication.ok()) {
        std::cerr
            << "rebuild-relative-slot-path: output publication failed ("
            << core::to_string(publication.status) << ")";
        if (!publication.detail.empty()) {
            std::cerr << ": " << publication.detail;
        }
        std::cerr << '\n';
        return 7;
    }

    std::cout
        << "Relative-slot path rebuild: VERIFIED\n"
        << "Depth: " << rebuilt.receipt->levels.size() << '\n'
        << "Source SHA-256: " << rebuilt.receipt->source_sha256 << '\n'
        << "Replacement SHA-256: " << rebuilt.receipt->replacement_sha256 << '\n'
        << "Output SHA-256: " << rebuilt.receipt->output_sha256 << '\n'
        << "Output: " << output_file.string() << '\n';
    return 0;
}

void print_relative_slot_help() {
    std::cout
        << "  rebuild-relative-slot <container-file> <slot-index> <replacement-file> <output-file>\n"
        << "                            Replace one populated PAC/PNST slot and safely reflow the packed container\n"
        << "  rebuild-relative-slot-path <container-file> <slot/path> <replacement-file> <output-file>\n"
        << "                            Replace a nested PAC/PNST child (for example 0/2/1) and reflow all ancestors\n";
}

int try_run_relative_slot_command(int argc, char** argv) {
    if (argc <= 1) {
        return -1;
    }
    const std::string_view command{argv[1]};
    if (command == "rebuild-relative-slot") {
        if (argc != 6) {
            std::cerr
                << "rebuild-relative-slot: expected <container-file> <slot-index> <replacement-file> <output-file>\n";
            return 1;
        }
        const auto slot_index = parse_slot_index(argv[3]);
        if (!slot_index.has_value()) {
            std::cerr << "rebuild-relative-slot: invalid slot index\n";
            return 1;
        }
        return run_rebuild_relative_slot(
            std::filesystem::path{argv[2]},
            *slot_index,
            std::filesystem::path{argv[4]},
            std::filesystem::path{argv[5]});
    }
    if (command == "rebuild-relative-slot-path") {
        if (argc != 6) {
            std::cerr
                << "rebuild-relative-slot-path: expected <container-file> <slot/path> <replacement-file> <output-file>\n";
            return 1;
        }
        const auto slot_path = parse_slot_path(argv[3]);
        if (!slot_path.has_value()) {
            std::cerr << "rebuild-relative-slot-path: invalid slot path\n";
            return 1;
        }
        return run_rebuild_relative_slot_path(
            std::filesystem::path{argv[2]},
            *slot_path,
            std::filesystem::path{argv[4]},
            std::filesystem::path{argv[5]});
    }
    return -1;
}

} // namespace dmc::rengine::cli
