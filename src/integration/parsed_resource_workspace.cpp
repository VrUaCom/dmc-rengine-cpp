#include "dmc_rengine/integration/resource_workspace.hpp"

#include <utility>

namespace dmc::rengine::integration {

bool ResourceWorkspaceSession::attach_parsed_resource(
    ParsedResourceResult parsed) {
    if (!valid() || !parsed.valid()) {
        add_diagnostic(
            gdspaces::DiagnosticSeverity::error,
            "workspace.parsed-resource-invalid",
            "The typed parser result is invalid and cannot be attached to the canonical resource workspace.");
        return false;
    }
    if (!format_.has_value() || format_->parser_id.empty() ||
        parsed.parser_id != format_->parser_id) {
        add_diagnostic(
            gdspaces::DiagnosticSeverity::error,
            "workspace.parsed-resource-parser-mismatch",
            "The typed parser result was produced by a parser different from the format integration descriptor.");
        return false;
    }

    const auto parser_id = parsed.parser_id;
    parsed_resource_ = std::move(parsed);
    static_cast<void>(events_.record(
        WorkspaceEventType::parsed_resource_attached,
        payload_.resource.id,
        gdspaces::ToolTarget::gdspaces,
        gdspaces::ToolTarget::stage_ops,
        working_copy_.has_value() ? working_copy_->revision() : 0U,
        parser_id,
        "The canonical typed parser result was retained for shared Stage Ops and inspection consumers."));
    return true;
}

const ParsedResourceResult*
ResourceWorkspaceSession::parsed_resource() const noexcept {
    return parsed_resource_.has_value() ? &*parsed_resource_ : nullptr;
}

} // namespace dmc::rengine::integration
