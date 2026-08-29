#include "dmc_rengine/gdspaces/classifier.hpp"

#include "dmc_rengine/formats/pnst.hpp"

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
        // A four-byte signature is sufficient to retain an acquisition
        // candidate, but it is not a DMC3-HD parser/backend authority. Keep it
        // outside the expandable container set until a supported raw artifact
        // and a structurally validated parser are promoted together.
        result.format = "afs-binary-candidate";
        result.magic_confirmed = true;
    } else if (starts_with(bytes, "PACK")) {
        // The Web DMC Rengine v6 parser is product source, not original-game or
        // raw-corpus evidence. Do not route PACK-prefixed bytes into a parser
        // merely because the four-byte candidate identity was observed.
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
    } else {
        const auto extension = extension_from_path(logical_path);
        if (extension == "afs") {
            result.format = is_dmc3_afs_namespace_identity(logical_path)
                ? "afs-namespace"
                : "afs-binary-candidate";
        } else {
            result.format = extension.empty() ? "unknown" : extension;
        }
    }

    result.container = is_container_format(result.format);
    result.logical_namespace = result.format == "afs-namespace";
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
    // GData.afs is a logical namespace on the evidenced DMC3-HD path. Binary
    // AFS/PACK candidates remain non-expandable until parser authority exists.
    return format == "nbz" || format == "pac" || format == "pnst";
}

} // namespace dmc::rengine::gdspaces
