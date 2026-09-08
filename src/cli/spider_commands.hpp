#pragma once

#include "dmc_rengine/core/no_replace_publication.hpp"
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
        << "  verify-l2-runtime-mapping-v1 --receipt <json>... --output <json>\n"
        << "                             Validate bounded L2 mapping receipts natively\n";
}

inline int try_run_spider_command(int argc, char** argv) {
    if (argc <= 1) {
        return -1;
    }
    const std::string_view command{argv[1]};
    if (command == "verify-l2-runtime-mapping-v1") {
        return spider_cli_detail::run_verify_l2_runtime_mapping_v1(argc, argv);
    }
    return -1;
}

} // namespace dmc::rengine::cli
