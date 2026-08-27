#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <string_view>

namespace dmc::rengine::profiles::dmc3 {

// Instruction-backed contract for how the original runtime decides what a
// resource *is*.
//
// This project had been identifying formats by magic bytes because that is what
// a tool does. The runtime does something narrower, and the difference matters:
// it reads a **three-byte** tag, not four, and it consults the *extension*
// first for the three types that have one. Recovering that changes which
// claims the product may make — a reader that requires `SCM ` with the trailing
// space is stricter than the game, and a reader that trusts a four-byte `PAC\0`
// is claiming a comparison the game never performs.
struct ResourceTypeContract final {
    static constexpr std::string_view canonical_target_sha256 =
        "e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082";
    static constexpr std::uint64_t image_base = 0x140000000ULL;

    // The content probe. Reads bytes 0..2 of the resource and returns a type
    // code, or -1 for a payload it does not recognize.
    static constexpr std::uint64_t content_type_probe_va = 0x1402DB1F0ULL;

    // The registrar that names a resource, loads it and classifies it. The
    // extension tests live here and run *before* the content probe.
    static constexpr std::uint64_t register_and_classify_va = 0x1402DB3C0ULL;

    // The fixed-capacity table the registrar fills, its reset, and its lookup.
    static constexpr std::uint64_t table_reset_va = 0x1402DB370ULL;
    static constexpr std::uint64_t table_find_va = 0x1402DB270ULL;

    // The second dispatcher: same tags, calling the same handlers, reached
    // from the container walk rather than from registration. Two independent
    // call sites agreeing is why the handler map below is stated as recovered
    // rather than as a single-site reading.
    static constexpr std::uint64_t type_dispatch_va = 0x1401B9FA0ULL;

    // Recovered type codes. The probe returns these; the registrar stores them
    // in the table's per-entry type array.
    //
    // **The stored code is not a dispatch key.** A whole-image search finds
    // the type array written and never read back through its offset. Dispatch
    // happens at registration, where the registrar calls the handler directly,
    // and the second dispatcher re-probes the payload's tag rather than
    // consulting what was stored. So the array is recorded state for a later
    // query, and a product that treated the code as the thing that selects a
    // reader would be describing a mechanism the runtime does not have.
    enum class TypeCode : std::int32_t {
        model = 0,          // tag "MOD"
        effect_model = 1,   // tag "EFM"
        scene_model = 2,    // tag "SCM"
        mrp = 3,            // tag "MRP"
        texture_pack = 4,   // extension ".ptx"
        palette = 5,        // extension ".clt"
        c1d = 6,            // extension ".c1d"
        shadow = 7,         // tag "SHW"
        unknown = -1,
    };

    struct TaggedType final {
        std::string_view tag;
        TypeCode code;
        // The handler the dispatcher calls for this type. Zero where the
        // recovered code stores the type and calls nothing.
        std::uint64_t handler_va;
    };

    // Three bytes, never four. `SCM` is compared without the trailing space the
    // stored payload happens to carry.
    static constexpr std::size_t content_tag_bytes = 3U;

    static constexpr std::array<TaggedType, 5> tagged_types{
        TaggedType{"MOD", TypeCode::model, 0x1402FE3B0ULL},
        TaggedType{"EFM", TypeCode::effect_model, 0x1402F7A90ULL},
        TaggedType{"SCM", TypeCode::scene_model, 0x1403051B0ULL},
        TaggedType{"MRP", TypeCode::mrp, 0U},
        TaggedType{"SHW", TypeCode::shadow, 0x1403204C0ULL},
    };

    struct ExtensionType final {
        std::string_view extension;
        TypeCode code;
    };

    // The literal table the registrar tests against, in image order. Case is
    // enumerated rather than folded, so `.PTx` is a type the runtime does not
    // recognize even though `.PTX` is — a product that lowercases the name
    // before testing is more permissive than the runtime, not equally so.
    static constexpr std::uint64_t extension_literal_table_va = 0x140507070ULL;
    static constexpr std::array<ExtensionType, 10> extension_types{
        ExtensionType{".ptx", TypeCode::texture_pack},
        ExtensionType{".PTX", TypeCode::texture_pack},
        ExtensionType{".Ptx", TypeCode::texture_pack},
        ExtensionType{".clt", TypeCode::palette},
        ExtensionType{".CLT", TypeCode::palette},
        ExtensionType{".Clt", TypeCode::palette},
        ExtensionType{".c1d", TypeCode::c1d},
        ExtensionType{".C1D", TypeCode::c1d},
        ExtensionType{".c1D", TypeCode::c1d},
        ExtensionType{".C1d", TypeCode::c1d},
    };

    // The extension is tested with a substring search over the whole name, not
    // against its suffix. `at.ptx.bak` therefore classifies as a texture pack
    // in the original runtime.
    //
    // The comparison goes through the same import slot as the animation
    // registry's — `0x14034F3D0`, `strstr` in `VCRUNTIME140.dll`. Both
    // registries share it. This flag stood here with nothing implementing it
    // until the animation side was caught disagreeing with its own flag, so
    // the matcher below exists to be asserted against.
    static constexpr bool extension_matched_as_substring = true;
    static constexpr std::string_view match_function = "strstr";
    static constexpr std::uint64_t match_import_slot_va = 0x14034F3D0ULL;
    static constexpr bool extension_outranks_content_tag = true;

    // Table geometry, from the reset routine and the registrar's index
    // arithmetic. A registration beyond the capacity is refused, not wrapped.
    static constexpr std::size_t table_capacity = 0x100U;
    static constexpr std::size_t table_entry_stride = 0x20U;
    static constexpr std::size_t table_count_offset = 0x0000U;
    static constexpr std::size_t table_entry_a_offset = 0x0008U;
    static constexpr std::size_t table_entry_b_offset = 0x2008U;
    static constexpr std::size_t table_entry_c_offset = 0x4008U;
    static constexpr std::size_t table_flag_offset = 0x6008U;
    static constexpr std::size_t table_type_offset = 0x6108U;
    static constexpr bool stored_type_is_read_back = false;
    static constexpr std::int32_t table_reset_type_value = -1;

    // The registrar's rule, applied to a whole name.
    //
    // Substring, not suffix, and case enumerated rather than folded — the same
    // two properties as the animation registry, because it is literally the
    // same comparison function over a different literal table.
    //
    // What this answers is narrow and worth being precise about: it is what
    // the *runtime* would call a name, which is not the same as what this
    // project can read. `at.ptx` is a texture pack by this rule whether or not
    // any parser here accepts its bytes.
    [[nodiscard]] static constexpr TypeCode type_for_name(
        std::string_view name) noexcept {
        for (const auto& entry : extension_types) {
            if (name.find(entry.extension) != std::string_view::npos) {
                return entry.code;
            }
        }
        return TypeCode::unknown;
    }

    // The name the registrar resolves is built from two components.
    static constexpr std::string_view registered_name_format = "%s/%s";

    [[nodiscard]] static consteval std::size_t table_bytes() noexcept {
        return table_type_offset +
            table_capacity * sizeof(std::int32_t);
    }

    [[nodiscard]] static constexpr TypeCode type_for_tag(
        std::string_view tag) noexcept {
        for (const auto& entry : tagged_types) {
            if (entry.tag == tag) {
                return entry.code;
            }
        }
        return TypeCode::unknown;
    }
};

} // namespace dmc::rengine::profiles::dmc3
