#include "dmc_rengine/integration/native_reader_modules.hpp"

#include "dmc_rengine/formats/scm.hpp"
#include "dmc_rengine/formats/scm_binary.hpp"
#include "dmc_rengine/integration/native_reader_support.hpp"

#include <span>

namespace dmc::rengine::integration::native_reader_modules {
namespace {

void analyze_scm(
    ProjectWorkspace& project,
    const ResourceWorkspaceSession& session,
    ResourceAnalysisReport& report) {
    const auto bytes = std::span<const std::byte>{session.source_payload().bytes};
    const auto parsed = formats::scm::Parser::parse(bytes);
    report.recognized = parsed.recognized;
    native_reader_support::append_parser_diagnostics(
        project, report, session.resource().id, parsed.diagnostics);

    // Deep-reader promotion: a valid SCM now exposes the canonical byte map to
    // the shared binary inspector instead of stopping at format recognition.
    // Preservation-only fields remain explicitly annotated and are never
    // promoted to invented semantics.
    if (parsed.ok()) {
        static_cast<void>(native_reader_support::attach_binary_document(
            project,
            report,
            session.resource().id,
            formats::scm::build_binary_document(
                session.resource(), bytes, parsed),
            "SCM"));
    }
}

} // namespace

NativeReaderModule scm() {
    return NativeReaderModule{
        .parser_id = "formats.scm-structural-v1",
        .consumer = gdspaces::ToolTarget::modviz_scene,
        .link_format_evidence = true,
        .analyze = &analyze_scm,
    };
}

} // namespace dmc::rengine::integration::native_reader_modules
