#include "dmc_rengine/integration/resource_analyzer.hpp"

#include "dmc_rengine/core/sha256.hpp"
#include "dmc_rengine/exe/pe_reader.hpp"
#include "dmc_rengine/formats/dds.hpp"
#include "dmc_rengine/formats/dds_binary.hpp"
#include "dmc_rengine/formats/dca.hpp"
#include "dmc_rengine/formats/dca_binary.hpp"
#include "dmc_rengine/formats/hits.hpp"
#include "dmc_rengine/formats/hits_binary.hpp"
#include "dmc_rengine/formats/lig2.hpp"
#include "dmc_rengine/formats/lig2_binary.hpp"
#include "dmc_rengine/formats/ptx.hpp"
#include "dmc_rengine/formats/ptx_binary.hpp"
#include "dmc_rengine/formats/stage_txt.hpp"
#include "dmc_rengine/formats/stage_txt_binary.hpp"
#include "dmc_rengine/profiles/dmc3/known_targets.hpp"

#include <algorithm>
#include <optional>
#include <span>
#include <string>
#include <utility>
#include <vector>

namespace dmc::rengine::integration {
namespace {

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

[[nodiscard]] gdspaces::DiagnosticSeverity map_severity(
    formats::ParseSeverity severity) noexcept {
    switch (severity) {
    case formats::ParseSeverity::info:
        return gdspaces::DiagnosticSeverity::info;
    case formats::ParseSeverity::warning:
        return gdspaces::DiagnosticSeverity::warning;
    case formats::ParseSeverity::error:
        return gdspaces::DiagnosticSeverity::error;
    }
    return gdspaces::DiagnosticSeverity::error;
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

[[nodiscard]] bool attach_binary_document(
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

} // namespace

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
        add_report_diagnostic(
            report,
            gdspaces::DiagnosticSeverity::error,
            "analysis.session-missing",
            "The canonical resource workspace session was not found.");
        return report;
    }

    report.format = session->resource().format;
    const auto* descriptor = session->format();
    if (descriptor == nullptr || descriptor->parser_id.empty()) {
        add_report_diagnostic(
            report,
            gdspaces::DiagnosticSeverity::warning,
            "analysis.parser-unavailable",
            "No implemented parser is registered for this resource format.");
        return report;
    }
    report.parser_id = descriptor->parser_id;

    const auto bytes = std::span<const std::byte>{
        session->source_payload().bytes};

    if (descriptor->parser_id == "formats.dds-dmc3-reader") {
        report.parser_available = true;
        const auto scan = formats::dds::Reader::scan(bytes);
        report.recognized = scan.recognized;
        static_cast<void>(project.record_parser_completed(
            resource,
            descriptor->parser_id,
            scan.recognized,
            gdspaces::ToolTarget::binary_inspector));
        append_parser_diagnostics(
            project, report, resource, scan.diagnostics);

        if (scan.ok()) {
            static_cast<void>(attach_binary_document(
                project,
                report,
                resource,
                formats::dds::build_binary_document(
                    session->resource(), bytes, scan),
                "DDS"));
        }
        static_cast<void>(project.link_format_evidence(resource));
        return report;
    }

    if (descriptor->parser_id == "formats.ptx-dmc3-reader") {
        report.parser_available = true;
        const auto scan = formats::ptx::Reader::scan(bytes);
        report.recognized = scan.recognized;
        static_cast<void>(project.record_parser_completed(
            resource,
            descriptor->parser_id,
            scan.recognized,
            gdspaces::ToolTarget::binary_inspector));
        append_parser_diagnostics(
            project, report, resource, scan.diagnostics);

        if (scan.ok()) {
            static_cast<void>(attach_binary_document(
                project,
                report,
                resource,
                formats::ptx::build_binary_document(
                    session->resource(), bytes, scan),
                "PTX"));
        }
        static_cast<void>(project.link_format_evidence(resource));
        return report;
    }

    if (descriptor->parser_id == "formats.hits-record-scanner") {
        report.parser_available = true;
        const auto scan = formats::hits::RecordScanner::scan(bytes);
        report.recognized = scan.recognized;
        static_cast<void>(project.record_parser_completed(
            resource,
            descriptor->parser_id,
            scan.recognized,
            gdspaces::ToolTarget::binary_inspector));
        append_parser_diagnostics(
            project, report, resource, scan.diagnostics);

        if (scan.recognized) {
            static_cast<void>(attach_binary_document(
                project,
                report,
                resource,
                formats::hits::build_binary_document(
                    session->resource(), bytes, scan),
                "HITS"));
        }
        static_cast<void>(project.link_format_evidence(resource));
        return report;
    }

    if (descriptor->parser_id == "formats.dca-record-scanner") {
        report.parser_available = true;
        const auto scan = formats::dca::RecordScanner::scan(bytes);
        report.recognized = scan.recognized;
        static_cast<void>(project.record_parser_completed(
            resource,
            descriptor->parser_id,
            scan.recognized,
            gdspaces::ToolTarget::binary_inspector));
        append_parser_diagnostics(
            project, report, resource, scan.diagnostics);

        if (scan.recognized) {
            static_cast<void>(attach_binary_document(
                project,
                report,
                resource,
                formats::dca::build_binary_document(
                    session->resource(), bytes, scan),
                "DCA"));
        }
        static_cast<void>(project.link_format_evidence(resource));
        return report;
    }

    if (descriptor->parser_id == "formats.lig2-record-scanner") {
        report.parser_available = true;
        const auto scan = formats::lig2::RecordScanner::scan(bytes);
        report.recognized = scan.recognized;
        static_cast<void>(project.record_parser_completed(
            resource,
            descriptor->parser_id,
            scan.recognized,
            gdspaces::ToolTarget::binary_inspector));
        append_parser_diagnostics(
            project, report, resource, scan.diagnostics);

        if (scan.recognized) {
            static_cast<void>(attach_binary_document(
                project,
                report,
                resource,
                formats::lig2::build_binary_document(
                    session->resource(), bytes, scan),
                "LIG2"));
        }
        static_cast<void>(project.link_format_evidence(resource));
        return report;
    }

    if (descriptor->parser_id == "formats.stage-txt-lexer") {
        report.parser_available = true;
        const auto lex = formats::stage_txt::Lexer::scan(bytes);
        report.recognized = lex.recognized;
        static_cast<void>(project.record_parser_completed(
            resource,
            descriptor->parser_id,
            lex.recognized,
            gdspaces::ToolTarget::binary_inspector));
        append_parser_diagnostics(
            project, report, resource, lex.diagnostics);

        if (lex.recognized) {
            static_cast<void>(attach_binary_document(
                project,
                report,
                resource,
                formats::stage_txt::build_binary_document(
                    session->resource(), bytes, lex),
                "Stage TXT"));
        }
        static_cast<void>(project.link_format_evidence(resource));
        return report;
    }

    if (descriptor->parser_id == "exe.pe-reader") {
        report.parser_available = true;
        const auto parsed = exe::PeReader::read(bytes);
        report.recognized = parsed.image.has_value();
        static_cast<void>(project.record_parser_completed(
            resource,
            descriptor->parser_id,
            report.recognized,
            gdspaces::ToolTarget::exe_editor));
        const auto parser_diagnostics = pe_diagnostics(parsed);
        append_parser_diagnostics(
            project, report, resource, parser_diagnostics);

        if (!parsed.ok()) {
            return report;
        }

        const auto digest = core::Sha256::compute(bytes).hex();
        const auto& target = profiles::dmc3::phase12_canonical_target();
        const auto hash_match = target.matches_hash(digest);
        const auto metadata_match = hash_match &&
            target.matches_metadata(*parsed.image);
        ExecutableResourceContext context{
            .image = *parsed.image,
            .sha256 = digest,
            .known_target_id = hash_match ? target.id : std::string{},
            .known_target_name = hash_match
                ? target.display_name
                : std::string{},
            .known_target_hash_match = hash_match,
            .known_target_metadata_match = metadata_match,
        };
        if (!project.attach_executable_context(
                resource, std::move(context))) {
            add_report_diagnostic(
                report,
                gdspaces::DiagnosticSeverity::error,
                "analysis.executable-context-failed",
                "The PE parser succeeded, but EXE Editor context could not be attached.");
            return report;
        }

        static_cast<void>(project.link_artifact_evidence(resource, digest));
        return report;
    }

    add_report_diagnostic(
        report,
        gdspaces::DiagnosticSeverity::warning,
        "analysis.parser-not-wired",
        "A parser ID is registered, but ResourceAnalyzer has no adapter for it yet.");
    return report;
}

} // namespace dmc::rengine::integration