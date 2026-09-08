#include "dmc_rengine/codecs/dds_bc.hpp"

#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <limits>

namespace dmc::rengine::codecs::dds_bc {
namespace {

[[nodiscard]] std::uint16_t read_u16_le(const std::byte* p) noexcept {
    return static_cast<std::uint16_t>(std::to_integer<std::uint8_t>(p[0])) |
        static_cast<std::uint16_t>(
            static_cast<std::uint16_t>(std::to_integer<std::uint8_t>(p[1])) << 8U);
}

[[nodiscard]] std::uint32_t read_u32_le(const std::byte* p) noexcept {
    return static_cast<std::uint32_t>(std::to_integer<std::uint8_t>(p[0])) |
        (static_cast<std::uint32_t>(std::to_integer<std::uint8_t>(p[1])) << 8U) |
        (static_cast<std::uint32_t>(std::to_integer<std::uint8_t>(p[2])) << 16U) |
        (static_cast<std::uint32_t>(std::to_integer<std::uint8_t>(p[3])) << 24U);
}

[[nodiscard]] bool level_payload_size(
    std::uint32_t width,
    std::uint32_t height,
    std::uint32_t block_bytes,
    std::uint64_t* output) noexcept {
    if (output == nullptr || width == 0U || height == 0U || block_bytes == 0U) {
        return false;
    }
    const auto blocks_w = (static_cast<std::uint64_t>(width) + 3ULL) / 4ULL;
    const auto blocks_h = (static_cast<std::uint64_t>(height) + 3ULL) / 4ULL;
    if (blocks_w != 0U && blocks_h >
            std::numeric_limits<std::uint64_t>::max() / blocks_w) {
        return false;
    }
    const auto blocks = blocks_w * blocks_h;
    if (blocks > std::numeric_limits<std::uint64_t>::max() / block_bytes) {
        return false;
    }
    *output = blocks * block_bytes;
    return true;
}

[[nodiscard]] ParseResult failure(Status status, std::string_view detail) noexcept {
    return ParseResult{.status = status, .document = {}, .detail = detail};
}

struct Rgba final {
    std::uint8_t r{};
    std::uint8_t g{};
    std::uint8_t b{};
    std::uint8_t a{255U};
};

[[nodiscard]] Rgba decode_565(std::uint16_t value) noexcept {
    const auto r5 = static_cast<std::uint32_t>((value >> 11U) & 0x1FU);
    const auto g6 = static_cast<std::uint32_t>((value >> 5U) & 0x3FU);
    const auto b5 = static_cast<std::uint32_t>(value & 0x1FU);
    return {
        static_cast<std::uint8_t>((r5 * 255U + 15U) / 31U),
        static_cast<std::uint8_t>((g6 * 255U + 31U) / 63U),
        static_cast<std::uint8_t>((b5 * 255U + 15U) / 31U),
        255U,
    };
}

[[nodiscard]] Rgba mix(
    const Rgba& a,
    const Rgba& b,
    std::uint32_t wa,
    std::uint32_t wb,
    std::uint32_t divisor) noexcept {
    return {
        static_cast<std::uint8_t>((wa * a.r + wb * b.r) / divisor),
        static_cast<std::uint8_t>((wa * a.g + wb * b.g) / divisor),
        static_cast<std::uint8_t>((wa * a.b + wb * b.b) / divisor),
        255U,
    };
}

void color_palette(
    const std::byte* block,
    bool dxt1,
    std::array<Rgba, 4U>* output) noexcept {
    const auto c0_raw = read_u16_le(block + 0U);
    const auto c1_raw = read_u16_le(block + 2U);
    const auto c0 = decode_565(c0_raw);
    const auto c1 = decode_565(c1_raw);
    (*output)[0] = c0;
    (*output)[1] = c1;
    if (!dxt1 || c0_raw > c1_raw) {
        (*output)[2] = mix(c0, c1, 2U, 1U, 3U);
        (*output)[3] = mix(c0, c1, 1U, 2U, 3U);
    } else {
        (*output)[2] = mix(c0, c1, 1U, 1U, 2U);
        (*output)[3] = {0U, 0U, 0U, 0U};
    }
}

void alpha_palette(
    const std::byte* block,
    std::array<std::uint8_t, 8U>* output) noexcept {
    const auto a0 = std::to_integer<std::uint8_t>(block[0]);
    const auto a1 = std::to_integer<std::uint8_t>(block[1]);
    (*output)[0] = a0;
    (*output)[1] = a1;
    if (a0 > a1) {
        (*output)[2] = static_cast<std::uint8_t>((6U * a0 + a1) / 7U);
        (*output)[3] = static_cast<std::uint8_t>((5U * a0 + 2U * a1) / 7U);
        (*output)[4] = static_cast<std::uint8_t>((4U * a0 + 3U * a1) / 7U);
        (*output)[5] = static_cast<std::uint8_t>((3U * a0 + 4U * a1) / 7U);
        (*output)[6] = static_cast<std::uint8_t>((2U * a0 + 5U * a1) / 7U);
        (*output)[7] = static_cast<std::uint8_t>((a0 + 6U * a1) / 7U);
    } else {
        (*output)[2] = static_cast<std::uint8_t>((4U * a0 + a1) / 5U);
        (*output)[3] = static_cast<std::uint8_t>((3U * a0 + 2U * a1) / 5U);
        (*output)[4] = static_cast<std::uint8_t>((2U * a0 + 3U * a1) / 5U);
        (*output)[5] = static_cast<std::uint8_t>((a0 + 4U * a1) / 5U);
        (*output)[6] = 0U;
        (*output)[7] = 255U;
    }
}

void write_pixel(
    RgbaImage* image,
    std::uint32_t x,
    std::uint32_t y,
    const Rgba& rgba) noexcept {
    const auto index =
        (static_cast<std::size_t>(y) * image->width + x) * 4U;
    image->rgba8[index + 0U] = rgba.r;
    image->rgba8[index + 1U] = rgba.g;
    image->rgba8[index + 2U] = rgba.b;
    image->rgba8[index + 3U] = rgba.a;
}

} // namespace

bool Document::valid() const noexcept {
    return width != 0U && height != 0U && mip_count != 0U &&
        payload_size != 0U && total_size == header_size + payload_size;
}

bool ParseResult::ok() const noexcept {
    return status == Status::ok && document.valid();
}

bool RgbaImage::available() const noexcept {
    if (width == 0U || height == 0U) return false;
    const auto pixels = static_cast<std::uint64_t>(width) * height;
    return pixels <= std::numeric_limits<std::size_t>::max() / 4U &&
        rgba8.size() == static_cast<std::size_t>(pixels * 4U);
}

std::uint32_t maximum_mip_count(
    std::uint32_t width,
    std::uint32_t height) noexcept {
    if (width == 0U || height == 0U) return 0U;
    auto dimension = std::max(width, height);
    std::uint32_t count = 1U;
    while (dimension > 1U) {
        dimension /= 2U;
        ++count;
    }
    return count;
}

bool payload_size(
    std::uint32_t width,
    std::uint32_t height,
    std::uint32_t mip_count,
    Compression compression,
    std::uint32_t* output) noexcept {
    if (output == nullptr || width == 0U || height == 0U || mip_count == 0U ||
        mip_count > maximum_mip_count(width, height)) {
        return false;
    }
    const std::uint32_t block_bytes = compression == Compression::dxt1 ? 8U : 16U;
    std::uint64_t total = 0U;
    for (std::uint32_t level = 0U; level < mip_count; ++level) {
        std::uint64_t level_bytes = 0U;
        if (!level_payload_size(width, height, block_bytes, &level_bytes) ||
            total > std::numeric_limits<std::uint32_t>::max() - level_bytes) {
            return false;
        }
        total += level_bytes;
        width = std::max(1U, width / 2U);
        height = std::max(1U, height / 2U);
    }
    *output = static_cast<std::uint32_t>(total);
    return true;
}

ParseResult parse(std::span<const std::byte> bytes) noexcept {
    if (bytes.size() < header_size) {
        return failure(Status::truncated, "DDS is shorter than the 128-byte header");
    }
    if (bytes[0U] != std::byte{'D'} || bytes[1U] != std::byte{'D'} ||
        bytes[2U] != std::byte{'S'} || bytes[3U] != std::byte{' '}) {
        return failure(Status::invalid_magic, "DDS magic is not present");
    }
    if (read_u32_le(bytes.data() + 4U) != 124U ||
        read_u32_le(bytes.data() + 76U) != 32U ||
        (read_u32_le(bytes.data() + 80U) & 4U) == 0U) {
        return failure(Status::invalid_header, "DDS header structure is not a FourCC image");
    }

    const auto height = read_u32_le(bytes.data() + 12U);
    const auto width = read_u32_le(bytes.data() + 16U);
    if (width == 0U || height == 0U) {
        return failure(Status::invalid_dimensions, "DDS dimensions are zero");
    }

    const auto mip_count = read_u32_le(bytes.data() + 28U);
    const auto max_mips = maximum_mip_count(width, height);
    if (mip_count == 0U || mip_count > max_mips) {
        return failure(Status::invalid_mip_count, "DDS mip count exceeds the bounded image pyramid");
    }

    Compression compression{};
    const auto* code = bytes.data() + 84U;
    if (code[0] == std::byte{'D'} && code[1] == std::byte{'X'} &&
        code[2] == std::byte{'T'} && code[3] == std::byte{'1'}) {
        compression = Compression::dxt1;
    } else if (code[0] == std::byte{'D'} && code[1] == std::byte{'X'} &&
               code[2] == std::byte{'T'} && code[3] == std::byte{'5'}) {
        compression = Compression::dxt5;
    } else {
        return failure(Status::unsupported_compression, "Only DXT1 and DXT5 are supported by the portable reader codec");
    }

    std::uint32_t payload = 0U;
    if (!payload_size(width, height, mip_count, compression, &payload)) {
        return failure(Status::payload_overflow, "DDS compressed mip payload size overflows");
    }
    const auto total = static_cast<std::uint64_t>(header_size) + payload;
    if (total > bytes.size() || total > std::numeric_limits<std::uint32_t>::max()) {
        return failure(Status::payload_out_of_bounds, "DDS compressed mip payload leaves the bounded input span");
    }

    return ParseResult{
        .status = Status::ok,
        .document = Document{
            .width = width,
            .height = height,
            .mip_count = mip_count,
            .compression = compression,
            .payload_size = payload,
            .total_size = static_cast<std::uint32_t>(total),
        },
        .detail = {},
    };
}

DecodeResult decode_base_mip_rgba8(
    std::span<const std::byte> bytes,
    const Document& document,
    std::uint64_t max_pixels) noexcept {
    if (!document.valid() || document.total_size > bytes.size()) {
        return {.ok = false, .image = {}, .detail = "DDS decode received an invalid parsed document span"};
    }
    const auto pixel_count =
        static_cast<std::uint64_t>(document.width) * document.height;
    if (pixel_count == 0U || pixel_count > max_pixels ||
        pixel_count > std::numeric_limits<std::size_t>::max() / 4U) {
        return {.ok = false, .image = {}, .detail = "DDS preview exceeds the bounded output pixel budget"};
    }

    const std::uint32_t block_bytes =
        document.compression == Compression::dxt1 ? 8U : 16U;
    std::uint64_t base_bytes = 0U;
    if (!level_payload_size(document.width, document.height, block_bytes, &base_bytes) ||
        base_bytes > bytes.size() - header_size) {
        return {.ok = false, .image = {}, .detail = "DDS base mip leaves the bounded input span"};
    }

    DecodeResult result;
    result.image.width = document.width;
    result.image.height = document.height;
    try {
        result.image.rgba8.assign(static_cast<std::size_t>(pixel_count * 4U), 0U);
    } catch (...) {
        return {.ok = false, .image = {}, .detail = "DDS preview allocation failed"};
    }

    const auto blocks_w = (static_cast<std::uint64_t>(document.width) + 3ULL) / 4ULL;
    const auto blocks_h = (static_cast<std::uint64_t>(document.height) + 3ULL) / 4ULL;
    const auto* base = bytes.data() + header_size;

    for (std::uint64_t by = 0U; by < blocks_h; ++by) {
        for (std::uint64_t bx = 0U; bx < blocks_w; ++bx) {
            const auto block_index = by * blocks_w + bx;
            const auto* block = base + static_cast<std::size_t>(block_index * block_bytes);
            std::array<Rgba, 4U> colors{};
            std::uint32_t color_indices = 0U;
            std::array<std::uint8_t, 8U> alphas{};
            std::uint64_t alpha_indices = 0U;

            if (document.compression == Compression::dxt1) {
                color_palette(block, true, &colors);
                color_indices = read_u32_le(block + 4U);
            } else {
                alpha_palette(block, &alphas);
                for (std::uint32_t i = 0U; i < 6U; ++i) {
                    alpha_indices |= static_cast<std::uint64_t>(
                        std::to_integer<std::uint8_t>(block[2U + i])) << (i * 8U);
                }
                color_palette(block + 8U, false, &colors);
                color_indices = read_u32_le(block + 12U);
            }

            for (std::uint32_t py = 0U; py < 4U; ++py) {
                for (std::uint32_t px = 0U; px < 4U; ++px) {
                    const auto x64 = bx * 4ULL + px;
                    const auto y64 = by * 4ULL + py;
                    if (x64 >= document.width || y64 >= document.height) continue;
                    const auto local = py * 4U + px;
                    const auto color_code = (color_indices >> (local * 2U)) & 0x3U;
                    auto rgba = colors[color_code];
                    if (document.compression == Compression::dxt5) {
                        const auto alpha_code = static_cast<std::uint32_t>(
                            (alpha_indices >> (local * 3U)) & 0x7ULL);
                        rgba.a = alphas[alpha_code];
                    }
                    write_pixel(
                        &result.image,
                        static_cast<std::uint32_t>(x64),
                        static_cast<std::uint32_t>(y64),
                        rgba);
                }
            }
        }
    }

    result.ok = result.image.available();
    result.detail = result.ok ? std::string_view{} :
        std::string_view{"DDS preview output failed self-validation"};
    return result;
}

} // namespace dmc::rengine::codecs::dds_bc
