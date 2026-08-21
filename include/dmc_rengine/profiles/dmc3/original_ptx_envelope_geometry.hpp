#pragma once

#include <cstddef>
#include <cstdint>
#include <span>
#include <string>
#include <string_view>
#include <vector>

namespace dmc::rengine::profiles::dmc3 {

inline constexpr std::uint64_t kOriginalPtxSectorSize = 0x800U;
inline constexpr std::uint64_t kOriginalPtxFirstEntryOffset = 0x800U;
inline constexpr std::uint32_t kOriginalPtxMaxTextureCount = 64U;

enum class OriginalPtxEnvelopeGeometryStatus : std::uint8_t {
    ok,
    descriptor_count_mismatch,
    texture_count_zero,
    texture_count_limit,
    zero_block_count,
    entry_offset_overflow,
    entry_header_out_of_bounds,
    entry_span_out_of_bounds,
    tim2_magic_mismatch,
    invalid_receipt,
};

[[nodiscard]] constexpr std::string_view to_string(
    OriginalPtxEnvelopeGeometryStatus status) noexcept {
    switch (status) {
    case OriginalPtxEnvelopeGeometryStatus::ok: return "ok";
    case OriginalPtxEnvelopeGeometryStatus::descriptor_count_mismatch:
        return "descriptor-count-mismatch";
    case OriginalPtxEnvelopeGeometryStatus::texture_count_zero:
        return "texture-count-zero";
    case OriginalPtxEnvelopeGeometryStatus::texture_count_limit:
        return "texture-count-limit";
    case OriginalPtxEnvelopeGeometryStatus::zero_block_count:
        return "zero-block-count";
    case OriginalPtxEnvelopeGeometryStatus::entry_offset_overflow:
        return "entry-offset-overflow";
    case OriginalPtxEnvelopeGeometryStatus::entry_header_out_of_bounds:
        return "entry-header-out-of-bounds";
    case OriginalPtxEnvelopeGeometryStatus::entry_span_out_of_bounds:
        return "entry-span-out-of-bounds";
    case OriginalPtxEnvelopeGeometryStatus::tim2_magic_mismatch:
        return "tim2-magic-mismatch";
    case OriginalPtxEnvelopeGeometryStatus::invalid_receipt:
        return "invalid-receipt";
    }
    return "invalid-receipt";
}

// Phase 16 confirms the runtime envelope relation but the preserved evidence
// does not retain the exact byte offsets of textureCount/blockCount[] inside
// the 0x800-byte envelope header. Callers therefore supply those already
// decoded values. This type intentionally does not imply a header parser.
struct OriginalPtxEnvelopeDescriptorView final {
    std::uint32_t texture_count{};
    std::span<const std::uint32_t> block_counts;
};

struct OriginalPtxTim2EntryGeometry final {
    std::uint32_t index{};
    std::uint32_t block_count{};
    std::uint64_t offset{};
    std::uint64_t span_size{};

    [[nodiscard]] bool valid() const noexcept;
};

struct OriginalPtxEnvelopeGeometryReceipt final {
    std::uint32_t texture_count{};
    std::uint64_t source_size{};
    std::uint64_t first_entry_offset{kOriginalPtxFirstEntryOffset};
    std::uint64_t sector_size{kOriginalPtxSectorSize};
    std::uint64_t consumed_end{};
    std::uint64_t trailing_bytes{};
    std::vector<OriginalPtxTim2EntryGeometry> entries;

    [[nodiscard]] bool valid() const noexcept;
};

struct OriginalPtxEnvelopeGeometryResult final {
    OriginalPtxEnvelopeGeometryStatus status{
        OriginalPtxEnvelopeGeometryStatus::invalid_receipt};
    OriginalPtxEnvelopeGeometryReceipt receipt;
    std::string detail;

    [[nodiscard]] bool ok() const noexcept;
};

class OriginalPtxEnvelopeGeometryValidator final {
public:
    // Validates only the EXE-confirmed original PTX envelope geometry:
    // - <= 64 runtime texture entries;
    // - first TIM2 entry at +0x800;
    // - each entry begins with the EXE-confirmed `TM2\0` marker;
    // - entry advancement is blockCount[i] * 0x800;
    // - every declared physical span is bounded by the supplied source bytes.
    //
    // The first 0x800 bytes remain opaque because the exact locations of
    // textureCount and blockCount[] are not preserved in the current Phase 16
    // artifact set. This validator must not be presented as a complete PTX
    // parser or PTX/TIM2 writer.
    [[nodiscard]] static OriginalPtxEnvelopeGeometryResult validate(
        std::span<const std::byte> bytes,
        OriginalPtxEnvelopeDescriptorView descriptor);
};

} // namespace dmc::rengine::profiles::dmc3
