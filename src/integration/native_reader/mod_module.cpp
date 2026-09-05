#include "dmc_rengine/integration/native_reader_modules.hpp"

#include "dmc_rengine/formats/mod.hpp"
#include "dmc_rengine/integration/native_reader_support.hpp"

#include <span>

namespace dmc::rengine::integration::native_reader_modules {
namespace {

void analyze_mod(
    ProjectWorkspace& project,
    const ResourceWorkspaceSession& session,
    ResourceAnalysisReport& report) {
    const auto bytes = std::span<const std::byte>{session.source_payload().bytes};
    const auto parsed = formats::mod::Parser::parse(bytes);
    report.recognized = parsed.recognized;
    native_reader_support::append_parser_diagnostics(
        project, report, session.resource().id, parsed.diagnostics);
}

} // namespace

NativeReaderModule mod() {
    return NativeReaderModule{
        .parser_id = "formats.mod-structural-v1",
        .format = "mod",
        .consumer = gdspaces::ToolTarget::modviz_scene,
        .link_format_evidence = true,
        .analyze = &analyze_mod,
    };
}

} // namespace dmc::rengine::integration::native_reader_modules
