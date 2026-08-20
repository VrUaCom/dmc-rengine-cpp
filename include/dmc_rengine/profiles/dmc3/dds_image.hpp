#pragma once

#include <cstddef>
#include <cstdint>
#include <span>
#include <string_view>

namespace dmc::rengine::profiles::dmc3 {

enum class DdsCompressionKind : std::uint8_t {
    dxt1,
    dxt5,
};

enum class DdsHeaderProfile : std::uint8_t {
    standard_corpus,
    observed_depth1_exception,
};

enum class DdsImageStatus : std::uint8_t {
    ok,
    truncated,
    invalid_magic,
    invalid_header_size,
    invalid_flags,
    invalid_dimensions,
    invalid_mip_chain,
    unsupported_compression,
    invalid_linear_size,
    invalid_depth,
    nonzero_reserved_fields,
    invalid_pixel_format,
    invalid_caps,
    size_mismatch,
};

[[nodiscard]] constexpr std::string_view to_string(
    DdsImageStatus status) noexcept {
    switch (status) {
    case DdsImageStatus::ok: return "ok";
    case DdsImageStatus::truncated: return "truncated";
    case DdsImageStatus::invalid_magic: return "invalid-magic";
    case DdsImageStatus::invalid_header_size: return "invalid-header-size";
    case DdsImageStatus::invalid_flags: return "invalid-flags";
    case DdsImageStatus::invalid_dimensions: return "invalid-dimensions";
    case DdsImageStatus::invalid_mip_chain: return "invalid-mip-chain";
    case DdsImageStatus::unsupported_compression:
        return "unsupported-compression";
    case DdsImageStatus::invalid_linear_size: return "invalid-linear-size";
    case DdsImageStatus::invalid_depth: return "invalid-depth";
    case DdsImageStatus::nonzero_reserved_fields:
        return "nonzero-reserved-fields";
    case DdsImageStatus::invalid_pixel_format: return "invalid-pixel-format";
    case DdsImageStatus::invalid_caps: return "invalid-caps";
    case DdsImageStatus::size_mismatch: return "size-mismatch";
    }
    return "truncated";
}

struct DdsImageDocument final {
    DdsHeaderProfile profile{DdsHeaderProfile::standard_corpus};
    DdsCompressionKind compression{DdsCompressionKind::dxt1};
    std::uint32_t width{};
    std::uint32_t height{};
    std::uint32_t mip_map_count{};
    std::uint32_t pitch_or_linear_size{};
    std::uint32_t depth{};
    std::uint32_t payload_size{};
    std::uint64_t total_size{};

    [[nodiscard]] bool valid() const noexcept;
};

struct DdsImageResult final {
    DdsImageStatus status{DdsImageStatus::truncated};
    DdsImageDocument document;
    std::string_view detail;

    [[nodiscard]] bool ok() const noexcept;
};

class DdsImageParser final {
public:
    static constexpr std::size_t k_file_header_size = 128U;
    static constexpr std::uint32_t k_header_struct_size = 124U;
    static constexpr std::uint32_t k_required_flags = 0x000A1007U;
    static constexpr std::uint32_t k_pixel_format_size = 32U;
    static constexpr std::uint32_t k_pixel_format_fourcc_flag = 0x00000004U;
    static constexpr std::uint32_t k_required_caps = 0x00401008U;
    static constexpr std::uint32_t k_standard_dxt1_linear_size = 0x00010000U;
    static constexpr std::uint32_t k_standard_dxt5_linear_size = 0x00020000U;

    // Parses the DDS image envelope evidenced by the preserved DMC3 v6 corpus.
    // This is a product/corpus structural contract, not a claim that every DDS
    // accepted by the original executable must match these exact fields.
    //
    // Standard corpus profile (242/243 observed images):
    // - DXT1 or DXT5;
    // - power-of-two dimensions and complete mip chain;
    // - exact DXT payload byte count;
    // - depth = 0;
    // - pitchOrLinearSize = 0x10000 (DXT1) or 0x20000 (DXT5);
    // - fixed flags/pixel-format/caps and zero reserved fields.
    //
    // One separately-labelled observed exception is accepted only at its exact
    // corpus geometry: DXT5 1024x2048, 12 mips, depth=1, linear size=0x200000.
    // It is read authority only and must not be generalized by writers.
    [[nodiscard]] static DdsImageResult parse(
        std::span<const std::byte> bytes) noexcept;
};

} // namespace dmc::rengine::profiles::dmc3
