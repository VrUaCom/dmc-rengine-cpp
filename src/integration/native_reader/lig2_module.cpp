#include "dmc_rengine/integration/native_reader_modules.hpp"

#include "dmc_rengine/formats/lig2.hpp"
#include "dmc_rengine/formats/lig2_binary.hpp"
#include "dmc_rengine/integration/native_reader_support.hpp"

#include <span>

namespace dmc::rengine::integration::native_reader_modules {
namespace {

void analyze_lig2(
    ProjectWorkspace& project,
    const ResourceWorkspaceSession& session,
    ResourceAnalysisReport& report) {
    const auto bytes = std::span<const std::byte>{session.source_payload().bytes};
    const auto scan = formats::lig2::RecordScanner::scan(bytes);
    report.recognized = scan.recognized;
    native_reader_support::append_parser_diagnostics(
        project, report, session.resource().id, scan.diagnostics);
    if (scan.recognized) {
        static_cast<void>(native_reader_support::attach_binary_document(
            project,
            report,
            session.resource().id,
            formats::lig2::build_binary_document(session.resource(), bytes, scan),
            "LIG2"));
    }
}

} // namespace

NativeReaderModule lig2() {
    return NativeReaderModule{
        .parser_id = "formats.lig2-record-scanner",
        .consumer = gdspaces::ToolTarget::binary_inspector,
        .link_format_evidence = true,
        .analyze = &analyze_lig2,
    };
}

} // namespace dmc::rengine::integration::native_reader_modules
