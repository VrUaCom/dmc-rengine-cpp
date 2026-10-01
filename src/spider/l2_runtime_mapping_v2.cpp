#include "dmc_rengine/spider/l2_runtime_mapping_v2.hpp"

#include "dmc_rengine/core/json.hpp"
#include "dmc_rengine/core/sha256.hpp"
#include "dmc_rengine/exe/pe_reader.hpp"

#include <algorithm>
#include <charconv>
#include <cstdint>
#include <limits>
#include <map>
#include <optional>
#include <set>
#include <sstream>
#include <string>
#include <string_view>
#include <utility>

namespace dmc::rengine::spider {
namespace {

constexpr std::string_view k_process_window_v2_schema =
    "dmc-rengine.exe-process-window.v2";
constexpr std::string_view k_protected_distribution_sha256 =
    "81c7e61983564113b5105e931d9f185accc14e44ae147d27f720c2d50935c7d6";
constexpr std::uint64_t k_protected_distribution_size = 6567320U;
constexpr std::uint64_t k_window_size = 0x40U;

const std::map<std::uint64_t, std::string_view> k_anchors{
    {0x0002FCA0U, "OpenGameResource"},
    {0x00326D20U, "type0_mount_registration"},
    {0x00326DA0U, "type1_archive_mount_registration"},
    {0x00327430U, "ResourceMountResolve"},
    {0x00327800U, "type0_final_open"},
    {0x00328160U, "archive_normalized_lookup"},
    {0x00328290U, "archive_wrapper_open"},
};

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

[[nodiscard]] bool canonical_sha256(std::string_view value) noexcept {
    if (value.size() != 64U) {
        return false;
    }
    return std::all_of(value.begin(), value.end(), [](char ch) {
        return (ch >= '0' && ch <= '9') || (ch >= 'a' && ch <= 'f');
    });
}

[[nodiscard]] std::optional<std::uint64_t> parse_hex_u64(std::string_view value) {
    if (value.size() <= 2U || !value.starts_with("0x")) {
        return std::nullopt;
    }
    std::uint64_t parsed = 0U;
    const auto* begin = value.data() + 2;
    const auto* end = value.data() + value.size();
    const auto converted = std::from_chars(begin, end, parsed, 16);
    if (converted.ec != std::errc{} || converted.ptr != end) {
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

struct CanonicalWindows final {
    std::map<std::uint64_t, std::string> sha_by_rva;
};

[[nodiscard]] std::optional<CanonicalWindows> derive_canonical_windows(
    std::span<const std::byte> canonical_exe,
    const L2RuntimeMappingV2Authority& authority,
    std::string& error) {
    if (canonical_exe.size() != authority.canonical_analysis_size) {
        error = "canonical EXE size mismatch";
        return std::nullopt;
    }
    if (!canonical_sha256(authority.canonical_analysis_sha256)) {
        error = "canonical analysis authority SHA is invalid";
        return std::nullopt;
    }
    const auto actual_sha = core::Sha256::compute(canonical_exe).hex();
    if (actual_sha != authority.canonical_analysis_sha256) {
        error = "canonical EXE SHA-256 mismatch; mapping v2 requires the exact analysis artifact";
        return std::nullopt;
    }

    const auto parsed = exe::PeReader::read(canonical_exe);
    if (!parsed.ok()) {
        error = "canonical EXE does not parse as a valid PE";
        return std::nullopt;
    }
    const auto& image = *parsed.image;
    if (image.kind != exe::PeKind::pe32_plus) {
        error = "canonical EXE is not PE32+";
        return std::nullopt;
    }
    if (image.image_base != authority.preferred_image_base) {
        error = "canonical EXE has unexpected preferred image base";
        return std::nullopt;
    }

    CanonicalWindows output;
    for (const auto& [rva64, ignored_name] : k_anchors) {
        (void)ignored_name;
        if (rva64 > std::numeric_limits<std::uint32_t>::max() ||
            rva64 > std::numeric_limits<std::uint32_t>::max() - (k_window_size - 1U)) {
            error = "canonical anchor RVA is outside PE32 RVA range";
            return std::nullopt;
        }
        const auto rva = static_cast<std::uint32_t>(rva64);
        const auto end_rva = static_cast<std::uint32_t>(rva64 + k_window_size - 1U);

        const exe::PeSection* section = nullptr;
        for (const auto& candidate : image.sections) {
            if (candidate.contains_rva(rva) && candidate.contains_rva(end_rva)) {
                section = &candidate;
                break;
            }
        }
        if (section == nullptr) {
            error = "canonical anchor RVA is not mapped by a PE section";
            return std::nullopt;
        }
        if (section->name != ".text") {
            error = "canonical anchor RVA is not in .text";
            return std::nullopt;
        }
        const auto delta = static_cast<std::uint64_t>(rva) - section->virtual_address;
        if (delta + k_window_size > section->raw_size) {
            error = "canonical anchor RVA has no full raw-file window";
            return std::nullopt;
        }
        const auto offset = image.rva_to_file_offset(rva);
        if (!offset.has_value() ||
            static_cast<std::uint64_t>(*offset) + k_window_size > canonical_exe.size()) {
            error = "canonical anchor RVA exceeds the artifact";
            return std::nullopt;
        }
        const auto window = canonical_exe.subspan(*offset, static_cast<std::size_t>(k_window_size));
        output.sha_by_rva.emplace(rva64, core::Sha256::compute(window).hex());
    }
    return output;
}

struct Child final {
    L2RuntimeMappingAnchorV2 anchor;
    std::uint64_t pid{};
    std::uint64_t process_creation_filetime{};
    std::uint64_t module_base{};
    std::string image_path;
};

[[nodiscard]] std::optional<Child> parse_child(
    std::string_view input,
    const CanonicalWindows& canonical,
    const L2RuntimeMappingV2Authority& authority,
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
    const auto* schema = string_field(*object, "schema");
    if (schema == nullptr || *schema != k_process_window_v2_schema) {
        error = "child receipt must use process-window v2; legacy v1 is not promotion authority";
        return std::nullopt;
    }
    if (object->contains("bytes_hex")) {
        error = "mapping packets accept metadata-only receipts; raw bytes_hex is forbidden";
        return std::nullopt;
    }

    const auto* artifact_sha = string_field(*object, "artifact_sha256");
    const auto* artifact_size = u64_field(*object, "artifact_size");
    if (artifact_sha == nullptr || !canonical_sha256(*artifact_sha) ||
        *artifact_sha != k_protected_distribution_sha256) {
        error = "child receipt is not bound to the protected distribution SHA";
        return std::nullopt;
    }
    if (artifact_size == nullptr || *artifact_size != k_protected_distribution_size) {
        error = "child receipt has wrong protected distribution size";
        return std::nullopt;
    }

    const auto* pid = u64_field(*object, "pid");
    if (pid == nullptr || *pid == 0U) {
        error = "child receipt has invalid pid";
        return std::nullopt;
    }
    const auto* creation = u64_field(*object, "process_creation_filetime");
    if (creation == nullptr || *creation == 0U) {
        error = "process_creation_filetime must be a non-zero unsigned 64-bit integer";
        return std::nullopt;
    }
    const auto* size = u64_field(*object, "size");
    if (size == nullptr || *size != k_window_size) {
        error = "child receipt must use exact mapping window size 0x40";
        return std::nullopt;
    }
    const auto* section_name = string_field(*object, "section");
    if (section_name == nullptr || *section_name != ".text") {
        error = "child receipt must map an L2 .text anchor";
        return std::nullopt;
    }
    const auto* image_path = string_field(*object, "image_path");
    if (image_path == nullptr || image_path->empty()) {
        error = "child receipt has invalid image_path";
        return std::nullopt;
    }

    const auto* preferred_text = string_field(*object, "preferred_image_base");
    const auto* module_text = string_field(*object, "module_base");
    const auto* rva_text = string_field(*object, "rva");
    const auto* runtime_text = string_field(*object, "runtime_va");
    const auto preferred = preferred_text == nullptr
        ? std::optional<std::uint64_t>{}
        : parse_hex_u64(*preferred_text);
    const auto module_base = module_text == nullptr
        ? std::optional<std::uint64_t>{}
        : parse_hex_u64(*module_text);
    const auto rva = rva_text == nullptr
        ? std::optional<std::uint64_t>{}
        : parse_hex_u64(*rva_text);
    const auto runtime_va = runtime_text == nullptr
        ? std::optional<std::uint64_t>{}
        : parse_hex_u64(*runtime_text);
    if (!preferred.has_value() || *preferred != authority.preferred_image_base) {
        error = "child receipt has unexpected preferred image base";
        return std::nullopt;
    }
    if (!module_base.has_value() || !rva.has_value() || !runtime_va.has_value()) {
        error = "child receipt has invalid module_base/RVA/runtime_va";
        return std::nullopt;
    }
    const auto anchor_definition = k_anchors.find(*rva);
    if (anchor_definition == k_anchors.end()) {
        error = "child receipt RVA is not an approved L2 mapping anchor";
        return std::nullopt;
    }
    if (*rva > std::numeric_limits<std::uint64_t>::max() - *module_base ||
        *module_base + *rva != *runtime_va) {
        error = "child receipt has inconsistent module_base/RVA/runtime_va";
        return std::nullopt;
    }

    const auto canonical_hash = canonical.sha_by_rva.find(*rva);
    if (canonical_hash == canonical.sha_by_rva.end()) {
        error = "validator has no independently derived canonical anchor hash";
        return std::nullopt;
    }
    const auto* live_hash = string_field(*object, "window_sha256");
    if (live_hash == nullptr || !canonical_sha256(*live_hash) ||
        *live_hash != canonical_hash->second) {
        error = "child receipt live window does not match the independently derived canonical window";
        return std::nullopt;
    }

    const bool has_expected_artifact = object->contains("expected_window_artifact_sha256");
    const bool has_expected_window = object->contains("expected_window_sha256");
    const bool has_expected_match = object->contains("matches_expected_window");
    if (has_expected_artifact || has_expected_window || has_expected_match) {
        if (!(has_expected_artifact && has_expected_window && has_expected_match)) {
            error = "child receipt has a partial diagnostic expectation tuple";
            return std::nullopt;
        }
        const auto* expected_artifact = string_field(*object, "expected_window_artifact_sha256");
        const auto* expected_window = string_field(*object, "expected_window_sha256");
        const auto* expected_match = bool_field(*object, "matches_expected_window");
        if (expected_artifact == nullptr || expected_window == nullptr || expected_match == nullptr ||
            !canonical_sha256(*expected_artifact) || !canonical_sha256(*expected_window) ||
            *expected_artifact != authority.canonical_analysis_sha256 ||
            *expected_window != canonical_hash->second || !*expected_match) {
            error = "child receipt diagnostic expectation does not match independently derived canonical evidence";
            return std::nullopt;
        }
    }

    return Child{
        .anchor = L2RuntimeMappingAnchorV2{
            .id = std::string(anchor_definition->second),
            .rva = *rva,
            .runtime_va = *runtime_va,
            .size = *size,
            .window_sha256 = *live_hash,
            .canonical_window_sha256 = canonical_hash->second,
        },
        .pid = *pid,
        .process_creation_filetime = *creation,
        .module_base = *module_base,
        .image_path = *image_path,
    };
}

} // namespace

L2RuntimeMappingBuildResultV2 build_l2_runtime_mapping_v2(
    std::span<const std::string_view> receipt_jsons,
    std::span<const std::byte> canonical_exe,
    const L2RuntimeMappingV2Authority& authority) {
    L2RuntimeMappingBuildResultV2 result;
    if (receipt_jsons.size() < k_anchors.size()) {
        result.errors.emplace_back(
            "at least " + std::to_string(k_anchors.size()) + " child receipts are required");
        return result;
    }

    std::string canonical_error;
    const auto canonical = derive_canonical_windows(canonical_exe, authority, canonical_error);
    if (!canonical.has_value()) {
        result.errors.push_back(std::move(canonical_error));
        return result;
    }

    std::set<std::uint64_t> seen_rvas;
    std::vector<L2RuntimeMappingAnchorV2> anchors;
    anchors.reserve(receipt_jsons.size());
    std::optional<std::uint64_t> common_pid;
    std::optional<std::uint64_t> common_creation;
    std::optional<std::uint64_t> common_module_base;
    std::optional<std::string> common_image_path;

    for (std::size_t index = 0U; index < receipt_jsons.size(); ++index) {
        std::string child_error;
        auto child = parse_child(receipt_jsons[index], *canonical, authority, child_error);
        if (!child.has_value()) {
            result.errors.emplace_back(
                "receipt[" + std::to_string(index) + "]: " + child_error);
            return result;
        }
        if (!seen_rvas.insert(child->anchor.rva).second) {
            result.errors.emplace_back("duplicate mapping anchor RVA");
            return result;
        }

        if (!common_pid.has_value()) {
            common_pid = child->pid;
            common_creation = child->process_creation_filetime;
            common_module_base = child->module_base;
            common_image_path = child->image_path;
        } else if (
            child->pid != *common_pid ||
            child->process_creation_filetime != *common_creation ||
            child->module_base != *common_module_base ||
            child->image_path != *common_image_path) {
            result.errors.emplace_back(
                "all mapping receipts must come from one exact process instance/module session");
            return result;
        }
        anchors.push_back(std::move(child->anchor));
    }

    for (const auto& [rva, ignored_name] : k_anchors) {
        (void)ignored_name;
        if (!seen_rvas.contains(rva)) {
            result.errors.emplace_back(
                "mapping packet is missing required R2B/R3 anchor: " + hex_u64(rva));
            return result;
        }
    }
    if (!common_image_path.has_value() || !common_creation.has_value()) {
        result.errors.emplace_back("mapping packet has no process-instance identity");
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
    result.packet = L2RuntimeMappingPacketV2{
        .protected_artifact_sha256 = std::string(k_protected_distribution_sha256),
        .protected_artifact_size = k_protected_distribution_size,
        .canonical_analysis_artifact_sha256 = authority.canonical_analysis_sha256,
        .canonical_analysis_artifact_size = authority.canonical_analysis_size,
        .preferred_image_base = authority.preferred_image_base,
        .pid = *common_pid,
        .process_creation_filetime = *common_creation,
        .module_base = *common_module_base,
        .image_name = image_name,
        .anchors = std::move(anchors),
    };
    return result;
}

std::string l2_runtime_mapping_v2_to_json(const L2RuntimeMappingPacketV2& packet) {
    if (packet.anchors.empty() || packet.pid == 0U ||
        packet.process_creation_filetime == 0U || packet.image_name.empty()) {
        return {};
    }

    std::ostringstream output;
    output << "{\n"
           << "  \"schema\": \"" << k_l2_runtime_mapping_v2_schema << "\",\n"
           << "  \"status\": \"bounded_process_instance_match\",\n"
           << "  \"scope\": \"required-r2b-r3-anchors-one-process-instance-only\",\n"
           << "  \"protected_artifact_sha256\": \"" << packet.protected_artifact_sha256 << "\",\n"
           << "  \"protected_artifact_size\": " << packet.protected_artifact_size << ",\n"
           << "  \"canonical_analysis_artifact_sha256\": \""
           << packet.canonical_analysis_artifact_sha256 << "\",\n"
           << "  \"canonical_analysis_artifact_size\": "
           << packet.canonical_analysis_artifact_size << ",\n"
           << "  \"canonical_window_authority\": "
              "\"derived-directly-from-exact-canonical-exe-by-validator\",\n"
           << "  \"preferred_image_base\": \"" << hex_u64(packet.preferred_image_base) << "\",\n"
           << "  \"pid\": " << packet.pid << ",\n"
           << "  \"process_creation_filetime\": " << packet.process_creation_filetime << ",\n"
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
               << "      \"window_sha256\": \"" << anchor.window_sha256 << "\",\n"
               << "      \"canonical_window_sha256\": \""
               << anchor.canonical_window_sha256 << "\"\n"
               << "    }" << (index + 1U == packet.anchors.size() ? "\n" : ",\n");
    }
    output << "  ],\n"
           << "  \"proves\": [\n"
           << "    \"exact-live-byte-match-at-listed-l2-ranges-against-validator-derived-canonical-windows\",\n"
           << "    \"bounded-protected-runtime-rva-mapping-for-listed-anchors\",\n"
           << "    \"all-listed-anchors-belong-to-one-os-identified-process-instance\"\n"
           << "  ],\n"
           << "  \"does_not_prove\": [\n"
           << "    \"global-build-byte-equivalence\",\n"
           << "    \"unlisted-function-address-equivalence\",\n"
           << "    \"retail-nbz-collision-freedom\",\n"
           << "    \"all-discovered-numbered-archives-mounted-successfully\",\n"
           << "    \"original-process-selected-provider-identity\",\n"
           << "    \"layer-1-or-layer-3-completion\"\n"
           << "  ]\n"
           << "}\n";
    return output.str();
}

} // namespace dmc::rengine::spider
