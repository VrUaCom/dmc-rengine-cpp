#include "dmc_rengine/spider/l2_runtime_mapping.hpp"

#include "dmc_rengine/core/json.hpp"

#include <algorithm>
#include <charconv>
#include <cstdint>
#include <limits>
#include <map>
#include <optional>
#include <sstream>
#include <string>
#include <string_view>

namespace dmc::rengine::spider {
namespace {

constexpr std::string_view k_process_window_v1_schema =
    "dmc-rengine.exe-process-window.v1";
constexpr std::string_view k_canonical_analysis_sha256 =
    "e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082";
constexpr std::string_view k_protected_distribution_sha256 =
    "81c7e61983564113b5105e931d9f185accc14e44ae147d27f720c2d50935c7d6";
constexpr std::uint64_t k_protected_distribution_size = 6567320U;
constexpr std::uint64_t k_preferred_image_base = 0x140000000ULL;
constexpr std::uint64_t k_window_size = 0x40U;
constexpr std::size_t k_min_anchors = 3U;
constexpr std::uint64_t k_open_game_resource_rva = 0x0002FCA0U;

const std::map<std::uint64_t, std::string_view> k_anchors{
    {0x0002FCA0U, "OpenGameResource"},
    {0x00326D20U, "type0_mount_registration"},
    {0x00327430U, "type0_mount_resolve"},
    {0x00327800U, "type0_final_open"},
};

[[nodiscard]] bool is_physical_anchor(std::uint64_t rva) noexcept {
    return rva == 0x00326D20U || rva == 0x00327430U || rva == 0x00327800U;
}

[[nodiscard]] bool is_canonical_sha256(std::string_view value) noexcept {
    if (value.size() != 64U) {
        return false;
    }
    return std::all_of(value.begin(), value.end(), [](char ch) {
        return (ch >= '0' && ch <= '9') || (ch >= 'a' && ch <= 'f');
    });
}

[[nodiscard]] const core::json::Value* field(
    const core::json::Value::Object& object,
    std::string_view name) noexcept {
    const auto iterator = object.find(name);
    return iterator == object.end() ? nullptr : &iterator->second;
}

[[nodiscard]] const std::string* string_field(
    const core::json::Value::Object& object,
    std::string_view name) noexcept {
    const auto* value = field(object, name);
    return value == nullptr ? nullptr : value->as_string();
}

[[nodiscard]] const std::uint64_t* u64_field(
    const core::json::Value::Object& object,
    std::string_view name) noexcept {
    const auto* value = field(object, name);
    return value == nullptr ? nullptr : value->as_u64();
}

[[nodiscard]] const bool* bool_field(
    const core::json::Value::Object& object,
    std::string_view name) noexcept {
    const auto* value = field(object, name);
    return value == nullptr ? nullptr : value->as_bool();
}

[[nodiscard]] std::optional<std::uint64_t> parse_hex_u64(std::string_view value) {
    if (value.size() <= 2U || !value.starts_with("0x")) {
        return std::nullopt;
    }
    std::uint64_t parsed = 0U;
    const auto* begin = value.data() + 2;
    const auto* end = value.data() + value.size();
    const auto conversion = std::from_chars(begin, end, parsed, 16);
    if (conversion.ec != std::errc{} || conversion.ptr != end) {
        return std::nullopt;
    }
    return parsed;
}

[[nodiscard]] std::string basename_windows(std::string_view path) {
    const auto position = path.find_last_of("/\\");
    return std::string(position == std::string_view::npos ? path : path.substr(position + 1U));
}

[[nodiscard]] std::string escape_json(std::string_view value) {
    std::ostringstream output;
    constexpr char hex[] = "0123456789abcdef";
    for (const unsigned char character : value) {
        switch (character) {
        case '"': output << "\\\""; break;
        case '\\': output << "\\\\"; break;
        case '\b': output << "\\b"; break;
        case '\f': output << "\\f"; break;
        case '\n': output << "\\n"; break;
        case '\r': output << "\\r"; break;
        case '\t': output << "\\t"; break;
        default:
            if (character < 0x20U) {
                output << "\\u00"
                       << hex[(character >> 4U) & 0x0FU]
                       << hex[character & 0x0FU];
            } else {
                output << static_cast<char>(character);
            }
            break;
        }
    }
    return output.str();
}

[[nodiscard]] std::string hex_u64(std::uint64_t value) {
    std::ostringstream output;
    output << "0x" << std::hex << std::uppercase << value;
    return output.str();
}

struct Child final {
    L2RuntimeMappingAnchor anchor;
    std::uint32_t pid{};
    std::uint64_t module_base{};
    std::string image_path;
};

[[nodiscard]] std::optional<Child> parse_child(
    std::string_view input,
    std::string& error) {
    const auto parsed = core::json::Parser::parse(input);
    if (!parsed.ok()) {
        error = "child receipt is not valid JSON";
        return std::nullopt;
    }
    const auto* object = parsed.value->as_object();
    if (object == nullptr) {
        error = "child receipt must contain one JSON object";
        return std::nullopt;
    }
    if (object->contains("bytes_hex")) {
        error = "mapping packets accept metadata-only receipts; raw bytes_hex is forbidden";
        return std::nullopt;
    }

    const auto* schema = string_field(*object, "schema");
    if (schema == nullptr || *schema != k_process_window_v1_schema) {
        error = "child receipt has unsupported schema";
        return std::nullopt;
    }

    const auto* artifact_sha = string_field(*object, "artifact_sha256");
    if (artifact_sha == nullptr || !is_canonical_sha256(*artifact_sha) ||
        *artifact_sha != k_protected_distribution_sha256) {
        error = "child receipt is not bound to the protected distribution SHA";
        return std::nullopt;
    }
    const auto* artifact_size = u64_field(*object, "artifact_size");
    if (artifact_size == nullptr || *artifact_size != k_protected_distribution_size) {
        error = "child receipt has wrong protected distribution size";
        return std::nullopt;
    }

    const auto* expected_artifact =
        string_field(*object, "expected_window_artifact_sha256");
    if (expected_artifact == nullptr || !is_canonical_sha256(*expected_artifact) ||
        *expected_artifact != k_canonical_analysis_sha256) {
        error = "child receipt is not bound to the canonical analysis artifact";
        return std::nullopt;
    }
    const auto* window_sha = string_field(*object, "window_sha256");
    const auto* expected_window_sha = string_field(*object, "expected_window_sha256");
    const auto* matches_expected = bool_field(*object, "matches_expected_window");
    if (window_sha == nullptr || expected_window_sha == nullptr ||
        !is_canonical_sha256(*window_sha) || !is_canonical_sha256(*expected_window_sha) ||
        matches_expected == nullptr || !*matches_expected || *window_sha != *expected_window_sha) {
        error = "child receipt does not prove an exact canonical window match";
        return std::nullopt;
    }

    const auto* pid = u64_field(*object, "pid");
    if (pid == nullptr || *pid == 0U || *pid > std::numeric_limits<std::uint32_t>::max()) {
        error = "child receipt has invalid pid";
        return std::nullopt;
    }
    const auto* size = u64_field(*object, "size");
    if (size == nullptr || *size != k_window_size) {
        error = "child receipt must use exact mapping window size 0x40";
        return std::nullopt;
    }
    const auto* section = string_field(*object, "section");
    if (section == nullptr || *section != ".text") {
        error = "child receipt must map an L2 .text anchor";
        return std::nullopt;
    }
    const auto* image_path = string_field(*object, "image_path");
    if (image_path == nullptr || image_path->empty()) {
        error = "child receipt has invalid image_path";
        return std::nullopt;
    }

    const auto* preferred_text = string_field(*object, "preferred_image_base");
    const auto preferred = preferred_text == nullptr
        ? std::optional<std::uint64_t>{}
        : parse_hex_u64(*preferred_text);
    if (!preferred.has_value() || *preferred != k_preferred_image_base) {
        error = "child receipt has unexpected preferred image base";
        return std::nullopt;
    }

    const auto* module_text = string_field(*object, "module_base");
    const auto* rva_text = string_field(*object, "rva");
    const auto* runtime_text = string_field(*object, "runtime_va");
    const auto module_base = module_text == nullptr
        ? std::optional<std::uint64_t>{}
        : parse_hex_u64(*module_text);
    const auto rva = rva_text == nullptr
        ? std::optional<std::uint64_t>{}
        : parse_hex_u64(*rva_text);
    const auto runtime_va = runtime_text == nullptr
        ? std::optional<std::uint64_t>{}
        : parse_hex_u64(*runtime_text);
    if (!module_base.has_value() || !rva.has_value() || !runtime_va.has_value()) {
        error = "child receipt has invalid module_base/RVA/runtime_va";
        return std::nullopt;
    }
    const auto anchor = k_anchors.find(*rva);
    if (anchor == k_anchors.end()) {
        error = "child receipt RVA is not an approved L2 mapping anchor";
        return std::nullopt;
    }
    if (*rva > std::numeric_limits<std::uint64_t>::max() - *module_base ||
        *module_base + *rva != *runtime_va) {
        error = "child receipt has inconsistent module_base/RVA/runtime_va";
        return std::nullopt;
    }

    return Child{
        .anchor = L2RuntimeMappingAnchor{
            .id = std::string(anchor->second),
            .rva = *rva,
            .runtime_va = *runtime_va,
            .size = *size,
            .window_sha256 = *window_sha,
        },
        .pid = static_cast<std::uint32_t>(*pid),
        .module_base = *module_base,
        .image_path = *image_path,
    };
}

} // namespace

L2RuntimeMappingBuildResult build_l2_runtime_mapping_v1(
    std::span<const std::string_view> receipt_jsons) {
    L2RuntimeMappingBuildResult result;
    if (receipt_jsons.size() < k_min_anchors) {
        result.errors.emplace_back("at least 3 child receipts are required");
        return result;
    }

    std::vector<L2RuntimeMappingAnchor> anchors;
    anchors.reserve(receipt_jsons.size());
    std::vector<std::uint64_t> seen_rvas;
    std::optional<std::uint32_t> common_pid;
    std::optional<std::uint64_t> common_module_base;
    std::optional<std::string> common_image_path;

    for (std::size_t index = 0U; index < receipt_jsons.size(); ++index) {
        std::string error;
        auto child = parse_child(receipt_jsons[index], error);
        if (!child.has_value()) {
            result.errors.emplace_back(
                "receipt[" + std::to_string(index) + "]: " + error);
            return result;
        }
        if (std::find(seen_rvas.begin(), seen_rvas.end(), child->anchor.rva) != seen_rvas.end()) {
            result.errors.emplace_back("duplicate mapping anchor RVA");
            return result;
        }
        seen_rvas.push_back(child->anchor.rva);

        if (!common_pid.has_value()) {
            common_pid = child->pid;
            common_module_base = child->module_base;
            common_image_path = child->image_path;
        } else if (
            child->pid != *common_pid ||
            child->module_base != *common_module_base ||
            child->image_path != *common_image_path) {
            result.errors.emplace_back(
                "all mapping receipts must come from one process/module session");
            return result;
        }
        anchors.push_back(std::move(child->anchor));
    }

    if (std::find(seen_rvas.begin(), seen_rvas.end(), k_open_game_resource_rva) ==
        seen_rvas.end()) {
        result.errors.emplace_back(
            "mapping packet requires the OpenGameResource anchor RVA 0x2FCA0");
        return result;
    }
    const auto physical_count = static_cast<std::size_t>(std::count_if(
        seen_rvas.begin(), seen_rvas.end(), is_physical_anchor));
    if (physical_count < 2U) {
        result.errors.emplace_back(
            "mapping packet requires at least two independent type-0 physical anchors");
        return result;
    }
    if (!common_image_path.has_value()) {
        result.errors.emplace_back("mapping packet has no process image identity");
        return result;
    }
    const auto image_name = basename_windows(*common_image_path);
    if (image_name.empty()) {
        result.errors.emplace_back("mapping packet process image has no basename");
        return result;
    }

    std::sort(anchors.begin(), anchors.end(), [](const auto& left, const auto& right) {
        return left.rva < right.rva;
    });
    result.packet = L2RuntimeMappingPacketV1{
        .protected_artifact_sha256 = std::string(k_protected_distribution_sha256),
        .protected_artifact_size = k_protected_distribution_size,
        .canonical_analysis_artifact_sha256 = std::string(k_canonical_analysis_sha256),
        .preferred_image_base = k_preferred_image_base,
        .pid = *common_pid,
        .module_base = *common_module_base,
        .image_name = image_name,
        .anchors = std::move(anchors),
    };
    return result;
}

std::string l2_runtime_mapping_v1_to_json(const L2RuntimeMappingPacketV1& packet) {
    if (packet.anchors.empty() || packet.pid == 0U || packet.image_name.empty()) {
        return {};
    }

    std::ostringstream output;
    output << "{\n"
           << "  \"schema\": \"" << k_l2_runtime_mapping_v1_schema << "\",\n"
           << "  \"status\": \"bounded_match\",\n"
           << "  \"scope\": \"approved-l2-rva-anchors-only\",\n"
           << "  \"protected_artifact_sha256\": \""
           << packet.protected_artifact_sha256 << "\",\n"
           << "  \"protected_artifact_size\": " << packet.protected_artifact_size << ",\n"
           << "  \"canonical_analysis_artifact_sha256\": \""
           << packet.canonical_analysis_artifact_sha256 << "\",\n"
           << "  \"preferred_image_base\": \"" << hex_u64(packet.preferred_image_base)
           << "\",\n"
           << "  \"pid\": " << packet.pid << ",\n"
           << "  \"module_base\": \"" << hex_u64(packet.module_base) << "\",\n"
           << "  \"image_name\": \"" << escape_json(packet.image_name) << "\",\n"
           << "  \"anchor_count\": " << packet.anchors.size() << ",\n"
           << "  \"anchors\": [\n";
    for (std::size_t index = 0U; index < packet.anchors.size(); ++index) {
        const auto& anchor = packet.anchors[index];
        output << "    {\n"
               << "      \"id\": \"" << escape_json(anchor.id) << "\",\n"
               << "      \"rva\": \"" << hex_u64(anchor.rva) << "\",\n"
               << "      \"runtime_va\": \"" << hex_u64(anchor.runtime_va) << "\",\n"
               << "      \"size\": " << anchor.size << ",\n"
               << "      \"window_sha256\": \"" << anchor.window_sha256 << "\"\n"
               << "    }" << (index + 1U == packet.anchors.size() ? "\n" : ",\n");
    }
    output << "  ],\n"
           << "  \"proves\": [\n"
           << "    \"exact-live-byte-match-at-listed-l2-ranges\",\n"
           << "    \"bounded-protected-runtime-rva-mapping-for-listed-anchors\"\n"
           << "  ],\n"
           << "  \"does_not_prove\": [\n"
           << "    \"global-build-byte-equivalence\",\n"
           << "    \"unlisted-function-address-equivalence\",\n"
           << "    \"retail-nbz-collision-freedom\",\n"
           << "    \"original-process-selected-provider-identity\",\n"
           << "    \"layer-1-or-layer-3-completion\"\n"
           << "  ]\n"
           << "}\n";
    return output.str();
}

} // namespace dmc::rengine::spider
