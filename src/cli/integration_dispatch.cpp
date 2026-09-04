#include "integration_commands.hpp"
#include "hits_commands.hpp"
#include "dmc3_build_authority_commands.hpp"
#include "dmc3_l1_closure_commands.hpp"
#include "dmc3_overlay_commands.hpp"
#include "dmc3_retail_acquisition_commands.hpp"
#include "nbz_copy_commands.hpp"
#include "relative_slot_commands.hpp"
#include "scm_corpus_commands.hpp"

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
    print_dmc3_build_authority_help();
    print_dmc3_l1_closure_help();
    print_dmc3_overlay_help();
    print_dmc3_retail_acquisition_help();
    print_nbz_copy_help();
    print_relative_slot_help();
    print_hits_help();
    print_scm_corpus_help();
}

int try_run_integration_command(int argc, char** argv) {
    const auto build_authority_result =
        try_run_dmc3_build_authority_command(argc, argv);
    if (build_authority_result != -1) {
        return build_authority_result;
    }

    const auto l1_closure_result = try_run_dmc3_l1_closure_command(argc, argv);
    if (l1_closure_result != -1) {
        return l1_closure_result;
    }

    const auto overlay_result = try_run_dmc3_overlay_command(argc, argv);
    if (overlay_result != -1) {
        return overlay_result;
    }

    const auto retail_acquisition_result =
        try_run_dmc3_retail_acquisition_command(argc, argv);
    if (retail_acquisition_result != -1) {
        return retail_acquisition_result;
    }

    const auto nbz_copy_result = try_run_nbz_copy_command(argc, argv);
    if (nbz_copy_result != -1) {
        return nbz_copy_result;
    }

    const auto relative_slot_result = try_run_relative_slot_command(argc, argv);
    if (relative_slot_result != -1) {
        return relative_slot_result;
    }

    const auto hits_result = try_run_hits_command(argc, argv);
    if (hits_result != -1) {
        return hits_result;
    }

    const auto scm_corpus_result = try_run_scm_corpus_command(argc, argv);
    if (scm_corpus_result != -1) {
        return scm_corpus_result;
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
