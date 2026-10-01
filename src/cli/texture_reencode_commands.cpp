#include "texture_reencode_commands.hpp"

#include <cstdio>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <iterator>
#include <string>
#include <string_view>
#include <vector>

#include "dmc_rengine/codecs/dds_bcn_encode.hpp"
#include "dmc_rengine/profiles/dmc3/texture_reencode.hpp"
#include "dmc_rengine/spider/texture_reencode_workflow.hpp"

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
        << "                   [--dx10] [--slot N] [--replace]\n"
        << "                             Spider Tarantula workflow: re-encode every texture of a\n"
        << "                             DDS / PTX / .tm2 / PAC (game mips kept, layout kept when\n"
        << "                             it fits); <out> is never overwritten without --replace\n";
}

int try_run_texture_reencode_command(int argc, char** argv) {
    if (argc <= 1 || std::string_view{argv[1]} != "texture-reencode") return -1;
    if (argc < 4) {
        std::cerr << "texture-reencode: usage: texture-reencode <in> <out> --format <name> [--dx10] [--slot N]\n";
        return 1;
    }
    dmc3::TextureReencodeOptions options{};
    bool have_format = false;
    bool replace = false;
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
        } else if (arg == "--replace") {
            replace = true;
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
    const spider::tarantula::TextureReencodeRequest request{
        .options = options,
        .output = std::filesystem::path{argv[3]},
        .replace_existing = replace,
    };
    const auto workflow = spider::tarantula::run_texture_reencode(std::filesystem::path{argv[2]}, request);
    for (const auto& step : workflow.steps) {
        std::cout << (step.ok ? "[OK]   " : "[FAIL] ") << step.name;
        if (!step.detail.empty()) std::cout << "  " << step.detail;
        std::cout << "\n";
    }
    if (!workflow.ok) {
        std::cerr << "texture-reencode: " << workflow.detail << "\n";
        return 1;
    }
    const auto& result = workflow.result;
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
