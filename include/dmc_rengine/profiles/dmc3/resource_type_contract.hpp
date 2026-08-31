#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <span>
#include <string_view>

namespace dmc::rengine::profiles::dmc3 {

// Instruction-backed contracts for several distinct DMC3 runtime type-evidence
// sites. There is no single global "DMC3 type detector": the original runtime
// uses different byte comparisons for registry classification, container
// traversal, and higher-level resource/object family masks.
//
// Keep those evidence paths separate. In particular, the three-byte registry
// probe is exact for its site, but it is not proof that the fourth byte is
// globally ignored by the game.
struct ResourceTypeContract final {
    static constexpr std::string_view canonical_target_sha256 =
        "e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082";
    static constexpr std::uint64_t image_base = 0x140000000ULL;

    // ---------------------------------------------------------------------
    // Evidence site A: registry content probe.
    // ---------------------------------------------------------------------
    // Reads bytes 0..2 only and returns the registry TypeCode, or -1.
    // Exact recovered window:
    //   VA          0x1402DB1F0
    //   file offset 0x2DA5F0
    //   size        0x72
    static constexpr std::uint64_t registry_content_probe_va = 0x1402DB1F0ULL;
    static constexpr std::string_view registry_content_probe_window_sha256 =
        "4e614cc2d0168d6049a449ed4a1c6a78e0ebdd6b5c4b9699fabd98a63c153d19";

    // Compatibility name retained for callers written before the evidence-site
    // split. Its scope is explicitly the registry probe above.
    static constexpr std::uint64_t content_type_probe_va = registry_content_probe_va;

    // Registrar/classifier. Extension tests run before registry_content_probe_va.
    static constexpr std::uint64_t register_and_classify_va = 0x1402DB3C0ULL;

    static constexpr std::uint64_t table_reset_va = 0x1402DB370ULL;
    static constexpr std::uint64_t table_find_va = 0x1402DB270ULL;

    // ---------------------------------------------------------------------
    // Evidence site B: PAC/PNST container dispatch.
    // ---------------------------------------------------------------------
    // Independent dispatcher reached while walking materialized container
    // children. It shares the normal handlers for MOD/EFM/SCM/SHW, but also
    // compares EFW/EFE as recognized sentinel/prefix cases that do not prove a
    // normal format handler or exact semantic schema.
    static constexpr std::uint64_t container_dispatch_va = 0x1401B9FA0ULL;

    // Compatibility name retained for older callers/docs.
    static constexpr std::uint64_t type_dispatch_va = container_dispatch_va;

    // ---------------------------------------------------------------------
    // Evidence site C: higher-level four-byte family mask probe.
    // ---------------------------------------------------------------------
    // This is a separate runtime type system. It compares exactly four bytes,
    // including the trailing ASCII space, and returns a high-nibble family mask.
    // Exact recovered window:
    //   VA          0x1402FD650
    //   file offset 0x2FCA50
    //   size        0x273
    static constexpr std::uint64_t family_mask_probe_va = 0x1402FD650ULL;
    static constexpr std::string_view family_mask_probe_window_sha256 =
        "a31a8c1e225bc62c07dea05921c42eeff85c28b2f4872713594262e579b91961";

    enum class TypeCode : std::int32_t {
        model = 0,          // registry tag "MOD"
        effect_model = 1,   // registry tag "EFM"
        scene_model = 2,    // registry tag "SCM"
        mrp = 3,            // registry tag "MRP"
        texture_pack = 4,   // extension ".ptx"
        palette = 5,        // extension ".clt"
        c1d = 6,            // extension ".c1d"
        shadow = 7,         // registry tag "SHW"
        unknown = -1,
    };

    struct TaggedType final {
        std::string_view tag;
        TypeCode code;
        std::uint64_t handler_va;
    };

    static constexpr std::size_t registry_content_tag_bytes = 3U;
    static constexpr std::size_t content_tag_bytes = registry_content_tag_bytes;

    // Census for registry_content_probe_va only. Do not describe this array as
    // the complete global runtime tag census.
    static constexpr std::array<TaggedType, 5> registry_tagged_types{
        TaggedType{"MOD", TypeCode::model, 0x1402FE3B0ULL},
        TaggedType{"EFM", TypeCode::effect_model, 0x1402F7A90ULL},
        TaggedType{"SCM", TypeCode::scene_model, 0x1403051B0ULL},
        TaggedType{"MRP", TypeCode::mrp, 0U},
        TaggedType{"SHW", TypeCode::shadow, 0x1403204C0ULL},
    };

    // Compatibility alias for pre-split callers.
    static constexpr auto tagged_types = registry_tagged_types;

    enum class ContainerDispatchDisposition : std::uint8_t {
        normal_handler,
        recognized_sentinel,
    };

    struct ContainerDispatchPrefix final {
        std::string_view tag;
        std::uint64_t handler_va;
        ContainerDispatchDisposition disposition;
    };

    // These are the directly confirmed three-byte cases relevant to the
    // correction pass. EFW/EFE are deliberately not assigned invented formats.
    static constexpr std::array<ContainerDispatchPrefix, 6> container_dispatch_prefixes{
        ContainerDispatchPrefix{
            "MOD", 0x1402FE3B0ULL,
            ContainerDispatchDisposition::normal_handler},
        ContainerDispatchPrefix{
            "EFM", 0x1402F7A90ULL,
            ContainerDispatchDisposition::normal_handler},
        ContainerDispatchPrefix{
            "SCM", 0x1403051B0ULL,
            ContainerDispatchDisposition::normal_handler},
        ContainerDispatchPrefix{
            "SHW", 0x1403204C0ULL,
            ContainerDispatchDisposition::normal_handler},
        ContainerDispatchPrefix{
            "EFW", 0U,
            ContainerDispatchDisposition::recognized_sentinel},
        ContainerDispatchPrefix{
            "EFE", 0U,
            ContainerDispatchDisposition::recognized_sentinel},
    };

    enum class FamilyMask : std::uint32_t {
        unknown = 0x00000000U,
        mod = 0x10000000U,
        efm = 0x20000000U,
        scm = 0x30000000U,
        mrp = 0x40000000U,
        mcv = 0x50000000U,
        shw = 0x60000000U,
    };

    struct FamilyMaskTag final {
        std::string_view tag4;
        FamilyMask mask;
    };

    static constexpr std::size_t family_mask_tag_bytes = 4U;
    static constexpr std::array<FamilyMaskTag, 6> family_mask_tags{
        FamilyMaskTag{"MOD ", FamilyMask::mod},
        FamilyMaskTag{"EFM ", FamilyMask::efm},
        FamilyMaskTag{"SCM ", FamilyMask::scm},
        FamilyMaskTag{"MRP ", FamilyMask::mrp},
        FamilyMaskTag{"MCV ", FamilyMask::mcv},
        FamilyMaskTag{"SHW ", FamilyMask::shw},
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
    static constexpr bool extension_outranks_registry_content_tag = true;
    static constexpr bool extension_outranks_content_tag =
        extension_outranks_registry_content_tag;

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
        for (const auto& entry : registry_tagged_types) {
            if (entry.tag == tag) {
                return entry.code;
            }
        }
        return TypeCode::unknown;
    }

    [[nodiscard]] static constexpr bool prefix_matches(
        std::span<const std::byte> bytes,
        std::string_view tag) noexcept {
        if (bytes.size() < tag.size()) {
            return false;
        }
        for (std::size_t index = 0U; index < tag.size(); ++index) {
            if (std::to_integer<unsigned char>(bytes[index]) !=
                static_cast<unsigned char>(tag[index])) {
                return false;
            }
        }
        return true;
    }

    // Exact implementation of registry_content_probe_va. No fourth byte is
    // inspected at this site and no case folding occurs.
    [[nodiscard]] static constexpr TypeCode registry_type_for_prefix(
        std::span<const std::byte> bytes) noexcept {
        if (bytes.size() < registry_content_tag_bytes) {
            return TypeCode::unknown;
        }
        for (const auto& entry : registry_tagged_types) {
            if (prefix_matches(bytes, entry.tag)) {
                return entry.code;
            }
        }
        return TypeCode::unknown;
    }

    // Compatibility wrapper: this means the registry three-byte probe only.
    [[nodiscard]] static constexpr TypeCode type_for_prefix(
        std::span<const std::byte> bytes) noexcept {
        return registry_type_for_prefix(bytes);
    }

    [[nodiscard]] static constexpr ContainerDispatchDisposition
    container_dispatch_disposition(
        std::span<const std::byte> bytes,
        std::string_view tag) noexcept {
        for (const auto& entry : container_dispatch_prefixes) {
            if (entry.tag == tag && prefix_matches(bytes, entry.tag)) {
                return entry.disposition;
            }
        }
        // The return value is meaningful only together with
        // container_dispatch_recognizes_prefix(); normal_handler is chosen as a
        // harmless default so no synthetic "unknown" disposition is invented.
        return ContainerDispatchDisposition::normal_handler;
    }

    [[nodiscard]] static constexpr bool container_dispatch_recognizes_prefix(
        std::span<const std::byte> bytes) noexcept {
        for (const auto& entry : container_dispatch_prefixes) {
            if (prefix_matches(bytes, entry.tag)) {
                return true;
            }
        }
        return false;
    }

    // Exact implementation of the recovered four-byte family classifier.
    // Unlike registry_type_for_prefix(), this requires the trailing ASCII space.
    [[nodiscard]] static constexpr FamilyMask family_mask_for_prefix(
        std::span<const std::byte> bytes) noexcept {
        if (bytes.size() < family_mask_tag_bytes) {
            return FamilyMask::unknown;
        }
        for (const auto& entry : family_mask_tags) {
            if (prefix_matches(bytes, entry.tag4)) {
                return entry.mask;
            }
        }
        return FamilyMask::unknown;
    }

    // Canonical semantic/presentation extension for a registry type. This is
    // deliberately separate from historical filename authority: it states what
    // that registry evidence site identifies the bytes as, not what an extractor
    // happened to call them.
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

// Registry probe: fourth byte is deliberately ignored at this one site.
static_assert([] {
    constexpr std::array<std::byte, 4> bytes{
        std::byte{'M'}, std::byte{'O'}, std::byte{'D'}, std::byte{0x7F}};
    return ResourceTypeContract::registry_type_for_prefix(bytes) ==
        ResourceTypeContract::TypeCode::model;
}());
static_assert([] {
    constexpr std::array<std::byte, 4> bytes{
        std::byte{'S'}, std::byte{'C'}, std::byte{'M'}, std::byte{' '}};
    return ResourceTypeContract::registry_type_for_prefix(bytes) ==
        ResourceTypeContract::TypeCode::scene_model;
}());
static_assert([] {
    constexpr std::array<std::byte, 3> bytes{
        std::byte{'m'}, std::byte{'o'}, std::byte{'d'}};
    return ResourceTypeContract::registry_type_for_prefix(bytes) ==
        ResourceTypeContract::TypeCode::unknown;
}());

// Family-mask probe: the fourth byte is authoritative and must be ASCII space.
static_assert([] {
    constexpr std::array<std::byte, 4> bytes{
        std::byte{'M'}, std::byte{'O'}, std::byte{'D'}, std::byte{' '}};
    return ResourceTypeContract::family_mask_for_prefix(bytes) ==
        ResourceTypeContract::FamilyMask::mod;
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
        ResourceTypeContract::FamilyMask::mcv;
}());
static_assert([] {
    constexpr std::array<std::byte, 4> bytes{
        std::byte{'M'}, std::byte{'C'}, std::byte{'V'}, std::byte{' '}};
    return ResourceTypeContract::registry_type_for_prefix(bytes) ==
        ResourceTypeContract::TypeCode::unknown;
}());

// Container dispatcher recognizes EFW/EFE, but that does not manufacture a
// normal handler or semantic format for them.
static_assert([] {
    constexpr std::array<std::byte, 4> bytes{
        std::byte{'E'}, std::byte{'F'}, std::byte{'W'}, std::byte{0x00}};
    return ResourceTypeContract::container_dispatch_recognizes_prefix(bytes);
}());
static_assert([] {
    constexpr std::array<std::byte, 4> bytes{
        std::byte{'E'}, std::byte{'F'}, std::byte{'E'}, std::byte{0x00}};
    return ResourceTypeContract::container_dispatch_recognizes_prefix(bytes);
}());

} // namespace dmc::rengine::profiles::dmc3
