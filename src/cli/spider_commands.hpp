#pragma once

#include "dmc_rengine/core/no_replace_publication.hpp"
#include "dmc_rengine/spider/exe_window_acquirer.hpp"
#include "dmc_rengine/spider/exe_window_packet_publication.hpp"
#include "dmc_rengine/spider/l2_original_selection.hpp"
#include "dmc_rengine/spider/l2_runtime_mapping.hpp"
#include "dmc_rengine/spider/l2_runtime_mapping_v2.hpp"

#include <algorithm>
#include <array>
#include <cstddef>
#include <map>
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

/// Writes `json` (plus a newline) to a new file and echoes it, as the Python scripts did.
inline int publish_python_json(const std::filesystem::path& output, const std::string& json,
                               std::string_view rejected, std::string_view suffix) {
    const auto encoded = json + "\n";
    const auto publication = core::publish_bytes_no_replace(
        output, std::as_bytes(std::span<const char>{encoded.data(), encoded.size()}), {}, suffix);
    if (!publication.ok()) {
        std::cerr << rejected << ": " << publication.detail << '\n';
        return 2;
    }
    std::cout << encoded;
    return 0;
}

/// Reads `--name value` pairs; repeatable names collect every value.
struct OptionValues final {
    std::map<std::string, std::vector<std::string>, std::less<>> values;
    std::string error;
};

inline OptionValues read_options(int argc, char** argv, std::span<const std::string_view> known) {
    OptionValues options;
    for (int index = 2; index < argc; ++index) {
        const std::string_view option{argv[index]};
        if (std::find(known.begin(), known.end(), option) == known.end()) {
            options.error = "unrecognized argument: " + std::string{option};
            return options;
        }
        if (index + 1 >= argc) {
            options.error = std::string{option} + " requires a value";
            return options;
        }
        options.values[std::string{option}].emplace_back(argv[++index]);
    }
    return options;
}

inline int run_normalize_l2_selection_candidate(int argc, char** argv) {
    constexpr std::array<std::string_view, 2> known{"--input", "--output"};
    const auto options = read_options(argc, argv, known);
    const auto one = [&options](std::string_view name) -> const std::string* {
        const auto found = options.values.find(name);
        return found == options.values.end() || found->second.size() != 1U ? nullptr : &found->second.front();
    };
    if (!options.error.empty() || one("--input") == nullptr || one("--output") == nullptr) {
        std::cerr << "Usage: dmc-rengine normalize-l2-original-selection-candidate --input <legacy.json> "
                     "--output <candidate.json>\n";
        if (!options.error.empty()) std::cerr << options.error << '\n';
        return 2;
    }
    constexpr std::string_view rejected = "selection candidate normalization rejected";
    const auto text = read_text_file(*one("--input"));
    if (!text.has_value()) {
        std::cerr << rejected << ": could not read legacy selection JSON: cannot open " << *one("--input") << '\n';
        return 2;
    }
    const auto normalized = spider::normalize_l2_selection_candidate(*text);
    if (!normalized.ok()) {
        std::cerr << rejected << ": " << normalized.error << '\n';
        return 2;
    }
    return publish_python_json(*one("--output"), spider::dump_python(*normalized.value), rejected,
                               ".dmc-rengine-l2-selection-candidate.staging");
}

inline int run_verify_l2_selection_evidence(int argc, char** argv) {
    constexpr std::array<std::string_view, 6> known{
        "--mapping", "--mapping-child", "--selection", "--observer-artifact", "--archive-artifact", "--output"};
    const auto options = read_options(argc, argv, known);
    const auto one = [&options](std::string_view name) -> const std::string* {
        const auto found = options.values.find(name);
        return found == options.values.end() || found->second.size() != 1U ? nullptr : &found->second.front();
    };
    const auto children = options.values.find("--mapping-child");
    if (!options.error.empty() || one("--mapping") == nullptr || one("--selection") == nullptr ||
        one("--observer-artifact") == nullptr || one("--output") == nullptr || children == options.values.end()) {
        std::cerr << "Usage: dmc-rengine verify-l2-original-selection-evidence --mapping <json> "
                     "--mapping-child <json>... --selection <json> --observer-artifact <file> "
                     "[--archive-artifact INDEX=PATH ...] --output <json>\n";
        if (!options.error.empty()) std::cerr << options.error << '\n';
        return 2;
    }
    constexpr std::string_view rejected = "original selection candidate rejected";
    spider::L2SelectionBindingInputs inputs{
        .mapping = *one("--mapping"),
        .selection = *one("--selection"),
        .mapping_children = {children->second.begin(), children->second.end()},
        .observer_artifact = *one("--observer-artifact"),
        .archive_artifacts = {},
    };
    const auto archives = options.values.find("--archive-artifact");
    if (archives != options.values.end()) {
        const auto error = spider::parse_l2_archive_artifacts(archives->second, inputs.archive_artifacts);
        if (!error.empty()) {
            std::cerr << rejected << ": " << error << '\n';
            return 2;
        }
    }
    const auto bound = spider::bind_l2_selection_candidate(inputs);
    if (!bound.ok()) {
        std::cerr << rejected << ": " << bound.error << '\n';
        return 2;
    }
    return publish_python_json(*one("--output"), spider::dump_python(*bound.value), rejected,
                               ".dmc-rengine-l2-selection-bound.staging");
}

inline int run_verify_l2_runtime_mapping_v2(int argc, char** argv) {
    constexpr std::array<std::string_view, 3> known{"--canonical-exe", "--receipt", "--output"};
    const auto options = read_options(argc, argv, known);
    const auto one = [&options](std::string_view name) -> const std::string* {
        const auto found = options.values.find(name);
        return found == options.values.end() || found->second.size() != 1U ? nullptr : &found->second.front();
    };
    const auto receipts = options.values.find("--receipt");
    if (!options.error.empty() || one("--canonical-exe") == nullptr || one("--output") == nullptr ||
        receipts == options.values.end()) {
        std::cerr << "Usage: dmc-rengine verify-l2-runtime-mapping-v2 --canonical-exe <exe> "
                     "--receipt <json>... --output <json>\n";
        if (!options.error.empty()) std::cerr << options.error << '\n';
        return 2;
    }
    constexpr std::string_view rejected = "runtime mapping v2 packet rejected";
    std::vector<std::string> texts;
    for (const auto& path : receipts->second) {
        auto text = read_text_file(path);
        if (!text.has_value()) {
            std::cerr << rejected << ": could not read receipt " << path << '\n';
            return 2;
        }
        texts.push_back(std::move(*text));
    }
    const auto exe = read_text_file(*one("--canonical-exe"));
    if (!exe.has_value()) {
        std::cerr << rejected << ": could not read canonical executable " << *one("--canonical-exe") << '\n';
        return 2;
    }
    const std::vector<std::string_view> views{texts.begin(), texts.end()};
    const auto built = spider::build_l2_runtime_mapping_v2(
        views, std::as_bytes(std::span<const char>{exe->data(), exe->size()}));
    if (!built.ok()) {
        std::cerr << rejected << ':';
        for (const auto& error : built.errors) std::cerr << "\n  - " << error;
        std::cerr << '\n';
        return 2;
    }
    auto encoded = spider::l2_runtime_mapping_v2_to_json(*built.packet);
    if (encoded.empty()) {
        std::cerr << rejected << ": native serializer failed\n";
        return 2;
    }
    if (encoded.back() == '\n') encoded.pop_back();
    return publish_python_json(*one("--output"), encoded, rejected, ".dmc-rengine-l2-mapping-v2.staging");
}

} // namespace spider_cli_detail

inline void print_spider_help() {
    std::cout
        << "  extract-exe-window-packet --plan <json> --validate-plan-only\n"
        << "  extract-exe-window-packet --plan <json> --exe <exe> --expected-sha256 <sha> --output <new-dir>\n"
        << "                             Acquire a metadata-only packet in one native process\n"
        << "  verify-l2-runtime-mapping-v1 --receipt <json>... --output <json>\n"
        << "                             Validate bounded L2 mapping receipts natively\n"
        << "  verify-l2-runtime-mapping-v2 --canonical-exe <exe> --receipt <json>... --output <json>\n"
        << "                             Bind L2 mapping receipts to one process instance natively\n"
        << "  normalize-l2-original-selection-candidate --input <json> --output <json>\n"
        << "                             Turn a legacy L2 selection into a non-promotable candidate\n"
        << "  verify-l2-original-selection-evidence --mapping <json> --mapping-child <json>...\n"
        << "      --selection <json> --observer-artifact <file> [--archive-artifact I=PATH...] --output <json>\n"
        << "                             Hash-bind an L2 selection candidate to its artifacts\n";
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
    if (command == "verify-l2-runtime-mapping-v2") {
        return spider_cli_detail::run_verify_l2_runtime_mapping_v2(argc, argv);
    }
    if (command == "normalize-l2-original-selection-candidate") {
        return spider_cli_detail::run_normalize_l2_selection_candidate(argc, argv);
    }
    if (command == "verify-l2-original-selection-evidence") {
        return spider_cli_detail::run_verify_l2_selection_evidence(argc, argv);
    }
    return -1;
}

} // namespace dmc::rengine::cli
