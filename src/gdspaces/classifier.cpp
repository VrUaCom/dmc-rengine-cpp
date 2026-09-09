#include "dmc_rengine/gdspaces/classifier.hpp"

#include "dmc_rengine/core/sha256.hpp"
#include "dmc_rengine/formats/pnst.hpp"
#include "dmc_rengine/gdspaces/resource_payload.hpp"
#include "dmc_rengine/profiles/dmc3/resource_type_contract.hpp"

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

[[nodiscard]] bool is_dmc3_afs_namespace_identity(
    std::string_view logical_path) {
    auto path = lower_copy(logical_path);
    std::replace(path.begin(), path.end(), '\\', '/');
    while (!path.empty() && path.back() == '/') {
        path.pop_back();
    }

    const auto separator = path.find_last_of('/');
    const auto leaf = separator == std::string::npos
        ? std::string_view{path}
        : std::string_view{path}.substr(separator + 1U);
    return leaf == "gdata.afs" || leaf == "gdatax360.afs";
}

[[nodiscard]] bool structurally_valid_binary_pnst(
    std::span<const std::byte> bytes) {
    if (!starts_with(bytes, "PNST")) {
        return false;
    }

    // Real DMC3 extracted corpora also contain text .index manifests whose
    // first line is literally "PNST\r\n". The four-byte prefix is therefore a
    // probe candidate, not sufficient binary-container authority. Reuse the
    // canonical structural parser so classification and materialization cannot
    // disagree about whether the supplied byte image is a relative-slot PNST.
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
        // Binary PNST commonly survives under a misleading .pac extension in
        // the extracted corpus. Structurally validated byte identity therefore
        // outranks extension, while PNST-prefixed text .index manifests fall
        // through to their path extension instead of becoming fake containers.
        result.format = "pnst";
        result.magic_confirmed = true;
    } else if (starts_with(bytes, std::string_view{"AFS\0", 4U})) {
        // Retain exact-signature input as an acquisition candidate only. The
        // DMC3-HD evidence establishes .afs/ namespace strings, not an opaque
        // binary AFS backend on the canonical path.
        result.format = "afs-binary-candidate";
        result.magic_confirmed = true;
    } else if (starts_with(bytes, "PACK")) {
        // Historical product-side PACK parsing is not original-runtime parser
        // authority. Keep the byte identity visible without making it an
        // expandable container.
        result.format = "pack-binary-candidate";
        result.magic_confirmed = true;
    } else if (starts_with(bytes, "SCM")) {
        result.format = "scm";
        result.magic_confirmed = true;
    } else if (starts_with(bytes, std::string_view{"DCA\0", 4U})) {
        result.format = "dca";
        result.magic_confirmed = true;
    } else if (starts_with(bytes, "HITS")) {
        result.format = "hits";
        result.magic_confirmed = true;
    } else if (starts_with(bytes, "DDS ")) {
        result.format = "dds";
        result.magic_confirmed = true;
    } else if (starts_with(bytes, std::string_view{"PK\x03\x04", 4U})) {
        // NBZ is a ZIP container. Without this the format was only ever
        // reached through a ".nbz" path suffix, so a nested archive under any
        // other name stopped the container walk.
        result.format = "nbz";
        result.magic_confirmed = true;
    } else {
        // Remaining recognition is driven by the recovered runtime contract
        // rather than a parallel literal list here, so a type census added to
        // ResourceTypeContract cannot silently miss the classifier.
        using profiles::dmc3::ResourceTypeContract;
        const auto family = ResourceTypeContract::family_mask_for_prefix(bytes);
        if (family != ResourceTypeContract::FamilyMask::unknown) {
            result.format =
                std::string{ResourceTypeContract::canonical_extension(family)};
            result.magic_confirmed = true;
            result.runtime_family_mask_confirmed = true;
        } else if (is_dmc3_afs_namespace_identity(logical_path)) {
            // `GData.afs/` and `GDataX360.afs/` are logical namespaces inside
            // the recovered DMC3 resource lookup policy. They are not a second
            // binary container layer.
            result.format = "afs-namespace";
        } else {
            const auto extension = extension_from_path(logical_path);
            result.format = extension == "afs"
                ? "afs-binary-candidate"
                : (extension.empty() ? "unknown" : extension);
        }
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
    // DMC3-HD `.afs/` tokens are logical namespace identities. Binary AFS and
    // PACK candidates stay non-expandable until a profile-specific backend is
    // independently evidenced and promoted.
    return format == "nbz" || format == "pac" || format == "pnst";
}

} // namespace dmc::rengine::gdspaces
