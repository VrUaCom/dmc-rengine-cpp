#pragma once

#include "dmc_rengine/gdspaces/open_router.hpp"
#include "dmc_rengine/gdspaces/resource_id.hpp"

#include <cstdint>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace dmc::rengine::integration {

enum class WorkspaceEventType {
    workspace_created,
    routes_resolved,
    parser_completed,
    parser_diagnostics_added,
    parsed_resource_attached,
    binary_document_attached,
    executable_context_attached,
    evidence_record_linked,
    stage_context_attached,
    working_copy_enabled,
    edit_applied,
    edit_undone,
    working_copy_reset,
    runtime_change_requested,
    validation_requested,
    validation_plan_created,
    patch_plan_compiled,
    patch_copy_executed,
    manifest_exported,
};

[[nodiscard]] constexpr std::string_view to_string(
    WorkspaceEventType type) noexcept {
    switch (type) {
    case WorkspaceEventType::workspace_created: return "workspace-created";
    case WorkspaceEventType::routes_resolved: return "routes-resolved";
    case WorkspaceEventType::parser_completed: return "parser-completed";
    case WorkspaceEventType::parser_diagnostics_added: return "parser-diagnostics-added";
    case WorkspaceEventType::parsed_resource_attached: return "parsed-resource-attached";
    case WorkspaceEventType::binary_document_attached: return "binary-document-attached";
    case WorkspaceEventType::executable_context_attached: return "executable-context-attached";
    case WorkspaceEventType::evidence_record_linked: return "evidence-record-linked";
    case WorkspaceEventType::stage_context_attached: return "stage-context-attached";
    case WorkspaceEventType::working_copy_enabled: return "working-copy-enabled";
    case WorkspaceEventType::edit_applied: return "edit-applied";
    case WorkspaceEventType::edit_undone: return "edit-undone";
    case WorkspaceEventType::working_copy_reset: return "working-copy-reset";
    case WorkspaceEventType::runtime_change_requested: return "runtime-change-requested";
    case WorkspaceEventType::validation_requested: return "validation-requested";
    case WorkspaceEventType::validation_plan_created: return "validation-plan-created";
    case WorkspaceEventType::patch_plan_compiled: return "patch-plan-compiled";
    case WorkspaceEventType::patch_copy_executed: return "patch-copy-executed";
    case WorkspaceEventType::manifest_exported: return "manifest-exported";
    }
    return "workspace-created";
}

struct WorkspaceEvent final {
    std::uint64_t sequence{};
    WorkspaceEventType type{WorkspaceEventType::workspace_created};
    gdspaces::ResourceId resource;
    gdspaces::ToolTarget producer{gdspaces::ToolTarget::gdspaces};
    std::optional<gdspaces::ToolTarget> consumer;
    std::uint64_t revision{};
    std::string subject_id;
    std::string message;

    [[nodiscard]] bool valid() const noexcept {
        return sequence != 0U && resource.valid() && !message.empty();
    }
};

class WorkspaceEventJournal final {
public:
    [[nodiscard]] std::uint64_t record(
        WorkspaceEventType type,
        const gdspaces::ResourceId& resource,
        gdspaces::ToolTarget producer,
        std::optional<gdspaces::ToolTarget> consumer,
        std::uint64_t revision,
        std::string subject_id,
        std::string message);

    [[nodiscard]] const std::vector<WorkspaceEvent>& events() const noexcept;
    [[nodiscard]] std::vector<const WorkspaceEvent*> by_type(
        WorkspaceEventType type) const;
    [[nodiscard]] std::vector<const WorkspaceEvent*> by_tool(
        gdspaces::ToolTarget tool) const;
    [[nodiscard]] const WorkspaceEvent* latest() const noexcept;
    [[nodiscard]] std::size_t size() const noexcept;
    [[nodiscard]] bool empty() const noexcept;

private:
    std::uint64_t next_sequence_{1U};
    std::vector<WorkspaceEvent> events_;
};

} // namespace dmc::rengine::integration
