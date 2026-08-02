#include "dmc_rengine/core/version.hpp"
#include "dmc_rengine/exe/pe_reader.hpp"
#include "dmc_rengine/gdspaces/local_directory_source.hpp"
#include "dmc_rengine/gdspaces/open_router.hpp"
#include "dmc_rengine/gdspaces/source_registry.hpp"

#include <cstdint>
#include <filesystem>
#include <iomanip>
#include <iostream>
#include <limits>
#include <memory>
#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <utility>

namespace {

using dmc::rengine::gdspaces::LocalDirectorySource;
using dmc::rengine::gdspaces::OpenRequest;
using dmc::rengine::gdspaces::OpenRouter;
using dmc::rengine::gdspaces::ResourceId;
using dmc::rengine::gdspaces::ResourceRef;
using dmc::rengine::gdspaces::SourceRegistry;

void print_help() {
    std::cout
        << "DMC Rengine commands:\n"
        << "  version | --version       Print the project version\n"
        << "  doctor                    Print architecture invariants\n"
        << "  scan <directory>          Mount and enumerate a local directory\n"
        << "  route <format>            Show the default tool route for a format\n"
        << "  inspect-exe <path>        Inspect a PE file through GDSpaces\n"
        << "  help | --help             Show this help\n";
}

int run_doctor() {
    std::cout
        << "DMC Rengine " << dmc::rengine::version() << '\n'
        << dmc::rengine::architecture_name() << '\n'
        << "[ok] GDSpaces is the only resource access API.\n"
        << "[ok] Editors receive ResourceRef/ResourcePayload contracts.\n"
        << "[ok] Reverse claims use confidence and evidence records.\n"
        << "[ok] Original game files remain outside the repository.\n"
        << "[ok] Writes require future working-copy and validation contracts.\n";
    return 0;
}

int run_scan(const std::filesystem::path& root) {
    std::error_code error;
    if (!std::filesystem::is_directory(root, error) || error) {
        std::cerr << "scan: not a readable directory: " << root.string() << '\n';
        return 2;
    }

    SourceRegistry registry;
    if (!registry.mount(std::make_unique<LocalDirectorySource>(
            "local-scan", root, true))) {
        std::cerr << "scan: failed to mount directory\n";
        return 3;
    }

    const auto resources = registry.enumerate_all();
    std::cout << "Mounted source: " << root.string() << '\n'
              << "Resources: " << resources.size() << '\n';

    for (const auto& resource : resources) {
        std::cout << "- [" << resource.format << "] "
                  << resource.id.canonical();
        if (resource.container) {
            std::cout << " (container)";
        }
        std::cout << '\n';
    }

    return 0;
}

int run_route(std::string format) {
    const ResourceRef resource{
        .id = ResourceId{
            .source_id = "route-preview",
            .logical_path = "preview/resource." + format,
            .container_chain = {},
            .offset = 0,
            .size = 0,
        },
        .display_name = "resource." + format,
        .format = std::move(format),
        .profile = "unknown",
        .synthetic_name = true,
        .container = false,
    };

    const OpenRouter router;
    const OpenRequest request{
        .resource = resource,
        .preferred_target = std::nullopt,
        .stage_context = false,
        .menu_context = false,
        .evidence_context = true,
    };

    std::cout << dmc::rengine::gdspaces::to_string(router.route(request)) << '\n';
    return 0;
}

int run_inspect_exe(const std::filesystem::path& input_path) {
    std::error_code error;
    const auto absolute = std::filesystem::absolute(input_path, error);
    if (error || !std::filesystem::is_regular_file(absolute, error) || error) {
        std::cerr << "inspect-exe: not a readable file: "
                  << input_path.string() << '\n';
        return 2;
    }

    const auto raw_size = std::filesystem::file_size(absolute, error);
    if (error || raw_size > std::numeric_limits<std::uint64_t>::max()) {
        std::cerr << "inspect-exe: file size is unsupported\n";
        return 3;
    }

    const auto parent = absolute.parent_path();
    SourceRegistry registry;
    if (!registry.mount(std::make_unique<LocalDirectorySource>(
            "exe-inspect", parent, false))) {
        std::cerr << "inspect-exe: failed to mount parent directory\n";
        return 4;
    }

    const ResourceId resource{
        .source_id = "exe-inspect",
        .logical_path = absolute.filename().generic_string(),
        .container_chain = {},
        .offset = 0,
        .size = static_cast<std::uint64_t>(raw_size),
    };

    const auto payload = registry.read(resource);
    if (!payload.has_value() || !payload->readable()) {
        std::cerr << "inspect-exe: GDSpaces could not read the resource\n";
        if (payload.has_value()) {
            for (const auto& diagnostic : payload->diagnostics) {
                std::cerr << "  ["
                          << dmc::rengine::gdspaces::to_string(diagnostic.severity)
                          << "] " << diagnostic.code << ": "
                          << diagnostic.message << '\n';
            }
        }
        return 5;
    }

    const auto result = dmc::rengine::exe::PeReader::read(
        std::span<const std::byte>{payload->bytes});
    for (const auto& warning : result.warnings) {
        std::cerr << "[warning] " << warning << '\n';
    }
    for (const auto& parse_error : result.errors) {
        std::cerr << "[error] " << parse_error << '\n';
    }

    if (!result.ok()) {
        return 6;
    }

    const auto& image = *result.image;
    std::cout << "Format: " << dmc::rengine::exe::to_string(image.kind) << '\n'
              << "Machine: " << dmc::rengine::exe::to_string(image.machine) << '\n'
              << "Sections: " << image.section_count << '\n'
              << "ImageBase: 0x" << std::hex << image.image_base << '\n'
              << "EntryPoint RVA: 0x" << image.entry_point_rva << '\n'
              << "SizeOfImage: 0x" << image.size_of_image << '\n'
              << "SizeOfHeaders: 0x" << image.size_of_headers << '\n'
              << "Subsystem: 0x" << image.subsystem << std::dec << '\n';

    for (const auto& section : image.sections) {
        std::cout << "- " << section.name
                  << " RVA=0x" << std::hex << section.virtual_address
                  << " VS=0x" << section.virtual_size
                  << " RAW=0x" << section.raw_offset
                  << "+0x" << section.raw_size
                  << " CH=0x" << section.characteristics
                  << std::dec << '\n';
    }

    return 0;
}

} // namespace

int main(int argc, char** argv) {
    if (argc <= 1) {
        print_help();
        return 0;
    }

    const std::string_view command{argv[1]};
    if (command == "version" || command == "--version") {
        std::cout << "DMC Rengine " << dmc::rengine::version() << '\n';
        return 0;
    }

    if (command == "doctor") {
        return run_doctor();
    }

    if (command == "scan") {
        if (argc < 3) {
            std::cerr << "scan: missing directory\n";
            return 1;
        }
        return run_scan(std::filesystem::path{argv[2]});
    }

    if (command == "route") {
        if (argc < 3) {
            std::cerr << "route: missing format\n";
            return 1;
        }
        return run_route(std::string{argv[2]});
    }

    if (command == "inspect-exe") {
        if (argc < 3) {
            std::cerr << "inspect-exe: missing path\n";
            return 1;
        }
        return run_inspect_exe(std::filesystem::path{argv[2]});
    }

    if (command == "help" || command == "--help" || command == "-h") {
        print_help();
        return 0;
    }

    std::cerr << "Unknown command: " << command << '\n';
    print_help();
    return 1;
}
