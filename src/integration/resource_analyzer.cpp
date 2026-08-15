#include "dmc_rengine/integration/resource_analyzer.hpp"

#include "dmc_rengine/core/sha256.hpp"
#include "dmc_rengine/exe/pe_reader.hpp"
#include "dmc_rengine/formats/dca.hpp"
#include "dmc_rengine/formats/dca_binary.hpp"
#include "dmc_rengine/formats/hits.hpp"
#include "dmc_rengine/formats/hits_binary.hpp"
#include "dmc_rengine/formats/lig2.hpp"
#include "dmc_rengine/formats/lig2_binary.hpp"
#include "dmc_rengine/formats/stage_txt.hpp"
#include "dmc_rengine/formats/stage_txt_binary.hpp"
#include "dmc_rengine/profiles/dmc3/known_targets.hpp"

#include <algorithm>
#include <optional>
#include <span>
#include <string>
#include <string_view>
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
    std::span<const formats::ParseDiagnostic> diagnostics,
    bool persist_in_session) {
    if (persist_in_session) {
        static_cast<void>(project.add_parser_diagnostics(resource, diagnostics));
    }
    for (const auto& diagnostic : diagnostics) {
        add_report_diagnostic(
            report,
            map_severity(diagnostic.severity),
            diagnostic.code,
            diagnostic.message);
    }
}

[[nodiscard]] bool attach_typed_result(
    ProjectWorkspace& project,
    ResourceAnalysisReport& report,
    const gdspaces::ResourceId& resource,
    ParsedResourceResult parsed,
    std::string_view format_name) {
    if (project.attach_parsed_resource(resource, std::move(parsed))) {
        return true;
    }

    add_report_diagnostic(
        report,
        gdspaces::DiagnosticSeverity::error,
        "analysis.parsed-resource-attach-failed",
        std::string(format_name) +
            " parser executed, but its canonical typed result could not be retained in the shared resource workspace.");
    return false;
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

[[nodiscard]] ResourceAnalysisReport make_report(
    const gdspaces::ResourceId& resource,
    ParsedByteSource byte_source,
    std::uint64_t byte_revision) {
    return ResourceAnalysisReport{
        .resource = resource,
        .format = {},
        .parser_id = {},
        .parser_available = false,
        .recognized = false,
        .binary_document_attached = false,
        .byte_source = byte_source,
        .byte_revision = byte_revision,
        .diagnostics = {},
    };
}

[[nodiscard]] bool structural_parser(std::string_view parser_id) noexcept {
    return parser_id == "formats.hits-record-scanner" ||
        parser_id == "formats.dca-record-scanner" ||
        parser_id == "formats.lig2-record-scanner" ||
        parser_id == "formats.stage-txt-lexer";
}

[[nodiscard]] ResourceAnalysisReport analyze_structural_bytes(
    ProjectWorkspace& project,
    const ResourceWorkspaceSession& session,
    std::span<const std::byte> bytes,
    ParsedByteSource byte_source,
    std::uint64_t byte_revision,
    bool persist_diagnostics) {
    auto report = make_report(session.resource().id, byte_source, byte_revision);
    report.format = session.resource().format;

    const auto* descriptor = session.format();
    if (descriptor == nullptr || descriptor->parser_id.empty()) {
        add_report_diagnostic(
            report,
            gdspaces::DiagnosticSeverity::warning,
            "analysis.parser-unavailable",
            "No implemented parser is registered for this resource format.");
        return report;
    }
    report.parser_id = descriptor->parser_id;

    if (descriptor->parser_id == "formats.hits-record-scanner") {
        report.parser_available = true;
        const auto scan = formats::hits::RecordScanner::scan(bytes);
        report.recognized = scan.recognized;
        static_cast<void>(project.record_parser_completed(
            session.resource().id,
            descriptor->parser_id,
            scan.recognized,
            gdspaces::ToolTarget::binary_inspector,
            byte_revision));
        append_parser_diagnostics(
            project,
            report,
            session.resource().id,
            scan.diagnostics,
            persist_diagnostics);
        static_cast<void>(attach_typed_result(
            project,
            report,
            session.resource().id,
            ParsedResourceResult{
                .parser_id = descriptor->parser_id,
                .byte_source = byte_source,
                .byte_revision = byte_revision,
                .data = scan,
            },
            "HITS"));

        if (scan.recognized) {
            static_cast<void>(attach_binary_document(
                project,
                report,
                session.resource().id,
                formats::hits::build_binary_document(
                    session.resource(), bytes, scan),
                "HITS"));
        }
        static_cast<void>(project.link_format_evidence(session.resource().id));
        return report;
    }

    if (descriptor->parser_id == "formats.dca-record-scanner") {
        report.parser_available = true;
        const auto scan = formats::dca::RecordScanner::scan(bytes);
        report.recognized = scan.recognized;
        static_cast<void>(project.record_parser_completed(
            session.resource().id,
            descriptor->parser_id,
            scan.recognized,
            gdspaces::ToolTarget::binary_inspector,
            byte_revision));
        append_parser_diagnostics(
            project,
            report,
            session.resource().id,
            scan.diagnostics,
            persist_diagnostics);
        static_cast<void>(attach_typed_result(
            project,
            report,
            session.resource().id,
            ParsedResourceResult{
                .parser_id = descriptor->parser_id,
                .byte_source = byte_source,
                .byte_revision = byte_revision,
                .data = scan,
            },
            "DCA"));

        if (scan.recognized) {
            static_cast<void>(attach_binary_document(
                project,
                report,
                session.resource().id,
                formats::dca::build_binary_document(
                    session.resource(), bytes, scan),
                "DCA"));
        }
        static_cast<void>(project.link_format_evidence(session.resource().id));
        return report;
    }

    if (descriptor->parser_id == "formats.lig2-record-scanner") {
        report.parser_available = true;
        const auto scan = formats::lig2::RecordScanner::scan(bytes);
        report.recognized = scan.recognized;
        static_cast<void>(project.record_parser_completed(
            session.resource().id,
            descriptor->parser_id,
            scan.recognized,
            gdspaces::ToolTarget::binary_inspector,
            byte_revision));
        append_parser_diagnostics(
            project,
            report,
            session.resource().id,
            scan.diagnostics,
            persist_diagnostics);
        static_cast<void>(attach_typed_result(
            project,
            report,
            session.resource().id,
            ParsedResourceResult{
                .parser_id = descriptor->parser_id,
                .byte_source = byte_source,
                .byte_revision = byte_revision,
                .data = scan,
            },
            "LIG2"));

        if (scan.recognized) {
            static_cast<void>(attach_binary_document(
                project,
                report,
                session.resource().id,
                formats::lig2::build_binary_document(
                    session.resource(), bytes, scan),
                "LIG2"));
        }
        static_cast<void>(project.link_format_evidence(session.resource().id));
        return report;
    }

    if (descriptor->parser_id == "formats.stage-txt-lexer") {
        report.parser_available = true;
        const auto lex = formats::stage_txt::Lexer::scan(bytes);
        report.recognized = lex.recognized;
        static_cast<void>(project.record_parser_completed(
            session.resource().id,
            descriptor->parser_id,
            lex.recognized,
            gdspaces::ToolTarget::binary_inspector,
            byte_revision));
        append_parser_diagnostics(
            project,
            report,
            session.resource().id,
            lex.diagnostics,
            persist_diagnostics);
        static_cast<void>(attach_typed_result(
            project,
            report,
            session.resource().id,
            ParsedResourceResult{
                .parser_id = descriptor->parser_id,
                .byte_source = byte_source,
                .byte_revision = byte_revision,
                .data = lex,
            },
            "Stage TXT"));

        if (lex.recognized) {
            static_cast<void>(attach_binary_document(
                project,
                report,
                session.resource().id,
                formats::stage_txt::build_binary_document(
                    session.resource(), bytes, lex),
                "Stage TXT"));
        }
        static_cast<void>(project.link_format_evidence(session.resource().id));
        return report;
    }

    add_report_diagnostic(
        report,
        gdspaces::DiagnosticSeverity::warning,
        "analysis.parser-not-structural",
        "The registered parser is not part of the shared structural resource analyzer path.");
    return report;
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
    auto report = make_report(
        resource,
        ParsedByteSource::immutable_source,
        0U);

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
    if (structural_parser(descriptor->parser_id)) {
        return analyze_structural_bytes(
            project,
            *session,
            bytes,
            ParsedByteSource::immutable_source,
            0U,
            true);
    }

    if (descriptor->parser_id == "exe.pe-reader") {
        report.parser_available = true;
        const auto parsed = exe::PeReader::read(bytes);
        report.recognized = parsed.image.has_value();
        static_cast<void>(project.record_parser_completed(
            resource,
            descriptor->parser_id,
            report.recognized,
            gdspaces::ToolTarget::exe_editor,
            0U));
        const auto parser_diagnostics = pe_diagnostics(parsed);
        append_parser_diagnostics(
            project,
            report,
            resource,
            parser_diagnostics,
            true);

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

ResourceAnalysisReport ResourceAnalyzer::analyze_working_copy(
    ProjectWorkspace& project,
    const gdspaces::ResourceId& resource) {
    auto report = make_report(
        resource,
        ParsedByteSource::working_copy,
        0U);

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

    const auto* working = session->working_copy();
    if (working == nullptr) {
        add_report_diagnostic(
            report,
            gdspaces::DiagnosticSeverity::error,
            "analysis.working-copy-missing",
            "WorkingCopy analysis was requested, but no WorkingCopy is enabled for this resource.");
        return report;
    }

    report.byte_revision = working->revision();
    if (!structural_parser(descriptor->parser_id)) {
        report.parser_available = true;
        add_report_diagnostic(
            report,
            gdspaces::DiagnosticSeverity::warning,
            "analysis.working-copy-parser-unsupported",
            "WorkingCopy re-analysis is currently restricted to the shared structural format adapters; executable and other parser contexts remain fail-closed.");
        return report;
    }

    return analyze_structural_bytes(
        project,
        *session,
        std::span<const std::byte>{working->bytes()},
        ParsedByteSource::working_copy,
        working->revision(),
        false);
}

} // namespace dmc::rengine::integration
