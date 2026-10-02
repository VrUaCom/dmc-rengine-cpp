// Native Reader modules for the formats ported from the DMC Native Reader
// reverses: EFM (MOD layout behind an EFM tag), CLT, TSC, EVT and BCn / DX10
// DDS outside the retail DXT1/DXT5 profile. Each goes classifier -> format
// registry row -> module -> analysis report, as the GDS browser runs it.
#include "dmc_rengine/codecs/dds_bcn_encode.hpp"
#include "dmc_rengine/gdspaces/classifier.hpp"
#include "dmc_rengine/integration/format_registry.hpp"
#include "dmc_rengine/integration/project_workspace.hpp"
#include "dmc_rengine/integration/resource_analyzer.hpp"

#include <bit>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <span>
#include <string>
#include <string_view>
#include <vector>

namespace {

namespace gd = dmc::rengine::gdspaces;
namespace integration = dmc::rengine::integration;

void put_u8(std::vector<std::byte>& b, std::size_t o, std::uint8_t v) { b[o] = static_cast<std::byte>(v); }
void put_u16(std::vector<std::byte>& b, std::size_t o, std::uint16_t v) {
    put_u8(b, o, static_cast<std::uint8_t>(v & 0xFFU));
    put_u8(b, o + 1U, static_cast<std::uint8_t>(v >> 8U));
}
void put_u32(std::vector<std::byte>& b, std::size_t o, std::uint32_t v) {
    for (std::size_t i = 0U; i < 4U; ++i) put_u8(b, o + i, static_cast<std::uint8_t>((v >> (i * 8U)) & 0xFFU));
}
void put_u64(std::vector<std::byte>& b, std::size_t o, std::uint64_t v) {
    for (std::size_t i = 0U; i < 8U; ++i) put_u8(b, o + i, static_cast<std::uint8_t>((v >> (i * 8U)) & 0xFFU));
}
void put_f32(std::vector<std::byte>& b, std::size_t o, float v) { put_u32(b, o, std::bit_cast<std::uint32_t>(v)); }

// The one-mesh MOD document of mod_tests.cpp, tagged EFM.
std::vector<std::byte> efm_fixture() {
    std::vector<std::byte> b(0x260U, std::byte{0});
    std::memcpy(b.data(), "EFM ", 4U);
    put_f32(b, 0x04U, 1.01F);
    put_u8(b, 0x10U, 1U);
    put_u8(b, 0x11U, 1U);
    put_u8(b, 0x12U, 8U);
    put_u8(b, 0x13U, 0x5AU);
    put_u32(b, 0x14U, 0x12345678U);
    put_u64(b, 0x20U, 0x200U);
    put_u8(b, 0x40U, 1U);
    put_u8(b, 0x41U, 0x90U);
    put_u16(b, 0x42U, 1U);
    put_u64(b, 0x48U, 0x80U);
    put_u32(b, 0x50U, 0x00004000U);
    put_f32(b, 0x70U, 10.0F);
    put_f32(b, 0x74U, -20.0F);
    put_f32(b, 0x78U, 30.0F);
    put_f32(b, 0x7CU, 42.5F);
    put_u16(b, 0x80U, 1U);
    put_u16(b, 0x82U, 7U);
    put_u16(b, 0x84U, 1U);
    put_u16(b, 0x86U, 2U);
    put_u16(b, 0x88U, 3U);
    put_u16(b, 0x8AU, 4U);
    put_u64(b, 0x90U, 0xD0U);
    put_u64(b, 0x98U, 0xE0U);
    put_u64(b, 0xA0U, 0xF0U);
    put_u64(b, 0xA8U, 0x100U);
    put_u64(b, 0xB0U, 0x110U);
    put_u64(b, 0xC0U, 0xA0U);
    put_f32(b, 0xD0U, 1.0F);
    put_f32(b, 0xD4U, 2.0F);
    put_f32(b, 0xD8U, 3.0F);
    put_f32(b, 0xE4U, 1.0F);
    put_u16(b, 0xF0U, 4096U);
    put_u16(b, 0xF2U, 2048U);
    put_u16(b, 0x110U, 0x001FU);
    put_u32(b, 0x200U, 0x20U);
    put_u32(b, 0x204U, 0x24U);
    put_u32(b, 0x208U, 0x28U);
    put_u32(b, 0x20CU, 0x30U);
    put_u8(b, 0x220U, 0xFFU);
    return b;
}

std::vector<std::byte> evt_fixture() {
    std::vector<std::byte> out(0x60U, std::byte{0});
    std::memcpy(out.data(), "EVT\0", 4U);
    put_u32(out, 0x04U, 0x00010001U);
    std::size_t cursor = 0x20U;
    put_u32(out, cursor, 0x00000102U);
    put_u32(out, cursor + 4U, 0x37U);
    cursor += 8U;
    put_u32(out, cursor, 0x00000203U);
    put_u32(out, cursor + 4U, 0x11U);
    put_u32(out, cursor + 8U, 0x22U);
    cursor += 12U;
    put_u32(out, cursor, 0x00000001U);
    cursor += 4U;
    put_u32(out, cursor, 0x00000020U);
    put_u32(out, 0x08U, static_cast<std::uint32_t>(cursor));
    return out;
}

std::vector<std::byte> bytes_of(std::string_view text) {
    std::vector<std::byte> out(text.size());
    std::memcpy(out.data(), text.data(), text.size());
    return out;
}

// Classifies like the browser, opens a session and analyzes it.
integration::ResourceAnalysisReport analyze(std::string_view name, const std::vector<std::byte>& bytes,
                                            std::string_view expect_format) {
    const auto classified = gd::ResourceClassifier::classify(name, bytes);
    assert(classified.format == expect_format);
    gd::ResourcePayload payload{
        .resource = gd::ResourceRef{
            .id = gd::ResourceId{
                .source_id = "reader-ports-test",
                .logical_path = std::string{name},
                .container_chain = "PAC[0]",
                .offset = 0U,
                .size = bytes.size(),
            },
            .display_name = std::string{name},
            .format = classified.format,
            .profile = "dmc3-hd",
            .synthetic_name = false,
            .container = false,
        },
        .bytes = bytes,
        .diagnostics = {},
        .byte_provenance = {},
        .name_evidence = {},
        .enclosing_container_name_evidence = {},
        .semantic_evidence = {},
    };
    integration::ProjectWorkspace project;
    assert(project.create_session(payload));
    auto report = integration::ResourceAnalyzer::analyze(project, payload.resource.id);
    if (!report.ok()) {
        std::fprintf(stderr, "%s: parser %s available %d recognized %d\n", std::string{name}.c_str(),
                     report.parser_id.c_str(), report.parser_available, report.recognized);
        for (const auto& d : report.diagnostics) std::fprintf(stderr, "  %s: %s\n", d.code.c_str(), d.message.c_str());
    }
    return report;
}

bool has_info(const integration::ResourceAnalysisReport& report, std::string_view code, std::string_view text) {
    for (const auto& d : report.diagnostics) {
        if (d.code == code && d.message.find(text) != std::string::npos) return true;
    }
    return false;
}

} // namespace

int main() {
    integration::FormatIntegrationRegistry registry;
    for (const auto* format : {"efm", "clt", "tsc", "evt"}) {
        const auto* row = registry.find(format);
        assert(row != nullptr && row->valid());
        assert(row->maturity == integration::IntegrationMaturity::structural);
        assert(!row->parser_id.empty());
    }

    {
        const auto report = analyze("em000_017.efm", efm_fixture(), "efm");
        assert(report.ok());
        assert(report.parser_id == "formats.efm-mod-layout-v1");
        assert(has_info(report, "efm-models", "1 model(s)"));
    }
    {
        const std::string_view clt =
            ";test.clt\n\nClothNum\t1\n\nClothNo     0\nClothId     0\n"
            "Gravity     0.500000  0.000000  0.000000\nSpringForce 0.020000\n"
            "MaxSpeed    50.000000\nStiffness   0.000000\n"
            "Wind        0.000000  0.000000  0.000000\nWindLocal   1\nWindParent  0\n"
            "WindType    1\nBone      1    Y\nBone      2    NZ\nEnd\n$\n";
        const auto report = analyze("slot_0005.bin", bytes_of(clt), "clt");
        assert(report.ok());
        assert(report.parser_id == "profiles.dmc3.clt-cloth-chain-v1");
        assert(has_info(report, "clt-chains", "1 cloth chain(s), 2 bone(s)"));
    }
    {
        const std::string_view tsc =
            "\r\n.TSC\t\r\n\t# RELATIVE\t\r\n\t\t<Start\r\n\t\t\tScrlNo\t\t0\r\n"
            "\t\t\tScrlType\t1\r\n\t\t\tTexNo\t\t3\r\n\t\t\tDirUV\t\tstay,   up\r\n"
            "\t\t\tTimeUV\t\t0, \t90\r\n\t\tEnd>\r\n\t\t<Start\r\n\t\t\tScrlNo\t\t1\r\n"
            "\t\t\tScrlType\t3\r\n\t\t\tTexNo\t\t2\r\n\t\t\tDirUV\t\tleft, stay\r\n"
            "\t\t\tTimeUV\t\t400,      0\r\n\t\tEnd>\r\n\t\t<Finish>\r\n$\t\r\n";
        const auto report = analyze("slot_0013.bin", bytes_of(tsc), "tsc");
        assert(report.ok());
        assert(report.parser_id == "profiles.dmc3.tsc-uv-scroll-v1");
        assert(has_info(report, "tsc-records", "2 scroll record(s)"));
        assert(has_info(report, "tsc-records", "ScrlType 1, 3"));
    }
    {
        const auto report = analyze("EventTbl20.bin", evt_fixture(), "evt");
        assert(report.ok());
        assert(report.parser_id == "formats.evt-structural-v1");
        assert(has_info(report, "evt-commands", "4 command(s), 1 stream(s)"));
    }
    {
        // A BC7 DDS with a DX10 header: outside the retail profile, still read.
        namespace bcn = dmc::rengine::codecs::dds_bcn;
        bcn::RgbaImage level;
        level.width = 16U;
        level.height = 16U;
        level.rgba8.resize(16U * 16U * 4U);
        for (std::size_t i = 0U; i < level.rgba8.size(); ++i) level.rgba8[i] = static_cast<std::uint8_t>(i * 7U);
        const auto encoded = bcn::encode_dds(bcn::Format::bc7, std::span<const bcn::RgbaImage>{&level, 1U});
        assert(encoded.ok);
        const auto report = analyze("tex.dds", encoded.bytes, "dds");
        assert(report.ok());
        assert(has_info(report, "dds-bcn", "BC7 (DX10 header), 16x16"));
    }
    return 0;
}
