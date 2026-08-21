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
    entry_offset_overflow,
    entry_header_out_of_bounds,
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
    case OriginalPtxEnvelopeGeometryStatus::entry_offset_overflow:
        return "entry-offset-overflow";
    case OriginalPtxEnvelopeGeometryStatus::entry_header_out_of_bounds:
        return "entry-header-out-of-bounds";
    case OriginalPtxEnvelopeGeometryStatus::tim2_magic_mismatch:
        return "tim2-magic-mismatch";
    case OriginalPtxEnvelopeGeometryStatus::invalid_receipt:
        return "invalid-receipt";
    }
    return "invalid-receipt";
}

// Phase 16 confirms the runtime envelope relation but the promoted Pass 87
// geometry contract intentionally receives textureCount/blockCount[] as
// already-decoded values. Exact raw-header offsets are a separate evidence
// gate and must not be silently inferred by this type.
struct OriginalPtxEnvelopeDescriptorView final {
    std::uint32_t texture_count{};
    std::span<const std::uint32_t> block_counts;
};

struct OriginalPtxTim2EntryGeometry final {
    std::uint32_t index{};
    std::uint32_t block_count{};
    std::uint64_t offset{};
    // This is the runtime cursor advance encoded by blockCount*0x800. It is
    // not promoted as the intrinsic byte length of the TIM2 entry. In
    // particular a zero advance is preserved instead of rejected.
    std::uint64_t advance_size{};

    [[nodiscard]] bool valid() const noexcept;
};

struct OriginalPtxEnvelopeGeometryReceipt final {
    std::uint32_t texture_count{};
    std::uint64_t source_size{};
    std::uint64_t first_entry_offset{kOriginalPtxFirstEntryOffset};
    std::uint64_t sector_size{kOriginalPtxSectorSize};
    // Runtime cursor after applying every block-count advance. The final
    // advance is observed but is not treated as a proven physical EOF.
    std::uint64_t cursor_after_entries{};
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
    // Validates only the recovered original PTX envelope relation:
    // - <= 64 runtime texture entries (product safety ceiling from the
    //   recovered PtxRuntimeBundle capacity);
    // - first TIM2 entry at +0x800;
    // - each entry begins with the recovered `TM2\0` marker;
    // - runtime cursor advancement is blockCount[i] * 0x800;
    // - every entry that is actually dereferenced is bounded by the supplied
    //   source bytes;
    // - zero block-count advances are preserved, not normalized or rejected.
    //
    // The first 0x800 bytes remain opaque in Pass 87. This validator must not
    // be presented as a complete raw-header PTX parser or PTX/TIM2 writer.
    [[nodiscard]] static OriginalPtxEnvelopeGeometryResult validate(
        std::span<const std::byte> bytes,
        OriginalPtxEnvelopeDescriptorView descriptor);
};

} // namespace dmc::rengine::profiles::dmc3
