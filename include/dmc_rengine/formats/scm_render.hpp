#pragma once

#include <array>
#include <cstddef>
#include <cstdint>

namespace dmc::rengine::formats::scm {

struct MeshRenderWords final {
    // Serialized mesh +0x04/+0x06/+0x08/+0x0A. Canonical DMC3 consumes
    // these four u16 values in 0x1402F9890. Their higher-level material or
    // render semantics remain unresolved, so keep neutral names.
    std::array<std::uint16_t, 4> values{};

    [[nodiscard]] constexpr std::uint16_t operator[](
        std::size_t index) const noexcept {
        return values[index];
    }
};

// Reconstruct the exact packed runtime state built by 0x1402F9890.
// A zero first word disables the packed state entirely. The stock 68-file
// SCM corpus uses zero for all four words, but the original executable has
// an explicit non-zero path and therefore these bytes are not reserved.
[[nodiscard]] constexpr std::uint64_t pack_mesh_render_words(
    const MeshRenderWords& words) noexcept {
    if (words.values[0] == 0U) return 0U;

    return (static_cast<std::uint64_t>(words.values[0]) << 4U) |
           0x0FULL |
           (static_cast<std::uint64_t>(words.values[1]) << 14U) |
           (static_cast<std::uint64_t>(words.values[2]) << 24U) |
           (static_cast<std::uint64_t>(words.values[3]) << 34U);
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
