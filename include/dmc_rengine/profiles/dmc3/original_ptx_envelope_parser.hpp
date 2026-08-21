#pragma once

#include "dmc_rengine/profiles/dmc3/original_ptx_envelope_geometry.hpp"

#include <cstddef>
#include <cstdint>
#include <span>
#include <string>
#include <string_view>
#include <vector>

namespace dmc::rengine::profiles::dmc3 {

inline constexpr std::uint64_t kOriginalPtxTextureCountOffset = 0x00U;
inline constexpr std::uint64_t kOriginalPtxBlockCountArrayOffset = 0x04U;

enum class OriginalPtxEnvelopeParseStatus : std::uint8_t {
    ok,
    header_truncated,
    texture_count_limit,
    block_count_table_truncated,
    geometry_rejected,
    invalid_receipt,
};

[[nodiscard]] constexpr std::string_view to_string(
    OriginalPtxEnvelopeParseStatus status) noexcept {
    switch (status) {
    case OriginalPtxEnvelopeParseStatus::ok: return "ok";
    case OriginalPtxEnvelopeParseStatus::header_truncated: return "header-truncated";
    case OriginalPtxEnvelopeParseStatus::texture_count_limit: return "texture-count-limit";
    case OriginalPtxEnvelopeParseStatus::block_count_table_truncated:
        return "block-count-table-truncated";
    case OriginalPtxEnvelopeParseStatus::geometry_rejected: return "geometry-rejected";
    case OriginalPtxEnvelopeParseStatus::invalid_receipt: return "invalid-receipt";
    }
    return "invalid-receipt";
}

struct OriginalPtxEnvelopeParseReceipt final {
    std::uint32_t texture_count{};
    std::vector<std::uint32_t> block_counts;
    OriginalPtxEnvelopeGeometryReceipt geometry;

    [[nodiscard]] bool valid() const noexcept;
};

struct OriginalPtxEnvelopeParseResult final {
    OriginalPtxEnvelopeParseStatus status{OriginalPtxEnvelopeParseStatus::invalid_receipt};
    OriginalPtxEnvelopeGeometryStatus geometry_status{
        OriginalPtxEnvelopeGeometryStatus::invalid_receipt};
    OriginalPtxEnvelopeParseReceipt receipt;
    std::string detail;

    [[nodiscard]] bool ok() const noexcept;
};

class OriginalPtxEnvelopeParser final {
public:
    // Pass 88 raw-header decoding is backed by two independent derivative EXE
    // artifacts whose PTX parser windows are byte-identical. The recovered
    // loop reads:
    //   textureCount  = u32le(base + 0x00)
    //   blockCount[i] = u32le(base + 0x04 + i*4)
    //   firstEntry    = base + 0x800
    //
    // This implementation remains a draft/high-confidence promotion candidate
    // until the same instruction window is byte-compared against the canonical
    // original dmc3.exe SHA-256 e454272e...dcbdd082.
    [[nodiscard]] static OriginalPtxEnvelopeParseResult parse(
        std::span<const std::byte> bytes);
};

} // namespace dmc::rengine::profiles::dmc3
