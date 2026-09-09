#include "dmc_rengine/integration/native_reader_modules.hpp"

#include "dmc_rengine/formats/diagnostic.hpp"
#include "dmc_rengine/formats/mot.hpp"
#include "dmc_rengine/integration/native_reader_support.hpp"

#include <array>
#include <span>
#include <string>

namespace dmc::rengine::integration::native_reader_modules {
namespace {

// A motion is refused whole or read whole: the track chain either closes on
// the payload's own alignment or it does not, so there is one diagnostic to
// carry and it is the refusal's reason.
void analyze_mot(
    ProjectWorkspace& project,
    const ResourceWorkspaceSession& session,
    ResourceAnalysisReport& report) {
    const auto bytes = std::span<const std::byte>{session.source_payload().bytes};
    const auto parsed = formats::MotParser::parse(bytes);
    report.recognized = parsed.ok();

    if (parsed.ok()) {
        return;
    }

    const std::array<formats::ParseDiagnostic, 1U> diagnostics{
        formats::ParseDiagnostic{
            .severity = formats::ParseSeverity::error,
            .code = "mot.structure-refused",
            .message = parsed.message.empty()
                ? std::string{"the payload is not a structurally valid motion"}
                : parsed.message,
            .offset = 0U,
        },
    };
    native_reader_support::append_parser_diagnostics(
        project, report, session.resource().id, diagnostics);
}

} // namespace

NativeReaderModule mot() {
    return NativeReaderModule{
        .parser_id = "formats.mot-structural-v1",
        .consumer = gdspaces::ToolTarget::modviz_scene,
        .link_format_evidence = true,
        .analyze = &analyze_mot,
    };
}

} // namespace dmc::rengine::integration::native_reader_modules
