#include "dmc_rengine/integration/native_reader_support.hpp"

#include <utility>

namespace dmc::rengine::integration::native_reader_support {
namespace {

[[nodiscard]] gdspaces::DiagnosticSeverity map_severity(
    formats::ParseSeverity severity) noexcept {
    switch (severity) {
    case formats::ParseSeverity::info: return gdspaces::DiagnosticSeverity::info;
    case formats::ParseSeverity::warning: return gdspaces::DiagnosticSeverity::warning;
    case formats::ParseSeverity::error: return gdspaces::DiagnosticSeverity::error;
    }
    return gdspaces::DiagnosticSeverity::error;
}

} // namespace

void add_report_diagnostic(
    ResourceAnalysisReport& report,
    gdspaces::DiagnosticSeverity severity,
    std::string code,
    std::string message) {
    report.diagnostics.push_back(gdspaces::Diagnostic{
        .severity = severity,
        .code = std::move(code),
        .message = std::move(message),
        .resource = report.resource,
    });
}

void append_parser_diagnostics(
    ProjectWorkspace& project,
    ResourceAnalysisReport& report,
    const gdspaces::ResourceId& resource,
    std::span<const formats::ParseDiagnostic> diagnostics) {
    static_cast<void>(project.add_parser_diagnostics(resource, diagnostics));
    for (const auto& diagnostic : diagnostics) {
        add_report_diagnostic(
            report,
            map_severity(diagnostic.severity),
            diagnostic.code,
            diagnostic.message);
    }
}

bool attach_binary_document(
    ProjectWorkspace& project,
    ResourceAnalysisReport& report,
    const gdspaces::ResourceId& resource,
    std::optional<binary::Document> document,
    std::string_view format_name) {
    if (document.has_value() &&
        project.attach_binary_document(resource, std::move(*document))) {
        report.binary_document_attached = true;
        return true;
    }

    add_report_diagnostic(
        report,
        gdspaces::DiagnosticSeverity::error,
        "analysis.binary-adapter-failed",
        std::string(format_name) +
            " analysis completed, but the Binary Inspector document could not be attached.");
    return false;
}

} // namespace dmc::rengine::integration::native_reader_support
