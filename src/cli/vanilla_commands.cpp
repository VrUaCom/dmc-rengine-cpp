#include "vanilla_commands.hpp"

#include "dmc_rengine/core/sha256.hpp"
#include "dmc_rengine/exe/pe_reader.hpp"
#include "dmc_rengine/gdspaces/local_directory_source.hpp"
#include "dmc_rengine/gdspaces/source_registry.hpp"
#include "dmc_rengine/profiles/dmc3/vanilla_runtime.hpp"
#include "dmc_rengine/profiles/dmc3/vanilla_static_tables.hpp"

#include <algorithm>
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
#include <system_error>
#include <utility>

namespace dmc::rengine::cli {
namespace {

[[nodiscard]] std::optional<gdspaces::ResourcePayload> load_local_file(
    const std::filesystem::path& input_path) {
    std::error_code error;
    const auto absolute = std::filesystem::absolute(input_path, error);
    if (error || !std::filesystem::is_regular_file(absolute, error) || error) {
        std::cerr << "inspect-vanilla-static: not a readable file: "
                  << input_path.string() << '\n';
        return std::nullopt;
    }

    const auto raw_size = std::filesystem::file_size(absolute, error);
    if (error || raw_size > std::numeric_limits<std::uint64_t>::max()) {
        std::cerr << "inspect-vanilla-static: file size is unsupported\n";
        return std::nullopt;
    }

    gdspaces::SourceRegistry sources;
    if (!sources.mount(std::make_unique<gdspaces::LocalDirectorySource>(
            "vanilla-static-exe", absolute.parent_path(), false))) {
        std::cerr << "inspect-vanilla-static: failed to mount parent directory\n";
        return std::nullopt;
    }

    const gdspaces::ResourceId resource{
        .source_id = "vanilla-static-exe",
        .logical_path = absolute.filename().generic_string(),
        .container_chain = {},
        .offset = 0U,
        .size = static_cast<std::uint64_t>(raw_size),
    };
    auto payload = sources.read(resource);
    if (!payload.has_value() || !payload->readable()) {
        std::cerr << "inspect-vanilla-static: GDSpaces could not read the executable\n";
        return std::nullopt;
    }
    return payload;
}

[[nodiscard]] std::string lower_extension(std::string_view name) {
    const auto slash = name.find_last_of("/\\");
    const auto dot = name.find_last_of('.');
    if (dot == std::string_view::npos ||
        (slash != std::string_view::npos && dot < slash)) {
        return {};
    }

    std::string result{name.substr(dot + 1U)};
    std::transform(
        result.begin(), result.end(), result.begin(),
        [](unsigned char value) {
            return static_cast<char>(std::tolower(value));
        });
    return result;
}

struct ExtensionCounts final {
    std::uint32_t pac{};
    std::uint32_t txt{};
    std::uint32_t adx{};
    std::uint32_t bin{};
    std::uint32_t sfd{};
    std::uint32_t other{};
};

[[nodiscard]] ExtensionCounts count_extensions(
    const profiles::dmc3::vanilla::MasterResourceCatalogReadResult& catalog) {
    ExtensionCounts counts;
    for (const auto& entry : catalog.names) {
        const auto extension = lower_extension(entry.logical_name);
        if (extension == "pac") {
            ++counts.pac;
        } else if (extension == "txt") {
            ++counts.txt;
        } else if (extension == "adx") {
            ++counts.adx;
        } else if (extension == "bin") {
            ++counts.bin;
        } else if (extension == "sfd") {
            ++counts.sfd;
        } else {
            ++counts.other;
        }
    }
    return counts;
}

int run_inspect_vanilla_static(
    const std::filesystem::path& executable_path,
    bool list_names,
    bool list_audio) {
    const auto payload = load_local_file(executable_path);
    if (!payload.has_value()) {
        return 2;
    }

    const auto bytes = std::span<const std::byte>{payload->bytes};
    const auto sha256 = core::Sha256::compute(bytes).hex();
    const auto canonical =
        sha256 == profiles::dmc3::vanilla::canonical_executable_sha256;
    const auto distribution =
        sha256 == profiles::dmc3::vanilla::distribution_executable_sha256;
    if (!canonical && !distribution) {
        std::cerr
            << "inspect-vanilla-static: executable hash is not one of the two cross-validated DMC3 artifacts; refusing to apply recovered table VAs\n"
            << "sha256=" << sha256 << '\n';
        return 3;
    }

    const auto pe = exe::PeReader::read(bytes);
    for (const auto& warning : pe.warnings) {
        std::cerr << "[warning] " << warning << '\n';
    }
    for (const auto& parse_error : pe.errors) {
        std::cerr << "[error] " << parse_error << '\n';
    }
    if (!pe.ok()) {
        return 4;
    }

    const auto master =
        profiles::dmc3::vanilla::VanillaStaticTableReader::read_master_resource_catalog(
            bytes, *pe.image);
    const auto audio =
        profiles::dmc3::vanilla::VanillaStaticTableReader::read_hd_audio_descriptors(
            bytes, *pe.image);

    for (const auto& diagnostic : master.diagnostics) {
        std::cerr << '[' << gdspaces::to_string(diagnostic.severity) << "] "
                  << diagnostic.code << ": " << diagnostic.message << '\n';
    }
    for (const auto& diagnostic : audio.diagnostics) {
        std::cerr << '[' << gdspaces::to_string(diagnostic.severity) << "] "
                  << diagnostic.code << ": " << diagnostic.message << '\n';
    }
    if (!master.complete() || !audio.complete()) {
        return 5;
    }

    const auto counts = count_extensions(master);
    std::uint32_t explicit_loops{};
    for (const auto& descriptor : audio.descriptors) {
        if (descriptor.has_explicit_loop()) {
            ++explicit_loops;
        }
    }

    std::cout
        << "DMC3 Vanilla Static Runtime Audit\n"
        << "sha256=" << sha256 << '\n'
        << "artifact=" << (canonical ? "canonical-unpacked" : "distribution-protected") << '\n'
        << "master_resource_names=" << master.names.size() << '\n'
        << "master_pac=" << counts.pac << '\n'
        << "master_txt=" << counts.txt << '\n'
        << "master_adx=" << counts.adx << '\n'
        << "master_bin=" << counts.bin << '\n'
        << "master_sfd=" << counts.sfd << '\n'
        << "master_other=" << counts.other << '\n'
        << "ogg_descriptors=" << audio.descriptors.size() << '\n'
        << "ogg_explicit_loop_descriptors=" << explicit_loops << '\n';

    if (list_names) {
        std::cout << "Master resource-name catalog:\n";
        for (const auto& entry : master.names) {
            std::cout << entry.index << '\t' << entry.logical_name << '\n';
        }
    }

    if (list_audio) {
        std::cout << "HD audio descriptors:\n";
        for (const auto& descriptor : audio.descriptors) {
            std::cout << descriptor.index << '\t' << descriptor.filename;
            if (descriptor.has_explicit_loop()) {
                std::cout << "\tloop_ms=" << descriptor.loop_start_ms
                          << ':' << descriptor.loop_end_ms;
            } else {
                std::cout << "\tloop_ms=none";
            }
            std::cout << '\n';
        }
    }

    return 0;
}

} // namespace

void print_vanilla_help() {
    std::cout
        << "  inspect-vanilla-static <dmc3.exe> [--list-names] [--list-audio]\n"
        << "                             Reproduce recovered master-catalog and HD-audio static tables read-only\n";
}

int try_run_vanilla_command(int argc, char** argv) {
    if (argc <= 1) {
        return -1;
    }

    const std::string_view command{argv[1]};
    if (command != "inspect-vanilla-static") {
        return -1;
    }
    if (argc < 3 || argc > 5) {
        std::cerr
            << "inspect-vanilla-static: usage: inspect-vanilla-static <dmc3.exe> [--list-names] [--list-audio]\n";
        return 1;
    }

    bool list_names = false;
    bool list_audio = false;
    for (int index = 3; index < argc; ++index) {
        const std::string_view option{argv[index]};
        if (option == "--list-names") {
            list_names = true;
        } else if (option == "--list-audio") {
            list_audio = true;
        } else {
            std::cerr << "inspect-vanilla-static: unknown option: "
                      << option << '\n';
            return 1;
        }
    }

    return run_inspect_vanilla_static(
        std::filesystem::path{argv[2]}, list_names, list_audio);
}

} // namespace dmc::rengine::cli
