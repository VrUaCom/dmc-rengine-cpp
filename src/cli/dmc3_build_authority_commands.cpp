#include "dmc3_build_authority_commands.hpp"

#include "dmc_rengine/core/sha256.hpp"
#include "dmc_rengine/gdspaces/local_directory_source.hpp"
#include "dmc_rengine/gdspaces/source_registry.hpp"
#include "dmc_rengine/profiles/dmc3/executable_authority.hpp"

#include <cstdint>
#include <filesystem>
#include <iostream>
#include <memory>
#include <span>
#include <string>
#include <string_view>

namespace dmc::rengine::cli {
namespace {

namespace gdspaces = dmc::rengine::gdspaces;
namespace dmc3 = dmc::rengine::profiles::dmc3;

int run_preflight_dmc3_game_test(const std::filesystem::path& executable_directory) {
    std::error_code error;
    const auto executable_path = executable_directory / "dmc3.exe";
    if (!std::filesystem::is_regular_file(executable_path, error) || error) {
        std::cerr << "preflight-dmc3-game-test: dmc3.exe not found: "
                  << executable_path.string() << '\n';
        return 2;
    }

    const auto file_size = std::filesystem::file_size(executable_path, error);
    if (error) {
        std::cerr << "preflight-dmc3-game-test: cannot read dmc3.exe size\n";
        return 2;
    }

    constexpr std::string_view source_id = "dmc3-game-test-executable";
    gdspaces::SourceRegistry registry;
    if (!registry.mount(std::make_unique<gdspaces::LocalDirectorySource>(
            std::string{source_id}, executable_directory, false))) {
        std::cerr << "preflight-dmc3-game-test: failed to mount executable directory\n";
        return 3;
    }

    const gdspaces::ResourceId executable_id{
        .source_id = std::string{source_id},
        .logical_path = "dmc3.exe",
        .container_chain = {},
        .offset = 0U,
        .size = file_size,
    };
    const auto payload = registry.read(executable_id);
    if (!payload.has_value() || !payload->readable() ||
        payload->bytes.size() != file_size) {
        std::cerr << "preflight-dmc3-game-test: GDSpaces could not materialize dmc3.exe\n";
        return 3;
    }

    const auto digest = dmc::rengine::core::Sha256::compute(
        std::span<const std::byte>{payload->bytes});
    const auto digest_hex = digest.hex();
    const auto match = dmc3::classify_executable_authority(digest_hex, file_size);

    std::cout << "DMC3 game-test executable preflight\n"
              << "Executable: " << executable_path.string() << '\n'
              << "SHA-256: " << digest_hex << '\n'
              << "Bytes: " << file_size << '\n'
              << "Authority match: " << dmc3::to_string(match.status) << '\n';

    if (match.authority != nullptr) {
        std::cout << "Authority ID: " << match.authority->id << '\n'
                  << "Authority role: " << dmc3::to_string(match.authority->role) << '\n'
                  << "Instruction reverse authority: "
                  << (match.authority->instruction_reverse_authority ? "yes" : "no") << '\n'
                  << "Distribution provenance authority: "
                  << (match.authority->distribution_provenance_authority ? "yes" : "no") << '\n'
                  << "Original execution candidate: "
                  << (match.authority->original_execution_candidate ? "yes" : "no") << '\n';
    }

    if (!match.recognized()) {
        std::cerr
            << "preflight-dmc3-game-test: executable is not an exact recognized DMC3 authority; refusing original-game acceptance promotion\n";
        return 4;
    }

    if (!match.authority->original_execution_candidate ||
        !match.authority->distribution_provenance_authority) {
        std::cerr
            << "preflight-dmc3-game-test: recognized executable is analysis-only and must not be used as the protected distribution game-test authority\n";
        return 5;
    }

    const auto data_directory = executable_directory / "data" / "dmc3";
    if (!std::filesystem::is_directory(data_directory, error) || error) {
        std::cerr
            << "preflight-dmc3-game-test: distribution executable recognized, but executable-relative data/dmc3 is missing\n";
        return 6;
    }

    const auto first_volume = data_directory / "dmc3-0.nbz";
    const auto alternate_case_volume = data_directory / "DMC3-0.nbz";
    error.clear();
    const bool lower_exists =
        std::filesystem::is_regular_file(first_volume, error) && !error;
    error.clear();
    const bool upper_exists =
        std::filesystem::is_regular_file(alternate_case_volume, error) && !error;
    if (!lower_exists && !upper_exists) {
        std::cerr
            << "preflight-dmc3-game-test: protected distribution executable recognized, but contiguous archive bootstrap cannot start because DMC3-0.nbz is missing\n";
        return 6;
    }

    std::cout
        << "Game-test preflight: READY\n"
        << "Build authority: protected distribution provenance\n"
        << "Analysis authority: separate e454 unpacked/decrypted target; no cryptographic EXE equivalence is claimed\n"
        << "Archive authority: executable-relative DMC3-0.nbz present; co-location only until archive hash/member evidence is recorded\n";
    return 0;
}

} // namespace

void print_dmc3_build_authority_help() {
    std::cout
        << "  preflight-dmc3-game-test <exe-dir>\n"
        << "                            Verify the protected distribution EXE authority before an original-game acceptance run\n";
}

int try_run_dmc3_build_authority_command(int argc, char** argv) {
    if (argc <= 1 || std::string_view{argv[1]} != "preflight-dmc3-game-test") {
        return -1;
    }
    if (argc != 3) {
        std::cerr << "preflight-dmc3-game-test: expected <exe-dir>\n";
        return 1;
    }
    return run_preflight_dmc3_game_test(std::filesystem::path{argv[2]});
}

} // namespace dmc::rengine::cli
