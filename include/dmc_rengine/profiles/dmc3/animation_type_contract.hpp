#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <string_view>

namespace dmc::rengine::profiles::dmc3 {

// Instruction-backed contract for the second resource registry — the one that
// types motion, curve, camera, hide, palette and TSC resources.
//
// `ResourceTypeContract` recovered a registry that asks the name first and the
// bytes second. This is a *different* registry, at a different address, with a
// different capacity and a different type table, and it never falls back to
// the bytes at all: a name that matches none of its extensions is refused.
//
// Finding it matters because the first registry looked complete. Animation is
// the largest thing missing from an unpacked stage folder, and it is missing
// because it is typed here, not there.
struct AnimationTypeContract final {
    static constexpr std::string_view canonical_target_sha256 =
        "e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082";
    static constexpr std::uint64_t image_base = 0x140000000ULL;

    static constexpr std::uint64_t register_and_classify_va = 0x1402E01A0ULL;
    static constexpr std::uint64_t table_reset_va = 0x1402E0150ULL;
    static constexpr std::uint64_t extension_literal_table_va = 0x1405071D0ULL;

    enum class TypeCode : std::int32_t {
        motion = 0,   // .mot
        curve = 1,    // .mcv
        camera = 2,   // .cam
        hide = 3,     // .hid
        palette = 4,  // .clt
        tsc = 5,      // .tsc
        unregistered = -1,
    };

    struct ExtensionType final {
        std::string_view extension;
        TypeCode code;
    };

    // In image order. Case is enumerated in pairs rather than folded, exactly
    // as in the first registry.
    static constexpr std::array<ExtensionType, 12> extension_types{
        ExtensionType{".mot", TypeCode::motion},
        ExtensionType{".MOT", TypeCode::motion},
        ExtensionType{".mcv", TypeCode::curve},
        ExtensionType{".MCV", TypeCode::curve},
        ExtensionType{".cam", TypeCode::camera},
        ExtensionType{".CAM", TypeCode::camera},
        ExtensionType{".hid", TypeCode::hide},
        ExtensionType{".HID", TypeCode::hide},
        ExtensionType{".clt", TypeCode::palette},
        ExtensionType{".CLT", TypeCode::palette},
        ExtensionType{".tsc", TypeCode::tsc},
        ExtensionType{".TSC", TypeCode::tsc},
    };

    // The payload's own tag is inert here. A `.mot` record carries `MOT` at
    // `+4`, and a whole-image search finds that four-byte value nowhere in
    // `.text` — not as a comparison and not as a constructor's type field, the
    // way `LIG2` appears once. The runtime knows a motion by its name and by
    // nothing else, so the tag in the file is for whoever made the file.
    static constexpr bool payload_tag_is_compared = false;

    // The difference that matters. The first registry types by extension and,
    // failing that, by the payload's own three-byte tag. This one has no
    // content probe: an unmatched name is not registered at all.
    static constexpr bool has_content_tag_fallback = false;
    static constexpr bool extension_matched_as_substring = true;

    // A separate, larger table: 1024 entries against the first registry's 256.
    static constexpr std::size_t table_capacity = 0x400U;
    static constexpr std::size_t table_entry_stride = 0x20U;
    static constexpr std::size_t table_count_offset = 0x0000U;
    static constexpr std::size_t table_name_offset = 0x8008U;
    static constexpr std::size_t table_flag_offset = 0x18008U;
    static constexpr std::size_t table_type_offset = 0x18408U;
    // Written and never read back, exactly as in the first registry.
    static constexpr bool stored_type_is_read_back = false;
    static constexpr std::int32_t table_reset_type_value = -1;

    // `.clt` appears in both registries. It is the one extension two different
    // tables claim, and they give it different codes — 5 there, 4 here — so a
    // type code is only meaningful together with the registry that issued it.
    static constexpr std::string_view shared_extension = ".clt";

    // And it is shared literally, not just conceptually.
    //
    // This registry's own literal block holds ten entries — five extensions in
    // lower and upper case — in 80 bytes at `extension_literal_table_va`. The
    // classifier compares twelve. The other two are `.clt` and `.CLT`, and it
    // reaches into the *first* registry's block for them rather than carrying
    // its own copies. That is why a reader that measures this table by its
    // contiguous bytes finds five formats and the code implements six.
    static constexpr std::size_t own_literal_count = 10U;
    static constexpr std::size_t own_literal_bytes = 0x50U;
    static constexpr std::size_t compared_literal_count = 12U;
    static constexpr std::uint64_t shared_lowercase_literal_va = 0x140507088ULL;
    static constexpr std::uint64_t shared_uppercase_literal_va = 0x140507090ULL;
    // The first registry carries a third, capitalized variant of the shared
    // extension. This classifier does not compare it.
    static constexpr std::string_view uncompared_case_variant = ".Clt";

    // The key the registrar builds before storing: the group and the name,
    // joined by a slash. A resource is identified by a pair, not by a name
    // alone, which is what lets two containers hold the same member name.
    static constexpr std::string_view registry_key_format = "%s/%s";
    static constexpr std::uint64_t registry_key_format_va = 0x140507068ULL;
    static constexpr std::size_t registry_key_arity = 2U;

    // The lookup that runs first; the registrar is called only when it returns
    // a negative index, so registration is idempotent per key.
    static constexpr std::uint64_t find_or_register_va = 0x1402E0020ULL;
    static constexpr std::uint64_t lookup_va = 0x1402E0060ULL;

    // The extension is matched against the name's tail, and the whole chain is
    // an ordered sequence of comparisons rather than a table walk: the first
    // match wins and the last pair falls through to its own store. Recorded
    // because a reader that sorts the table changes which extension wins for a
    // name that ends in two of them.
    static constexpr bool comparison_order_is_significant = true;
    static constexpr std::uint64_t first_comparison_va = 0x1402E01D2ULL;
    static constexpr std::uint64_t fallthrough_store_va = 0x1402E0377ULL;

    [[nodiscard]] static consteval std::size_t table_bytes() noexcept {
        return table_type_offset + table_capacity * sizeof(std::int32_t);
    }

    [[nodiscard]] static constexpr TypeCode type_for_extension(
        std::string_view extension) noexcept {
        for (const auto& entry : extension_types) {
            if (entry.extension == extension) {
                return entry.code;
            }
        }
        return TypeCode::unregistered;
    }

    // The registry's own rule, applied to a whole name.
    //
    // The comparison is `strstr`, resolved through the import table at
    // `0x14034F3D0`: the extension is looked for **anywhere in the name**, not
    // at its end. That is not a detail. It is why the game reads `pl000.mot1`
    // through `pl000.mot6` as motions without carrying a single numbered
    // literal — a whole-image search finds no `.mot1` anywhere, because none
    // is needed.
    //
    // This reader first matched the tail instead, which is the obvious reading
    // and the wrong one: it refused exactly the numbered names the game
    // accepts. `extension_matched_as_substring` had said so since the registry
    // was recovered, and nothing asserted the implementation against it.
    //
    // Case is still enumerated in pairs rather than folded, so `.Mot` is not a
    // motion here, and neither is `.Clt`, which exists as a literal but is
    // compared only by the other registry.
    [[nodiscard]] static constexpr TypeCode type_for_name(
        std::string_view name) noexcept {
        for (const auto& entry : extension_types) {
            if (name.find(entry.extension) != std::string_view::npos) {
                return entry.code;
            }
        }
        return TypeCode::unregistered;
    }

    // The import the comparison actually goes through, and its slot.
    static constexpr std::string_view match_function = "strstr";
    static constexpr std::uint64_t match_import_slot_va = 0x14034F3D0ULL;

    // A consequence of substring matching worth stating rather than
    // rediscovering: a name carrying two of these extensions is typed by
    // whichever comes first in the *chain*, not by whichever appears first in
    // the name. The chain order is the code's, not the table's.
    static constexpr bool first_match_wins_by_chain_order = true;

    [[nodiscard]] static constexpr bool is_animation_format(
        std::string_view format) noexcept {
        // A format string carries no dot, so compare against the tail.
        for (const auto& entry : extension_types) {
            if (entry.extension.substr(1U) == format) {
                return true;
            }
        }
        return false;
    }

    // Whether this project can read the kind, as opposed to name it. Recorded
    // in the contract so the gap is a fact the code holds rather than a note
    // in a document that drifts.
    [[nodiscard]] static constexpr bool structure_is_recovered(
        TypeCode code) noexcept {
        return code == TypeCode::motion;
    }
};

} // namespace dmc::rengine::profiles::dmc3
