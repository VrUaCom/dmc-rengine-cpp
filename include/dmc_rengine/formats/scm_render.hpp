#pragma once

#include <cstdint>

namespace dmc::rengine::formats::scm {

// Serialized mesh +0x04/+0x06/+0x08/+0x0A are consumed by the canonical
// executable as the four payload fields of a legacy PlayStation 2 GS CLAMP
// register with WMS=WMT=3 (REGION_REPEAT). The shift pattern is bit-for-bit
// identical to GS_SETREG_CLAMP(3,3,MINU,MAXU,MINV,MAXV).
struct LegacyGsClampRegionRepeat final {
    std::uint16_t min_u{}; // GS CLAMP.MINU; region-repeat U mask semantics.
    std::uint16_t max_u{}; // GS CLAMP.MAXU; region-repeat U fix semantics.
    std::uint16_t min_v{}; // GS CLAMP.MINV; region-repeat V mask semantics.
    std::uint16_t max_v{}; // GS CLAMP.MAXV; region-repeat V fix semantics.
};

inline constexpr std::uint16_t gs_clamp_field_max = 0x03FFU;
inline constexpr std::uint64_t gs_clamp_region_repeat_modes = 0x0FULL;

[[nodiscard]] constexpr bool legacy_gs_clamp_fields_fit_register(
    const LegacyGsClampRegionRepeat& clamp) noexcept {
    return clamp.min_u <= gs_clamp_field_max &&
           clamp.max_u <= gs_clamp_field_max &&
           clamp.min_v <= gs_clamp_field_max &&
           clamp.max_v <= gs_clamp_field_max;
}

// Exact reconstruction of 0x1402F9890. The original DMC3 runtime uses
// MINU==0 as a disabled/sentinel case and returns zero. It also deliberately
// does not mask source u16 values before shifting, so anomalous bytes remain
// observable instead of being silently truncated to the GS 10-bit fields.
[[nodiscard]] constexpr std::uint64_t pack_legacy_gs_clamp_region_repeat(
    const LegacyGsClampRegionRepeat& clamp) noexcept {
    if (clamp.min_u == 0U) return 0U;

    return (static_cast<std::uint64_t>(clamp.min_u) << 4U) |
           gs_clamp_region_repeat_modes |
           (static_cast<std::uint64_t>(clamp.max_u) << 14U) |
           (static_cast<std::uint64_t>(clamp.min_v) << 24U) |
           (static_cast<std::uint64_t>(clamp.max_v) << 34U);
}

// The adjacent descriptor qword at +0x08 is a legacy PS2 GS TEX1 filtering
// state. 0x60 decodes exactly as MMAG=1 and MMIN=1: linear magnification and
// linear minification, with mipmapping fields zero. Serialized object flag
// 0x00004000 forces TEX1=0, i.e. nearest magnification/minification.
inline constexpr std::uint32_t object_flag_nearest_texture_filter =
    0x00004000U;
inline constexpr std::uint64_t legacy_gs_tex1_nearest_filter = 0x00U;
inline constexpr std::uint64_t legacy_gs_tex1_linear_filter = 0x60U;

[[nodiscard]] constexpr std::uint64_t legacy_gs_tex1_filter_from_object_flags(
    std::uint32_t object_flags) noexcept {
    return (object_flags & object_flag_nearest_texture_filter) != 0U
        ? legacy_gs_tex1_nearest_filter
        : legacy_gs_tex1_linear_filter;
}

struct AlphaControlProjection final {
    // These values correspond to runtime object +0x17C / +0x178 and
    // MDL_PARTS_COLOR_PKT.alpha.w respectively.
    std::uint32_t runtime_control_value{};
    std::uint32_t runtime_override_code{};
    float packet_alpha_w{};
};

inline constexpr std::uint8_t low_mode_forced_alpha_control = 0x80U;
inline constexpr float alpha_byte_scale = 1.0F / 255.0F;

// Reconstruct the common post-correction alpha-control path:
//   0x1403032A2..0x1403032DB -> runtime +0x17C/+0x178
//   0x140304111..0x140304167 -> MDL_PARTS_COLOR_PKT.alpha.w
//
// `effective_control` is intentionally the byte *after* the executable's
// narrow hard-coded C4/EA corrections. A non-zero low source mode bypasses
// those corrections and forces effective_control = 0x80.
[[nodiscard]] constexpr AlphaControlProjection project_effective_alpha_control(
    std::uint8_t effective_control) noexcept {
    AlphaControlProjection out{};
    out.runtime_control_value = effective_control;
    if (effective_control > 0x80U) {
        out.runtime_override_code = effective_control;
        out.packet_alpha_w = 1.0F;
    } else {
        out.packet_alpha_w =
            static_cast<float>(effective_control) * alpha_byte_scale;
    }
    return out;
}

} // namespace dmc::rengine::formats::scm
