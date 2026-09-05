#include "dmc_rengine/integration/native_reader_modules.hpp"

#include "dmc_rengine/formats/stage_txt.hpp"
#include "dmc_rengine/formats/stage_txt_binary.hpp"
#include "dmc_rengine/integration/native_reader_support.hpp"

#include <span>

namespace dmc::rengine::integration::native_reader_modules {
namespace {

void analyze_stage_txt(
    ProjectWorkspace& project,
    const ResourceWorkspaceSession& session,
    ResourceAnalysisReport& report) {
    const auto bytes = std::span<const std::byte>{session.source_payload().bytes};
    const auto lex = formats::stage_txt::Lexer::scan(bytes);
    report.recognized = lex.recognized;
    native_reader_support::append_parser_diagnostics(
        project, report, session.resource().id, lex.diagnostics);
    if (lex.recognized) {
        static_cast<void>(native_reader_support::attach_binary_document(
            project,
            report,
            session.resource().id,
            formats::stage_txt::build_binary_document(session.resource(), bytes, lex),
            "Stage TXT"));
    }
}

} // namespace

NativeReaderModule stage_txt() {
    return NativeReaderModule{
        .parser_id = "formats.stage-txt-lexer",
        .consumer = gdspaces::ToolTarget::binary_inspector,
        .link_format_evidence = true,
        .analyze = &analyze_stage_txt,
    };
}

} // namespace dmc::rengine::integration::native_reader_modules
