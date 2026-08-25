#include "dmc3_l1_closure_commands.hpp"

#include "dmc3_build_authority_commands.hpp"
#include "dmc3_overlay_commands.hpp"
#include "dmc3_retail_acquisition_commands.hpp"
#include "relative_slot_commands.hpp"

#include "dmc_rengine/core/no_replace_publication.hpp"
#include "dmc_rengine/core/sha256.hpp"
#include "dmc_rengine/gdspaces/container_expander.hpp"
#include "dmc_rengine/gdspaces/local_directory_source.hpp"
#include "dmc_rengine/gdspaces/nbz_zip_source.hpp"
#include "dmc_rengine/gdspaces/source_registry.hpp"
#include "dmc_rengine/profiles/dmc3/container_parsers.hpp"
#include "dmc_rengine/profiles/dmc3/runtime_resource_resolver.hpp"
#include "dmc_rengine/profiles/dmc3/volume_bootstrap_policy.hpp"

#include <algorithm>
#include <charconv>
#include <cctype>
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <iostream>
#include <limits>
#include <memory>
#include <optional>
#include <span>
#include <sstream>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace dmc::rengine::cli {
namespace {

namespace core = dmc::rengine::core;
namespace gdspaces = dmc::rengine::gdspaces;
namespace dmc3 = dmc::rengine::profiles::dmc3;

struct DiscoveredVolume final {
    std::uint32_t index{};
    std::filesystem::path path;
};

[[nodiscard]] std::string lower_ascii(std::string value) {
    std::transform(value.begin(), value.end(), value.begin(), [](unsigned char ch) {
        return static_cast<char>(std::tolower(ch));
    });
    return value;
}

[[nodiscard]] std::optional<unsigned int> parse_slot_index(
    std::string_view text) noexcept {
    if (text.empty()) {
        return std::nullopt;
    }
    unsigned long long value{};
    const auto* first = text.data();
    const auto* last = text.data() + text.size();
    const auto parsed = std::from_chars(first, last, value, 10);
    if (parsed.ec != std::errc{} || parsed.ptr != last ||
        value > std::numeric_limits<unsigned int>::max()) {
        return std::nullopt;
    }
    return static_cast<unsigned int>(value);
}

[[nodiscard]] std::optional<std::uint32_t> numbered_volume_index(
    const std::filesystem::path& path) {
    const auto name = lower_ascii(path.filename().string());
    constexpr std::string_view prefix = "dmc3-";
    constexpr std::string_view suffix = ".nbz";
    if (name.size() <= prefix.size() + suffix.size() ||
        !name.starts_with(prefix) || !name.ends_with(suffix)) {
        return std::nullopt;
    }
    const auto digits = std::string_view{name}.substr(
        prefix.size(), name.size() - prefix.size() - suffix.size());
    std::uint64_t value{};
    const auto* first = digits.data();
    const auto* last = digits.data() + digits.size();
    const auto parsed = std::from_chars(first, last, value, 10);
    if (digits.empty() || parsed.ec != std::errc{} || parsed.ptr != last ||
        value > std::numeric_limits<std::uint32_t>::max()) {
        return std::nullopt;
    }
    return static_cast<std::uint32_t>(value);
}

[[nodiscard]] std::optional<std::vector<DiscoveredVolume>> discover_volumes(
    const std::filesystem::path& data_directory) {
    std::error_code error;
    if (!std::filesystem::is_directory(data_directory, error) || error) {
        return std::nullopt;
    }
    std::vector<DiscoveredVolume> volumes;
    for (std::filesystem::directory_iterator it{data_directory, error}, end;
         !error && it != end; it.increment(error)) {
        error.clear();
        if (!it->is_regular_file(error) || error) {
            error.clear();
            continue;
        }
        if (const auto index = numbered_volume_index(it->path()); index.has_value()) {
            volumes.push_back(DiscoveredVolume{*index, it->path()});
        }
    }
    if (error) {
        return std::nullopt;
    }
    std::sort(
        volumes.begin(), volumes.end(),
        [](const DiscoveredVolume& left, const DiscoveredVolume& right) {
            if (left.index != right.index) {
                return left.index < right.index;
            }
            return left.path.generic_string() < right.path.generic_string();
        });
    for (std::size_t index = 1U; index < volumes.size(); ++index) {
        if (volumes[index - 1U].index == volumes[index].index) {
            return std::nullopt;
        }
    }
    return volumes;
}

[[nodiscard]] const DiscoveredVolume* find_volume(
    const std::vector<DiscoveredVolume>& volumes,
    std::uint32_t index) noexcept {
    const auto iterator = std::lower_bound(
        volumes.begin(), volumes.end(), index,
        [](const DiscoveredVolume& volume, std::uint32_t candidate) {
            return volume.index < candidate;
        });
    return iterator == volumes.end() || iterator->index != index
        ? nullptr
        : &*iterator;
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

[[nodiscard]] bool path_component_equal(
    const std::filesystem::path& left,
    const std::filesystem::path& right) {
#ifdef _WIN32
    return lower_ascii(left.generic_string()) == lower_ascii(right.generic_string());
#else
    return left == right;
#endif
}

[[nodiscard]] bool is_within(
    const std::filesystem::path& child,
    const std::filesystem::path& parent) {
    const auto child_normalized = normalized_absolute(child);
    const auto parent_normalized = normalized_absolute(parent);
    auto child_it = child_normalized.begin();
    auto parent_it = parent_normalized.begin();
    for (; parent_it != parent_normalized.end(); ++parent_it, ++child_it) {
        if (child_it == child_normalized.end() ||
            !path_component_equal(*child_it, *parent_it)) {
            return false;
        }
    }
    return true;
}

[[nodiscard]] std::string request_basename(std::string_view request) {
    const auto separator = request.find_last_of("/\\");
    const auto basename = separator == std::string_view::npos
        ? request
        : request.substr(separator + 1U);
    return basename.empty() ? std::string{"resource.pac"} : std::string{basename};
}

[[nodiscard]] std::optional<std::vector<std::byte>> read_file_bytes(
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
    auto payload = registry.read(id);
    if (!payload.has_value() || !payload->readable()) {
        return std::nullopt;
    }
    return std::move(payload->bytes);
}

[[nodiscard]] std::string sha256_of(std::span<const std::byte> bytes) {
    return core::Sha256::compute(bytes).hex();
}

[[nodiscard]] std::string escape_json(std::string_view value) {
    std::ostringstream output;
    constexpr char hex[] = "0123456789abcdef";
    for (const unsigned char character : value) {
        switch (character) {
        case '"': output << "\\\""; break;
        case '\\': output << "\\\\"; break;
        case '\b': output << "\\b"; break;
        case '\f': output << "\\f"; break;
        case '\n': output << "\\n"; break;
        case '\r': output << "\\r"; break;
        case '\t': output << "\\t"; break;
        default:
            if (character < 0x20U) {
                output << "\\u00"
                       << hex[(character >> 4U) & 0x0FU]
                       << hex[character & 0x0FU];
            } else {
                output << static_cast<char>(character);
            }
            break;
        }
    }
    return output.str();
}

[[nodiscard]] bool verify_rematerialized_slot(
    const gdspaces::ResourcePayload& payload,
    unsigned int slot_index,
    std::span<const std::byte> expected_replacement) {
    const auto registry = dmc3::make_container_parser_registry();
    const auto parsed = registry.parse(
        std::span<const std::byte>{payload.bytes.data(), payload.bytes.size()},
        payload.resource.id.logical_path);
    if (!parsed.ok() ||
        (parsed.document.format != "PAC" && parsed.document.format != "PNST")) {
        return false;
    }
    const auto expanded = gdspaces::ContainerExpander::expand(payload, parsed);
    if (!expanded.usable() || slot_index >= expanded.children.size()) {
        return false;
    }
    const auto& child = expanded.children[slot_index];
    return child.entry.populated && child.payload.readable() &&
        child.payload.bytes.size() == expected_replacement.size() &&
        std::equal(
            child.payload.bytes.begin(), child.payload.bytes.end(),
            expected_replacement.begin());
}

[[nodiscard]] int run_verify_dmc3_l1_authoring(
    const std::filesystem::path& executable_directory,
    std::string_view game_request,
    unsigned int slot_index,
    const std::filesystem::path& replacement_file,
    const std::filesystem::path& workspace_directory) {
    const auto data_directory = executable_directory /
        std::filesystem::path{dmc3::VolumeBootstrapPolicy::data_subdirectory()};
    if (workspace_directory.empty() ||
        is_within(workspace_directory, executable_directory)) {
        std::cerr
            << "verify-dmc3-l1-authoring: workspace must be outside the complete retail executable tree\n";
        return 2;
    }

    if (run_preflight_dmc3_game_test(executable_directory) != 0) {
        std::cerr
            << "verify-dmc3-l1-authoring: protected distribution preflight failed\n";
        return 3;
    }

    const auto discovered = discover_volumes(data_directory);
    if (!discovered.has_value()) {
        std::cerr
            << "verify-dmc3-l1-authoring: cannot uniquely enumerate retail numbered volumes\n";
        return 4;
    }
    std::vector<std::uint32_t> present_indices;
    present_indices.reserve(discovered->size());
    for (const auto& volume : *discovered) {
        present_indices.push_back(volume.index);
    }
    const auto base_bootstrap = dmc3::VolumeBootstrapPolicy::plan(present_indices);
    if (!base_bootstrap.valid() || base_bootstrap.registered_archives.empty() ||
        !base_bootstrap.present_after_first_gap.empty() ||
        !base_bootstrap.present_outside_runtime_index_domain.empty()) {
        std::cerr
            << "verify-dmc3-l1-authoring: retail volume topology is not safe for deterministic next-volume authoring\n";
        return 4;
    }

    std::error_code error;
    std::filesystem::create_directories(workspace_directory, error);
    if (error) {
        std::cerr << "verify-dmc3-l1-authoring: cannot create workspace\n";
        return 5;
    }
    const auto basename = request_basename(game_request);
    const auto retail_member = workspace_directory / ("retail-" + basename);
    const auto rebuilt_member = workspace_directory / ("rebuilt-" + basename);
    const auto overlay_directory = workspace_directory / "overlay";
    const auto closure_receipt = workspace_directory / "l1-closure.receipt.json";

    if (run_extract_dmc3_retail_member(
            executable_directory, game_request, retail_member) != 0) {
        std::cerr
            << "verify-dmc3-l1-authoring: direct-retail acquisition failed\n";
        return 6;
    }

    if (run_rebuild_relative_slot(
            retail_member, slot_index, replacement_file, rebuilt_member) != 0) {
        std::cerr
            << "verify-dmc3-l1-authoring: retail representation is not inside the currently proven PAC/PNST authoring domain, or rebuild failed\n";
        return 7;
    }

    if (run_build_dmc3_overlay(
            executable_directory, game_request, rebuilt_member,
            overlay_directory) != 0) {
        std::cerr
            << "verify-dmc3-l1-authoring: next-volume overlay authoring failed\n";
        return 8;
    }

    const auto overlay_index = base_bootstrap.first_missing_index;
    const auto overlay_filename = dmc3::VolumeBootstrapPolicy::volume_filename(
        overlay_index);
    if (overlay_filename.empty()) {
        std::cerr
            << "verify-dmc3-l1-authoring: next runtime volume index is outside the recovered domain\n";
        return 8;
    }
    const auto overlay_path = overlay_directory / overlay_filename;

    auto rebuilt_bytes = read_file_bytes(rebuilt_member, "l1-closure-rebuilt");
    auto replacement_bytes = read_file_bytes(replacement_file, "l1-closure-replacement");
    auto retail_bytes = read_file_bytes(retail_member, "l1-closure-retail");
    auto overlay_bytes = read_file_bytes(overlay_path, "l1-closure-overlay-artifact");
    auto executable_bytes = read_file_bytes(
        executable_directory / "dmc3.exe", "l1-closure-executable");
    if (!rebuilt_bytes.has_value() || !replacement_bytes.has_value() ||
        !retail_bytes.has_value() || !overlay_bytes.has_value() ||
        !executable_bytes.has_value()) {
        std::cerr
            << "verify-dmc3-l1-authoring: one or more closure artifacts are unreadable\n";
        return 9;
    }

    std::vector<std::uint32_t> verification_indices = present_indices;
    verification_indices.push_back(overlay_index);
    const auto verification_bootstrap =
        dmc3::VolumeBootstrapPolicy::plan(verification_indices);
    if (!verification_bootstrap.valid() ||
        verification_bootstrap.first_missing_index != overlay_index + 1U) {
        std::cerr
            << "verify-dmc3-l1-authoring: generated overlay does not extend the contiguous runtime volume set exactly once\n";
        return 10;
    }

    gdspaces::SourceRegistry sources;
    constexpr std::string_view physical_source_id = "l1-closure-physical";
    if (!sources.mount(std::make_unique<gdspaces::LocalDirectorySource>(
            std::string{physical_source_id}, data_directory, false))) {
        std::cerr << "verify-dmc3-l1-authoring: cannot mount physical retail source\n";
        return 10;
    }

    dmc3::RuntimeSourceBindings bindings;
    bindings.physical_source_id = std::string{physical_source_id};
    std::string overlay_source_id;
    for (const auto& archive : verification_bootstrap.registered_archives) {
        const bool is_overlay = archive.index == overlay_index;
        const auto* retail_volume = is_overlay
            ? nullptr
            : find_volume(*discovered, archive.index);
        const auto archive_path = is_overlay
            ? overlay_path
            : (retail_volume == nullptr ? std::filesystem::path{} : retail_volume->path);
        if (archive_path.empty()) {
            std::cerr
                << "verify-dmc3-l1-authoring: verification bootstrap references an unavailable archive\n";
            return 10;
        }
        auto source_id = std::string{"l1-closure-volume-"} +
            std::to_string(archive.index);
        auto source = std::make_unique<gdspaces::NbzZipSource>(source_id, archive_path);
        if (!source->valid() || !sources.mount(std::move(source))) {
            std::cerr
                << "verify-dmc3-l1-authoring: cannot mount verification archive\n";
            return 10;
        }
        if (is_overlay) {
            overlay_source_id = source_id;
        }
    }
    for (const auto index : verification_bootstrap.archive_resolution_order) {
        bindings.archives.push_back(dmc3::ArchiveSourceBinding{
            .volume_index = index,
            .source_id = std::string{"l1-closure-volume-"} + std::to_string(index),
        });
    }
    if (!bindings.valid_for(verification_bootstrap)) {
        std::cerr
            << "verify-dmc3-l1-authoring: verification source bindings are invalid\n";
        return 10;
    }

    const auto resolved = dmc3::RuntimeResourceResolver::resolve(
        game_request, verification_bootstrap, bindings, sources);
    if (!resolved.ok() || resolved.resolved->id.source_id != overlay_source_id) {
        std::cerr
            << "verify-dmc3-l1-authoring: generated next-volume artifact did not win canonical resolution\n";
        return 11;
    }
    const auto rematerialized = sources.read(resolved.resolved->id);
    if (!rematerialized.has_value() || !rematerialized->readable() ||
        rematerialized->bytes != *rebuilt_bytes) {
        std::cerr
            << "verify-dmc3-l1-authoring: resolver winner did not rematerialize the exact rebuilt container bytes\n";
        return 12;
    }
    if (!verify_rematerialized_slot(
            *rematerialized, slot_index,
            std::span<const std::byte>{replacement_bytes->data(), replacement_bytes->size()})) {
        std::cerr
            << "verify-dmc3-l1-authoring: rematerialized target slot does not equal the authored replacement\n";
        return 13;
    }

    const auto executable_sha = sha256_of(
        std::span<const std::byte>{executable_bytes->data(), executable_bytes->size()});
    const auto retail_sha = sha256_of(
        std::span<const std::byte>{retail_bytes->data(), retail_bytes->size()});
    const auto replacement_sha = sha256_of(
        std::span<const std::byte>{replacement_bytes->data(), replacement_bytes->size()});
    const auto rebuilt_sha = sha256_of(
        std::span<const std::byte>{rebuilt_bytes->data(), rebuilt_bytes->size()});
    const auto overlay_sha = sha256_of(
        std::span<const std::byte>{overlay_bytes->data(), overlay_bytes->size()});
    const auto rematerialized_sha = sha256_of(
        std::span<const std::byte>{rematerialized->bytes.data(), rematerialized->bytes.size()});

    std::ostringstream receipt;
    receipt
        << "{\n"
        << "  \"schema_version\": 1,\n"
        << "  \"evidence_class\": \"gdspaces-l1-product-end-to-end-authoring\",\n"
        << "  \"status\": \"product-end-to-end-verified\",\n"
        << "  \"game_request\": \"" << escape_json(game_request) << "\",\n"
        << "  \"target_slot\": " << slot_index << ",\n"
        << "  \"executable\": {\"sha256\": \"" << executable_sha
        << "\", \"preflight\": \"protected-distribution-authority-passed\"},\n"
        << "  \"retail_member\": {\"path\": \""
        << escape_json(retail_member.generic_string()) << "\", \"size\": "
        << retail_bytes->size() << ", \"sha256\": \"" << retail_sha << "\"},\n"
        << "  \"replacement\": {\"path\": \""
        << escape_json(replacement_file.generic_string()) << "\", \"size\": "
        << replacement_bytes->size() << ", \"sha256\": \""
        << replacement_sha << "\"},\n"
        << "  \"rebuilt_member\": {\"path\": \""
        << escape_json(rebuilt_member.generic_string()) << "\", \"size\": "
        << rebuilt_bytes->size() << ", \"sha256\": \"" << rebuilt_sha << "\"},\n"
        << "  \"overlay\": {\"volume_index\": " << overlay_index
        << ", \"path\": \"" << escape_json(overlay_path.generic_string())
        << "\", \"size\": " << overlay_bytes->size()
        << ", \"sha256\": \"" << overlay_sha << "\"},\n"
        << "  \"resolver_winner\": {\"source_id\": \""
        << escape_json(resolved.resolved->id.source_id)
        << "\", \"logical_path\": \""
        << escape_json(resolved.resolved->id.logical_path) << "\"},\n"
        << "  \"rematerialized\": {\"size\": "
        << rematerialized->bytes.size() << ", \"sha256\": \""
        << rematerialized_sha << "\", \"target_slot_matches_replacement\": true},\n"
        << "  \"original_game_consumption\": \"pending-external-level-e-receipt\"\n"
        << "}\n";
    const auto receipt_text = receipt.str();
    const auto publication = core::publish_bytes_no_replace(
        closure_receipt,
        std::as_bytes(std::span<const char>{receipt_text.data(), receipt_text.size()}),
        {},
        ".dmc-rengine-l1-closure-receipt.staging");
    if (!publication.ok()) {
        std::cerr
            << "verify-dmc3-l1-authoring: closure receipt publication failed ("
            << core::to_string(publication.status) << ")\n";
        return 14;
    }

    std::cout
        << "GDSpaces L1 product authoring closure: VERIFIED\n"
        << "Game request: " << game_request << '\n'
        << "Retail member SHA-256: " << retail_sha << '\n'
        << "Replacement SHA-256: " << replacement_sha << '\n'
        << "Rebuilt member SHA-256: " << rebuilt_sha << '\n'
        << "Overlay volume: " << overlay_index << '\n'
        << "Overlay SHA-256: " << overlay_sha << '\n'
        << "Resolver winner: " << resolved.resolved->id.logical_path << '\n'
        << "Rematerialized SHA-256: " << rematerialized_sha << '\n'
        << "Target slot: exact authored replacement\n"
        << "Product-side L1 chain: CLOSED FOR THIS RECEIPT\n"
        << "Original DMC3 consumption: PENDING EXTERNAL LEVEL-E RECEIPT\n"
        << "Receipt: " << closure_receipt.string() << '\n';
    return 0;
}

} // namespace

void print_dmc3_l1_closure_help() {
    std::cout
        << "  verify-dmc3-l1-authoring <exe-dir> <game-request> <slot-index> <replacement-file> <workspace-dir>\n"
        << "                            Run protected-executable preflight, direct-retail acquisition, PAC/PNST slot rebuild, next-volume authoring, resolver win and exact rematerialization receipt\n";
}

int try_run_dmc3_l1_closure_command(int argc, char** argv) {
    if (argc <= 1 || std::string_view{argv[1]} != "verify-dmc3-l1-authoring") {
        return -1;
    }
    if (argc != 7) {
        std::cerr
            << "verify-dmc3-l1-authoring: expected <exe-dir> <game-request> <slot-index> <replacement-file> <workspace-dir>\n";
        return 1;
    }
    const auto slot_index = parse_slot_index(argv[4]);
    if (!slot_index.has_value()) {
        std::cerr << "verify-dmc3-l1-authoring: invalid slot index\n";
        return 1;
    }
    return run_verify_dmc3_l1_authoring(
        std::filesystem::path{argv[2]},
        std::string_view{argv[3]},
        *slot_index,
        std::filesystem::path{argv[5]},
        std::filesystem::path{argv[6]});
}

} // namespace dmc::rengine::cli
