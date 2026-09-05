#include "dmc_rengine/integration/native_reader_modules.hpp"

#include "dmc_rengine/formats/ptx.hpp"
#include "dmc_rengine/formats/ptx_binary.hpp"
#include "dmc_rengine/integration/native_reader_support.hpp"

#include <span>

namespace dmc::rengine::integration::native_reader_modules {
namespace {

void analyze_ptx(
    ProjectWorkspace& project,
    const ResourceWorkspaceSession& session,
    ResourceAnalysisReport& report) {
    const auto bytes = std::span<const std::byte>{session.source_payload().bytes};
    const auto scan = formats::ptx::Reader::scan(bytes);
    report.recognized = scan.recognized;
    native_reader_support::append_parser_diagnostics(
        project, report, session.resource().id, scan.diagnostics);
    if (scan.ok()) {
        static_cast<void>(native_reader_support::attach_binary_document(
            project,
            report,
            session.resource().id,
            formats::ptx::build_binary_document(session.resource(), bytes, scan),
            "PTX"));
    }
}

} // namespace

NativeReaderModule ptx() {
    return NativeReaderModule{
        .parser_id = "formats.ptx-dmc3-reader",
        .consumer = gdspaces::ToolTarget::binary_inspector,
        .link_format_evidence = true,
        .analyze = &analyze_ptx,
    };
}

} // namespace dmc::rengine::integration::native_reader_modules
