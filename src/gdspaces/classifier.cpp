#include "dmc_rengine/gdspaces/classifier.hpp"

#include "dmc_rengine/core/sha256.hpp"
#include "dmc_rengine/formats/pnst.hpp"
#include "dmc_rengine/gdspaces/resource_payload.hpp"

#include <algorithm>
#include <cctype>
#include <filesystem>

namespace dmc::rengine::gdspaces {
namespace {

[[nodiscard]] std::string lower_copy(std::string_view value) {
    std::string result(value);
    std::transform(
        result.begin(), result.end(), result.begin(),
        [](unsigned char character) {
            return static_cast<char>(std::tolower(character));
        });
    return result;
}

[[nodiscard]] bool starts_with(
    std::span<const std::byte> bytes,
    std::string_view signature) noexcept {
    if (bytes.size() < signature.size()) {
        return false;
    }

    for (std::size_t index = 0; index < signature.size(); ++index) {
        if (std::to_integer<unsigned char>(bytes[index]) !=
            static_cast<unsigned char>(signature[index])) {
            return false;
        }
    }
    return true;
}

[[nodiscard]] std::string extension_from_path(std::string_view logical_path) {
    auto extension = std::filesystem::path(logical_path).extension().string();
    if (!extension.empty() && extension.front() == '.') {
        extension.erase(extension.begin());
    }
    return lower_copy(extension);
}

[[nodiscard]] bool structurally_valid_binary_pnst(
    std::span<const std::byte> bytes) {
    if (!starts_with(bytes, "PNST")) {
        return false;
    }

    return formats::PnstParser::parse(bytes).ok();
}

} // namespace

ResourceClassification ResourceClassifier::classify(
    std::string_view logical_path,
    std::span<const std::byte> bytes) {
    ResourceClassification result;
    result.profile = profile_from_path(logical_path);

    if (starts_with(bytes, "MZ")) {
        result.format = "pe";
        result.magic_confirmed = true;
    } else if (starts_with(bytes, std::string_view{"PAC\0", 4U})) {
        result.format = "pac";
        result.magic_confirmed = true;
    } else if (structurally_valid_binary_pnst(bytes)) {
        result.format = "pnst";
        result.magic_confirmed = true;
    } else if (starts_with(bytes, "SCM")) {
        // Generic clean classification retains the historical SCM prefix rule.
        // Exact DMC3 runtime three-byte vs four-byte identity semantics are
        // preserved separately by sealed profile evidence and must not be
        // generalized to all formats here.
        result.format = "scm";
        result.magic_confirmed = true;
    } else if (starts_with(bytes, std::string_view{"DCA\0", 4U})) {
        result.format = "dca";
        result.magic_confirmed = true;
    } else if (starts_with(bytes, "HITS")) {
        // DATA_CONFIRMED corpus/parser magic. The bounded canonical EXE census
        // does not establish HITS as an EXE runtime type tag; HITS$ is rejected.
        result.format = "hits";
        result.magic_confirmed = true;
    } else if (starts_with(bytes, "DDS ")) {
        // Canonical DMC3 EXE directly compares this DWORD at 0x140049A8E and
        // 0x14004AD9D, so DDS content identity is EXE-backed as well as standard.
        result.format = "dds";
        result.magic_confirmed = true;
    } else if (starts_with(bytes, std::string_view{"TM2\0", 4U})) {
        // Canonical EXE content check at 0x1403365BA is exactly TM2\0.
        // TIM2 remains an alias/historical label, not canonical direct magic.
        result.format = "tm2";
        result.magic_confirmed = true;
    } else if (starts_with(bytes, "VAGp")) {
        // Canonical EXE direct first-DWORD check at 0x140032970.
        result.format = "vagp";
        result.magic_confirmed = true;
    } else {
        const auto extension = extension_from_path(logical_path);
        result.format = extension.empty() ? "unknown" : extension;
    }

    result.container = is_container_format(result.format);
    return result;
}

ResourceClassification ResourceClassifier::classify(
    const ResourcePayload& payload,
    std::string_view naming_hint) {
    const auto bytes = std::span<const std::byte>{
        payload.bytes.data(), payload.bytes.size()};
    const auto physical_profile = profile_from_path(
        payload.resource.id.logical_path);

    const bool has_semantic_records = !payload.semantic_evidence.empty();
    if (has_semantic_records) {
        const auto digest = core::Sha256::compute(bytes).hex();
        for (const auto& evidence : payload.semantic_evidence) {
            if (!evidence.valid() ||
                evidence.authority_resource() != payload.resource.id ||
                evidence.authority_sha256() != digest) {
                continue;
            }

            ResourceClassification result;
            result.format = std::string{evidence.semantic_format()};
            result.profile = physical_profile;
            result.container = is_container_format(result.format);

            switch (evidence.kind()) {
            case ResourceSemanticEvidenceKind::embedded_name_list:
            case ResourceSemanticEvidenceKind::profile_structural_format:
                result.structural_confirmed = true;
                break;
            case ResourceSemanticEvidenceKind::magic_confirmed_format:
                result.magic_confirmed = true;
                break;
            case ResourceSemanticEvidenceKind::profile_runtime_content_tag:
                result.runtime_content_tag_confirmed = true;
                break;
            case ResourceSemanticEvidenceKind::profile_runtime_family_mask_tag:
                result.runtime_family_mask_confirmed = true;
                break;
            }
            return result;
        }

        auto result = classify(payload.resource.id.logical_path, bytes);
        result.profile = physical_profile;
        return result;
    }

    auto result = classify(
        naming_hint.empty() ? std::string_view{payload.resource.id.logical_path}
                            : naming_hint,
        bytes);
    result.profile = physical_profile;
    return result;
}

GameProfile ResourceClassifier::profile_from_path(
    std::string_view logical_path) {
    const auto path = lower_copy(logical_path);

    if (path.find("dmclauncher") != std::string::npos ||
        path.find("dmc launcher") != std::string::npos) {
        return GameProfile::dmc_launcher_hd;
    }
    if (path.find("dmc3") != std::string::npos ||
        path.find("devil may cry 3") != std::string::npos) {
        return GameProfile::dmc3_hd;
    }
    if (path.find("dmc2") != std::string::npos ||
        path.find("devil may cry 2") != std::string::npos) {
        return GameProfile::dmc2_hd;
    }
    if (path.find("dmc1") != std::string::npos ||
        path.find("devil may cry 1") != std::string::npos) {
        return GameProfile::dmc1_hd;
    }
    return GameProfile::unknown;
}

bool ResourceClassifier::is_container_format(
    std::string_view format) noexcept {
    return format == "nbz" || format == "afs" || format == "pac" ||
           format == "pnst";
}

} // namespace dmc::rengine::gdspaces
