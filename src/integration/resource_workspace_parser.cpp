#include "dmc_rengine/integration/resource_workspace.hpp"

#include "dmc_rengine/core/sha256.hpp"

#include <span>
#include <utility>

namespace dmc::rengine::integration {

bool ResourceWorkspaceSession::record_parser_completed(
    std::string parser_id,
    bool recognized,
    gdspaces::ToolTarget consumer) {
    if (!valid() || parser_id.empty() || !has_tool_route(consumer)) {
        return false;
    }

    if (format_.has_value() && format_->parser_validation_required &&
        parser_id != format_->parser_id) {
        add_diagnostic(
            gdspaces::DiagnosticSeverity::error,
            "workspace.parser-authority-mismatch",
            "Parser completion does not match the canonical parser authority declared for this format.");
        return false;
    }

    const auto source_sha = core::Sha256::compute(
        std::span<const std::byte>{payload_.bytes.data(), payload_.bytes.size()}).hex();
    parser_validation_ = ParserValidationState{
        .parser_id = parser_id,
        .source_sha256 = source_sha,
        .recognized = recognized,
        .error_diagnostics = parser_error_diagnostics_,
    };

    return events_.record(
        WorkspaceEventType::parser_completed,
        payload_.resource.id,
        gdspaces::ToolTarget::gdspaces,
        consumer,
        working_copy_.has_value() ? working_copy_->revision() : 0U,
        std::move(parser_id),
        recognized
            ? "The registered parser recognized and completed analysis of the canonical resource."
            : "The registered parser completed without recognizing the canonical resource.") != 0U;
}

const ParserValidationState*
ResourceWorkspaceSession::parser_validation() const noexcept {
    return parser_validation_.has_value() ? &*parser_validation_ : nullptr;
}

} // namespace dmc::rengine::integration
