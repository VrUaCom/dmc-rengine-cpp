#include "dmc_rengine/codecs/dds_bcn.hpp"
#include "dmc_rengine/codecs/dds_bcn_encode.hpp"

#include <cassert>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <span>
#include <string>
#include <array>
#include <vector>

namespace {

namespace bcn = dmc::rengine::codecs::dds_bcn;

// Smooth colour gradients, a hard edge and a soft alpha ramp: the content
// texture formats are judged on.
bcn::RgbaImage test_image(std::uint32_t w, std::uint32_t h) {
    bcn::RgbaImage img;
    img.width = w;
    img.height = h;
    img.rgba8.resize(static_cast<std::size_t>(w) * h * 4U);
    for (std::uint32_t y = 0; y < h; ++y) {
        for (std::uint32_t x = 0; x < w; ++x) {
            auto* p = &img.rgba8[(static_cast<std::size_t>(y) * w + x) * 4U];
            p[0] = static_cast<std::uint8_t>(x * 255U / (w - 1U));
            p[1] = static_cast<std::uint8_t>(y * 255U / (h - 1U));
            p[2] = static_cast<std::uint8_t>((x / 8U + y / 8U) % 2U == 0U ? 200U : 40U);
            p[3] = static_cast<std::uint8_t>(128.0 + 127.0 * std::sin(static_cast<double>(x + y) * 0.1));
        }
    }
    return img;
}

double psnr(const std::vector<std::uint8_t>& a, const std::vector<std::uint8_t>& b,
            std::size_t first, std::size_t count) {
    double mse = 0.0;
    std::size_t n = 0;
    for (std::size_t i = 0; i < a.size(); i += 4U) {
        for (std::size_t c = first; c < first + count; ++c) {
            const double d = static_cast<double>(a[i + c]) - b[i + c];
            mse += d * d;
            ++n;
        }
    }
    mse /= static_cast<double>(n);
    return mse == 0.0 ? 99.0 : 10.0 * std::log10(255.0 * 255.0 / mse);
}

std::span<const std::byte> view(const std::vector<std::byte>& b) { return {b.data(), b.size()}; }

bool fail(bcn::Format, double) { return false; }

bool round_trip_every_format() {
    const auto img = test_image(64, 64);
    std::vector<bcn::RgbaImage> levels{img};
    while (levels.back().width > 1U || levels.back().height > 1U) levels.push_back(bcn::downsample(levels.back()));
    assert(levels.size() == 7U && levels.back().width == 1U);

    struct Case { bcn::Format f; std::size_t first, count; double min_psnr; bool dx10; };
    const Case cases[] = {
        {bcn::Format::bc1, 0, 3, 30.0, false},
        {bcn::Format::bc2, 0, 4, 29.0, false},
        {bcn::Format::bc3, 0, 4, 31.0, false},
        {bcn::Format::bc4_unorm, 0, 1, 0.0, false},
        {bcn::Format::bc5_unorm, 0, 2, 40.0, false},
        {bcn::Format::bc6h_uf16, 0, 3, 30.0, true},
        {bcn::Format::bc7, 0, 4, 38.0, true},
    };
    // BC1 alpha is 1-bit: judge its colour on an opaque copy.
    auto opaque = img;
    for (std::size_t i = 3; i < opaque.rgba8.size(); i += 4U) opaque.rgba8[i] = 255;
    std::vector<bcn::RgbaImage> opaque_levels{opaque};
    while (opaque_levels.back().width > 1U) opaque_levels.push_back(bcn::downsample(opaque_levels.back()));
    for (const auto& c : cases) {
        const bool bc1 = c.f == bcn::Format::bc1;
        const auto dds = bcn::encode_dds(c.f, bc1 ? opaque_levels : levels);
        assert(dds.ok);
        const auto parsed = bcn::parse(view(dds.bytes));
        assert(parsed.ok() && parsed.document.format == c.f);
        assert(parsed.document.mip_count == levels.size() && parsed.document.total_size == dds.bytes.size());
        assert(parsed.document.dx10_header == c.dx10);
        const auto decoded = bcn::decode_level_rgba8(view(dds.bytes), parsed.document, 0U);
        assert(decoded.ok);
        if (c.min_psnr > 0.0) {
            const double q = psnr(bc1 ? opaque.rgba8 : img.rgba8, decoded.image.rgba8, c.first, c.count);
            std::fprintf(stderr, "  %-10s PSNR %.1f dB\n", std::string(bcn::format_name(c.f)).c_str(), q);
            if (q < c.min_psnr) return fail(c.f, q);
        }
        // Last level decodes too (1x1 from a replicated block).
        assert(bcn::decode_level_rgba8(view(dds.bytes), parsed.document, 6U).ok);
    }
    return true;
}

void bc4_is_luminance_and_snorm_ranges() {
    bcn::RgbaImage grey;
    grey.width = grey.height = 4;
    grey.rgba8.assign(64, 0);
    for (std::size_t i = 0; i < 16; ++i) {
        const auto v = static_cast<std::uint8_t>(i * 17);
        grey.rgba8[i * 4] = grey.rgba8[i * 4 + 1] = grey.rgba8[i * 4 + 2] = v;
        grey.rgba8[i * 4 + 3] = 255;
    }
    for (const auto f : {bcn::Format::bc4_unorm, bcn::Format::bc4_snorm}) {
        std::vector<bcn::RgbaImage> one{grey};
        const auto dds = bcn::encode_dds(f, one);
        const auto parsed = bcn::parse(view(dds.bytes));
        const auto d = bcn::decode_level_rgba8(view(dds.bytes), parsed.document, 0U);
        assert(d.ok);
        for (std::size_t i = 0; i < 16; ++i) assert(std::abs(int{d.image.rgba8[i * 4]} - int(i * 17)) <= 19);  // 16 shades on 8 levels: half a step
        assert(parsed.document.mip_count == 1U);
    }
}

void bc1_transparency() {
    bcn::RgbaImage img;
    img.width = img.height = 4;
    img.rgba8.assign(64, 0);
    for (std::size_t i = 0; i < 16; ++i) {
        img.rgba8[i * 4] = 250;
        img.rgba8[i * 4 + 3] = (i % 2 == 0) ? 255 : 0;
    }
    std::array<std::byte, 8> block{};
    bcn::encode_block(bcn::Format::bc1, img.rgba8.data(), block.data());
    std::array<std::uint8_t, 64> out{};
    bcn::decode_block(bcn::Format::bc1, block.data(), out.data());
    for (std::size_t i = 0; i < 16; ++i) {
        assert((out[i * 4 + 3] == 255) == (i % 2 == 0));
        if (i % 2 == 0) assert(std::abs(int{out[i * 4]} - 250) <= 4);
    }
}

void exact_flat_colours() {
    // A flat block survives BC7 exactly, every channel value, via p-bits/mode 5.
    for (int v = 0; v < 256; v += 17) {
        std::array<std::uint8_t, 64> px{};
        for (std::size_t i = 0; i < 16; ++i) {
            px[i * 4] = static_cast<std::uint8_t>(v);
            px[i * 4 + 1] = static_cast<std::uint8_t>(255 - v);
            px[i * 4 + 2] = static_cast<std::uint8_t>(v / 2);
            px[i * 4 + 3] = 255;
        }
        std::array<std::byte, 16> block{};
        bcn::encode_block(bcn::Format::bc7, px.data(), block.data());
        std::array<std::uint8_t, 64> out{};
        bcn::decode_block(bcn::Format::bc7, block.data(), out.data());
        for (std::size_t i = 0; i < 64; ++i) assert(std::abs(int{out[i]} - int{px[i]}) <= 1);
    }
}

void names_and_headers() {
    assert(bcn::parse_format_name("DXT5") == bcn::Format::bc3);
    assert(bcn::parse_format_name("bc7") == bcn::Format::bc7);
    assert(bcn::parse_format_name("BC6H-SF16") == bcn::Format::bc6h_sf16);
    assert(!bcn::parse_format_name("png").has_value());
    assert(bcn::writable_formats().size() == 10U);
    // Forced DX10 for a format with a legacy FourCC.
    std::vector<bcn::RgbaImage> one{test_image(8, 8)};
    const auto dds = bcn::encode_dds(bcn::Format::bc3, one, {.force_dx10 = true});
    const auto parsed = bcn::parse(view(dds.bytes));
    assert(parsed.ok() && parsed.document.dx10_header && parsed.document.dxgi_format == 77U);
    // Non-halving level chain is refused.
    std::vector<bcn::RgbaImage> bad{test_image(8, 8), test_image(8, 8)};
    assert(!bcn::encode_dds(bcn::Format::bc1, bad).ok);
}

}  // namespace

int main() {
    const bool quality = round_trip_every_format();
    bc4_is_luminance_and_snorm_ranges();
    bc1_transparency();
    exact_flat_colours();
    names_and_headers();
    assert(quality);
    return 0;
}
