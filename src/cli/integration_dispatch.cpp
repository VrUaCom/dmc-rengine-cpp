#include "integration_commands.hpp"
#include "hits_commands.hpp"

#include <filesystem>
#include <iostream>
#include <string_view>

namespace dmc::rengine::cli {

void print_integration_help() {
    std::cout
        << "  list-tools                 List canonical tools, lore names, and capabilities\n"
        << "  list-formats               List format maturity, parser, and write policies\n"
        << "  integration-status         Summarize the cross-tool integration layer\n"
        << "  inspect-workspace <path> [--stage] [--menu]\n"
        << "                             Build a GDSpaces workspace manifest for a local resource\n";
    print_hits_help();
}

int try_run_integration_command(int argc, char** argv) {
    const auto hits_result = try_run_hits_command(argc, argv);
    if (hits_result != -1) {
        return hits_result;
    }
    if (argc <= 1) {
        return -1;
    }

    const std::string_view command{argv[1]};
    if (command == "list-tools") {
        return run_list_tools();
    }
    if (command == "list-formats") {
        return run_list_formats();
    }
    if (command == "integration-status") {
        return run_integration_status();
    }
    if (command == "inspect-workspace") {
        if (argc < 3) {
            std::cerr << "inspect-workspace: missing path\n";
            return 1;
        }

        bool stage_context = false;
        bool menu_context = false;
        for (int index = 3; index < argc; ++index) {
            const std::string_view option{argv[index]};
            if (option == "--stage") {
                stage_context = true;
            } else if (option == "--menu") {
                menu_context = true;
            } else {
                std::cerr << "inspect-workspace: unknown option: "
                          << option << '\n';
                return 1;
            }
        }
        return run_inspect_workspace(
            std::filesystem::path{argv[2]},
            stage_context,
            menu_context);
    }
    return -1;
}

} // namespace dmc::rengine::cli
