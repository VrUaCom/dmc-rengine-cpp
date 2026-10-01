#pragma once

#include <cstddef>
#include <cstdint>
#include <optional>
#include <span>
#include <string_view>
#include <vector>

#include "dmc_rengine/codecs/dds_bcn.hpp"

// Block-compressed DDS writer: the encoding side of dds_bcn.
//
// Every format dds_bcn reads can be written, so a texture can be moved to any
// of them:
//   BC1 (DXT1)  colour 5:6:5, 1-bit alpha
//   BC2 (DXT3)  BC1 colour + explicit 4-bit alpha
//   BC3 (DXT5)  BC1 colour + interpolated 8-bit alpha (retail DMC3 format)
//   BC4         one channel (luminance of the input), UNORM or SNORM
//   BC5         two channels (R, G), UNORM or SNORM
//   BC6H        HDR RGB half floats (single-region mode 11; LDR input)
//   BC7         RGBA, modes 6 (one line in RGBA) and 5 (colour line +
//               separate alpha, with channel rotation), best per block
// Each encoder is verified by decoding with dds_bcn (tests/dds_bcn_encode_tests).
namespace dmc::rengine::codecs::dds_bcn {

// Encodes one 4x4 block of RGBA8 pixels (row-major) into block_bytes(format)
// bytes.
void encode_block(Format format, const std::uint8_t* rgba64, std::byte* out) noexcept;

// Encodes an RGBA8 image of any size; partial edge blocks replicate the last
// row / column. Returns false on a size mismatch or allocation failure.
[[nodiscard]] bool encode_level(Format format, const RgbaImage& image, std::vector<std::byte>* out);

// Next mip level: 2x2 box filter (odd edges average what exists).
[[nodiscard]] RgbaImage downsample(const RgbaImage& image);

// Legacy FourCC for the format, if it has one ("DXT1", "DXT3", "DXT5",
// "BC4U", "BC4S", "BC5U", "BC5S"); BC6H and BC7 only exist with a DX10
// header.
[[nodiscard]] std::optional<std::uint32_t> legacy_fourcc(Format format) noexcept;

// DXGI_FORMAT written in a DX10 header (UNORM variants; BC6H UF16/SF16).
[[nodiscard]] std::uint32_t dxgi_format(Format format) noexcept;

struct DdsEncodeOptions final {
    // Write the DX10 extended header even when a legacy FourCC exists.
    bool force_dx10{};
};

struct DdsEncodeResult final {
    bool ok{};
    std::vector<std::byte> bytes;
    std::string_view detail;
};

// Builds a DDS from mip levels, level 0 first; each level must be half the
// previous one (at least 1). One level writes a single-image DDS with no mip
// count, as the retail interface textures do.
[[nodiscard]] DdsEncodeResult encode_dds(
    Format format, std::span<const RgbaImage> levels, const DdsEncodeOptions& options = {});

// "bc1".."bc7", "dxt1", "dxt3", "dxt5", "bc4s", "bc5s", "bc6h", "bc6h_sf16",
// "ati1", "ati2" (case-insensitive).
[[nodiscard]] std::optional<Format> parse_format_name(std::string_view name) noexcept;

// Every writable format, in menu order.
[[nodiscard]] std::span<const Format> writable_formats() noexcept;

}  // namespace dmc::rengine::codecs::dds_bcn
