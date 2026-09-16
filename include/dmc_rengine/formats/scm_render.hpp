#pragma once

#include <array>
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

// Canonical HD SCM does not translate texture state directly from serialized
// fields to D3D11. 0x1402F99B0/0x1402F9AC0 build a 0x50-byte legacy-GIF
// compatibility packet and 0x14030A5F8..0x14030A632 submit it through a
// REF-like DMA tag plus DIRECT(5). 0x140032CD0 decodes the packet as PACKED
// A+D. Only the low four register descriptors are authoritative because NREG=4.
inline constexpr std::uint64_t scm_material_gif_tag_qword =
    0x4000000000008001ULL; // NLOOP=1, EOP=1, FLG=PACKED, NREG=4.
inline constexpr std::uint64_t scm_material_gif_regs_qword =
    0x000000000020EEEEULL; // low four descriptors = A+D (0xE).
inline constexpr std::uint8_t scm_material_gif_nreg = 4U;
inline constexpr std::uint8_t legacy_gs_reg_tex0_1 = 0x06U;
inline constexpr std::uint8_t legacy_gs_reg_tex0_2 = 0x07U;
inline constexpr std::uint8_t legacy_gs_reg_clamp_1 = 0x08U;
inline constexpr std::uint8_t legacy_gs_reg_clamp_2 = 0x09U;
inline constexpr std::uint8_t legacy_gs_reg_tex1_1 = 0x14U;
inline constexpr std::uint8_t legacy_gs_reg_miptbp1_1 = 0x34U;
inline constexpr std::array<std::uint8_t, scm_material_gif_nreg>
    scm_material_ad_registers{
        legacy_gs_reg_tex0_1,
        legacy_gs_reg_tex1_1,
        legacy_gs_reg_clamp_1,
        legacy_gs_reg_miptbp1_1,
    };

static_assert(scm_material_ad_registers[0] == 0x06U);
static_assert(scm_material_ad_registers[1] == 0x14U);
static_assert(scm_material_ad_registers[2] == 0x08U);
static_assert(scm_material_ad_registers[3] == 0x34U);

// Live HD texture binding keeps the legacy TEX0 qword as compatibility state.
// 0x14002CC10 uses the low 14 TBP0 bits as the resource-registry key and also
// stores bits 36..37 as a two-bit base pixel-shader table selector. The same
// TBP0 key is registered by the external texture-companion resource path.
inline constexpr std::uint64_t legacy_gs_tex0_tbp0_mask = 0x3FFFULL;
inline constexpr unsigned legacy_gs_tex0_ps_base_key_shift = 36U;
inline constexpr std::uint32_t legacy_gs_tex0_ps_base_key_mask = 0x3U;

[[nodiscard]] constexpr std::uint16_t legacy_gs_tex0_resource_key(
    std::uint64_t tex0) noexcept {
    return static_cast<std::uint16_t>(tex0 & legacy_gs_tex0_tbp0_mask);
}

[[nodiscard]] constexpr std::uint8_t legacy_gs_tex0_ps_base_key(
    std::uint64_t tex0) noexcept {
    return static_cast<std::uint8_t>(
        (tex0 >> legacy_gs_tex0_ps_base_key_shift) &
        legacy_gs_tex0_ps_base_key_mask);
}

// 0x14002BC30 projects the active CLAMP_1/CLAMP_2 WMS/WMT modes into a compact
// sampler-state word passed to 0x140046910. This is the normal, non-one-shot
// override path. Modes 1/2 request D3D11 CLAMP for the corresponding axis;
// modes 0/3 retain WRAP. Therefore legacy REGION_REPEAT mode 3 is not directly
// represented by a native D3D11 address mode in this bridge. MIN/MAX remain
// preserved and present in GS compatibility state; this helper does not claim
// they are globally unused.
inline constexpr std::uint32_t hd_sampler_flag_filter = 0x01U;
inline constexpr std::uint32_t hd_sampler_flag_address_u_clamp = 0x02U;
inline constexpr std::uint32_t hd_sampler_flag_address_v_clamp = 0x04U;
inline constexpr std::uint32_t hd_sampler_flag_address_u_border = 0x20U;
inline constexpr std::uint32_t hd_sampler_flag_address_v_border = 0x40U;

inline constexpr std::uint32_t d3d11_filter_min_mag_point_mip_linear = 0x01U;
inline constexpr std::uint32_t d3d11_filter_anisotropic = 0x55U;
inline constexpr std::uint32_t d3d11_texture_address_wrap = 1U;
inline constexpr std::uint32_t d3d11_texture_address_clamp = 3U;
inline constexpr std::uint32_t d3d11_texture_address_border = 4U;

[[nodiscard]] constexpr std::uint32_t hd_sampler_flags_from_legacy_clamp(
    std::uint64_t clamp) noexcept {
    std::uint32_t flags = hd_sampler_flag_filter;
    const auto wms = static_cast<std::uint32_t>(clamp & 0x3ULL);
    const auto wmt = static_cast<std::uint32_t>((clamp >> 2U) & 0x3ULL);
    if (wms == 1U || wms == 2U) flags |= hd_sampler_flag_address_u_clamp;
    if (wmt == 1U || wmt == 2U) flags |= hd_sampler_flag_address_v_clamp;
    return flags;
}

struct HdD3d11SamplerProjection final {
    std::uint32_t filter{};
    std::uint32_t address_u{};
    std::uint32_t address_v{};
};

[[nodiscard]] constexpr HdD3d11SamplerProjection
project_hd_sampler_flags_to_d3d11(std::uint32_t flags) noexcept {
    HdD3d11SamplerProjection out{};
    out.filter = (flags & hd_sampler_flag_filter) != 0U
        ? d3d11_filter_anisotropic
        : d3d11_filter_min_mag_point_mip_linear;
    out.address_u = (flags & hd_sampler_flag_address_u_border) != 0U
        ? d3d11_texture_address_border
        : ((flags & hd_sampler_flag_address_u_clamp) != 0U
            ? d3d11_texture_address_clamp
            : d3d11_texture_address_wrap);
    out.address_v = (flags & hd_sampler_flag_address_v_border) != 0U
        ? d3d11_texture_address_border
        : ((flags & hd_sampler_flag_address_v_clamp) != 0U
            ? d3d11_texture_address_clamp
            : d3d11_texture_address_wrap);
    return out;
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
