#include "dmc_rengine/gdspaces/classifier.hpp"

#include "dmc_rengine/formats/pnst.hpp"
#include "dmc_rengine/formats/ptx.hpp"

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

// Four-byte record tags observed in the real DMC3 stage corpus. Each one is a
// tag the bytes actually carry, not a guess from a filename — a slot payload
// has no name to guess from.
[[nodiscard]] std::string_view tagged_record_format(
    std::span<const std::byte> bytes) noexcept {
    struct TaggedRecord final {
        std::string_view tag;
        std::string_view format;
    };
    static constexpr TaggedRecord records[]{
        {std::string_view{"LIG2", 4U}, "lig2"},
        {std::string_view{"SEF\0", 4U}, "sef"},
        {std::string_view{"CAM\0", 4U}, "cam"},
        {std::string_view{"EVE\0", 4U}, "eve"},
        {std::string_view{"POS\0", 4U}, "pos"},
        {std::string_view{"ITM\0", 4U}, "itm"},
        {std::string_view{"STE\0", 4U}, "ste"},
        {std::string_view{"EST\0", 4U}, "est"},
    };
    for (const auto& record : records) {
        if (starts_with(bytes, record.tag)) {
            return record.format;
        }
    }
    return {};
}

// A DMC3 authoring record is line-oriented text in the encoding the studio
// wrote it in: ASCII with Shift-JIS comments. `st001.pac` slot 4 decodes as
//
//     uv    0, 14, -6, 0    ; パーツ番号、テクスチャ番号、U、V
//
// so a rule that only admits ASCII would call that record binary. This
// validates the whole payload as Shift-JIS instead of sampling it, and it
// rejects on the first byte that cannot be part of a text stream, which is why
// a multi-megabyte binary record costs one comparison here and not a scan.
[[nodiscard]] bool textual_record(std::span<const std::byte> bytes) noexcept {
    // Records are padded to their container alignment with NUL. That padding
    // is not part of the text and must not be judged as if it were.
    auto end = bytes.size();
    while (end > 0U &&
           std::to_integer<unsigned char>(bytes[end - 1U]) == 0x00U) {
        --end;
    }
    if (end < 4U) {
        return false;
    }

    const auto is_single = [](unsigned char value) noexcept {
        return value == 0x09U || value == 0x0AU || value == 0x0DU ||
            (value >= 0x20U && value <= 0x7EU) ||
            // Half-width katakana occupy a single byte in Shift-JIS.
            (value >= 0xA1U && value <= 0xDFU);
    };
    const auto is_lead = [](unsigned char value) noexcept {
        return (value >= 0x81U && value <= 0x9FU) ||
            (value >= 0xE0U && value <= 0xEFU);
    };
    const auto is_trail = [](unsigned char value) noexcept {
        return (value >= 0x40U && value <= 0x7EU) ||
            (value >= 0x80U && value <= 0xFCU);
    };

    bool has_line_break = false;
    for (std::size_t index = 0; index < end;) {
        const auto value = std::to_integer<unsigned char>(bytes[index]);
        if (is_single(value)) {
            has_line_break = has_line_break || value == 0x0AU || value == 0x0DU;
            ++index;
            continue;
        }
        if (is_lead(value) && index + 1U < end &&
            is_trail(std::to_integer<unsigned char>(bytes[index + 1U]))) {
            index += 2U;
            continue;
        }
        return false;
    }

    // A single printable line with no terminator is far more likely to be the
    // opening of a binary record than a text file, so require the line
    // structure that every observed authoring record has.
    return has_line_break;
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
    std::span<const std::byte> bytes,
    bool path_names_the_resource) {
    ResourceClassification result;
    result.profile = profile_from_path(logical_path);

    if (starts_with(bytes, "MZ")) {
        result.format = "pe";
        result.magic_confirmed = true;
        result.byte_derived = true;
    } else if (starts_with(bytes, std::string_view{"PAC\0", 4U})) {
        result.format = "pac";
        result.magic_confirmed = true;
        result.byte_derived = true;
    } else if (structurally_valid_binary_pnst(bytes)) {
        // Binary PNST commonly survives under a misleading .pac extension in
        // the extracted corpus. Structurally validated byte identity therefore
        // outranks extension, while PNST-prefixed text .index manifests fall
        // through to their path extension instead of becoming fake containers.
        result.format = "pnst";
        result.magic_confirmed = true;
        result.byte_derived = true;
    } else if (formats::PtxParser::structurally_valid(bytes)) {
        // A texture pack has no magic. It is recognized by its own arithmetic
        // closing on the stored length and by every block it declares opening
        // with a DDS image — a check no other record in the corpus passes.
        result.format = "ptx";
        result.byte_derived = true;
    } else if (starts_with(bytes, "SCM")) {
        result.format = "scm";
        result.magic_confirmed = true;
        result.byte_derived = true;
    } else if (starts_with(bytes, std::string_view{"DCA\0", 4U})) {
        result.format = "dca";
        result.magic_confirmed = true;
        result.byte_derived = true;
    } else if (starts_with(bytes, "HITS")) {
        result.format = "hits";
        result.magic_confirmed = true;
        result.byte_derived = true;
    } else if (starts_with(bytes, "DDS ")) {
        result.format = "dds";
        result.magic_confirmed = true;
        result.byte_derived = true;
    } else if (const auto tagged = tagged_record_format(bytes);
               !tagged.empty()) {
        // Slot payloads inside a DMC3 relative-slot container carry a four-byte
        // type tag and no name. Reading the tag is what separates "this is a
        // light rig" from "this is bytes", and without it every typed record in
        // a stage reads as the same anonymous blob.
        result.format = std::string{tagged};
        result.magic_confirmed = true;
        result.byte_derived = true;
    } else if (const auto extension = path_names_the_resource
                   ? extension_from_path(logical_path)
                   : std::string{};
               !extension.empty()) {
        // A real name outranks a byte probe. `em035_057.index` is a text file
        // whose extension says more than "text" does, and saying less than the
        // name already says is a loss.
        result.format = extension;
    } else if (textual_record(bytes)) {
        // Stage containers carry authoring text next to their binary records:
        // the name manifest, the `# GAME` scene block, the `# DOOR` table, the
        // effect id list. Those read as `bin` only because nothing looked, and
        // that `bin` came from the placeholder name we invented ourselves.
        // There is no magic byte to confirm here — the confirmation is that
        // every byte of the record is text — so this claims byte-derived
        // authority without claiming a signature.
        result.format = "txt";
        result.byte_derived = true;
    } else {
        result.format = "unknown";
    }

    result.container = is_container_format(result.format);
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
           format == "pnst" || format == "ptx";
}

} // namespace dmc::rengine::gdspaces
