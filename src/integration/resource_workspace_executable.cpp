#include "dmc_rengine/integration/resource_workspace.hpp"

#include <utility>

namespace dmc::rengine::integration {

bool ResourceWorkspaceSession::attach_executable_context(
    ExecutableResourceContext context) {
    if (!valid() || !context.valid() || executable_context_.has_value()) {
        return false;
    }

    const auto& resource_format = payload_.resource.format;
    if (resource_format != "pe" && resource_format != "exe" &&
        resource_format != "dll") {
        add_diagnostic(
            gdspaces::DiagnosticSeverity::error,
            "workspace.executable-format-mismatch",
            "Executable analysis can only be attached to PE/EXE/DLL resources.");
        return false;
    }
    if (!has_tool_route(gdspaces::ToolTarget::exe_editor)) {
        add_diagnostic(
            gdspaces::DiagnosticSeverity::error,
            "workspace.exe-editor-route-missing",
            "The resource workspace does not route to EXE Editor.");
        return false;
    }

    const auto subject = context.recognized_target()
        ? context.known_target_id
        : context.sha256;
    const auto message = context.recognized_target()
        ? "EXE Editor attached PE metadata and recognized a known executable target."
        : "EXE Editor attached generic PE metadata for an unrecognized executable.";

    executable_context_ = std::move(context);
    static_cast<void>(events_.record(
        WorkspaceEventType::executable_context_attached,
        payload_.resource.id,
        gdspaces::ToolTarget::exe_editor,
        gdspaces::ToolTarget::spider_hub,
        working_copy_.has_value() ? working_copy_->revision() : 0U,
        subject,
        message));
    return true;
}

const ExecutableResourceContext*
ResourceWorkspaceSession::executable_context() const noexcept {
    return executable_context_.has_value() ? &*executable_context_ : nullptr;
}

} // namespace dmc::rengine::integration
