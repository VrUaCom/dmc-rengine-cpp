#include "dmc_rengine/integration/native_reader_modules.hpp"

#include "dmc_rengine/core/sha256.hpp"
#include "dmc_rengine/exe/pe_reader.hpp"
#include "dmc_rengine/integration/native_reader_support.hpp"
#include "dmc_rengine/profiles/dmc3/known_targets.hpp"

#include <span>
#include <string>
#include <utility>
#include <vector>

namespace dmc::rengine::integration::native_reader_modules {
namespace {

[[nodiscard]] std::vector<formats::ParseDiagnostic> pe_diagnostics(
    const exe::PeReadResult& result) {
    std::vector<formats::ParseDiagnostic> diagnostics;
    diagnostics.reserve(result.warnings.size() + result.errors.size());
    for (const auto& warning : result.warnings) {
        diagnostics.push_back(formats::ParseDiagnostic{
            .severity = formats::ParseSeverity::warning,
            .code = "pe.warning",
            .message = warning,
            .offset = 0U,
        });
    }
    for (const auto& error : result.errors) {
        diagnostics.push_back(formats::ParseDiagnostic{
            .severity = formats::ParseSeverity::error,
            .code = "pe.error",
            .message = error,
            .offset = 0U,
        });
    }
    return diagnostics;
}

void analyze_pe(
    ProjectWorkspace& project,
    const ResourceWorkspaceSession& session,
    ResourceAnalysisReport& report) {
    const auto bytes = std::span<const std::byte>{session.source_payload().bytes};
    const auto parsed = exe::PeReader::read(bytes);
    report.recognized = parsed.image.has_value();
    const auto diagnostics = pe_diagnostics(parsed);
    native_reader_support::append_parser_diagnostics(
        project, report, session.resource().id, diagnostics);
    if (!parsed.ok()) {
        return;
    }

    const auto digest = core::Sha256::compute(bytes).hex();
    const auto& target = profiles::dmc3::phase12_canonical_target();
    const auto hash_match = target.matches_hash(digest);
    const auto metadata_match = hash_match && target.matches_metadata(*parsed.image);
    ExecutableResourceContext context{
        .image = *parsed.image,
        .sha256 = digest,
        .known_target_id = hash_match ? target.id : std::string{},
        .known_target_name = hash_match ? target.display_name : std::string{},
        .known_target_hash_match = hash_match,
        .known_target_metadata_match = metadata_match,
    };
    if (!project.attach_executable_context(session.resource().id, std::move(context))) {
        native_reader_support::add_report_diagnostic(
            report,
            gdspaces::DiagnosticSeverity::error,
            "analysis.executable-context-failed",
            "The PE parser succeeded, but EXE Editor context could not be attached.");
        return;
    }
    static_cast<void>(project.link_artifact_evidence(session.resource().id, digest));
}

} // namespace

NativeReaderModule pe() {
    return NativeReaderModule{
        .parser_id = "exe.pe-reader",
        .consumer = gdspaces::ToolTarget::exe_editor,
        .link_format_evidence = false,
        .analyze = &analyze_pe,
    };
}

} // namespace dmc::rengine::integration::native_reader_modules
