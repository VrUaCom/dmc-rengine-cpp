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
    static constexpr std::int32_t table_reset_type_value = -1;

    // `.clt` appears in both registries. It is the one extension two different
    // tables claim, and they give it different codes — 5 there, 4 here — so a
    // type code is only meaningful together with the registry that issued it.
    static constexpr std::string_view shared_extension = ".clt";

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
};

} // namespace dmc::rengine::profiles::dmc3
