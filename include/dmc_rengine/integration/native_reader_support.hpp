#pragma once

#include "dmc_rengine/binary/document.hpp"
#include "dmc_rengine/formats/diagnostic.hpp"
#include "dmc_rengine/integration/resource_analyzer.hpp"

#include <optional>
#include <span>
#include <string>
#include <string_view>

namespace dmc::rengine::integration::native_reader_support {

void add_report_diagnostic(
    ResourceAnalysisReport& report,
    gdspaces::DiagnosticSeverity severity,
    std::string code,
    std::string message);

void append_parser_diagnostics(
    ProjectWorkspace& project,
    ResourceAnalysisReport& report,
    const gdspaces::ResourceId& resource,
    std::span<const formats::ParseDiagnostic> diagnostics);

[[nodiscard]] bool attach_binary_document(
    ProjectWorkspace& project,
    ResourceAnalysisReport& report,
    const gdspaces::ResourceId& resource,
    std::optional<binary::Document> document,
    std::string_view format_name);

} // namespace dmc::rengine::integration::native_reader_support
