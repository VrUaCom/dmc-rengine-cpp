#include "dmc_rengine/profiles/dmc3/original_ptx_envelope_geometry.hpp"

#include <array>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <vector>

using namespace dmc::rengine::profiles::dmc3;

namespace {

void put_tm2_magic(std::vector<std::byte>& bytes, std::size_t offset) {
    bytes[offset + 0U] = std::byte{0x54U};
    bytes[offset + 1U] = std::byte{0x4DU};
    bytes[offset + 2U] = std::byte{0x32U};
    bytes[offset + 3U] = std::byte{0x00U};
}

std::vector<std::byte> make_envelope(
    std::span<const std::uint32_t> block_counts,
    std::size_t trailing_bytes = 0U) {
    std::uint64_t total = kOriginalPtxFirstEntryOffset;
    for (const auto blocks : block_counts) {
        total += static_cast<std::uint64_t>(blocks) * kOriginalPtxSectorSize;
    }
    total += trailing_bytes;

    std::vector<std::byte> bytes(static_cast<std::size_t>(total), std::byte{0xA5U});
    std::uint64_t offset = kOriginalPtxFirstEntryOffset;
    for (const auto blocks : block_counts) {
        put_tm2_magic(bytes, static_cast<std::size_t>(offset));
        offset += static_cast<std::uint64_t>(blocks) * kOriginalPtxSectorSize;
    }
    return bytes;
}

} // namespace

int main() {
    {
        constexpr std::array<std::uint32_t, 3> counts{2U, 1U, 3U};
        const auto bytes = make_envelope(counts, 0x20U);
        const auto result = OriginalPtxEnvelopeGeometryValidator::validate(
            bytes,
            OriginalPtxEnvelopeDescriptorView{3U, counts});
        assert(result.ok());
        assert(result.receipt.entries.size() == 3U);
        assert(result.receipt.entries[0].offset == 0x800U);
        assert(result.receipt.entries[0].span_size == 0x1000U);
        assert(result.receipt.entries[1].offset == 0x1800U);
        assert(result.receipt.entries[2].offset == 0x2000U);
        assert(result.receipt.consumed_end == 0x3800U);
        assert(result.receipt.trailing_bytes == 0x20U);

        // Pass 87 intentionally treats the complete 0x800-byte PTX header as
        // opaque. Garbage-looking header bytes must not be interpreted as
        // textureCount/blockCount[] until their exact offsets are recovered.
        assert(bytes[0] == std::byte{0xA5U});
    }

    {
        constexpr std::array<std::uint32_t, 1> counts{1U};
        const auto bytes = make_envelope(counts);
        const auto result = OriginalPtxEnvelopeGeometryValidator::validate(
            bytes,
            OriginalPtxEnvelopeDescriptorView{2U, counts});
        assert(result.status ==
               OriginalPtxEnvelopeGeometryStatus::descriptor_count_mismatch);
    }

    {
        const std::array<std::uint32_t, 0> counts{};
        const std::vector<std::byte> bytes(0x800U, std::byte{0});
        const auto result = OriginalPtxEnvelopeGeometryValidator::validate(
            bytes,
            OriginalPtxEnvelopeDescriptorView{0U, counts});
        assert(result.status == OriginalPtxEnvelopeGeometryStatus::texture_count_zero);
    }

    {
        const std::vector<std::uint32_t> counts(65U, 1U);
        const std::vector<std::byte> bytes(0x800U, std::byte{0});
        const auto result = OriginalPtxEnvelopeGeometryValidator::validate(
            bytes,
            OriginalPtxEnvelopeDescriptorView{65U, counts});
        assert(result.status == OriginalPtxEnvelopeGeometryStatus::texture_count_limit);
    }

    {
        constexpr std::array<std::uint32_t, 1> counts{0U};
        const std::vector<std::byte> bytes(0x800U, std::byte{0});
        const auto result = OriginalPtxEnvelopeGeometryValidator::validate(
            bytes,
            OriginalPtxEnvelopeDescriptorView{1U, counts});
        assert(result.status == OriginalPtxEnvelopeGeometryStatus::zero_block_count);
    }

    {
        constexpr std::array<std::uint32_t, 1> counts{1U};
        std::vector<std::byte> bytes(0x803U, std::byte{0});
        const auto result = OriginalPtxEnvelopeGeometryValidator::validate(
            bytes,
            OriginalPtxEnvelopeDescriptorView{1U, counts});
        assert(result.status ==
               OriginalPtxEnvelopeGeometryStatus::entry_header_out_of_bounds);
    }

    {
        constexpr std::array<std::uint32_t, 1> counts{2U};
        std::vector<std::byte> bytes(0x804U, std::byte{0});
        put_tm2_magic(bytes, 0x800U);
        const auto result = OriginalPtxEnvelopeGeometryValidator::validate(
            bytes,
            OriginalPtxEnvelopeDescriptorView{1U, counts});
        assert(result.status ==
               OriginalPtxEnvelopeGeometryStatus::entry_span_out_of_bounds);
    }

    {
        constexpr std::array<std::uint32_t, 1> counts{1U};
        auto bytes = make_envelope(counts);
        bytes[0x800U] = std::byte{0x00U};
        const auto result = OriginalPtxEnvelopeGeometryValidator::validate(
            bytes,
            OriginalPtxEnvelopeDescriptorView{1U, counts});
        assert(result.status == OriginalPtxEnvelopeGeometryStatus::tim2_magic_mismatch);
    }

    return 0;
}
