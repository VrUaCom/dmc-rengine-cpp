#pragma once

#include <cstddef>
#include <cstdint>
#include <string_view>

namespace dmc::rengine::profiles::dmc3 {

// Instruction-backed contract for what the runtime does with a texture's
// declared mip count.
//
// This one exists because a rule got into the framing parser without ever
// having been recovered. Every texture in the supplied corpus carries a
// complete mip chain, and that observation was written down as a requirement:
// a DDS declaring three levels for a 128x128 image was refused as lying
// "outside the evidenced full-chain descriptor domain".
//
// That sentence covered four unrelated clauses in one condition — a zero
// dimension, a dimension past 0xFFFF, a zero or oversized count, and an
// incomplete chain — so an operator reading it could not tell which had
// fired, and neither can this comment reconstruct it after the fact. Three of
// the four were sanity bounds picked by eye. The fourth was an inference from
// a sample, and it is the one the image was asked about.
//
// So the question was put to the image. The runtime's texture loader is
// DirectXTK's DDSTextureLoader, reached through the only two sites that
// compare against `'DDS '`, and it answers plainly:
//
//   * header validation reads magic, dwSize, and the pixel format. It never
//     reads dwMipMapCount, so no declared chain length can fail validation;
//   * the loader takes dwMipMapCount verbatim and substitutes 1 only when it
//     is zero;
//   * the one bound it applies is an upper one, `cmp rdi, 0xF`, which is
//     D3D11's own limit;
//   * a file declaring exactly one level is not refused but *handled*: the
//     loader asks the device for MIP_AUTOGEN support and zeroes the count so
//     the rest are generated. The loader has a designed path for an
//     incomplete chain;
//   * the only refusal the count can cause is running the level walk past the
//     end of the supplied bytes, which returns ERROR_HANDLE_EOF.
//
// The requirement is therefore that a texture *contain* the levels it
// declares. It is not that the declared count reach the bottom of the chain.
//
// Receipts, all bound to the canonical image below, live in
// `data/reverse/dmc3-type-identification-windows.v1.json` under the
// `dds-header-validation-*`, `dds-mip-*` and `dds-texture-dimension-bounds`
// names.
struct TextureMipChainContract final {
    static constexpr std::string_view canonical_target_sha256 =
        "e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082";
    static constexpr std::uint64_t image_base = 0x140000000ULL;

    // The two header validators. Byte-for-byte the same sequence; one is
    // entered from a caller-supplied buffer, the other after a file read.
    static constexpr std::uint64_t validate_from_memory_va = 0x140049A8EULL;
    static constexpr std::uint64_t validate_from_file_va = 0x14004AD9DULL;

    // What validation actually checks.
    static constexpr std::uint32_t dds_magic = 0x20534444U; // 'DDS '
    static constexpr std::uint32_t header_size = 124U;      // dwSize
    static constexpr std::uint32_t pixel_format_size = 32U; // ddspf.dwSize
    static constexpr std::size_t minimum_header_bytes = 0x80U;
    static constexpr std::size_t minimum_header_bytes_dx10 = 0x94U;

    // And what it does not: the mip field is not among the offsets it reads.
    static constexpr bool validation_reads_mip_count = false;

    // The consumer. `mov edi, [r8 + 0x18]` / `test rdi, rdi` / `cmove`.
    static constexpr std::uint64_t consume_mip_count_va = 0x140049BFAULL;
    static constexpr std::size_t dds_header_mip_count_offset = 0x18U;
    static constexpr std::uint32_t mip_count_substituted_for_zero = 1U;

    // `cmp rdi, 0xF` at the head of the resource-dimension switch. This is
    // D3D11_REQ_MIP_LEVELS: the runtime bounds the count from above and never
    // from below.
    static constexpr std::uint64_t mip_count_bound_va = 0x140049D51ULL;
    static constexpr std::uint32_t max_mip_count = 15U;

    // The 2D dimension bounds from the same switch: D3D11's
    // REQ_TEXTURE2D_U_OR_V_DIMENSION and REQ_TEXTURE2D_ARRAY_AXIS_DIMENSION.
    static constexpr std::uint64_t dimension_bound_va = 0x140049D97ULL;
    static constexpr std::uint32_t max_dimension = 16384U;
    static constexpr std::uint32_t max_array_size = 2048U;

    // The autogeneration branch: proof that a short chain is contemplated.
    static constexpr std::uint64_t mip_autogen_va = 0x140049DDAULL;
    static constexpr std::uint32_t mip_count_that_triggers_autogen = 1U;
    // D3D11_FORMAT_SUPPORT_MIP_AUTOGEN, tested against CheckFormatSupport's
    // reply through ID3D11Device vtable slot 29 (+0xE8).
    static constexpr std::uint32_t format_support_mip_autogen = 0x2000U;
    static constexpr std::size_t check_format_support_vtable_offset = 0xE8U;

    // The level walk, and the only refusal a mip count can produce.
    static constexpr std::uint64_t mip_level_walk_va = 0x140049F8BULL;
    static constexpr std::uint32_t error_handle_eof_hresult = 0x80070026U;

    /**
     * The recovered rule, stated once so callers do not restate it wrongly.
     *
     * A declared count is loadable when it is at least one, at most
     * `max_mip_count`, and the payload holds every level it names. Whether it
     * reaches the bottom of the chain is not part of it.
     */
    [[nodiscard]] static constexpr bool mip_count_is_loadable(
        std::uint32_t declared) noexcept {
        return declared >= 1U && declared <= max_mip_count;
    }

    [[nodiscard]] static constexpr bool dimensions_are_loadable(
        std::uint32_t width, std::uint32_t height) noexcept {
        return width >= 1U && height >= 1U && width <= max_dimension &&
               height <= max_dimension;
    }
};

// A complete chain is loadable, which was never in doubt; the point of
// asserting it is that everything shorter is loadable too, down to one. These
// sit outside the class because a member `static_assert` would call these
// functions before the class is complete.
static_assert(TextureMipChainContract::mip_count_is_loadable(1U));
static_assert(TextureMipChainContract::mip_count_is_loadable(3U));
static_assert(TextureMipChainContract::mip_count_is_loadable(
    TextureMipChainContract::max_mip_count));
static_assert(!TextureMipChainContract::mip_count_is_loadable(0U));
static_assert(!TextureMipChainContract::mip_count_is_loadable(
    TextureMipChainContract::max_mip_count + 1U));
static_assert(TextureMipChainContract::dimensions_are_loadable(128U, 128U));
static_assert(!TextureMipChainContract::dimensions_are_loadable(
    TextureMipChainContract::max_dimension + 1U, 128U));

} // namespace dmc::rengine::profiles::dmc3
