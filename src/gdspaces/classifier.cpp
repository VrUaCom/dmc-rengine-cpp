#include "dmc_rengine/gdspaces/classifier.hpp"

#include "dmc_rengine/core/sha256.hpp"
#include "dmc_rengine/formats/mot.hpp"
#include "dmc_rengine/formats/pnst.hpp"
#include "dmc_rengine/formats/ptx.hpp"
#include "dmc_rengine/gdspaces/resource_payload.hpp"
#include "dmc_rengine/gdspaces/text_record.hpp"
#include "dmc_rengine/profiles/dmc3/animation_type_contract.hpp"
#include "dmc_rengine/profiles/dmc3/demo_script_contract.hpp"
#include "dmc_rengine/profiles/dmc3/relative_slot_walk_contract.hpp"
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

// Four-byte record tags observed in the real DMC3 stage corpus. Each one is a
// tag the bytes actually carry, not a guess from a filename — a slot payload
// has no name to guess from. These are corpus observations, not recovered
// comparisons, which is why they are checked after the recovered set.
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
    } else if (starts_with(
                   bytes,
                   profiles::dmc3::RelativeSlotWalkContract::pac_magic)) {
        // Three bytes, because that is what the recovered walk compares. The
        // stored fourth byte is NUL and the runtime never reads it, so a
        // product that demanded it would refuse a container the game accepts.
        // Whether the slot table then parses is the expander's answer, not
        // this one's: a truncated container is still a container, and saying
        // "unknown" about it would hide the damage rather than report it.
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
    } else if (formats::MotParser::structurally_valid(bytes)) {
        // A motion carries `MOT` at +4, but the runtime compares that tag
        // nowhere, so the tag alone is not authority. What is: the track chain
        // closing on the terminator with every declared size accounted for.
        result.format = "mot";
        result.byte_derived = true;
    } else if (formats::PtxParser::structurally_valid(bytes)) {
        // A texture pack has no magic. It is recognized by its own arithmetic
        // closing on the stored length and by every block it declares opening
        // with a DDS image — a check no other record in the corpus passes.
        result.format = "ptx";
        result.byte_derived = true;
    } else if (starts_with(bytes, std::string_view{"DCA\0", 4U})) {
        result.format = "dca";
        result.magic_confirmed = true;
        result.byte_derived = true;
    } else if (starts_with(bytes, "HITS")) {
        result.format = "hits";
        result.magic_confirmed = true;
        result.byte_derived = true;
    } else if (starts_with(bytes, profiles::dmc3::ClothSim1dContract::first_token)) {
        // The one self-identifying text format in this game: its parser
        // compares the file's first token for equality against `ClothSim1D`
        // and bails when it differs. Recognizing it costs a prefix compare and
        // claims nothing about the grammar, which is unrecovered.
        result.format = std::string{profiles::dmc3::ClothSim1dContract::extension};
        result.magic_confirmed = true;
        result.byte_derived = true;
    } else if (starts_with(bytes, std::string_view{"TM2\0", 4U})) {
        // Four bytes including the NUL, because the runtime compares the whole
        // dword at `0x1403365BA`. `PAC` is the opposite case and the
        // difference is the point: match what the game matches, no more and no
        // less.
        result.format = "tm2";
        result.magic_confirmed = true;
        result.byte_derived = true;
    } else if (starts_with(bytes, "DDS ")) {
        result.format = "dds";
        result.magic_confirmed = true;
        result.byte_derived = true;
    } else if (starts_with(bytes, std::string_view{"PK\x03\x04", 4U})) {
        // NBZ is a ZIP container. Without this the format was only ever
        // reached through a ".nbz" path suffix, so a nested archive under any
        // other name stopped the container walk.
        result.format = "nbz";
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
            result.byte_derived = true;
            result.runtime_family_mask_confirmed = true;
        } else if (const auto extension = path_names_the_resource
                       ? extension_from_path(logical_path)
                       : std::string{};
                   !extension.empty()) {
            // A real name outranks a byte probe. `em035_057.index` is a text
            // file whose extension says more than "text" does, and saying
            // less than the name already says is a loss.
            result.format = extension;
        } else if (TextRecord::inspect(bytes).recognized) {
            // Stage containers carry authoring text next to their binary
            // records: the name manifest, the `# GAME` scene block, the
            // `# DOOR` table, the effect id list. Those read as `bin` only
            // because nothing looked, and that `bin` came from the
            // placeholder name we invented ourselves. There is no magic byte
            // to confirm here — the confirmation is that every byte of the
            // record is text — so this claims byte-derived authority without
            // claiming a signature.
            result.format = "txt";
            result.byte_derived = true;
        } else {
            result.format = "unknown";
        }
    }

    // The second registry's verdict, recorded whether or not this project can
    // read the kind.
    //
    // Five of the six animation kinds have no corpus, so nothing here can
    // recognize one inside a container: the container stores no names and
    // there is no structure to probe. Saying that plainly is worth more than
    // leaving them indistinguishable from `unknown` — an operator looking for
    // animation deserves to know which of the six the tool can see and which
    // it can only be handed by name.
    // Asked of the path, because the registry is asked of the path.
    //
    // Deriving this from the format string instead loses the numbered names:
    // `pl000.mot1` has the extension `mot1`, which is not one of the six, but
    // the runtime runs `strstr(name, ".mot")` and finds one. So the name goes
    // in whole, exactly as the game passes it.
    //
    // Where the caller synthesized the name there is nothing to ask, and the
    // format is used instead — that is the nameless slot whose bytes were
    // recognized as a motion, the one case the registry itself could not type.
    using Animation = profiles::dmc3::AnimationTypeContract;
    const auto animation = path_names_the_resource
        ? Animation::type_for_name(logical_path)
        : (Animation::is_animation_format(result.format)
               ? Animation::type_for_name(std::string{"."} + result.format)
               : Animation::TypeCode::unregistered);
    result.animation_type = static_cast<std::int32_t>(animation);
    result.animation_structure_recovered =
        Animation::structure_is_recovered(animation);

    // A container claim is a claim about bytes, so it must not be made from a
    // name when the bytes were there to check.
    //
    // `at.ptx` in a real volume is exactly this case: nothing structural
    // recognized it, the extension named it `ptx`, and the product then
    // offered to expand it — a promise the expander refused the moment it was
    // taken up. The name is still worth keeping as the format, because the
    // name does say more than "bytes"; what it does not do is establish that
    // this file is a container.
    //
    // Where no bytes were supplied the claim stays optimistic, because an
    // index built before materialization has nothing better to go on, and a
    // tree that refused to offer expansion until every member was read would
    // be worse than one that occasionally has to take the offer back.
    const auto structural = !bytes.empty() &&
        is_structural_container_format(result.format) && !result.byte_derived;
    result.container = is_container_format(result.format) && !structural;
    return result;
}

ResourceClassification ResourceClassifier::classify(
    const ResourcePayload& payload,
    std::string_view naming_hint) {
    const auto bytes = std::span<const std::byte>{
        payload.bytes.data(), payload.bytes.size()};
    const auto physical_profile = profile_from_path(
        payload.resource.id.logical_path);

    if (!payload.semantic_evidence.empty()) {
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
            // A sealed record is still byte-derived because it is bound to the
            // exact current digest, but its *reason* must remain separate.
            result.byte_derived = true;
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

        // A semantic record is present but does not validate against the exact
        // current byte image. Ignore presentation and name hints entirely:
        // only the physical logical identity plus fresh bytes may classify a
        // stale resource. This is what stops a display name like
        // "st001_000.index" from turning stale embedded-name evidence into a
        // fake external-index semantic type.
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
           format == "pnst" || format == "ptx";
}

bool ResourceClassifier::is_structural_container_format(
    std::string_view format) noexcept {
    // The three whose container-ness is a statement about their bytes. `nbz`
    // and `afs` are volumes: they are mounted by name through a different
    // path, and a volume that fails to open says so as a volume.
    return format == "pac" || format == "pnst" || format == "ptx";
}

} // namespace dmc::rengine::gdspaces
