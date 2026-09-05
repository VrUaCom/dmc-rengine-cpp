#include "dmc_rengine/integration/native_reader_modules.hpp"

#include "dmc_rengine/formats/dca.hpp"
#include "dmc_rengine/formats/dca_binary.hpp"
#include "dmc_rengine/integration/native_reader_support.hpp"

#include <span>

namespace dmc::rengine::integration::native_reader_modules {
namespace {

void analyze_dca(
    ProjectWorkspace& project,
    const ResourceWorkspaceSession& session,
    ResourceAnalysisReport& report) {
    const auto bytes = std::span<const std::byte>{session.source_payload().bytes};
    const auto scan = formats::dca::RecordScanner::scan(bytes);
    report.recognized = scan.recognized;
    native_reader_support::append_parser_diagnostics(
        project, report, session.resource().id, scan.diagnostics);
    if (scan.recognized) {
        static_cast<void>(native_reader_support::attach_binary_document(
            project,
            report,
            session.resource().id,
            formats::dca::build_binary_document(session.resource(), bytes, scan),
            "DCA"));
    }
}

} // namespace

NativeReaderModule dca() {
    return NativeReaderModule{
        .parser_id = "formats.dca-record-scanner",
        .consumer = gdspaces::ToolTarget::binary_inspector,
        .link_format_evidence = true,
        .analyze = &analyze_dca,
    };
}

} // namespace dmc::rengine::integration::native_reader_modules
