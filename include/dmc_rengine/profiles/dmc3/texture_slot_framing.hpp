#pragma once

#include <cstddef>
#include <cstdint>
#include <span>
#include <string_view>
#include <vector>

namespace dmc::rengine::profiles::dmc3 {

enum class TextureSlotFramingKind : std::uint8_t {
    wrapped_dds,
    texture_bundle,
};

enum class TextureCompressionKind : std::uint8_t {
    dxt1,
    dxt5,
};

enum class TextureSlotFramingStatus : std::uint8_t {
    ok,
    not_recognized,
    invalid_count,
    truncated_header,
    truncated_descriptor,
    invalid_dds,
    unsupported_compression,
    descriptor_mismatch,
    invalid_sector_span,
    nonzero_alignment_padding,
    trailing_bytes,
};

[[nodiscard]] constexpr std::string_view to_string(
    TextureSlotFramingStatus status) noexcept {
    switch (status) {
    case TextureSlotFramingStatus::ok: return "ok";
    case TextureSlotFramingStatus::not_recognized: return "not-recognized";
    case TextureSlotFramingStatus::invalid_count: return "invalid-count";
    case TextureSlotFramingStatus::truncated_header: return "truncated-header";
    case TextureSlotFramingStatus::truncated_descriptor:
        return "truncated-descriptor";
    case TextureSlotFramingStatus::invalid_dds: return "invalid-dds";
    case TextureSlotFramingStatus::unsupported_compression:
        return "unsupported-compression";
    case TextureSlotFramingStatus::descriptor_mismatch:
        return "descriptor-mismatch";
    case TextureSlotFramingStatus::invalid_sector_span:
        return "invalid-sector-span";
    case TextureSlotFramingStatus::nonzero_alignment_padding:
        return "nonzero-alignment-padding";
    case TextureSlotFramingStatus::trailing_bytes: return "trailing-bytes";
    }
    return "not-recognized";
}

struct TextureSlotFramingSafety final {
    // Product-side denial-of-service bound. It is not an original DMC3 ABI
    // limit and must not be reported as one.
    std::uint32_t max_texture_count{4096U};
};

struct TextureSlotEntry final {
    std::uint32_t texture_index{};
    std::uint64_t descriptor_offset{};
    std::uint64_t dds_offset{};
    std::uint32_t dds_size{};
    std::uint32_t dds_payload_size{};
    std::uint32_t width{};
    std::uint32_t height{};
    std::uint32_t mip_map_count{};
    TextureCompressionKind compression{TextureCompressionKind::dxt1};

    // Corpus-confirmed structural descriptor fields. Their original runtime
    // semantics are intentionally not claimed here.
    std::uint32_t secondary_width{};
    std::uint32_t secondary_height{};
    std::uint32_t auxiliary_mode{};
    std::uint32_t auxiliary_value{};

    std::uint32_t sector_span{};

    [[nodiscard]] bool valid(std::uint64_t slot_size) const noexcept;
};

struct TextureSlotFramingDocument final {
    TextureSlotFramingKind kind{TextureSlotFramingKind::wrapped_dds};
    std::uint64_t slot_size{};
    std::vector<TextureSlotEntry> textures;

    [[nodiscard]] bool valid() const noexcept;
};

struct TextureSlotFramingResult final {
    TextureSlotFramingStatus status{TextureSlotFramingStatus::not_recognized};
    TextureSlotFramingDocument document;
    std::string_view detail;

    [[nodiscard]] bool ok() const noexcept;
};

class TextureSlotFramingParser final {
public:
    static constexpr std::size_t k_bundle_header_size = 0x800U;
    static constexpr std::size_t k_descriptor_size = 0x70U;
    static constexpr std::size_t k_sector_size = 0x800U;

    // Parses the two texture-slot physical framings evidenced by the preserved
    // DMC3 v6 real corpus. Pass 80 validates the complete 0x70-byte descriptor
    // structural envelope: constants/zeros, DDS-derived fields, secondary
    // dimensions + exact reciprocal floats, and the bounded auxiliary pair.
    // Semantic names are not inferred for the auxiliary/secondary fields.
    [[nodiscard]] static TextureSlotFramingResult parse(
        std::span<const std::byte> bytes,
        TextureSlotFramingSafety safety = {});
};

} // namespace dmc::rengine::profiles::dmc3
