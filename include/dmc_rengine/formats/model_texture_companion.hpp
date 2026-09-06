#pragma once

#include <cstddef>
#include <cstdint>
#include <limits>
#include <span>
#include <vector>

namespace dmc::rengine::formats::model_family {

// Serialized texture-companion envelope consumed by the canonical DMC3-HD
// model texture path before runtime descriptors are materialized. The envelope
// itself has no promoted magic: entry payloads are independently verified as
// TIM2/TM2 by the canonical helper path.
struct ModelTextureCompanionAbi final {
    static constexpr std::size_t texture_count_field = 0x000U;
    static constexpr std::size_t block_count_table = 0x004U;
    static constexpr std::size_t payload_base = 0x800U;
    static constexpr std::size_t payload_block_size = 0x800U;
    static constexpr std::uint32_t tm2_magic_le = 0x00324D54U; // "TM2\0"
};

static_assert(ModelTextureCompanionAbi::texture_count_field == 0x000U);
static_assert(ModelTextureCompanionAbi::block_count_table == 0x004U);
static_assert(ModelTextureCompanionAbi::payload_base == 0x800U);
static_assert(ModelTextureCompanionAbi::payload_block_size == 0x800U);
static_assert(ModelTextureCompanionAbi::tm2_magic_le == 0x00324D54U);

enum class TextureCompanionStatus : std::uint8_t {
    ok,
    truncated_count,
    block_table_out_of_bounds,
    block_table_overlaps_payload,
    payload_size_overflow,
    payload_out_of_bounds,
    tm2_magic_mismatch,
};

struct TextureCompanionEntry final {
    std::uint32_t index{};
    std::uint32_t block_count{};
    std::size_t payload_offset{};
    std::size_t allocated_size{};
};

struct TextureCompanionParseResult final {
    TextureCompanionStatus status{TextureCompanionStatus::truncated_count};
    std::uint32_t texture_count{};
    std::vector<TextureCompanionEntry> entries;

    [[nodiscard]] bool ok() const noexcept {
        return status == TextureCompanionStatus::ok;
    }
};

namespace texture_companion_detail {

[[nodiscard]] inline std::uint32_t read_u32_le(
    const std::span<const std::byte> bytes,
    const std::size_t offset) noexcept {
    std::uint32_t value{};
    for (std::size_t index = 0U; index < 4U; ++index) {
        value |= static_cast<std::uint32_t>(
            std::to_integer<std::uint8_t>(bytes[offset + index])) <<
            (index * 8U);
    }
    return value;
}

[[nodiscard]] inline bool range_fits(
    const std::size_t offset,
    const std::size_t size,
    const std::size_t total) noexcept {
    return offset <= total && size <= total - offset;
}

} // namespace texture_companion_detail

// Safe structural reconstruction of the serialized companion envelope consumed
// by canonical texture materialization. Each u32 table value is a count of
// 0x800-byte blocks. Payloads are laid out consecutively from +0x800 and each
// payload begins with the canonical little-endian "TM2\0" signature.
//
// This parser deliberately stops at the companion allocation boundary. It does
// not decode TIM2 internals or construct the later 0x40-byte runtime descriptor
// entries held by the model manager.
[[nodiscard]] inline TextureCompanionParseResult parse_texture_companion(
    const std::span<const std::byte> bytes) {
    using namespace texture_companion_detail;

    TextureCompanionParseResult out;
    if (!range_fits(
            ModelTextureCompanionAbi::texture_count_field,
            4U,
            bytes.size())) {
        return out;
    }

    out.texture_count = read_u32_le(
        bytes, ModelTextureCompanionAbi::texture_count_field);

    if (out.texture_count >
        std::numeric_limits<std::size_t>::max() / 4U) {
        out.status = TextureCompanionStatus::block_table_out_of_bounds;
        return out;
    }
    const auto table_size =
        static_cast<std::size_t>(out.texture_count) * 4U;
    if (!range_fits(
            ModelTextureCompanionAbi::block_count_table,
            table_size,
            bytes.size())) {
        out.status = TextureCompanionStatus::block_table_out_of_bounds;
        return out;
    }
    if (table_size >
        ModelTextureCompanionAbi::payload_base -
            ModelTextureCompanionAbi::block_count_table) {
        out.status = TextureCompanionStatus::block_table_overlaps_payload;
        return out;
    }

    out.entries.reserve(out.texture_count);
    std::size_t payload_offset = ModelTextureCompanionAbi::payload_base;
    for (std::uint32_t index = 0U; index < out.texture_count; ++index) {
        const auto blocks = read_u32_le(
            bytes,
            ModelTextureCompanionAbi::block_count_table +
                static_cast<std::size_t>(index) * 4U);
        if (blocks >
            std::numeric_limits<std::size_t>::max() /
                ModelTextureCompanionAbi::payload_block_size) {
            out.status = TextureCompanionStatus::payload_size_overflow;
            return out;
        }
        const auto allocated_size =
            static_cast<std::size_t>(blocks) *
            ModelTextureCompanionAbi::payload_block_size;
        if (!range_fits(payload_offset, allocated_size, bytes.size()) ||
            allocated_size < 4U) {
            out.status = TextureCompanionStatus::payload_out_of_bounds;
            return out;
        }
        if (read_u32_le(bytes, payload_offset) !=
            ModelTextureCompanionAbi::tm2_magic_le) {
            out.status = TextureCompanionStatus::tm2_magic_mismatch;
            return out;
        }

        out.entries.push_back(TextureCompanionEntry{
            .index = index,
            .block_count = blocks,
            .payload_offset = payload_offset,
            .allocated_size = allocated_size,
        });
        payload_offset += allocated_size;
    }

    out.status = TextureCompanionStatus::ok;
    return out;
}

} // namespace dmc::rengine::formats::model_family
