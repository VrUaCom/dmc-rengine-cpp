#include "dmc_rengine/integration/native_reader_modules.hpp"

#include "dmc_rengine/codecs/dds_bcn.hpp"
#include "dmc_rengine/formats/dds.hpp"
#include "dmc_rengine/formats/dds_binary.hpp"
#include "dmc_rengine/integration/native_reader_support.hpp"

#include <span>
#include <string>

namespace dmc::rengine::integration::native_reader_modules {
namespace {

void analyze_dds(
    ProjectWorkspace& project,
    const ResourceWorkspaceSession& session,
    ResourceAnalysisReport& report) {
    const auto bytes = std::span<const std::byte>{session.source_payload().bytes};
    const auto scan = formats::dds::Reader::scan(bytes);
    if (!scan.ok()) {
        // Outside the retail DXT1/DXT5 profile: BC1..BC7 and DX10 headers
        // (which dmc3.exe's DirectXTK loader 0x1400499C0 accepts) are read
        // by the BCn codec instead of being reported as broken.
        const auto bcn = codecs::dds_bcn::parse(bytes);
        if (bcn.ok()) {
            report.recognized = true;
            const auto& d = bcn.document;
            native_reader_support::add_report_diagnostic(
                report, gdspaces::DiagnosticSeverity::info, "dds-bcn",
                std::string{codecs::dds_bcn::format_name(d.format)} + (d.dx10_header ? " (DX10 header)" : "") +
                    (d.srgb ? " sRGB" : "") + ", " + std::to_string(d.width) + "x" + std::to_string(d.height) +
                    ", " + std::to_string(d.mip_count) +
                    " mip(s); outside the retail DXT1/DXT5 profile, read by codecs::dds_bcn.");
            return;
        }
    }
    report.recognized = scan.recognized;
    native_reader_support::append_parser_diagnostics(
        project, report, session.resource().id, scan.diagnostics);
    if (scan.ok()) {
        static_cast<void>(native_reader_support::attach_binary_document(
            project,
            report,
            session.resource().id,
            formats::dds::build_binary_document(session.resource(), bytes, scan),
            "DDS"));
    }
}

} // namespace

NativeReaderModule dds() {
    return NativeReaderModule{
        .parser_id = "formats.dds-dmc3-reader",
        .consumer = gdspaces::ToolTarget::binary_inspector,
        .link_format_evidence = true,
        .analyze = &analyze_dds,
    };
}

} // namespace dmc::rengine::integration::native_reader_modules
