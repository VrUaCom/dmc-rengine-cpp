#include "dmc3_overlay_commands.hpp"

#include "dmc_rengine/core/no_replace_publication.hpp"
#include "dmc_rengine/gdspaces/local_directory_source.hpp"
#include "dmc_rengine/gdspaces/nbz_zip_source.hpp"
#include "dmc_rengine/gdspaces/source_registry.hpp"
#include "dmc_rengine/profiles/dmc3/nbz_overlay_writer.hpp"
#include "dmc_rengine/profiles/dmc3/resource_lookup_policy.hpp"
#include "dmc_rengine/profiles/dmc3/volume_bootstrap_policy.hpp"

#include <algorithm>
#include <charconv>
#include <cctype>
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

namespace gdspaces = dmc::rengine::gdspaces;
namespace dmc3 = dmc::rengine::profiles::dmc3;

[[nodiscard]] std::string lower_ascii(std::string value) {
    std::transform(value.begin(), value.end(), value.begin(), [](unsigned char ch) {
        return static_cast<char>(std::tolower(ch));
    });
    return value;
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
    if (digits.empty()) {
        return std::nullopt;
    }

    std::uint64_t parsed{};
    const auto* first = digits.data();
    const auto* last = digits.data() + digits.size();
    const auto result = std::from_chars(first, last, parsed, 10);
    if (result.ec != std::errc{} || result.ptr != last ||
        parsed > std::numeric_limits<std::uint32_t>::max()) {
        return std::nullopt;
    }
    return static_cast<std::uint32_t>(parsed);
}

[[nodiscard]] std::optional<std::vector<std::uint32_t>> discover_volumes(
    const std::filesystem::path& data_directory) {
    std::error_code error;
    if (!std::filesystem::is_directory(data_directory, error) || error) {
        return std::nullopt;
    }

    std::vector<std::uint32_t> indices;
    for (std::filesystem::directory_iterator it{data_directory, error}, end;
         !error && it != end; it.increment(error)) {
        if (!it->is_regular_file(error) || error) {
            error.clear();
            continue;
        }
        if (const auto index = numbered_volume_index(it->path()); index.has_value()) {
            indices.push_back(*index);
        }
    }
    if (error) {
        return std::nullopt;
    }

    std::sort(indices.begin(), indices.end());
    indices.erase(std::unique(indices.begin(), indices.end()), indices.end());
    return indices;
}

[[nodiscard]] std::filesystem::path normalized_absolute(
    const std::filesystem::path& path) {
    std::error_code error;
    auto absolute = std::filesystem::absolute(path, error);
    if (error) {
        return path.lexically_normal();
    }
    auto normalized = std::filesystem::weakly_canonical(absolute, error);
    if (error) {
        return absolute.lexically_normal();
    }
    return normalized;
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

[[nodiscard]] std::optional<std::vector<std::byte>> read_authored_resource(
    const std::filesystem::path& input_path) {
    std::error_code error;
    const auto absolute = std::filesystem::absolute(input_path, error);
    if (error || !std::filesystem::is_regular_file(absolute, error) || error) {
        return std::nullopt;
    }

    const auto raw_size = std::filesystem::file_size(absolute, error);
    if (error) {
        return std::nullopt;
    }

    constexpr std::string_view source_id = "dmc3-overlay-authored-input";
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
        .size = raw_size,
    };
    auto payload = registry.read(id);
    if (!payload.has_value() || !payload->readable()) {
        return std::nullopt;
    }
    return std::move(payload->bytes);
}

[[nodiscard]] bool verify_overlay_file(
    const std::filesystem::path& archive_path,
    std::string_view member_path,
    std::span<const std::byte> expected_bytes) {
    constexpr std::string_view source_id = "dmc3-overlay-staging-verification";
    gdspaces::SourceRegistry registry;
    auto source = std::make_unique<gdspaces::NbzZipSource>(
        std::string{source_id}, archive_path);
    if (!source->valid()) {
        return false;
    }
    if (!registry.mount(std::move(source))) {
        return false;
    }

    const auto* mounted = registry.find(source_id);
    if (mounted == nullptr) {
        return false;
    }
    const auto resources = mounted->enumerate();
    const auto match = std::find_if(
        resources.begin(), resources.end(), [&](const gdspaces::ResourceRef& ref) {
            return ref.id.logical_path == member_path;
        });
    if (match == resources.end()) {
        return false;
    }

    const auto payload = registry.read(match->id);
    if (!payload.has_value() || !payload->readable() ||
        payload->bytes.size() != expected_bytes.size()) {
        return false;
    }
    return std::equal(
        payload->bytes.begin(), payload->bytes.end(), expected_bytes.begin());
}

int run_build_overlay(
    const std::filesystem::path& executable_directory,
    std::string_view game_request,
    const std::filesystem::path& authored_file,
    const std::filesystem::path& output_directory) {
    const auto data_directory = executable_directory /
        std::filesystem::path{dmc3::VolumeBootstrapPolicy::data_subdirectory()};
    const auto present = discover_volumes(data_directory);
    if (!present.has_value()) {
        std::cerr << "build-dmc3-overlay: cannot scan executable-relative game data directory: "
                  << data_directory.string() << '\n';
        return 2;
    }

    const auto bootstrap = dmc3::VolumeBootstrapPolicy::plan(*present);
    if (!bootstrap.valid()) {
        std::cerr << "build-dmc3-overlay: invalid numbered-volume bootstrap\n";
        return 3;
    }
    if (!bootstrap.present_after_first_gap.empty()) {
        std::cerr
            << "build-dmc3-overlay: numbered volumes exist after the first gap; refusing to create a non-runtime-equivalent overlay\n";
        return 3;
    }
    if (!bootstrap.present_outside_runtime_index_domain.empty()) {
        std::cerr
            << "build-dmc3-overlay: out-of-domain numbered volumes are present; refusing ambiguous publication\n";
        return 3;
    }

    const auto lookup = dmc3::ResourceLookupPolicy::plan(game_request);
    if (!lookup.valid() || lookup.attempts.empty() ||
        lookup.attempts.front().provider != dmc3::ResourceProviderClass::archive) {
        std::cerr << "build-dmc3-overlay: invalid game resource request\n";
        return 4;
    }
    const auto member_path = lookup.attempts.front().candidate;

    auto authored_bytes = read_authored_resource(authored_file);
    if (!authored_bytes.has_value() || authored_bytes->empty()) {
        std::cerr << "build-dmc3-overlay: authored resource is not readable or is empty\n";
        return 5;
    }

    if (is_within(output_directory, data_directory)) {
        std::cerr
            << "build-dmc3-overlay: output directory must be outside the retail game data directory\n";
        return 6;
    }

    std::error_code error;
    std::filesystem::create_directories(output_directory, error);
    if (error || !std::filesystem::is_directory(output_directory, error) || error) {
        std::cerr << "build-dmc3-overlay: cannot create output directory\n";
        return 6;
    }

    const std::vector<dmc3::NbzOverlayMember> members{
        dmc3::NbzOverlayMember{
            .logical_path = member_path,
            .bytes = std::move(*authored_bytes),
        },
    };
    const auto built = dmc3::NbzStoreOverlayWriter::build(bootstrap, members);
    if (!built.ok()) {
        std::cerr << "build-dmc3-overlay: writer failed: "
                  << dmc3::to_string(built.status);
        if (!built.detail.empty()) {
            std::cerr << ": " << built.detail;
        }
        std::cerr << '\n';
        return 7;
    }

    const auto& receipt = *built.receipt;
    const auto output_path = output_directory / receipt.filename;
    const auto publication = dmc::rengine::core::publish_bytes_no_replace(
        output_path,
        std::span<const std::byte>{built.bytes.data(), built.bytes.size()},
        [&](const std::filesystem::path& staged_file) {
            return verify_overlay_file(
                staged_file,
                member_path,
                std::span<const std::byte>{members.front().bytes});
        });
    if (!publication.ok()) {
        std::cerr
            << "build-dmc3-overlay: staged validation/no-replace publication failed ("
            << dmc::rengine::core::to_string(publication.status) << "): "
            << publication.detail << ": " << output_path.string() << '\n';
        return publication.status ==
                dmc::rengine::core::NoReplacePublicationStatus::staging_validation_failed
            ? 9
            : 8;
    }

    std::cout << "DMC3 overlay artifact: ready\n"
              << "Executable directory: " << executable_directory.string() << '\n'
              << "Game data: " << data_directory.string() << '\n'
              << "Game request: " << game_request << '\n'
              << "Archive member: " << member_path << '\n'
              << "Volume index: " << receipt.volume_index << '\n'
              << "Output: " << output_path.string() << '\n'
              << "SHA-256: " << receipt.archive_sha256 << '\n'
              << "Bytes: " << receipt.archive_size << '\n'
              << "Compression: STORE (method 0)\n"
              << "Verification: pre-publication GDSpaces staging reopen + exact member bytes\n"
              << "Publication: atomic/no-replace output-only; retail game files were not modified\n";
    return 0;
}

} // namespace

void print_dmc3_overlay_help() {
    std::cout
        << "  build-dmc3-overlay <exe-dir> <game-request> <authored-file> <output-dir>\n"
        << "                            Build and verify the executable-relative next contiguous STORE NBZ without modifying retail files\n";
}

int try_run_dmc3_overlay_command(int argc, char** argv) {
    if (argc <= 1 || std::string_view{argv[1]} != "build-dmc3-overlay") {
        return -1;
    }
    if (argc != 6) {
        std::cerr
            << "build-dmc3-overlay: expected <exe-dir> <game-request> <authored-file> <output-dir>\n";
        return 1;
    }
    return run_build_overlay(
        std::filesystem::path{argv[2]},
        std::string_view{argv[3]},
        std::filesystem::path{argv[4]},
        std::filesystem::path{argv[5]});
}

} // namespace dmc::rengine::cli
