#pragma once

#include <cstddef>
#include <cstdint>
#include <span>
#include <string_view>

namespace dmc::rengine::profiles::dmc3 {

enum class TextureSlotRuntimeMaterializationStatus : std::uint8_t {
    ok,
    entry_out_of_bounds,
    entry_too_small,
    source_vtable_not_zero,
    descriptor_pointer_out_of_bounds,
    descriptor_truncated,
    dds_pointer_out_of_bounds,
    dds_range_out_of_bounds,
    dds_too_small,
    dds_magic_mismatch,
    dds_header_size_mismatch,
    dds_pixel_format_size_mismatch,
};

[[nodiscard]] constexpr std::string_view to_string(
    TextureSlotRuntimeMaterializationStatus status) noexcept {
    switch (status) {
    case TextureSlotRuntimeMaterializationStatus::ok: return "ok";
    case TextureSlotRuntimeMaterializationStatus::entry_out_of_bounds:
        return "entry-out-of-bounds";
    case TextureSlotRuntimeMaterializationStatus::entry_too_small:
        return "entry-too-small";
    case TextureSlotRuntimeMaterializationStatus::source_vtable_not_zero:
        return "source-vtable-not-zero";
    case TextureSlotRuntimeMaterializationStatus::descriptor_pointer_out_of_bounds:
        return "descriptor-pointer-out-of-bounds";
    case TextureSlotRuntimeMaterializationStatus::descriptor_truncated:
        return "descriptor-truncated";
    case TextureSlotRuntimeMaterializationStatus::dds_pointer_out_of_bounds:
        return "dds-pointer-out-of-bounds";
    case TextureSlotRuntimeMaterializationStatus::dds_range_out_of_bounds:
        return "dds-range-out-of-bounds";
    case TextureSlotRuntimeMaterializationStatus::dds_too_small:
        return "dds-too-small";
    case TextureSlotRuntimeMaterializationStatus::dds_magic_mismatch:
        return "dds-magic-mismatch";
    case TextureSlotRuntimeMaterializationStatus::dds_header_size_mismatch:
        return "dds-header-size-mismatch";
    case TextureSlotRuntimeMaterializationStatus::dds_pixel_format_size_mismatch:
        return "dds-pixel-format-size-mismatch";
    }
    return "entry-out-of-bounds";
}

struct TextureSlotRuntimeMaterialization final {
    std::uint64_t entry_offset{};
    std::uint64_t entry_span_size{};
    std::uint64_t source_vtable_placeholder{};
    std::uint64_t cpu_payload_descriptor_relative_delta{};
    std::uint64_t cpu_payload_descriptor_offset{};
    std::uint32_t dds_byte_size{};
    std::uint64_t dds_relative_delta{};
    std::uint64_t dds_offset{};
    std::uint16_t width{};
    std::uint16_t height{};

    [[nodiscard]] bool valid() const noexcept;
};

struct TextureSlotRuntimeMaterializationResult final {
    TextureSlotRuntimeMaterializationStatus status{
        TextureSlotRuntimeMaterializationStatus::entry_out_of_bounds};
    TextureSlotRuntimeMaterialization materialization;
    std::string_view detail;

    [[nodiscard]] bool ok() const noexcept;
};

class TextureSlotRuntimeMaterializationInspector final {
public:
    // Canonical dmc3.exe evidence:
    //   0x1403365B0: non-TM2 entry -> 0x140046510
    //   0x140046510: in-place gfxTexture placement and two relative-pointer fixups
    //   0x140046AF0: descriptor+0x04 size / descriptor+0x08 DDS pointer
    //   0x140049A10: DDS-from-memory validation
    //
    // The relative-pointer rule is address(field) + serialized_delta. This
    // inspector reproduces the read-side arithmetic without mutating source bytes.
    static constexpr std::size_t k_vtable_placeholder_offset = 0x00U;
    static constexpr std::size_t k_width_offset = 0x10U;
    static constexpr std::size_t k_height_offset = 0x12U;
    static constexpr std::size_t k_cpu_payload_pointer_offset = 0x20U;
    static constexpr std::size_t k_cpu_payload_dds_size_offset = 0x04U;
    static constexpr std::size_t k_cpu_payload_dds_pointer_offset = 0x08U;
    static constexpr std::size_t k_cpu_payload_descriptor_size = 0x10U;
    static constexpr std::size_t k_dds_minimum_size = 0x80U;
    static constexpr std::size_t k_dds_pixel_format_size_offset = 0x4CU;
    static constexpr std::uint32_t k_dds_magic = 0x20534444U;
    static constexpr std::uint32_t k_dds_header_size = 0x7CU;
    static constexpr std::uint32_t k_dds_pixel_format_size = 0x20U;

    [[nodiscard]] static TextureSlotRuntimeMaterializationResult inspect(
        std::span<const std::byte> bytes,
        std::size_t entry_offset,
        std::size_t entry_span_size);
};

} // namespace dmc::rengine::profiles::dmc3
