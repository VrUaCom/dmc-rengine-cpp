#include "dmc_rengine/profiles/dmc3/original_ptx_envelope_geometry.hpp"

#include <array>
#include <limits>

namespace dmc::rengine::profiles::dmc3 {
namespace {

constexpr std::array<std::byte, 4> kTim2RuntimeMagic{
    std::byte{0x54U}, // T
    std::byte{0x4DU}, // M
    std::byte{0x32U}, // 2
    std::byte{0x00U},
};

[[nodiscard]] bool has_tim2_runtime_magic(
    std::span<const std::byte> bytes,
    std::uint64_t offset) noexcept {
    if (offset > static_cast<std::uint64_t>(bytes.size())) {
        return false;
    }
    const auto remaining = static_cast<std::uint64_t>(bytes.size()) - offset;
    if (remaining < kTim2RuntimeMagic.size()) {
        return false;
    }

    const auto start = static_cast<std::size_t>(offset);
    for (std::size_t i = 0; i < kTim2RuntimeMagic.size(); ++i) {
        if (bytes[start + i] != kTim2RuntimeMagic[i]) {
            return false;
        }
    }
    return true;
}

} // namespace

bool OriginalPtxTim2EntryGeometry::valid() const noexcept {
    return advance_size ==
           static_cast<std::uint64_t>(block_count) * kOriginalPtxSectorSize;
}

bool OriginalPtxEnvelopeGeometryReceipt::valid() const noexcept {
    if (texture_count == 0U || texture_count > kOriginalPtxMaxTextureCount) {
        return false;
    }
    if (entries.size() != static_cast<std::size_t>(texture_count)) {
        return false;
    }
    if (first_entry_offset != kOriginalPtxFirstEntryOffset ||
        sector_size != kOriginalPtxSectorSize) {
        return false;
    }

    std::uint64_t expected_offset = first_entry_offset;
    for (std::size_t i = 0; i < entries.size(); ++i) {
        const auto& entry = entries[i];
        if (!entry.valid() ||
            entry.index != static_cast<std::uint32_t>(i) ||
            entry.offset != expected_offset) {
            return false;
        }
        if (entry.advance_size >
            std::numeric_limits<std::uint64_t>::max() - expected_offset) {
            return false;
        }
        expected_offset += entry.advance_size;
    }

    return cursor_after_entries == expected_offset;
}

bool OriginalPtxEnvelopeGeometryResult::ok() const noexcept {
    return status == OriginalPtxEnvelopeGeometryStatus::ok && receipt.valid();
}

OriginalPtxEnvelopeGeometryResult OriginalPtxEnvelopeGeometryValidator::validate(
    std::span<const std::byte> bytes,
    OriginalPtxEnvelopeDescriptorView descriptor) {
    OriginalPtxEnvelopeGeometryResult result;
    result.receipt.texture_count = descriptor.texture_count;
    result.receipt.source_size = static_cast<std::uint64_t>(bytes.size());

    if (descriptor.block_counts.size() !=
        static_cast<std::size_t>(descriptor.texture_count)) {
        result.status =
            OriginalPtxEnvelopeGeometryStatus::descriptor_count_mismatch;
        result.detail = "texture_count does not match block_counts length";
        return result;
    }
    if (descriptor.texture_count == 0U) {
        result.status = OriginalPtxEnvelopeGeometryStatus::texture_count_zero;
        result.detail = "zero-texture PTX envelopes remain outside current promotion evidence";
        return result;
    }
    if (descriptor.texture_count > kOriginalPtxMaxTextureCount) {
        result.status = OriginalPtxEnvelopeGeometryStatus::texture_count_limit;
        result.detail = "texture_count exceeds the recovered 64-entry runtime bundle ceiling";
        return result;
    }

    std::uint64_t offset = kOriginalPtxFirstEntryOffset;
    result.receipt.entries.reserve(descriptor.texture_count);

    for (std::uint32_t index = 0; index < descriptor.texture_count; ++index) {
        const auto source_size = static_cast<std::uint64_t>(bytes.size());
        if (offset > source_size || source_size - offset < kTim2RuntimeMagic.size()) {
            result.status =
                OriginalPtxEnvelopeGeometryStatus::entry_header_out_of_bounds;
            result.detail = "TIM2 entry header lies outside the supplied source bytes";
            return result;
        }
        if (!has_tim2_runtime_magic(bytes, offset)) {
            result.status = OriginalPtxEnvelopeGeometryStatus::tim2_magic_mismatch;
            result.detail = "entry does not begin with the recovered TM2\\0 marker";
            return result;
        }

        const auto block_count = descriptor.block_counts[index];
        const auto advance_size =
            static_cast<std::uint64_t>(block_count) * kOriginalPtxSectorSize;
        if (advance_size > std::numeric_limits<std::uint64_t>::max() - offset) {
            result.status = OriginalPtxEnvelopeGeometryStatus::entry_offset_overflow;
            result.detail = "TIM2 runtime cursor arithmetic overflow";
            return result;
        }

        result.receipt.entries.push_back(OriginalPtxTim2EntryGeometry{
            index,
            block_count,
            offset,
            advance_size,
        });
        offset += advance_size;
    }

    result.receipt.cursor_after_entries = offset;

    if (!result.receipt.valid()) {
        result.status = OriginalPtxEnvelopeGeometryStatus::invalid_receipt;
        result.detail = "constructed PTX geometry receipt failed self-validation";
        return result;
    }

    result.status = OriginalPtxEnvelopeGeometryStatus::ok;
    result.detail = "recovered PTX envelope geometry validated";
    return result;
}

} // namespace dmc::rengine::profiles::dmc3
