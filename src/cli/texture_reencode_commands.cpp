#include "texture_reencode_commands.hpp"

#include <cstdio>
#include <fstream>
#include <iostream>
#include <iterator>
#include <string>
#include <string_view>
#include <vector>

#include "dmc_rengine/codecs/dds_bcn_encode.hpp"
#include "dmc_rengine/profiles/dmc3/texture_reencode.hpp"

namespace dmc::rengine::cli {
namespace {

namespace bcn = codecs::dds_bcn;
namespace dmc3 = profiles::dmc3;

[[nodiscard]] const char* container_name(dmc3::ReencodeContainer c) noexcept {
    switch (c) {
    case dmc3::ReencodeContainer::dds: return "DDS";
    case dmc3::ReencodeContainer::wrapped_texture: return "gfxTexture";
    case dmc3::ReencodeContainer::ptx: return "PTX";
    case dmc3::ReencodeContainer::pac: return "PAC";
    }
    return "?";
}

}  // namespace

void print_texture_reencode_help() {
    std::cout
        << "  texture-reencode <in> <out> --format <bc1|bc2|bc3|bc4|bc4s|bc5|bc5s|bc6h|bc6h_sf16|bc7>\n"
        << "                   [--dx10] [--slot N]\n"
        << "                             Re-encode every texture of a DDS / PTX / .tm2 / PAC\n"
        << "                             (game mips kept, layout kept when it fits)\n";
}

int try_run_texture_reencode_command(int argc, char** argv) {
    if (argc <= 1 || std::string_view{argv[1]} != "texture-reencode") return -1;
    if (argc < 4) {
        std::cerr << "texture-reencode: usage: texture-reencode <in> <out> --format <name> [--dx10] [--slot N]\n";
        return 1;
    }
    dmc3::TextureReencodeOptions options{};
    bool have_format = false;
    for (int i = 4; i < argc; ++i) {
        const std::string_view arg{argv[i]};
        if (arg == "--format" && i + 1 < argc) {
            const auto f = bcn::parse_format_name(argv[++i]);
            if (!f) {
                std::cerr << "texture-reencode: unknown format " << argv[i] << "\n";
                return 1;
            }
            options.format = *f;
            have_format = true;
        } else if (arg == "--dx10") {
            options.force_dx10 = true;
        } else if (arg == "--slot" && i + 1 < argc) {
            options.pac_slot = std::stoi(argv[++i]);
        } else {
            std::cerr << "texture-reencode: unknown argument " << arg << "\n";
            return 1;
        }
    }
    if (!have_format) {
        std::cerr << "texture-reencode: --format is required\n";
        return 1;
    }
    std::ifstream in(argv[2], std::ios::binary);
    if (!in) {
        std::cerr << "texture-reencode: cannot read " << argv[2] << "\n";
        return 1;
    }
    const std::vector<char> raw((std::istreambuf_iterator<char>(in)), {});
    const auto result = dmc3::reencode_textures(
        std::span<const std::byte>{reinterpret_cast<const std::byte*>(raw.data()), raw.size()}, options);
    if (!result.ok) {
        std::cerr << "texture-reencode: " << result.detail << "\n";
        return 1;
    }
    std::ofstream out(argv[3], std::ios::binary);
    out.write(reinterpret_cast<const char*>(result.bytes.data()), static_cast<std::streamsize>(result.bytes.size()));
    if (!out) {
        std::cerr << "texture-reencode: cannot write " << argv[3] << "\n";
        return 1;
    }
    std::cout << container_name(result.container) << ": " << result.detail << "\n";
    for (const auto& t : result.textures) {
        char line[256];
        std::snprintf(line, sizeof line, "  %s%u  %ux%u mips=%u  %s%s -> %s%s  %llu -> %llu bytes  PSNR %.1f dB\n",
                      t.pac_slot >= 0 ? ("slot " + std::to_string(t.pac_slot) + " tex ").c_str() : "tex ",
                      t.index, t.width, t.height, t.mips, std::string(bcn::format_name(t.from)).c_str(),
                      t.from_dx10 ? " DX10" : "", std::string(bcn::format_name(t.to)).c_str(),
                      t.to_dx10 ? " DX10" : "", static_cast<unsigned long long>(t.old_bytes),
                      static_cast<unsigned long long>(t.new_bytes), t.psnr_db);
        std::cout << line;
    }
    return 0;
}

}  // namespace dmc::rengine::cli
