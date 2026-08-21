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
    std::uint64_t cursor = kOriginalPtxFirstEntryOffset;
    std::uint64_t max_touched = cursor + 4U;
    for (const auto blocks : block_counts) {
        max_touched = std::max(max_touched, cursor + 4U);
        cursor += static_cast<std::uint64_t>(blocks) * kOriginalPtxSectorSize;
    }
    const auto total = std::max(cursor, max_touched) + trailing_bytes;

    std::vector<std::byte> bytes(static_cast<std::size_t>(total), std::byte{0xA5U});
    cursor = kOriginalPtxFirstEntryOffset;
    for (const auto blocks : block_counts) {
        put_tm2_magic(bytes, static_cast<std::size_t>(cursor));
        cursor += static_cast<std::uint64_t>(blocks) * kOriginalPtxSectorSize;
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
        assert(result.receipt.entries[0].advance_size == 0x1000U);
        assert(result.receipt.entries[1].offset == 0x1800U);
        assert(result.receipt.entries[2].offset == 0x2000U);
        assert(result.receipt.cursor_after_entries == 0x3800U);

        // Pass 87 keeps the complete 0x800-byte PTX header opaque.
        assert(bytes[0] == std::byte{0xA5U});
    }

    {
        constexpr std::array<std::uint32_t, 2> counts{0U, 1U};
        const auto bytes = make_envelope(counts);
        const auto result = OriginalPtxEnvelopeGeometryValidator::validate(
            bytes,
            OriginalPtxEnvelopeDescriptorView{2U, counts});
        assert(result.ok());
        assert(result.receipt.entries[0].offset == 0x800U);
        assert(result.receipt.entries[0].advance_size == 0U);
        assert(result.receipt.entries[1].offset == 0x800U);
        assert(result.receipt.cursor_after_entries == 0x1000U);
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
        constexpr std::array<std::uint32_t, 1> counts{1U};
        std::vector<std::byte> bytes(0x803U, std::byte{0});
        const auto result = OriginalPtxEnvelopeGeometryValidator::validate(
            bytes,
            OriginalPtxEnvelopeDescriptorView{1U, counts});
        assert(result.status ==
               OriginalPtxEnvelopeGeometryStatus::entry_header_out_of_bounds);
    }

    {
        constexpr std::array<std::uint32_t, 2> counts{2U, 1U};
        std::vector<std::byte> bytes(0x804U, std::byte{0});
        put_tm2_magic(bytes, 0x800U);
        const auto result = OriginalPtxEnvelopeGeometryValidator::validate(
            bytes,
            OriginalPtxEnvelopeDescriptorView{2U, counts});
        assert(result.status ==
               OriginalPtxEnvelopeGeometryStatus::entry_header_out_of_bounds);
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
