// Spider Tarantula texture re-encode workflow on synthetic data (no game
// files): step order, parity with the module, slot selection, no-replace
// publication, failure reporting.
#include "dmc_rengine/codecs/dds_bcn.hpp"
#include "dmc_rengine/codecs/dds_bcn_encode.hpp"
#include "dmc_rengine/profiles/dmc3/texture_reencode.hpp"
#include "dmc_rengine/spider/texture_reencode_workflow.hpp"

#include <bit>
#include <cassert>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <span>
#include <string>
#include <vector>

namespace {

namespace bcn = dmc::rengine::codecs::dds_bcn;
namespace dmc3 = dmc::rengine::profiles::dmc3;
namespace tarantula = dmc::rengine::spider::tarantula;

void put32(std::vector<std::byte>& b, std::size_t o, std::uint32_t v) {
    for (std::size_t i = 0; i < 4; ++i) b[o + i] = static_cast<std::byte>((v >> (8 * i)) & 0xFF);
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

void steps_and_parity() {
    const auto ptx = canonical_ptx();
    std::vector<std::byte> other(100, std::byte{0x5A});
    const auto source = pac({other, ptx, other, ptx});
    tarantula::TextureReencodeRequest request{};
    request.options.format = bcn::Format::bc7;
    const auto run = tarantula::run_texture_reencode(view(source), request);
    assert(run.ok && run.report.ok());
    const char* expected[] = {"acquire", "inspect", "transform[1]", "transform[3]", "assemble", "validate", "publish"};
    assert(run.steps.size() == std::size(expected));
    for (std::size_t i = 0; i < run.steps.size(); ++i) assert(run.steps[i].name == expected[i] && run.steps[i].ok);
    // Same bytes as the module run directly.
    const auto direct = dmc3::reencode_textures(view(source), request.options);
    assert(direct.ok && direct.bytes == run.result.bytes && run.result.textures.size() == 4U);
    assert(tarantula::family == dmc::rengine::spider::Family::tarantula);

    // One slot only.
    request.options.pac_slot = 3;
    const auto one = tarantula::run_texture_reencode(view(source), request);
    assert(one.ok && one.result.textures.size() == 2U && one.result.textures[0].pac_slot == 3);
    const auto slots = dmc3::read_pac_slots(view(one.result.bytes));
    assert(std::memcmp(one.result.bytes.data() + (*slots)[1].offset, ptx.data(), ptx.size()) == 0);
}

void single_payload() {
    auto dds = dxt5_dds(32, 32, 3);
    tarantula::TextureReencodeRequest request{};
    request.options.format = bcn::Format::bc1;
    const auto run = tarantula::run_texture_reencode(view(dds), request);
    assert(run.ok && run.result.container == dmc3::ReencodeContainer::dds);
    assert(run.steps.size() == 6U && run.steps[2].name == "transform");
    assert(bcn::parse(view(run.result.bytes)).document.format == bcn::Format::bc1);
}

void failures_and_publication() {
    tarantula::TextureReencodeRequest request{};
    // No texture: stops at inspect, says so, returns no bytes.
    std::vector<std::byte> junk(64, std::byte{1});
    const auto none = tarantula::run_texture_reencode(view(junk), request);
    assert(!none.ok && none.steps.back().name == "inspect" && none.result.bytes.empty());

    const auto dir = std::filesystem::temp_directory_path() / "dmc_rengine_tarantula_texture_test";
    std::filesystem::remove_all(dir);
    std::filesystem::create_directories(dir);
    const auto in_path = dir / "in.pac";
    const auto ptx = canonical_ptx();
    const auto source = pac({ptx});
    std::ofstream(in_path, std::ios::binary).write(reinterpret_cast<const char*>(source.data()),
                                                   static_cast<std::streamsize>(source.size()));
    request.output = dir / "out.pac";
    const auto first = tarantula::run_texture_reencode(in_path, request);
    assert(first.ok && std::filesystem::file_size(request.output) == first.result.bytes.size());
    // No-replace: a second run refuses to overwrite.
    const auto second = tarantula::run_texture_reencode(in_path, request);
    assert(!second.ok && second.steps.back().name == "publish");
    request.replace_existing = true;
    request.options.format = bcn::Format::bc1;
    const auto third = tarantula::run_texture_reencode(in_path, request);
    assert(third.ok && std::filesystem::file_size(request.output) == third.result.bytes.size());
    // Missing input fails at acquire.
    const auto missing = tarantula::run_texture_reencode(dir / "missing.pac", request);
    assert(!missing.ok && missing.steps.front().name == "acquire");
    std::filesystem::remove_all(dir);
}

}  // namespace

int main() {
    steps_and_parity();
    single_payload();
    failures_and_publication();
    return 0;
}
