#pragma once

#include <cstddef>
#include <cstdint>
#include <span>
#include <string_view>
#include <vector>

// Block-compressed DDS reader for the full BC1..BC7 family.
//
// Accepts what the DMC3 HD executable's DDS loader accepts for block formats
// (DirectXTK CreateDDSTextureFromMemory, dmc3.exe 0x1400499C0): the legacy
// FourCC header (DXT1..DXT5, ATI1/ATI2, BC4U/BC4S/BC5U/BC5S) and the "DX10"
// extended header with a DXGI BC format. Bounded to single 2D images, which is
// the only shape the game's texture records use.
//
// Decoding follows the Direct3D 11 block format rules (BPTC spec for BC6H and
// BC7). Output is RGBA8 for previews:
//   - BC4: grey (R replicated), BC5: R, G, B = 0;
//   - SNORM channels map [-1, 1] to [0, 255];
//   - BC6H HDR values are clamped to [0, 1] and rounded to 8 bits;
//   - reserved BC6H / BC7 modes decode to zero, as the D3D spec requires.
namespace dmc::rengine::codecs::dds_bcn {

enum class Format : std::uint8_t {
    bc1,
    bc2,
    bc3,
    bc4_unorm,
    bc4_snorm,
    bc5_unorm,
    bc5_snorm,
    bc6h_uf16,
    bc6h_sf16,
    bc7,
};

enum class Status : std::uint8_t {
    ok,
    truncated,
    invalid_magic,
    invalid_header,
    invalid_dimensions,
    invalid_mip_count,
    unsupported_format,
    payload_overflow,
    payload_out_of_bounds,
};

[[nodiscard]] std::string_view to_string(Status status) noexcept;

// Short name ("BC7", "BC1 (DXT1)", ...).
[[nodiscard]] std::string_view format_name(Format format) noexcept;

// 8 for BC1/BC4, 16 for the rest.
[[nodiscard]] std::uint32_t block_bytes(Format format) noexcept;

struct Document final {
    std::uint32_t width{};
    std::uint32_t height{};
    std::uint32_t mip_count{};
    Format format{Format::bc1};
    // DDS_HEADER_DXT10 present (header is 148 bytes instead of 128).
    bool dx10_header{};
    // *_SRGB DXGI format; decoded values are the stored ones.
    bool srgb{};
    // Legacy premultiplied-alpha FourCC (DXT2, DXT4).
    bool premultiplied{};
    std::uint32_t fourcc{};
    // DXGI_FORMAT value for DX10 headers, 0 otherwise.
    std::uint32_t dxgi_format{};
    std::uint32_t header_size{};
    std::uint32_t payload_size{};
    std::uint32_t total_size{};

    [[nodiscard]] bool valid() const noexcept;
};

struct ParseResult final {
    Status status{Status::invalid_header};
    Document document;
    std::string_view detail;

    [[nodiscard]] bool ok() const noexcept;
};

struct RgbaImage final {
    std::uint32_t width{};
    std::uint32_t height{};
    std::vector<std::uint8_t> rgba8;

    [[nodiscard]] bool available() const noexcept;
};

struct DecodeResult final {
    bool ok{};
    RgbaImage image;
    // Mip level the image was taken from, and the box-filter factor applied
    // to it (1 = exact pixels).
    std::uint32_t mip_level{};
    std::uint32_t downscale{1U};
    std::string_view detail;
};

inline constexpr std::size_t legacy_header_size = 128U;
inline constexpr std::size_t dx10_header_size = 148U;

[[nodiscard]] std::uint32_t maximum_mip_count(std::uint32_t width, std::uint32_t height) noexcept;

// Bytes of one mip level / of a whole chain; false on overflow.
[[nodiscard]] bool level_size(
    std::uint32_t width, std::uint32_t height, Format format, std::uint64_t* output) noexcept;
[[nodiscard]] bool chain_size(
    std::uint32_t width, std::uint32_t height, std::uint32_t mip_count, Format format,
    std::uint64_t* output) noexcept;

// Parses a DDS beginning at bytes[0]. Trailing bytes after the image are
// allowed; callers decide whether the span must end at the image.
[[nodiscard]] ParseResult parse(std::span<const std::byte> bytes) noexcept;

// Decodes one 4x4 block into 16 RGBA8 pixels (row-major).
void decode_block(Format format, const std::byte* block, std::uint8_t* rgba64) noexcept;

// Decodes mip `level` exactly.
[[nodiscard]] DecodeResult decode_level_rgba8(
    std::span<const std::byte> bytes, const Document& document, std::uint32_t level,
    std::uint64_t max_pixels = 4ULL * 1024ULL * 1024ULL) noexcept;

// Preview: mip 0 when it fits max_pixels, else the first stored mip that
// fits, else mip 0 box-filtered by the smallest power of two that fits.
// Never allocates more than max_pixels RGBA8 pixels (plus an accumulator of
// the same size when box-filtering).
[[nodiscard]] DecodeResult decode_preview_rgba8(
    std::span<const std::byte> bytes, const Document& document,
    std::uint64_t max_pixels = 4ULL * 1024ULL * 1024ULL) noexcept;

}  // namespace dmc::rengine::codecs::dds_bcn
