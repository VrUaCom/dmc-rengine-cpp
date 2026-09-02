#include "dmc3_retail_acquisition_commands.hpp"

#include "dmc_rengine/core/no_replace_publication.hpp"
#include "dmc_rengine/core/sha256.hpp"
#include "dmc_rengine/evidence/artifact.hpp"
#include "dmc_rengine/gdspaces/local_directory_source.hpp"
#include "dmc_rengine/gdspaces/nbz_zip_artifact_binding.hpp"
#include "dmc_rengine/gdspaces/nbz_zip_artifact_member.hpp"
#include "dmc_rengine/gdspaces/nbz_zip_source.hpp"
#include "dmc_rengine/gdspaces/source_registry.hpp"
#include "dmc_rengine/profiles/dmc3/runtime_resource_resolver.hpp"
#include "dmc_rengine/profiles/dmc3/retail_member_acquisition.hpp"
#include "dmc_rengine/profiles/dmc3/volume_bootstrap_policy.hpp"

#include <algorithm>
#include <charconv>
#include <cctype>
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <limits>
#include <memory>
#include <optional>
#include <span>
#include <sstream>
#include <string>
#include <string_view>
#include <vector>

namespace dmc::rengine::cli {
namespace {

namespace core = dmc::rengine::core;
namespace evidence = dmc::rengine::evidence;
namespace gdspaces = dmc::rengine::gdspaces;
namespace dmc3 = dmc::rengine::profiles::dmc3;

#ifdef _WIN32
// Only the Windows path comparison needs this now that discovery lives in the
// library, and an unguarded definition is dead code everywhere else.
[[nodiscard]] std::string lower_ascii(std::string value) {
    std::transform(value.begin(), value.end(), value.begin(), [](unsigned char ch) {
        return static_cast<char>(std::tolower(ch));
    });
    return value;
}
#endif

[[nodiscard]] std::filesystem::path normalized_absolute(
    const std::filesystem::path& path) {
    std::error_code error;
    auto absolute = std::filesystem::absolute(path, error);
    if (error) {
        return path.lexically_normal();
    }
    auto normalized = std::filesystem::weakly_canonical(absolute, error);
    return error ? absolute.lexically_normal() : normalized;
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

[[nodiscard]] std::filesystem::path receipt_path_for(
    const std::filesystem::path& output_file) {
    return std::filesystem::path{output_file.string() + ".receipt.json"};
}

} // namespace

int run_extract_dmc3_retail_member(
    const std::filesystem::path& executable_directory,
    std::string_view game_request,
    const std::filesystem::path& output_file) {
    const auto data_directory = executable_directory /
        std::filesystem::path{dmc3::VolumeBootstrapPolicy::data_subdirectory()};
    const auto receipt_path = receipt_path_for(output_file);

    // Where the artifacts may land is this command's concern; what the
    // acquisition is belongs to the library.
    if (output_file.empty() || output_file.filename().empty() ||
        is_within(output_file, data_directory) ||
        is_within(receipt_path, data_directory)) {
        std::cerr
            << "extract-dmc3-retail-member: output and receipt must be outside the retail game data tree\n";
        return 2;
    }

    const auto acquisition = dmc3::RetailMemberAcquisition::acquire(
        executable_directory, game_request);
    if (!acquisition.ok()) {
        std::cerr << "extract-dmc3-retail-member: "
                  << dmc3::to_string(acquisition.status);
        if (!acquisition.detail.empty()) {
            std::cerr << ": " << acquisition.detail;
        }
        std::cerr << '\n';
        return 3;
    }
    const auto& receipt = *acquisition.receipt;

    std::error_code error;
    auto output_parent = output_file.parent_path();
    if (output_parent.empty()) {
        output_parent = std::filesystem::current_path(error);
        if (error) {
            std::cerr
                << "extract-dmc3-retail-member: cannot resolve output parent directory\n";
            return 12;
        }
    }
    std::filesystem::create_directories(output_parent, error);
    if (error || !std::filesystem::is_directory(output_parent, error) || error) {
        std::cerr
            << "extract-dmc3-retail-member: cannot create output parent directory\n";
        return 12;
    }

    const auto output_publication = core::publish_bytes_no_replace(
        output_file,
        std::span<const std::byte>{acquisition.bytes},
        {},
        ".dmc-rengine-retail-member.staging");
    if (!output_publication.ok()) {
        std::cerr
            << "extract-dmc3-retail-member: extracted output publication failed ("
            << core::to_string(output_publication.status) << "): "
            << output_publication.detail << '\n';
        return 13;
    }

    const auto receipt_text =
        dmc3::RetailMemberAcquisition::receipt_json(receipt, output_file);
    const auto receipt_bytes = std::as_bytes(std::span<const char>{
        receipt_text.data(), receipt_text.size()});
    const auto receipt_publication = core::publish_bytes_no_replace(
        receipt_path,
        receipt_bytes,
        {},
        ".dmc-rengine-retail-receipt.staging");
    if (!receipt_publication.ok()) {
        std::cerr
            << "extract-dmc3-retail-member: receipt publication failed after output publication; output was left intact to avoid destructive ownership races ("
            << core::to_string(receipt_publication.status) << "): "
            << receipt_publication.detail << '\n';
        return 14;
    }

    std::cout
        << "DMC3 retail member acquisition: VERIFIED\n"
        << "Game request: " << receipt.game_request << '\n'
        << "Selected volume: " << receipt.selected_volume_index << '\n'
        << "Archive: " << receipt.archive_path.string() << '\n'
        << "Archive SHA-256: " << receipt.archive_sha256 << '\n'
        << "Member: " << receipt.entry.logical_path << '\n'
        << "Member SHA-256: " << receipt.materialized_sha256 << '\n'
        << "Materialized bytes: " << receipt.materialized_size << '\n'
        << "Transform: " << gdspaces::to_string(receipt.provenance.transform) << '\n'
        << "Ignored after first gap: " << receipt.ignored_after_first_gap << '\n'
        << "Ignored outside runtime domain: "
        << receipt.ignored_outside_runtime_domain << '\n'
        << "Output: " << output_file.string() << '\n'
        << "Receipt: " << receipt_path.string() << '\n';
    return 0;
}

void print_dmc3_retail_acquisition_help() {
    std::cout
        << "  extract-dmc3-retail-member <exe-dir> <game-request> <output-file>\n"
        << "                            Resolve and artifact-bind one exact member from runtime-contiguous retail DMC3 NBZ volumes with SHA/provenance receipt\n";
}

int try_run_dmc3_retail_acquisition_command(int argc, char** argv) {
    if (argc <= 1 || std::string_view{argv[1]} != "extract-dmc3-retail-member") {
        return -1;
    }
    if (argc != 5) {
        std::cerr
            << "extract-dmc3-retail-member: expected <exe-dir> <game-request> <output-file>\n";
        return 1;
    }
    return run_extract_dmc3_retail_member(
        std::filesystem::path{argv[2]},
        std::string_view{argv[3]},
        std::filesystem::path{argv[4]});
}

} // namespace dmc::rengine::cli
