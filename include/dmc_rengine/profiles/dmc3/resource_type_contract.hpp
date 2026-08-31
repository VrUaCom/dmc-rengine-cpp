#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <span>
#include <string_view>

namespace dmc::rengine::profiles::dmc3 {

// Instruction-backed contracts for the original DMC3 HD runtime's resource
// identification paths. There is no single global "magic detector": reverse of
// the canonical executable proves three distinct mechanisms with different
// byte widths and scopes.
struct ResourceTypeContract final {
    static constexpr std::string_view canonical_target_sha256 =
        "e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082";
    static constexpr std::uint64_t image_base = 0x140000000ULL;

    // Registry content probe. Reads exactly bytes 0..2 and returns a small
    // registry type code, or -1. This is one runtime identification path, not a
    // claim that every runtime consumer ignores byte 3.
    static constexpr std::uint64_t content_type_probe_va = 0x1402DB1F0ULL;

    // Registrar/classifier. Extension tests run before the three-byte probe.
    static constexpr std::uint64_t register_and_classify_va = 0x1402DB3C0ULL;
    static constexpr std::uint64_t table_reset_va = 0x1402DB370ULL;
    static constexpr std::uint64_t table_find_va = 0x1402DB270ULL;

    // Independent container dispatcher reached from PAC/PNST walks. It handles
    // MOD/EFM/SCM/SHW, recognizes EFW/EFE as no-handler sentinels, and recurses
    // into PNST.
    static constexpr std::uint64_t type_dispatch_va = 0x1401B9FA0ULL;

    // Independent four-byte family-mask classifier. Unlike the registry probe,
    // this site requires the fourth byte to be ASCII space and additionally
    // recognizes MCV.
    static constexpr std::uint64_t family_mask_probe_va = 0x1402FD650ULL;

    enum class TypeCode : std::int32_t {
        model = 0,          // three-byte tag "MOD"
        effect_model = 1,   // three-byte tag "EFM"
        scene_model = 2,    // three-byte tag "SCM"
        mrp = 3,            // three-byte tag "MRP"
        texture_pack = 4,   // extension ".ptx"
        palette = 5,        // extension ".clt"
        c1d = 6,            // extension ".c1d"
        shadow = 7,         // three-byte tag "SHW"
        unknown = -1,
    };

    struct TaggedType final {
        std::string_view tag;
        TypeCode code;
        std::uint64_t handler_va;
    };

    static constexpr std::size_t content_tag_bytes = 3U;
    static constexpr std::array<TaggedType, 5> tagged_types{
        TaggedType{"MOD", TypeCode::model, 0x1402FE3B0ULL},
        TaggedType{"EFM", TypeCode::effect_model, 0x1402F7A90ULL},
        TaggedType{"SCM", TypeCode::scene_model, 0x1403051B0ULL},
        TaggedType{"MRP", TypeCode::mrp, 0U},
        TaggedType{"SHW", TypeCode::shadow, 0x1403204C0ULL},
    };

    // The container dispatcher also compares these exact three-byte prefixes,
    // but returns without dispatching a recovered handler. They are therefore
    // runtime-recognized sentinels, not promoted here to semantic file formats.
    static constexpr std::array<std::string_view, 2> dispatcher_no_handler_tags{
        "EFW", "EFE"};

    enum class FamilyMask : std::uint32_t {
        unknown = 0x00000000U,
        model = 0x10000000U,          // "MOD "
        effect_model = 0x20000000U,   // "EFM "
        scene_model = 0x30000000U,    // "SCM "
        mrp = 0x40000000U,            // "MRP "
        motion_curve = 0x50000000U,   // "MCV "
        shadow = 0x60000000U,         // "SHW "
    };

    struct FamilyTaggedType final {
        std::string_view tag_with_space;
        FamilyMask mask;
        std::string_view canonical_extension;
    };

    static constexpr std::size_t family_tag_bytes = 4U;
    static constexpr std::array<FamilyTaggedType, 6> family_tagged_types{
        FamilyTaggedType{"MOD ", FamilyMask::model, "mod"},
        FamilyTaggedType{"EFM ", FamilyMask::effect_model, "efm"},
        FamilyTaggedType{"SCM ", FamilyMask::scene_model, "scm"},
        FamilyTaggedType{"MRP ", FamilyMask::mrp, "mrp"},
        FamilyTaggedType{"MCV ", FamilyMask::motion_curve, "mcv"},
        FamilyTaggedType{"SHW ", FamilyMask::shadow, "shw"},
    };

    struct ExtensionType final {
        std::string_view extension;
        TypeCode code;
    };

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

    static constexpr bool extension_matched_as_substring = true;
    static constexpr std::string_view match_function = "strstr";
    static constexpr std::uint64_t match_import_slot_va = 0x14034F3D0ULL;
    static constexpr bool extension_outranks_content_tag = true;

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

    [[nodiscard]] static constexpr TypeCode type_for_name(
        std::string_view name) noexcept {
        for (const auto& entry : extension_types) {
            if (name.find(entry.extension) != std::string_view::npos) {
                return entry.code;
            }
        }
        return TypeCode::unknown;
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

    // Exact implementation of the recovered registry three-byte probe. No
    // fourth byte is inspected and no case folding occurs at this site.
    [[nodiscard]] static constexpr TypeCode type_for_prefix(
        std::span<const std::byte> bytes) noexcept {
        if (bytes.size() < content_tag_bytes) {
            return TypeCode::unknown;
        }
        for (const auto& entry : tagged_types) {
            bool matches = true;
            for (std::size_t index = 0U; index < content_tag_bytes; ++index) {
                if (std::to_integer<unsigned char>(bytes[index]) !=
                    static_cast<unsigned char>(entry.tag[index])) {
                    matches = false;
                    break;
                }
            }
            if (matches) {
                return entry.code;
            }
        }
        return TypeCode::unknown;
    }

    // Exact implementation of the independent four-byte family-mask probe.
    // Here byte 3 is significant and must be ASCII space.
    [[nodiscard]] static constexpr FamilyMask family_mask_for_prefix(
        std::span<const std::byte> bytes) noexcept {
        if (bytes.size() < family_tag_bytes) {
            return FamilyMask::unknown;
        }
        for (const auto& entry : family_tagged_types) {
            bool matches = true;
            for (std::size_t index = 0U; index < family_tag_bytes; ++index) {
                if (std::to_integer<unsigned char>(bytes[index]) !=
                    static_cast<unsigned char>(entry.tag_with_space[index])) {
                    matches = false;
                    break;
                }
            }
            if (matches) {
                return entry.mask;
            }
        }
        return FamilyMask::unknown;
    }

    [[nodiscard]] static constexpr std::string_view canonical_extension(
        TypeCode code) noexcept {
        switch (code) {
        case TypeCode::model: return "mod";
        case TypeCode::effect_model: return "efm";
        case TypeCode::scene_model: return "scm";
        case TypeCode::mrp: return "mrp";
        case TypeCode::texture_pack: return "ptx";
        case TypeCode::palette: return "clt";
        case TypeCode::c1d: return "c1d";
        case TypeCode::shadow: return "shw";
        case TypeCode::unknown: return {};
        }
        return {};
    }

    [[nodiscard]] static constexpr std::string_view canonical_extension(
        FamilyMask mask) noexcept {
        for (const auto& entry : family_tagged_types) {
            if (entry.mask == mask) {
                return entry.canonical_extension;
            }
        }
        return {};
    }

    static constexpr std::string_view registered_name_format = "%s/%s";

    [[nodiscard]] static consteval std::size_t table_bytes() noexcept {
        return table_type_offset +
            table_capacity * sizeof(std::int32_t);
    }
};

static_assert(
    ResourceTypeContract::type_for_tag("MOD") ==
    ResourceTypeContract::TypeCode::model);
static_assert(ResourceTypeContract::canonical_extension(
                  ResourceTypeContract::TypeCode::model) == "mod");
static_assert(ResourceTypeContract::canonical_extension(
                  ResourceTypeContract::TypeCode::effect_model) == "efm");
static_assert(ResourceTypeContract::canonical_extension(
                  ResourceTypeContract::TypeCode::scene_model) == "scm");
static_assert(ResourceTypeContract::canonical_extension(
                  ResourceTypeContract::TypeCode::mrp) == "mrp");
static_assert(ResourceTypeContract::canonical_extension(
                  ResourceTypeContract::TypeCode::shadow) == "shw");

// Three-byte registry probe: byte 3 is deliberately ignored.
static_assert([] {
    constexpr std::array<std::byte, 4> bytes{
        std::byte{'M'}, std::byte{'O'}, std::byte{'D'}, std::byte{0x7F}};
    return ResourceTypeContract::type_for_prefix(bytes) ==
        ResourceTypeContract::TypeCode::model;
}());
static_assert([] {
    constexpr std::array<std::byte, 3> bytes{
        std::byte{'m'}, std::byte{'o'}, std::byte{'d'}};
    return ResourceTypeContract::type_for_prefix(bytes) ==
        ResourceTypeContract::TypeCode::unknown;
}());

// Four-byte family-mask probe: byte 3 is part of the recovered signature.
static_assert([] {
    constexpr std::array<std::byte, 4> bytes{
        std::byte{'M'}, std::byte{'O'}, std::byte{'D'}, std::byte{' '}};
    return ResourceTypeContract::family_mask_for_prefix(bytes) ==
        ResourceTypeContract::FamilyMask::model;
}());
static_assert([] {
    constexpr std::array<std::byte, 4> bytes{
        std::byte{'M'}, std::byte{'O'}, std::byte{'D'}, std::byte{'X'}};
    return ResourceTypeContract::family_mask_for_prefix(bytes) ==
        ResourceTypeContract::FamilyMask::unknown;
}());
static_assert([] {
    constexpr std::array<std::byte, 4> bytes{
        std::byte{'M'}, std::byte{'C'}, std::byte{'V'}, std::byte{' '}};
    return ResourceTypeContract::family_mask_for_prefix(bytes) ==
        ResourceTypeContract::FamilyMask::motion_curve &&
        ResourceTypeContract::canonical_extension(
            ResourceTypeContract::FamilyMask::motion_curve) == "mcv";
}());

} // namespace dmc::rengine::profiles::dmc3
