#pragma once

#include "dmc_rengine/core/no_replace_publication.hpp"
#include "dmc_rengine/spider/exe_window_acquirer.hpp"
#include "dmc_rengine/spider/exe_window_packet_publication.hpp"
#include "dmc_rengine/spider/l2_runtime_mapping.hpp"

#include <cstddef>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <iterator>
#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace dmc::rengine::cli {
namespace spider_cli_detail {

struct PacketSource final {
    std::filesystem::path path;
    std::string expected_sha;
    std::unique_ptr<spider::NativeExeWindowSource> source;
};

inline spider::ExeWindowAcquisition acquire_packet_window(
    void* context, const spider::ExeWindowRequest& request) {
    auto& state = *static_cast<PacketSource*>(context);
    if (!state.source) {
        std::string error;
        state.source = spider::NativeExeWindowSource::open(
            state.path, state.expected_sha, error);
        if (!state.source) {
            return {.receipt = std::nullopt, .error = std::move(error)};
        }
    }
    return state.source->acquire(request);
}

[[nodiscard]] inline std::optional<std::string> read_text_file(
    const std::filesystem::path& path) {
    std::ifstream stream(path, std::ios::binary);
    if (!stream.is_open()) {
        return std::nullopt;
    }
    std::string text{
        std::istreambuf_iterator<char>{stream},
        std::istreambuf_iterator<char>{}};
    if (!stream.good() && !stream.eof()) {
        return std::nullopt;
    }
    return text;
}

inline int run_extract_exe_window_packet(int argc, char** argv) {
    std::optional<std::filesystem::path> plan_path;
    std::optional<std::filesystem::path> exe_path;
    std::optional<std::filesystem::path> output_path;
    std::optional<std::string> expected_sha;
    bool validate_only = false;
    for (int index = 2; index < argc; ++index) {
        const std::string_view option{argv[index]};
        if (option == "--validate-plan-only" && !validate_only) {
            validate_only = true;
            continue;
        }
        if (option != "--plan" && option != "--exe" &&
            option != "--output" && option != "--expected-sha256") {
            std::cerr << "extract-exe-window-packet: unsupported option: " << option;
            if (option == "--hex") {
                std::cerr << "; raw-byte packets still require the Python command";
            }
            std::cerr << '\n';
            return 2;
        }
        if (index + 1 >= argc || std::string_view{argv[index + 1]}.starts_with("--")) {
            std::cerr << "extract-exe-window-packet: missing value for " << option << '\n';
            return 2;
        }
        const std::string value{argv[++index]};
        if (value.empty() ||
            (option == "--plan" && plan_path) ||
            (option == "--exe" && exe_path) ||
            (option == "--output" && output_path) ||
            (option == "--expected-sha256" && expected_sha)) {
            std::cerr << "extract-exe-window-packet: empty or repeated option " << option << '\n';
            return 2;
        }
        if (option == "--plan") plan_path = value;
        else if (option == "--exe") exe_path = value;
        else if (option == "--output") output_path = value;
        else expected_sha = value;
    }
    if (!plan_path || (!validate_only && (!exe_path || !output_path || !expected_sha))) {
        std::cerr << "Usage: dmc-rengine extract-exe-window-packet --plan <json> "
                     "[--validate-plan-only | --exe <exe> --expected-sha256 <sha> --output <new-dir>]\n";
        return 2;
    }
    const auto plan = read_text_file(*plan_path);
    if (!plan) {
        std::cerr << "extract-exe-window-packet: cannot read plan\n";
        return 2;
    }
    if (validate_only) {
        const auto compiled = spider::compile_exe_window_packet(*plan);
        if (!compiled.ok()) {
            for (const auto& error : compiled.errors) {
                std::cerr << "plan error: " << error << '\n';
            }
            return 2;
        }
        std::cout << spider::exe_window_packet_summary_to_json(compiled.program->summary);
        return 0;
    }

    // Lazy acquisition lets the workflow reject a bad plan/authority before
    // touching the executable. All windows reuse one immutable GDSpaces/PE source.
    PacketSource source{.path = *exe_path, .expected_sha = *expected_sha, .source = {}};
    const auto published = spider::publish_exe_window_packet(
        *plan, *expected_sha, *output_path, &acquire_packet_window, &source);
    if (!published.ok()) {
        std::cerr << "EXE packet rejected: " << published.message << '\n';
        if (published.error == spider::ExeWindowPacketPublicationError::invalid_plan) return 2;
        if (published.execution_error == spider::ExeWindowPacketError::expected_sha_mismatch) return 3;
        if (published.execution_error == spider::ExeWindowPacketError::known_body_mismatch) return 6;
        if (published.error == spider::ExeWindowPacketPublicationError::execution_failed) return 5;
        return 4;
    }
    std::cout << published.receipt_path.string() << '\n';
    return 0;
}

inline int run_verify_l2_runtime_mapping_v1(int argc, char** argv) {
    std::vector<std::filesystem::path> receipt_paths;
    std::optional<std::filesystem::path> output_path;

    for (int index = 2; index < argc; ++index) {
        const std::string_view option{argv[index]};
        if (option == "--receipt") {
            if (index + 1 >= argc) {
                std::cerr << "verify-l2-runtime-mapping-v1: --receipt requires a path\n";
                return 2;
            }
            receipt_paths.emplace_back(argv[++index]);
            continue;
        }
        if (option == "--output") {
            if (index + 1 >= argc) {
                std::cerr << "verify-l2-runtime-mapping-v1: --output requires a path\n";
                return 2;
            }
            output_path = std::filesystem::path{argv[++index]};
            continue;
        }
        std::cerr << "verify-l2-runtime-mapping-v1: unknown option: "
                  << option << '\n';
        return 2;
    }

    if (receipt_paths.empty() || !output_path.has_value()) {
        std::cerr
            << "Usage: dmc-rengine verify-l2-runtime-mapping-v1 "
               "--receipt <receipt.json> [--receipt <receipt.json> ...] "
               "--output <packet.json>\n";
        return 2;
    }

    std::vector<std::string> receipt_texts;
    receipt_texts.reserve(receipt_paths.size());
    for (const auto& path : receipt_paths) {
        auto text = read_text_file(path);
        if (!text.has_value()) {
            std::cerr << "runtime mapping packet rejected: could not read JSON receipt "
                      << path.string() << '\n';
            return 2;
        }
        receipt_texts.push_back(std::move(*text));
    }

    std::vector<std::string_view> receipt_views;
    receipt_views.reserve(receipt_texts.size());
    for (const auto& text : receipt_texts) {
        receipt_views.emplace_back(text);
    }

    const auto built = spider::build_l2_runtime_mapping_v1(receipt_views);
    if (!built.ok()) {
        std::cerr << "runtime mapping packet rejected:";
        for (const auto& error : built.errors) {
            std::cerr << "\n  - " << error;
        }
        std::cerr << '\n';
        return 2;
    }

    const auto encoded = spider::l2_runtime_mapping_v1_to_json(*built.packet);
    if (encoded.empty()) {
        std::cerr << "runtime mapping packet rejected: native serializer failed\n";
        return 2;
    }

    const auto bytes = std::as_bytes(std::span<const char>{encoded.data(), encoded.size()});
    const auto publication = core::publish_bytes_no_replace(
        *output_path,
        bytes,
        {},
        ".dmc-rengine-l2-mapping-v1.staging");
    if (!publication.ok()) {
        std::cerr << "runtime mapping packet rejected: " << publication.detail << '\n';
        return 2;
    }

    std::cout << encoded;
    return 0;
}

} // namespace spider_cli_detail

inline void print_spider_help() {
    std::cout
        << "  extract-exe-window-packet --plan <json> --validate-plan-only\n"
        << "  extract-exe-window-packet --plan <json> --exe <exe> --expected-sha256 <sha> --output <new-dir>\n"
        << "                             Acquire a metadata-only packet in one native process\n"
        << "  verify-l2-runtime-mapping-v1 --receipt <json>... --output <json>\n"
        << "                             Validate bounded L2 mapping receipts natively\n";
}

inline int try_run_spider_command(int argc, char** argv) {
    if (argc <= 1) {
        return -1;
    }
    const std::string_view command{argv[1]};
    if (command == "extract-exe-window-packet") {
        return spider_cli_detail::run_extract_exe_window_packet(argc, argv);
    }
    if (command == "verify-l2-runtime-mapping-v1") {
        return spider_cli_detail::run_verify_l2_runtime_mapping_v1(argc, argv);
    }
    return -1;
}

} // namespace dmc::rengine::cli
