#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <span>
#include <string_view>

namespace dmc::rengine::profiles::dmc3 {

// Instruction-backed contracts for the original DMC3 HD runtime's resource
// identification paths. There is no single global "magic detector": reverse of
// the canonical executable proves distinct mechanisms with different byte widths,
// call sites and authority scopes.
struct ResourceTypeContract final {
    static constexpr std::string_view canonical_target_sha256 =
        "e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082";
    static constexpr std::uint64_t image_base = 0x140000000ULL;

    // ---------------------------------------------------------------------
    // Evidence site A: registry content probe.
    // ---------------------------------------------------------------------
    // Reads exactly bytes 0..2 and returns a small registry type code, or -1.
    // This is one runtime identification path, not proof that every runtime
    // consumer ignores byte 3.
    static constexpr std::uint64_t registry_content_probe_va = 0x1402DB1F0ULL;
    static constexpr std::uint64_t registry_content_probe_file_offset = 0x2DA5F0ULL;
    static constexpr std::size_t registry_content_probe_size = 0x72U;
    static constexpr std::string_view registry_content_probe_window_sha256 =
        "4e614cc2d0168d6049a449ed4a1c6a78e0ebdd6b5c4b9699fabd98a63c153d19";

    // Compatibility alias retained for pre-split callers/docs. New code should
    // prefer registry_content_probe_va to make the evidence site explicit.
    static constexpr std::uint64_t content_type_probe_va = registry_content_probe_va;

    // Registrar/classifier. Extension tests run before the registry content probe.
    static constexpr std::uint64_t register_and_classify_va = 0x1402DB3C0ULL;
    static constexpr std::uint64_t table_reset_va = 0x1402DB370ULL;
    static constexpr std::uint64_t table_find_va = 0x1402DB270ULL;

    // ---------------------------------------------------------------------
    // Evidence site B: PAC/PNST container dispatcher.
    // ---------------------------------------------------------------------
    // Independent dispatcher reached from materialized container walks. It
    // reaches normal handlers for MOD/EFM/SCM/SHW, recognizes EFW/EFE as
    // no-handler sentinel prefixes, and recursively visits PNST children.
    static constexpr std::uint64_t container_dispatch_va = 0x1401B9FA0ULL;

    // Compatibility alias retained for pre-split callers/docs.
    static constexpr std::uint64_t type_dispatch_va = container_dispatch_va;

    // ---------------------------------------------------------------------
    // Evidence site C: higher-level four-byte family-mask classifier.
    // ---------------------------------------------------------------------
    // Unlike the registry probe, this site compares exactly four bytes,
    // requires trailing ASCII space and additionally recognizes MCV.
    static constexpr std::uint64_t family_mask_probe_va = 0x1402FD650ULL;
    static constexpr std::uint64_t family_mask_probe_file_offset = 0x2FCA50ULL;
    static constexpr std::size_t family_mask_probe_size = 0x273U;
    static constexpr std::string_view family_mask_probe_window_sha256 =
        "a31a8c1e225bc62c07dea05921c42eeff85c28b2f4872713594262e579b91961";

    enum class TypeCode : std::int32_t {
        model = 0,          // registry tag "MOD"
        effect_model = 1,   // registry tag "EFM"
        scene_model = 2,    // registry tag "SCM"
        mrp = 3,            // registry tag "MRP"
        texture_pack = 4,   // registrar extension ".ptx"
        palette = 5,        // registrar extension ".clt"
        c1d = 6,            // registrar extension ".c1d"
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

    // Census for registry_content_probe_va only. It is not the global DMC3
    // runtime type census.
    static constexpr std::array<TaggedType, 5> registry_tagged_types{
        TaggedType{"MOD", TypeCode::model, 0x1402FE3B0ULL},
        TaggedType{"EFM", TypeCode::effect_model, 0x1402F7A90ULL},
        TaggedType{"SCM", TypeCode::scene_model, 0x1403051B0ULL},
        TaggedType{"MRP", TypeCode::mrp, 0U},
        TaggedType{"SHW", TypeCode::shadow, 0x1403204C0ULL},
    };

    // Compatibility alias for old callers that treated this as a global census.
    static constexpr auto tagged_types = registry_tagged_types;

    enum class ContainerDispatchDisposition : std::uint8_t {
        unrecognized,
        normal_handler,
        recognized_sentinel,
    };

    struct ContainerDispatchPrefix final {
        std::string_view tag;
        std::uint64_t handler_va;
        ContainerDispatchDisposition disposition;
    };

    static constexpr std::array<ContainerDispatchPrefix, 6>
        container_dispatch_prefixes{
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

    // Compatibility view retained for older documentation/tests. These two tags
    // are recognized by the container dispatcher but do not prove normal format
    // handlers or decoded schemas.
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

    // Exact implementation of evidence site A. No fourth byte is inspected and
    // no case folding occurs at this site.
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
    container_dispatch_disposition_for_prefix(
        std::span<const std::byte> bytes) noexcept {
        for (const auto& entry : container_dispatch_prefixes) {
            if (prefix_matches(bytes, entry.tag)) {
                return entry.disposition;
            }
        }
        return ContainerDispatchDisposition::unrecognized;
    }

    [[nodiscard]] static constexpr bool container_dispatch_recognizes_prefix(
        std::span<const std::byte> bytes) noexcept {
        return container_dispatch_disposition_for_prefix(bytes) !=
            ContainerDispatchDisposition::unrecognized;
    }

    [[nodiscard]] static constexpr std::uint64_t
    container_dispatch_handler_for_prefix(
        std::span<const std::byte> bytes) noexcept {
        for (const auto& entry : container_dispatch_prefixes) {
            if (prefix_matches(bytes, entry.tag)) {
                return entry.handler_va;
            }
        }
        return 0U;
    }

    // Exact implementation of evidence site C. Here byte 3 is significant and
    // must be ASCII space.
    [[nodiscard]] static constexpr FamilyMask family_mask_for_prefix(
        std::span<const std::byte> bytes) noexcept {
        if (bytes.size() < family_tag_bytes) {
            return FamilyMask::unknown;
        }
        for (const auto& entry : family_tagged_types) {
            if (prefix_matches(bytes, entry.tag_with_space)) {
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

// Evidence site A: byte 3 is deliberately ignored only by this registry probe.
static_assert([] {
    constexpr std::array<std::byte, 4> bytes{
        std::byte{'M'}, std::byte{'O'}, std::byte{'D'}, std::byte{0x7F}};
    return ResourceTypeContract::registry_type_for_prefix(bytes) ==
        ResourceTypeContract::TypeCode::model;
}());
static_assert([] {
    constexpr std::array<std::byte, 3> bytes{
        std::byte{'m'}, std::byte{'o'}, std::byte{'d'}};
    return ResourceTypeContract::registry_type_for_prefix(bytes) ==
        ResourceTypeContract::TypeCode::unknown;
}());

// Evidence site B: EFW/EFE are recognized sentinels, not normal handlers.
static_assert([] {
    constexpr std::array<std::byte, 4> bytes{
        std::byte{'E'}, std::byte{'F'}, std::byte{'W'}, std::byte{0x00}};
    return ResourceTypeContract::container_dispatch_disposition_for_prefix(bytes) ==
            ResourceTypeContract::ContainerDispatchDisposition::recognized_sentinel &&
        ResourceTypeContract::container_dispatch_handler_for_prefix(bytes) == 0U;
}());
static_assert([] {
    constexpr std::array<std::byte, 4> bytes{
        std::byte{'E'}, std::byte{'F'}, std::byte{'E'}, std::byte{0x00}};
    return ResourceTypeContract::container_dispatch_disposition_for_prefix(bytes) ==
            ResourceTypeContract::ContainerDispatchDisposition::recognized_sentinel &&
        ResourceTypeContract::container_dispatch_handler_for_prefix(bytes) == 0U;
}());
static_assert([] {
    constexpr std::array<std::byte, 4> bytes{
        std::byte{'S'}, std::byte{'C'}, std::byte{'M'}, std::byte{' '}};
    return ResourceTypeContract::container_dispatch_disposition_for_prefix(bytes) ==
            ResourceTypeContract::ContainerDispatchDisposition::normal_handler &&
        ResourceTypeContract::container_dispatch_handler_for_prefix(bytes) ==
            0x1403051B0ULL;
}());

// Evidence site C: byte 3 is part of the recovered family signature.
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
            ResourceTypeContract::FamilyMask::motion_curve) == "mcv" &&
        ResourceTypeContract::registry_type_for_prefix(bytes) ==
            ResourceTypeContract::TypeCode::unknown;
}());

} // namespace dmc::rengine::profiles::dmc3
