#pragma once

#include <cstdint>

namespace dmc::rengine::formats::scm {

// Serialized mesh +0x04/+0x06/+0x08/+0x0A are consumed by the canonical
// executable as the four payload fields of a legacy PlayStation 2 GS CLAMP
// register with WMS=WMT=3 (REGION_REPEAT). The shift pattern is bit-for-bit
// identical to GS_SETREG_CLAMP(3,3,MINU,MAXU,MINV,MAXV).
//
// The original DMC3 runtime uses min_u == 0 as a disabled/sentinel case and
// returns a zero packed state rather than emitting the otherwise-valid low
// WMS/WMT bits. Keep that DMC-specific behavior exact.
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

// Exact reconstruction of 0x1402F9890. Deliberately does not mask the source
// u16 values before shifting: anomalous/modded bytes must remain observable
// rather than being silently normalized to the 10-bit GS field width.
[[nodiscard]] constexpr std::uint64_t pack_legacy_gs_clamp_region_repeat(
    const LegacyGsClampRegionRepeat& clamp) noexcept {
    if (clamp.min_u == 0U) return 0U;

    return (static_cast<std::uint64_t>(clamp.min_u) << 4U) |
           gs_clamp_region_repeat_modes |
           (static_cast<std::uint64_t>(clamp.max_u) << 14U) |
           (static_cast<std::uint64_t>(clamp.min_v) << 24U) |
           (static_cast<std::uint64_t>(clamp.max_v) << 34U);
}

// Serialized object +0x10 is copied to runtime object +0x14. The canonical
// SCM mesh-material builder 0x1402F9890 uses source bit 0x00004000 to select
// this otherwise-opaque runtime descriptor field. Do not assign a cull,
// blend, depth or sampler name until its downstream consumer is closed.
inline constexpr std::uint32_t object_flag_mesh_descriptor_00004000 =
    0x00004000U;
inline constexpr std::uint64_t mesh_descriptor_default_field_08 = 0x60U;

[[nodiscard]] constexpr std::uint64_t mesh_descriptor_field_08(
    std::uint32_t object_flags) noexcept {
    return (object_flags & object_flag_mesh_descriptor_00004000) != 0U
        ? 0U
        : mesh_descriptor_default_field_08;
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
