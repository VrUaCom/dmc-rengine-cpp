#pragma once

#include "dmc_rengine/spider/l2_runtime_mapping.hpp"

#include <cassert>
#include <cstdint>
#include <string>
#include <string_view>
#include <vector>

namespace dmc::rengine::tests {
namespace {

constexpr std::string_view k_mapping_canonical_sha =
    "e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082";
constexpr std::string_view k_mapping_protected_sha =
    "81c7e61983564113b5105e931d9f185accc14e44ae147d27f720c2d50935c7d6";

std::string make_mapping_receipt(
    std::uint64_t rva,
    char sha_digit,
    std::uint32_t pid = 4242U,
    std::string_view expected_artifact = k_mapping_canonical_sha,
    bool matches_expected = true,
    bool include_raw_bytes = false) {
    constexpr std::uint64_t module_base = 0x7FF600000000ULL;
    const std::string window_sha(64U, sha_digit);
    std::string json =
        "{\n"
        "  \"schema\": \"dmc-rengine.exe-process-window.v1\",\n"
        "  \"artifact_sha256\": \"" + std::string(k_mapping_protected_sha) + "\",\n"
        "  \"artifact_size\": 6567320,\n"
        "  \"image_path\": \"C:/Users/LocalUser/Games/DMC3/dmc3.exe\",\n"
        "  \"preferred_image_base\": \"0x140000000\",\n"
        "  \"pid\": " + std::to_string(pid) + ",\n"
        "  \"module_base\": \"0x7FF600000000\",\n";

    auto hex = [](std::uint64_t value) {
        constexpr char digits[] = "0123456789ABCDEF";
        std::string result = "0x";
        char buffer[16]{};
        std::size_t count = 0U;
        do {
            buffer[count++] = digits[value & 0xFU];
            value >>= 4U;
        } while (value != 0U);
        while (count != 0U) {
            result.push_back(buffer[--count]);
        }
        return result;
    };

    json += "  \"rva\": \"" + hex(rva) + "\",\n";
    json += "  \"runtime_va\": \"" + hex(module_base + rva) + "\",\n";
    json +=
        "  \"size\": 64,\n"
        "  \"section\": \".text\",\n"
        "  \"window_sha256\": \"" + window_sha + "\",\n"
        "  \"expected_window_artifact_sha256\": \"" +
        std::string(expected_artifact) + "\",\n"
        "  \"expected_window_sha256\": \"" + window_sha + "\",\n"
        "  \"matches_expected_window\": " +
        std::string(matches_expected ? "true" : "false");
    if (include_raw_bytes) {
        json += ",\n  \"bytes_hex\": \"" + std::string(128U, '0') + "\"";
    }
    json += "\n}\n";
    return json;
}

spider::L2RuntimeMappingBuildResult build_mapping(
    const std::vector<std::string>& receipts) {
    std::vector<std::string_view> views;
    views.reserve(receipts.size());
    for (const auto& receipt : receipts) {
        views.emplace_back(receipt);
    }
    return spider::build_l2_runtime_mapping_v1(views);
}

bool has_error(
    const spider::L2RuntimeMappingBuildResult& result,
    std::string_view fragment) {
    for (const auto& error : result.errors) {
        if (error.find(fragment) != std::string::npos) {
            return true;
        }
    }
    return false;
}

} // namespace

inline void run_spider_l2_runtime_mapping_tests() {
    const auto open_game = make_mapping_receipt(0x0002FCA0U, '1');
    const auto registration = make_mapping_receipt(0x00326D20U, '2');
    const auto resolve = make_mapping_receipt(0x00327430U, '3');

    const auto valid = build_mapping({open_game, registration, resolve});
    assert(valid.ok());
    assert(valid.packet->anchors.size() == 3U);
    assert(valid.packet->canonical_analysis_artifact_sha256 == k_mapping_canonical_sha);
    assert(valid.packet->image_name == "dmc3.exe");
    const auto encoded = spider::l2_runtime_mapping_v1_to_json(*valid.packet);
    assert(encoded.find("\"schema\": \"dmc-rengine.gdspaces-l2-runtime-mapping.v1\"") !=
        std::string::npos);
    assert(encoded.find("\"status\": \"bounded_match\"") != std::string::npos);
    assert(encoded.find("image_path") == std::string::npos);
    assert(encoded.find("LocalUser") == std::string::npos);
    assert(encoded.find("original-process-selected-provider-identity") != std::string::npos);

    const auto duplicate = build_mapping({open_game, registration, registration});
    assert(!duplicate.ok());
    assert(has_error(duplicate, "duplicate mapping anchor"));

    const auto wrong_pid = build_mapping({
        open_game,
        registration,
        make_mapping_receipt(0x00327430U, '3', 9999U),
    });
    assert(!wrong_pid.ok());
    assert(has_error(wrong_pid, "one process/module session"));

    const auto wrong_authority = build_mapping({
        open_game,
        registration,
        make_mapping_receipt(
            0x00327430U,
            '3',
            4242U,
            "ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff"),
    });
    assert(!wrong_authority.ok());
    assert(has_error(wrong_authority, "canonical analysis artifact"));

    const auto mismatch = build_mapping({
        open_game,
        registration,
        make_mapping_receipt(0x00327430U, '3', 4242U, k_mapping_canonical_sha, false),
    });
    assert(!mismatch.ok());
    assert(has_error(mismatch, "exact canonical window match"));

    const auto raw = build_mapping({
        open_game,
        registration,
        make_mapping_receipt(
            0x00327430U,
            '3',
            4242U,
            k_mapping_canonical_sha,
            true,
            true),
    });
    assert(!raw.ok());
    assert(has_error(raw, "metadata-only receipts"));

    const auto missing_open = build_mapping({
        registration,
        resolve,
        make_mapping_receipt(0x00327800U, '4'),
    });
    assert(!missing_open.ok());
    assert(has_error(missing_open, "OpenGameResource anchor"));
}

} // namespace dmc::rengine::tests
