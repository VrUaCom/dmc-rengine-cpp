#include "dmc_rengine/profiles/dmc3/original_ptx_envelope_parser.hpp"

#include <algorithm>
#include <array>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <span>
#include <vector>

using namespace dmc::rengine::profiles::dmc3;

namespace {

void write_u32_le(std::vector<std::byte>& bytes, std::size_t offset, std::uint32_t value) {
    bytes[offset + 0U] = std::byte{static_cast<std::uint8_t>(value & 0xFFU)};
    bytes[offset + 1U] = std::byte{static_cast<std::uint8_t>((value >> 8U) & 0xFFU)};
    bytes[offset + 2U] = std::byte{static_cast<std::uint8_t>((value >> 16U) & 0xFFU)};
    bytes[offset + 3U] = std::byte{static_cast<std::uint8_t>((value >> 24U) & 0xFFU)};
}

void put_tm2_magic(std::vector<std::byte>& bytes, std::size_t offset) {
    bytes[offset + 0U] = std::byte{0x54U};
    bytes[offset + 1U] = std::byte{0x4DU};
    bytes[offset + 2U] = std::byte{0x32U};
    bytes[offset + 3U] = std::byte{0x00U};
}

std::vector<std::byte> make_raw_ptx(std::span<const std::uint32_t> counts) {
    std::uint64_t cursor = kOriginalPtxFirstEntryOffset;
    std::uint64_t max_touched = cursor + 4U;
    for (const auto blocks : counts) {
        max_touched = std::max(max_touched, cursor + 4U);
        cursor += static_cast<std::uint64_t>(blocks) * kOriginalPtxSectorSize;
    }
    const auto total = std::max(cursor, max_touched);
    std::vector<std::byte> bytes(static_cast<std::size_t>(total), std::byte{0xCCU});

    write_u32_le(bytes, 0U, static_cast<std::uint32_t>(counts.size()));
    for (std::size_t i = 0; i < counts.size(); ++i) {
        write_u32_le(bytes, 4U + i * 4U, counts[i]);
    }

    cursor = kOriginalPtxFirstEntryOffset;
    for (const auto blocks : counts) {
        put_tm2_magic(bytes, static_cast<std::size_t>(cursor));
        cursor += static_cast<std::uint64_t>(blocks) * kOriginalPtxSectorSize;
    }
    return bytes;
}

} // namespace

int main() {
    {
        constexpr std::array<std::uint32_t, 3> counts{2U, 1U, 3U};
        auto bytes = make_raw_ptx(counts);
        // Bytes after the decoded count table remain intentionally opaque.
        bytes[0x100U] = std::byte{0x7BU};

        const auto result = OriginalPtxEnvelopeParser::parse(bytes);
        assert(result.ok());
        assert(result.receipt.texture_count == 3U);
        assert(result.receipt.block_counts == std::vector<std::uint32_t>({2U, 1U, 3U}));
        assert(result.receipt.geometry.entries[0].offset == 0x800U);
        assert(result.receipt.geometry.entries[1].offset == 0x1800U);
        assert(result.receipt.geometry.entries[2].offset == 0x2000U);
        assert(bytes[0x100U] == std::byte{0x7BU});
    }

    {
        constexpr std::array<std::uint32_t, 2> counts{0U, 1U};
        const auto bytes = make_raw_ptx(counts);
        const auto result = OriginalPtxEnvelopeParser::parse(bytes);
        assert(result.ok());
        assert(result.receipt.block_counts[0] == 0U);
        assert(result.receipt.geometry.entries[0].offset == 0x800U);
        assert(result.receipt.geometry.entries[1].offset == 0x800U);
    }

    {
        std::vector<std::byte> bytes(3U, std::byte{0});
        const auto result = OriginalPtxEnvelopeParser::parse(bytes);
        assert(result.status == OriginalPtxEnvelopeParseStatus::header_truncated);
    }

    {
        std::vector<std::byte> bytes(0x800U, std::byte{0});
        write_u32_le(bytes, 0U, 65U);
        const auto result = OriginalPtxEnvelopeParser::parse(bytes);
        assert(result.status == OriginalPtxEnvelopeParseStatus::texture_count_limit);
    }

    {
        std::vector<std::byte> bytes(8U, std::byte{0});
        write_u32_le(bytes, 0U, 2U);
        write_u32_le(bytes, 4U, 1U);
        const auto result = OriginalPtxEnvelopeParser::parse(bytes);
        assert(result.status == OriginalPtxEnvelopeParseStatus::block_count_table_truncated);
    }

    {
        std::vector<std::byte> bytes(0x800U, std::byte{0});
        write_u32_le(bytes, 0U, 0U);
        const auto result = OriginalPtxEnvelopeParser::parse(bytes);
        assert(result.status == OriginalPtxEnvelopeParseStatus::geometry_rejected);
        assert(result.geometry_status == OriginalPtxEnvelopeGeometryStatus::texture_count_zero);
    }

    {
        constexpr std::array<std::uint32_t, 1> counts{1U};
        auto bytes = make_raw_ptx(counts);
        bytes[0x800U] = std::byte{0x00U};
        const auto result = OriginalPtxEnvelopeParser::parse(bytes);
        assert(result.status == OriginalPtxEnvelopeParseStatus::geometry_rejected);
        assert(result.geometry_status == OriginalPtxEnvelopeGeometryStatus::tim2_magic_mismatch);
    }

    return 0;
}
