#include "dmc_rengine/integration/resource_analyzer.hpp"

#include "dmc_rengine/integration/native_reader_registry.hpp"
#include "dmc_rengine/integration/native_reader_support.hpp"

#include <algorithm>

namespace dmc::rengine::integration {

bool ResourceAnalysisReport::ok() const noexcept {
    return parser_available && recognized &&
        std::none_of(
            diagnostics.begin(), diagnostics.end(),
            [](const gdspaces::Diagnostic& diagnostic) {
                return diagnostic.severity == gdspaces::DiagnosticSeverity::error;
            });
}

ResourceAnalysisReport ResourceAnalyzer::analyze(
    ProjectWorkspace& project,
    const gdspaces::ResourceId& resource) {
    ResourceAnalysisReport report{
        .resource = resource,
        .format = {},
        .parser_id = {},
        .parser_available = false,
        .recognized = false,
        .binary_document_attached = false,
        .diagnostics = {},
    };

    const auto* session = project.find_session(resource);
    if (session == nullptr) {
        native_reader_support::add_report_diagnostic(
            report,
            gdspaces::DiagnosticSeverity::error,
            "analysis.session-missing",
            "The canonical resource workspace session was not found.");
        return report;
    }

    report.format = session->resource().format;
    const auto* descriptor = session->format();
    if (descriptor == nullptr || descriptor->parser_id.empty()) {
        native_reader_support::add_report_diagnostic(
            report,
            gdspaces::DiagnosticSeverity::warning,
            "analysis.parser-unavailable",
            "No implemented parser is registered for this resource format.");
        return report;
    }
    report.parser_id = descriptor->parser_id;

    static const NativeReaderModuleRegistry modules;
    const auto* module = modules.find(descriptor->parser_id);
    if (module == nullptr) {
        native_reader_support::add_report_diagnostic(
            report,
            gdspaces::DiagnosticSeverity::warning,
            "analysis.module-unavailable",
            "The format declares a parser ID, but no Native Reader module is registered for it.");
        return report;
    }

    report.parser_available = true;
    module->analyze(project, *session, report);

    static_cast<void>(project.record_parser_completed(
        resource,
        descriptor->parser_id,
        report.recognized,
        module->consumer));
    if (module->link_format_evidence) {
        static_cast<void>(project.link_format_evidence(resource));
    }
    return report;
}

} // namespace dmc::rengine::integration
