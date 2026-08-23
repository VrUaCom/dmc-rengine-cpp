#include "dmc_rengine/profiles/dmc3/texture_slot_runtime_materialization.hpp"

#include <limits>

namespace dmc::rengine::profiles::dmc3 {
namespace {

[[nodiscard]] std::uint16_t read_u16_le(
    std::span<const std::byte> bytes,
    std::size_t offset) noexcept {
    return static_cast<std::uint16_t>(
        std::to_integer<std::uint16_t>(bytes[offset + 0U]) |
        (std::to_integer<std::uint16_t>(bytes[offset + 1U]) << 8U));
}

[[nodiscard]] std::uint32_t read_u32_le(
    std::span<const std::byte> bytes,
    std::size_t offset) noexcept {
    return std::to_integer<std::uint32_t>(bytes[offset + 0U]) |
        (std::to_integer<std::uint32_t>(bytes[offset + 1U]) << 8U) |
        (std::to_integer<std::uint32_t>(bytes[offset + 2U]) << 16U) |
        (std::to_integer<std::uint32_t>(bytes[offset + 3U]) << 24U);
}

[[nodiscard]] std::uint64_t read_u64_le(
    std::span<const std::byte> bytes,
    std::size_t offset) noexcept {
    std::uint64_t value = 0U;
    for (std::size_t index = 0U; index < sizeof(std::uint64_t); ++index) {
        value |= static_cast<std::uint64_t>(
                     std::to_integer<std::uint8_t>(bytes[offset + index]))
            << (index * 8U);
    }
    return value;
}

[[nodiscard]] bool contains(
    std::span<const std::byte> bytes,
    std::size_t offset,
    std::size_t size) noexcept {
    return offset <= bytes.size() && size <= bytes.size() - offset;
}

[[nodiscard]] bool resolve_relative_pointer(
    std::size_t field_offset,
    std::uint64_t serialized_delta,
    std::size_t entry_offset,
    std::size_t entry_span_size,
    std::size_t& resolved) noexcept {
    if (serialized_delta > std::numeric_limits<std::size_t>::max()) {
        return false;
    }
    const auto delta = static_cast<std::size_t>(serialized_delta);
    if (field_offset > std::numeric_limits<std::size_t>::max() - delta) {
        return false;
    }
    resolved = field_offset + delta;
    if (resolved < entry_offset) {
        return false;
    }
    return resolved - entry_offset < entry_span_size;
}

[[nodiscard]] TextureSlotRuntimeMaterializationResult failure(
    TextureSlotRuntimeMaterializationStatus status,
    std::string_view detail) {
    return TextureSlotRuntimeMaterializationResult{
        .status = status,
        .materialization = {},
        .detail = detail,
    };
}

} // namespace

bool TextureSlotRuntimeMaterialization::valid() const noexcept {
    if (entry_span_size == 0U || source_vtable_placeholder != 0U ||
        dds_byte_size < TextureSlotRuntimeMaterializationInspector::k_dds_minimum_size ||
        cpu_payload_descriptor_offset < entry_offset ||
        dds_offset < entry_offset) {
        return false;
    }
    const auto descriptor_relative = cpu_payload_descriptor_offset - entry_offset;
    const auto dds_relative = dds_offset - entry_offset;
    return descriptor_relative < entry_span_size && dds_relative < entry_span_size &&
        dds_byte_size <= entry_span_size - dds_relative;
}

bool TextureSlotRuntimeMaterializationResult::ok() const noexcept {
    return status == TextureSlotRuntimeMaterializationStatus::ok &&
        materialization.valid();
}

TextureSlotRuntimeMaterializationResult
TextureSlotRuntimeMaterializationInspector::inspect(
    std::span<const std::byte> bytes,
    std::size_t entry_offset,
    std::size_t entry_span_size) {
    if (entry_offset > bytes.size() ||
        entry_span_size > bytes.size() - entry_offset) {
        return failure(
            TextureSlotRuntimeMaterializationStatus::entry_out_of_bounds,
            "Serialized gfxTexture entry exceeds the supplied byte span.");
    }
    if (entry_span_size < k_cpu_payload_pointer_offset + sizeof(std::uint64_t)) {
        return failure(
            TextureSlotRuntimeMaterializationStatus::entry_too_small,
            "Serialized gfxTexture entry is too small to contain the runtime pointer fields.");
    }

    const auto source_vtable = read_u64_le(
        bytes, entry_offset + k_vtable_placeholder_offset);
    if (source_vtable != 0U) {
        return failure(
            TextureSlotRuntimeMaterializationStatus::source_vtable_not_zero,
            "Source serialized gfxTexture image must carry a zero vtable placeholder before placement.");
    }

    const auto width = read_u16_le(bytes, entry_offset + k_width_offset);
    const auto height = read_u16_le(bytes, entry_offset + k_height_offset);

    const auto descriptor_pointer_field =
        entry_offset + k_cpu_payload_pointer_offset;
    const auto descriptor_delta = read_u64_le(bytes, descriptor_pointer_field);
    std::size_t descriptor_offset = 0U;
    if (!resolve_relative_pointer(
            descriptor_pointer_field,
            descriptor_delta,
            entry_offset,
            entry_span_size,
            descriptor_offset)) {
        return failure(
            TextureSlotRuntimeMaterializationStatus::descriptor_pointer_out_of_bounds,
            "Canonical address(field)+delta descriptor relocation escapes the bounded entry.");
    }
    if (!contains(bytes, descriptor_offset, k_cpu_payload_descriptor_size) ||
        descriptor_offset - entry_offset >
            entry_span_size - k_cpu_payload_descriptor_size) {
        return failure(
            TextureSlotRuntimeMaterializationStatus::descriptor_truncated,
            "Resolved CPU-payload descriptor is truncated inside the bounded entry.");
    }

    const auto dds_byte_size = read_u32_le(
        bytes, descriptor_offset + k_cpu_payload_dds_size_offset);
    const auto dds_pointer_field =
        descriptor_offset + k_cpu_payload_dds_pointer_offset;
    const auto dds_delta = read_u64_le(bytes, dds_pointer_field);
    std::size_t dds_offset = 0U;
    if (!resolve_relative_pointer(
            dds_pointer_field,
            dds_delta,
            entry_offset,
            entry_span_size,
            dds_offset)) {
        return failure(
            TextureSlotRuntimeMaterializationStatus::dds_pointer_out_of_bounds,
            "Canonical address(field)+delta DDS relocation escapes the bounded entry.");
    }

    const auto dds_relative = dds_offset - entry_offset;
    if (dds_byte_size > entry_span_size - dds_relative ||
        !contains(bytes, dds_offset, static_cast<std::size_t>(dds_byte_size))) {
        return failure(
            TextureSlotRuntimeMaterializationStatus::dds_range_out_of_bounds,
            "Resolved DDS byte range escapes the bounded serialized entry.");
    }
    if (dds_byte_size < k_dds_minimum_size) {
        return failure(
            TextureSlotRuntimeMaterializationStatus::dds_too_small,
            "Resolved DDS byte range is smaller than the canonical minimum header span.");
    }
    if (read_u32_le(bytes, dds_offset) != k_dds_magic) {
        return failure(
            TextureSlotRuntimeMaterializationStatus::dds_magic_mismatch,
            "Resolved DDS pointer does not target DDS magic.");
    }
    if (read_u32_le(bytes, dds_offset + 0x04U) != k_dds_header_size) {
        return failure(
            TextureSlotRuntimeMaterializationStatus::dds_header_size_mismatch,
            "Resolved DDS header size does not match the canonical runtime validator.");
    }
    if (read_u32_le(bytes, dds_offset + k_dds_pixel_format_size_offset) !=
        k_dds_pixel_format_size) {
        return failure(
            TextureSlotRuntimeMaterializationStatus::dds_pixel_format_size_mismatch,
            "Resolved DDS pixel-format header size does not match the canonical runtime validator.");
    }

    TextureSlotRuntimeMaterialization materialization{
        .entry_offset = entry_offset,
        .entry_span_size = entry_span_size,
        .source_vtable_placeholder = source_vtable,
        .cpu_payload_descriptor_relative_delta = descriptor_delta,
        .cpu_payload_descriptor_offset = descriptor_offset,
        .dds_byte_size = dds_byte_size,
        .dds_relative_delta = dds_delta,
        .dds_offset = dds_offset,
        .width = width,
        .height = height,
    };
    if (!materialization.valid()) {
        return failure(
            TextureSlotRuntimeMaterializationStatus::dds_range_out_of_bounds,
            "Recovered runtime materialization view failed internal bounds validation.");
    }

    return TextureSlotRuntimeMaterializationResult{
        .status = TextureSlotRuntimeMaterializationStatus::ok,
        .materialization = materialization,
        .detail = {},
    };
}

} // namespace dmc::rengine::profiles::dmc3
