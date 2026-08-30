#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <string_view>

namespace dmc::rengine::profiles::dmc3 {

// Instruction-backed contract for how the original runtime decides what a
// resource *is*.
//
// The runtime's content probe reads exactly three bytes. Extension-backed
// types are handled by the registrar before this probe; nameless PAC/PNST slot
// payloads therefore rely on the content tag when one exists. Keeping this
// contract in the canonical core prevents mobile/tooling consumers from
// inventing their own format authority.
struct ResourceTypeContract final {
    static constexpr std::string_view canonical_target_sha256 =
        "e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082";
    static constexpr std::uint64_t image_base = 0x140000000ULL;

    // Reads bytes 0..2 of the resource and returns a type code, or -1.
    static constexpr std::uint64_t content_type_probe_va = 0x1402DB1F0ULL;

    // Registrar/classifier. Extension tests run before the content probe.
    static constexpr std::uint64_t register_and_classify_va = 0x1402DB3C0ULL;

    static constexpr std::uint64_t table_reset_va = 0x1402DB370ULL;
    static constexpr std::uint64_t table_find_va = 0x1402DB270ULL;

    // Independent dispatcher reached from the container walk.
    static constexpr std::uint64_t type_dispatch_va = 0x1401B9FA0ULL;

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

    // Canonical semantic/presentation extension for a recovered runtime type.
    // This is deliberately separate from historical filename authority: it
    // states what the runtime identifies the bytes as, not what an extractor
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

} // namespace dmc::rengine::profiles::dmc3
