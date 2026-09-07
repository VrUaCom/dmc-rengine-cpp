#pragma once

#include "dmc_rengine/formats/mod.hpp"

#include <bit>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <span>

namespace dmc::rengine::analysis::mod {

// MOD-specific serialized fields proven live by the canonical MOD/EFM object
// initializer 0x1403029E0. They are deliberately not added to the shared
// ObjectCoreAbi: homologous SCM bytes do not yet have the same proven contract.
struct ObjectRuntimeSerializedAbi final {
    static constexpr std::size_t raw_parameter18_f32_field = 0x18U;
    static constexpr std::size_t raw_parameter1c_u32_field = 0x1CU;
};

struct ObjectRuntimeSourceFields final {
    float raw_parameter18_f32{};
    std::uint32_t raw_parameter1c_u32{};
};

[[nodiscard]] constexpr std::optional<ObjectRuntimeSourceFields>
decode_object_runtime_source_fields(
    std::span<const std::byte> bytes,
    std::size_t object_record_offset) noexcept {
    constexpr std::size_t required_end =
        ObjectRuntimeSerializedAbi::raw_parameter1c_u32_field + 4U;
    if (object_record_offset > bytes.size() ||
        required_end > bytes.size() - object_record_offset) {
        return std::nullopt;
    }

    const auto read_u32_le = [&](std::size_t relative) constexpr {
        std::uint32_t value{};
        for (std::size_t i = 0U; i < 4U; ++i) {
            value |= static_cast<std::uint32_t>(
                std::to_integer<std::uint8_t>(
                    bytes[object_record_offset + relative + i])) << (8U * i);
        }
        return value;
    };

    const auto raw18 = read_u32_le(
        ObjectRuntimeSerializedAbi::raw_parameter18_f32_field);
    const auto raw1c = read_u32_le(
        ObjectRuntimeSerializedAbi::raw_parameter1c_u32_field);
    return ObjectRuntimeSourceFields{
        .raw_parameter18_f32 = std::bit_cast<float>(raw18),
        .raw_parameter1c_u32 = raw1c,
    };
}

[[nodiscard]] inline std::optional<ObjectRuntimeSourceFields>
read_object_runtime_source_fields(
    const dmc::rengine::formats::mod::Document& document,
    std::size_t outer_index) noexcept {
    if (outer_index >= document.outer_models.size()) return std::nullopt;
    const auto offset64 = document.outer_models[outer_index].record_offset;
    if (offset64 > static_cast<std::uint64_t>(document.source_bytes.size()))
        return std::nullopt;
    return decode_object_runtime_source_fields(
        std::span<const std::byte>(document.source_bytes),
        static_cast<std::size_t>(offset64));
}

// Runtime-object flag word starts at 3 before the source bitmap is projected.
inline constexpr std::uint32_t runtime_flag_baseline = 0x00000003U;
inline constexpr float source_flag_0x20000_xyz_preset = 128.0F;

struct ObjectRuntimeProjection final {
    std::uint32_t source_flags{};
    std::uint32_t runtime_flags{runtime_flag_baseline};

    // Low source nibble is passed to the legacy packet helper and also forces
    // runtime alpha/control state to 0x80 in 0x1403029E0.
    bool low_mode_active{false};
    std::uint8_t low_mode{};
    bool forces_alpha_control_0x80{false};

    // Source 0x0F000000 maps to runtime bit15 and an encoded zero-based byte.
    bool high_mode_active{false};
    std::uint8_t high_mode_index{};

    // Source 0x200/0x400 both request manager-global bit21 and copy the two
    // serialized +0x18/+0x1C values into runtime render-parameter storage.
    bool requests_manager_global_bit21{false};
    bool copies_conditional_render_parameters{false};
    float raw_parameter18_f32{};
    std::uint32_t raw_parameter1c_u32{};

    // Source 0x20000 seeds runtime +0x160/+0x164/+0x168 with 128.0f and +0x16C
    // with 0 before any conditional +0x18 overwrite.
    bool seeds_xyz_128_preset{false};

    // Shared material helper 0x1402F9890 consumes 0x4000 as the nearest-filter
    // selection. Keep this separate from the runtime object flag word.
    bool nearest_texture_filter{false};

    // MOD-specific post-initializer 0x1402FF570 changes runtime bytes +0x05 and
    // +0x06 from 4 to 12 when either source 0x1000 or 0x2000 is present.
    std::uint8_t mod_runtime_byte05{4U};
    std::uint8_t mod_runtime_byte06{4U};
};

[[nodiscard]] constexpr ObjectRuntimeProjection project_object_runtime(
    std::uint32_t source_flags,
    ObjectRuntimeSourceFields source_fields = {}) noexcept {
    ObjectRuntimeProjection out;
    out.source_flags = source_flags;
    out.raw_parameter18_f32 = source_fields.raw_parameter18_f32;
    out.raw_parameter1c_u32 = source_fields.raw_parameter1c_u32;

    const auto low_mode = static_cast<std::uint8_t>(source_flags & 0x0FU);
    if (low_mode != 0U) {
        out.low_mode_active = true;
        out.low_mode = low_mode;
        out.forces_alpha_control_0x80 = true;
        out.runtime_flags |= 0x00000100U; // runtime bit 8
    }
    if ((source_flags & 0x00000020U) != 0U)
        out.runtime_flags |= 0x00000400U; // runtime bit 10
    if ((source_flags & 0x00020000U) != 0U) {
        out.runtime_flags |= 0x00000200U; // runtime bit 9
        out.seeds_xyz_128_preset = true;
    }
    if ((source_flags & 0x00010000U) != 0U)
        out.runtime_flags |= 0x00000080U; // runtime bit 7
    if ((source_flags & 0x00000200U) != 0U) {
        out.runtime_flags |= 0x00001000U; // runtime bit 12
        out.requests_manager_global_bit21 = true;
        out.copies_conditional_render_parameters = true;
    }
    if ((source_flags & 0x00000400U) != 0U) {
        out.runtime_flags |= 0x00002000U; // runtime bit 13
        out.requests_manager_global_bit21 = true;
        out.copies_conditional_render_parameters = true;
    }
    if ((source_flags & 0x00000010U) != 0U)
        out.runtime_flags |= 0x00000800U; // runtime bit 11
    if ((source_flags & 0x00040000U) != 0U)
        out.runtime_flags |= 0x00000010U; // runtime bit 4

    const auto high_mode = static_cast<std::uint8_t>(
        (source_flags >> 24U) & 0x0FU);
    if (high_mode != 0U) {
        out.runtime_flags |= 0x00008000U; // runtime bit 15
        out.high_mode_active = true;
        out.high_mode_index = static_cast<std::uint8_t>(high_mode - 1U);
    }

    out.nearest_texture_filter =
        (source_flags & 0x00004000U) != 0U;

    if ((source_flags & 0x00003000U) != 0U) {
        out.mod_runtime_byte05 = 0x0CU;
        out.mod_runtime_byte06 = 0x0CU;
    }
    return out;
}

[[nodiscard]] inline std::optional<ObjectRuntimeProjection>
analyze_object_runtime(
    const dmc::rengine::formats::mod::Document& document,
    std::size_t outer_index) noexcept {
    if (outer_index >= document.outer_models.size()) return std::nullopt;
    const auto fields = read_object_runtime_source_fields(document, outer_index);
    if (!fields) return std::nullopt;
    return project_object_runtime(
        document.outer_models[outer_index].source_flags,
        *fields);
}

static_assert(ObjectRuntimeSerializedAbi::raw_parameter18_f32_field == 0x18U);
static_assert(ObjectRuntimeSerializedAbi::raw_parameter1c_u32_field == 0x1CU);

} // namespace dmc::rengine::analysis::mod
