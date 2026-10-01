// Texture re-encode on synthetic DMC3 layouts (no game data): a canonical
// full-chain DXT5 PTX in a PAC, a single gfxTexture and a bare DDS.
#include "dmc_rengine/codecs/dds_bcn.hpp"
#include "dmc_rengine/codecs/dds_bcn_encode.hpp"
#include "dmc_rengine/profiles/dmc3/texture_reencode.hpp"
#include "dmc_rengine/profiles/dmc3/texture_slot_framing.hpp"

#include <bit>
#include <cassert>
#include <cstring>
#include <span>
#include <vector>

namespace {

namespace bcn = dmc::rengine::codecs::dds_bcn;
namespace dmc3 = dmc::rengine::profiles::dmc3;

void put32(std::vector<std::byte>& b, std::size_t o, std::uint32_t v) {
    for (std::size_t i = 0; i < 4; ++i) b[o + i] = static_cast<std::byte>((v >> (8 * i)) & 0xFF);
}
std::uint32_t get32(const std::vector<std::byte>& b, std::size_t o) {
    std::uint32_t v = 0;
    for (std::size_t i = 0; i < 4; ++i) v |= std::to_integer<std::uint32_t>(b[o + i]) << (8 * i);
    return v;
}
std::span<const std::byte> view(const std::vector<std::byte>& b) { return {b.data(), b.size()}; }

bcn::RgbaImage image(std::uint32_t w, std::uint32_t h, std::uint8_t seed) {
    bcn::RgbaImage img;
    img.width = w;
    img.height = h;
    img.rgba8.resize(static_cast<std::size_t>(w) * h * 4);
    for (std::uint32_t y = 0; y < h; ++y) {
        for (std::uint32_t x = 0; x < w; ++x) {
            auto* p = &img.rgba8[(static_cast<std::size_t>(y) * w + x) * 4];
            p[0] = static_cast<std::uint8_t>(x * 4 + seed);
            p[1] = static_cast<std::uint8_t>(y * 4);
            p[2] = seed;
            p[3] = 255;
        }
    }
    return img;
}

// Full-chain DXT5 DDS of a w x h image.
std::vector<std::byte> dxt5_dds(std::uint32_t w, std::uint32_t h, std::uint8_t seed) {
    std::vector<bcn::RgbaImage> levels{image(w, h, seed)};
    while (levels.back().width > 1 || levels.back().height > 1) levels.push_back(bcn::downsample(levels.back()));
    auto r = bcn::encode_dds(bcn::Format::bc3, levels);
    assert(r.ok);
    return r.bytes;
}

// gfxTexture header the strict canonical reader accepts for a DXT5 DDS.
std::vector<std::byte> canonical_descriptor(std::uint32_t w, std::uint32_t h, std::uint32_t mips,
                                            std::uint32_t dds_size) {
    std::vector<std::byte> d(0x70);
    put32(d, 0x08, 0x20000U | (mips << 8) | 0x88U);
    put32(d, 0x0C, 0xAAE4U);
    put32(d, 0x10, (h << 16) | w);
    put32(d, 0x14, 1U);
    put32(d, 0x18, w * 4U);
    put32(d, 0x20, 0x40U);
    put32(d, 0x38, dds_size - 128U);
    put32(d, 0x44, (h << 16) | w);
    put32(d, 0x48, std::bit_cast<std::uint32_t>(1.0F / static_cast<float>(w)));
    put32(d, 0x4C, std::bit_cast<std::uint32_t>(1.0F / static_cast<float>(h)));
    put32(d, 0x60, 4U);
    put32(d, 0x64, dds_size);
    put32(d, 0x68, 8U);
    return d;
}

std::vector<std::byte> canonical_ptx() {
    const std::uint32_t dims[2][2] = {{64, 64}, {32, 64}};
    std::vector<std::vector<std::byte>> records;
    for (int k = 0; k < 2; ++k) {
        auto dds = dxt5_dds(dims[k][0], dims[k][1], static_cast<std::uint8_t>(40 * k + 10));
        const auto mips = bcn::parse(view(dds)).document.mip_count;
        auto rec = canonical_descriptor(dims[k][0], dims[k][1], mips, static_cast<std::uint32_t>(dds.size()));
        rec.insert(rec.end(), dds.begin(), dds.end());
        rec.resize((rec.size() + 0x7FF) / 0x800 * 0x800);
        records.push_back(rec);
    }
    std::vector<std::byte> ptx(0x800);
    put32(ptx, 0, 2);
    for (int k = 0; k < 2; ++k) {
        put32(ptx, 4 + 4 * k, static_cast<std::uint32_t>(records[k].size() / 0x800));
        ptx.insert(ptx.end(), records[k].begin(), records[k].end());
    }
    return ptx;
}

std::vector<std::byte> pac(const std::vector<std::vector<std::byte>>& slots) {
    std::vector<std::byte> b((8 + 4 * slots.size() + 15) / 16 * 16);
    std::memcpy(b.data(), "PAC\0", 4);
    put32(b, 4, static_cast<std::uint32_t>(slots.size()));
    for (std::size_t i = 0; i < slots.size(); ++i) {
        b.resize((b.size() + 15) / 16 * 16);
        put32(b, 8 + 4 * i, static_cast<std::uint32_t>(b.size()));
        b.insert(b.end(), slots[i].begin(), slots[i].end());
    }
    return b;
}

void ptx_in_pac_to_every_format() {
    const auto ptx = canonical_ptx();
    assert(dmc3::TextureSlotFramingParser::parse(view(ptx)).ok());
    assert(dmc3::is_texture_bundle(view(ptx)));
    std::vector<std::byte> other(100);
    for (std::size_t i = 0; i < other.size(); ++i) other[i] = static_cast<std::byte>(i);
    const auto source = pac({other, ptx, other});

    for (const auto format : bcn::writable_formats()) {
        const auto r = dmc3::reencode_textures(view(source), {.format = format});
        assert(r.ok && r.container == dmc3::ReencodeContainer::pac && r.textures.size() == 2U);
        assert(r.textures[0].pac_slot == 1 && r.textures[1].index == 1U);
        const auto slots = dmc3::read_pac_slots(view(r.bytes));
        assert(slots && slots->size() == 3U);
        // Untouched slots byte for byte (slot 2 moved but kept its bytes).
        for (const auto i : {0, 2}) {
            const auto& e = (*slots)[static_cast<std::size_t>(i)];
            assert(std::memcmp(r.bytes.data() + e.offset, other.data(), other.size()) == 0);
            assert(e.offset % 16U == 0U);
        }
        const auto& e = (*slots)[1];
        const auto out_ptx = view(r.bytes).subspan(e.offset, ptx.size() <= e.size ? e.size : e.size);
        assert(dmc3::is_texture_bundle(out_ptx.first(out_ptx.size() - out_ptx.size() % 0x800)));
        // Every texture is in the requested format, mips kept.
        const auto dds0 = out_ptx.subspan(0x800 + 0x70);
        const auto p = bcn::parse(dds0);
        assert(p.ok() && p.document.format == format && p.document.mip_count == 7U);
        // Re-encoding the original format again reproduces the strict
        // retail envelope; DXT1 too.
        if (format == bcn::Format::bc3 || format == bcn::Format::bc1) {
            assert(dmc3::TextureSlotFramingParser::parse(out_ptx.first(out_ptx.size() - out_ptx.size() % 0x800)).ok());
        }
    }
}

void single_slot_and_dx10_option() {
    const auto ptx = canonical_ptx();
    const auto source = pac({ptx, ptx});
    const auto r = dmc3::reencode_textures(view(source), {.format = bcn::Format::bc3, .force_dx10 = true, .pac_slot = 1});
    assert(r.ok && r.textures.size() == 2U && r.textures[0].pac_slot == 1 && r.textures[0].to_dx10);
    const auto slots = dmc3::read_pac_slots(view(r.bytes));
    // Slot 0 untouched.
    assert(std::memcmp(r.bytes.data() + (*slots)[0].offset, ptx.data(), ptx.size()) == 0);
    // A slot that holds no texture is an error when asked for explicitly.
    const auto none = dmc3::reencode_textures(view(pac({std::vector<std::byte>(64)})), {.pac_slot = 0});
    assert(!none.ok);
}

void wrapped_texture_and_bare_dds() {
    auto dds = dxt5_dds(32, 32, 7);
    const auto mips = bcn::parse(view(dds)).document.mip_count;
    auto wrapped = canonical_descriptor(32, 32, mips, static_cast<std::uint32_t>(dds.size()));
    wrapped.insert(wrapped.end(), dds.begin(), dds.end());
    const auto w = dmc3::reencode_textures(view(wrapped), {.format = bcn::Format::bc7});
    assert(w.ok && w.container == dmc3::ReencodeContainer::wrapped_texture);
    assert(get32(w.bytes, 0x64) + 0x70U == w.bytes.size());
    assert(bcn::parse(view(w.bytes).subspan(0x70)).document.format == bcn::Format::bc7);
    assert(get32(w.bytes, 0x38) == bcn::parse(view(w.bytes).subspan(0x70)).document.payload_size);

    const auto d = dmc3::reencode_textures(view(dds), {.format = bcn::Format::bc1});
    assert(d.ok && d.container == dmc3::ReencodeContainer::dds && d.textures[0].psnr_db > 30.0);
}

}  // namespace

int main() {
    ptx_in_pac_to_every_format();
    single_slot_and_dx10_option();
    wrapped_texture_and_bare_dds();
    return 0;
}
