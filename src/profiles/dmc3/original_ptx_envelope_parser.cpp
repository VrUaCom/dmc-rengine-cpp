#include "dmc_rengine/profiles/dmc3/original_ptx_envelope_parser.hpp"

#include <limits>

namespace dmc::rengine::profiles::dmc3 {
namespace {

[[nodiscard]] bool read_u32_le(
    std::span<const std::byte> bytes,
    std::uint64_t offset,
    std::uint32_t& value) noexcept {
    const auto size = static_cast<std::uint64_t>(bytes.size());
    if (offset > size || size - offset < 4U) {
        return false;
    }
    const auto i = static_cast<std::size_t>(offset);
    value = static_cast<std::uint32_t>(std::to_integer<std::uint8_t>(bytes[i + 0U])) |
            (static_cast<std::uint32_t>(std::to_integer<std::uint8_t>(bytes[i + 1U])) << 8U) |
            (static_cast<std::uint32_t>(std::to_integer<std::uint8_t>(bytes[i + 2U])) << 16U) |
            (static_cast<std::uint32_t>(std::to_integer<std::uint8_t>(bytes[i + 3U])) << 24U);
    return true;
}

} // namespace

bool OriginalPtxEnvelopeParseReceipt::valid() const noexcept {
    return block_counts.size() == static_cast<std::size_t>(texture_count) &&
           geometry.texture_count == texture_count &&
           geometry.entries.size() == static_cast<std::size_t>(texture_count) &&
           geometry.valid();
}

bool OriginalPtxEnvelopeParseResult::ok() const noexcept {
    return status == OriginalPtxEnvelopeParseStatus::ok &&
           geometry_status == OriginalPtxEnvelopeGeometryStatus::ok &&
           receipt.valid();
}

OriginalPtxEnvelopeParseResult OriginalPtxEnvelopeParser::parse(
    std::span<const std::byte> bytes) {
    OriginalPtxEnvelopeParseResult result;

    std::uint32_t texture_count = 0U;
    if (!read_u32_le(bytes, kOriginalPtxTextureCountOffset, texture_count)) {
        result.status = OriginalPtxEnvelopeParseStatus::header_truncated;
        result.detail = "PTX header does not contain the corroborated textureCount field";
        return result;
    }
    if (texture_count > kOriginalPtxMaxTextureCount) {
        result.status = OriginalPtxEnvelopeParseStatus::texture_count_limit;
        result.detail = "textureCount exceeds the recovered 64-entry runtime bundle ceiling";
        return result;
    }

    const auto table_bytes = static_cast<std::uint64_t>(texture_count) * 4U;
    if (table_bytes > std::numeric_limits<std::uint64_t>::max() -
                          kOriginalPtxBlockCountArrayOffset) {
        result.status = OriginalPtxEnvelopeParseStatus::block_count_table_truncated;
        result.detail = "blockCount table size overflow";
        return result;
    }
    const auto table_end = kOriginalPtxBlockCountArrayOffset + table_bytes;
    const auto source_size = static_cast<std::uint64_t>(bytes.size());
    if (table_end > kOriginalPtxFirstEntryOffset || table_end > source_size) {
        result.status = OriginalPtxEnvelopeParseStatus::block_count_table_truncated;
        result.detail = "PTX blockCount table is outside the corroborated 0x800-byte envelope header";
        return result;
    }

    result.receipt.texture_count = texture_count;
    result.receipt.block_counts.reserve(texture_count);
    for (std::uint32_t index = 0U; index < texture_count; ++index) {
        std::uint32_t block_count = 0U;
        const auto offset = kOriginalPtxBlockCountArrayOffset +
                            static_cast<std::uint64_t>(index) * 4U;
        if (!read_u32_le(bytes, offset, block_count)) {
            result.status = OriginalPtxEnvelopeParseStatus::block_count_table_truncated;
            result.detail = "PTX blockCount table ended before textureCount entries were decoded";
            return result;
        }
        result.receipt.block_counts.push_back(block_count);
    }

    const auto geometry_result = OriginalPtxEnvelopeGeometryValidator::validate(
        bytes,
        OriginalPtxEnvelopeDescriptorView{
            texture_count,
            std::span<const std::uint32_t>(result.receipt.block_counts),
        });
    result.geometry_status = geometry_result.status;
    if (!geometry_result.ok()) {
        result.status = OriginalPtxEnvelopeParseStatus::geometry_rejected;
        result.detail = geometry_result.detail;
        return result;
    }

    result.receipt.geometry = geometry_result.receipt;
    if (!result.receipt.valid()) {
        result.status = OriginalPtxEnvelopeParseStatus::invalid_receipt;
        result.detail = "constructed raw PTX parse receipt failed self-validation";
        return result;
    }

    result.status = OriginalPtxEnvelopeParseStatus::ok;
    result.detail = "corroborated raw PTX header and recovered TIM2 geometry parsed";
    return result;
}

} // namespace dmc::rengine::profiles::dmc3
