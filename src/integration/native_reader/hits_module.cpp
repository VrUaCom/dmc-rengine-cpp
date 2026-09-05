#include "dmc_rengine/integration/native_reader_modules.hpp"

#include "dmc_rengine/formats/hits.hpp"
#include "dmc_rengine/formats/hits_binary.hpp"
#include "dmc_rengine/integration/native_reader_support.hpp"

#include <span>

namespace dmc::rengine::integration::native_reader_modules {
namespace {

void analyze_hits(
    ProjectWorkspace& project,
    const ResourceWorkspaceSession& session,
    ResourceAnalysisReport& report) {
    const auto bytes = std::span<const std::byte>{session.source_payload().bytes};
    const auto scan = formats::hits::RecordScanner::scan(bytes);
    report.recognized = scan.recognized;
    native_reader_support::append_parser_diagnostics(
        project, report, session.resource().id, scan.diagnostics);
    if (scan.recognized) {
        static_cast<void>(native_reader_support::attach_binary_document(
            project,
            report,
            session.resource().id,
            formats::hits::build_binary_document(session.resource(), bytes, scan),
            "HITS"));
    }
}

} // namespace

NativeReaderModule hits() {
    return NativeReaderModule{
        .parser_id = "formats.hits-record-scanner",
        .consumer = gdspaces::ToolTarget::binary_inspector,
        .link_format_evidence = true,
        .analyze = &analyze_hits,
    };
}

} // namespace dmc::rengine::integration::native_reader_modules
