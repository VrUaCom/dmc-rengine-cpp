#include "dmc_rengine/core/version.hpp"
#include "dmc_rengine/gdspaces/local_directory_source.hpp"
#include "dmc_rengine/gdspaces/open_router.hpp"
#include "dmc_rengine/gdspaces/source_registry.hpp"

#include <filesystem>
#include <iostream>
#include <memory>
#include <string>
#include <string_view>

namespace {

using dmc::rengine::gdspaces::LocalDirectorySource;
using dmc::rengine::gdspaces::OpenRequest;
using dmc::rengine::gdspaces::OpenRouter;
using dmc::rengine::gdspaces::ResourceId;
using dmc::rengine::gdspaces::ResourceRef;
using dmc::rengine::gdspaces::SourceRegistry;
using dmc::rengine::gdspaces::to_string;

void print_help() {
    std::cout
        << "DMC Rengine commands:\n"
        << "  version | --version       Print the project version\n"
        << "  doctor                    Print architecture invariants\n"
        << "  scan <directory>          Mount and enumerate a local directory\n"
        << "  route <format>            Show the default tool route for a format\n"
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

    std::cout << to_string(router.route(request)) << '\n';
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

    if (command == "help" || command == "--help" || command == "-h") {
        print_help();
        return 0;
    }

    std::cerr << "Unknown command: " << command << '\n';
    print_help();
    return 1;
}
