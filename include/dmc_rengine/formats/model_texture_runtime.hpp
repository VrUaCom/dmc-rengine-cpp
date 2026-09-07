#pragma once

#include <array>
#include <cstddef>
#include <cstdint>

namespace dmc::rengine::formats::model_family {

// Runtime descriptor table built by the canonical DMC3-HD model texture path
// (0x14030D040). This is not a serialized companion structure and does not
// grant writer authority. It documents the in-memory 0x40-byte descriptor that
// model meshes index through texture_slot * 0x40.
struct RuntimeTextureDescriptorAbi final {
    static constexpr std::size_t record_size = 0x40U;
    static constexpr std::array<std::size_t, 4> mip_source_pointer_fields{
        0x00U, 0x08U, 0x10U, 0x18U};
    static constexpr std::size_t gs_tex0_field = 0x20U;
    static constexpr std::size_t gs_miptbp1_field = 0x28U;
    static constexpr std::size_t mip_count_minus_one_field = 0x30U;
    static constexpr std::size_t base_width_field = 0x34U;
};

// Exact bit decomposition of the PlayStation 2 GS TEX0 register image stored
// at runtime descriptor +0x20. The canonical builder explicitly populates the
// TBP0/TBW/PSM/TW/TH/TCC/CBP/CPSM/CLD fields; TFX/CSM/CSA are retained here
// because they are part of the register image and must not be silently lost.
struct LegacyGsTex0Fields final {
    std::uint16_t tbp0{};
    std::uint8_t tbw{};
    std::uint8_t psm{};
    std::uint8_t tw{};
    std::uint8_t th{};
    bool tcc{};
    std::uint8_t tfx{};
    std::uint16_t cbp{};
    std::uint8_t cpsm{};
    bool csm{};
    std::uint8_t csa{};
    std::uint8_t cld{};
};

[[nodiscard]] constexpr LegacyGsTex0Fields decode_legacy_gs_tex0(
    std::uint64_t raw) noexcept {
    return LegacyGsTex0Fields{
        .tbp0 = static_cast<std::uint16_t>((raw >> 0U) & 0x3FFFULL),
        .tbw = static_cast<std::uint8_t>((raw >> 14U) & 0x3FULL),
        .psm = static_cast<std::uint8_t>((raw >> 20U) & 0x3FULL),
        .tw = static_cast<std::uint8_t>((raw >> 26U) & 0x0FULL),
        .th = static_cast<std::uint8_t>((raw >> 30U) & 0x0FULL),
        .tcc = ((raw >> 34U) & 0x01ULL) != 0ULL,
        .tfx = static_cast<std::uint8_t>((raw >> 35U) & 0x03ULL),
        .cbp = static_cast<std::uint16_t>((raw >> 37U) & 0x3FFFULL),
        .cpsm = static_cast<std::uint8_t>((raw >> 51U) & 0x0FULL),
        .csm = ((raw >> 55U) & 0x01ULL) != 0ULL,
        .csa = static_cast<std::uint8_t>((raw >> 56U) & 0x1FULL),
        .cld = static_cast<std::uint8_t>((raw >> 61U) & 0x07ULL),
    };
}

struct LegacyGsMiptbpLevel final {
    std::uint16_t tbp{};
    std::uint8_t tbw{};
};

// GS MIPTBP1 image stored at runtime descriptor +0x28. It carries base-pointer
// and buffer-width pairs for mip levels 1..3. Bits 60..63 are not assigned by
// MIPTBP1 and are intentionally not exposed as semantic fields.
struct LegacyGsMiptbp1Fields final {
    LegacyGsMiptbpLevel mip1{};
    LegacyGsMiptbpLevel mip2{};
    LegacyGsMiptbpLevel mip3{};
};

[[nodiscard]] constexpr LegacyGsMiptbp1Fields decode_legacy_gs_miptbp1(
    std::uint64_t raw) noexcept {
    return LegacyGsMiptbp1Fields{
        .mip1 = LegacyGsMiptbpLevel{
            .tbp = static_cast<std::uint16_t>((raw >> 0U) & 0x3FFFULL),
            .tbw = static_cast<std::uint8_t>((raw >> 14U) & 0x3FULL),
        },
        .mip2 = LegacyGsMiptbpLevel{
            .tbp = static_cast<std::uint16_t>((raw >> 20U) & 0x3FFFULL),
            .tbw = static_cast<std::uint8_t>((raw >> 34U) & 0x3FULL),
        },
        .mip3 = LegacyGsMiptbpLevel{
            .tbp = static_cast<std::uint16_t>((raw >> 40U) & 0x3FFFULL),
            .tbw = static_cast<std::uint8_t>((raw >> 54U) & 0x3FULL),
        },
    };
}

static_assert(RuntimeTextureDescriptorAbi::record_size == 0x40U);
static_assert(RuntimeTextureDescriptorAbi::gs_tex0_field == 0x20U);
static_assert(RuntimeTextureDescriptorAbi::gs_miptbp1_field == 0x28U);
static_assert(RuntimeTextureDescriptorAbi::mip_count_minus_one_field == 0x30U);
static_assert(RuntimeTextureDescriptorAbi::base_width_field == 0x34U);

} // namespace dmc::rengine::formats::model_family
