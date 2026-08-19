#pragma once

#include "dmc_rengine/binary/document.hpp"
#include "dmc_rengine/evidence/registry.hpp"
#include "dmc_rengine/formats/diagnostic.hpp"
#include "dmc_rengine/gdspaces/resource_payload.hpp"
#include "dmc_rengine/gdspaces/stage_bundle.hpp"
#include "dmc_rengine/gdspaces/working_copy.hpp"
#include "dmc_rengine/integration/executable_context.hpp"
#include "dmc_rengine/integration/format_registry.hpp"
#include "dmc_rengine/integration/tool_registry.hpp"
#include "dmc_rengine/integration/workspace_event.hpp"

#include <cstddef>
#include <cstdint>
#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <vector>

namespace dmc::rengine::integration {

struct WorkspaceContext final {
    bool stage_context{false};
    bool menu_context{false};
    bool evidence_context{true};
};

struct StageResourceContext final {
    gdspaces::StageIdentity identity;
    gdspaces::StageResourceCategory category{
        gdspaces::StageResourceCategory::unknown};
    std::string role;

    [[nodiscard]] bool valid() const noexcept {
        return identity.valid() && !role.empty();
    }
};

struct ParserValidationState final {
    std::string parser_id;
    std::string source_sha256;
    bool recognized{false};
    bool error_diagnostics{false};

    [[nodiscard]] bool valid() const noexcept {
        return !parser_id.empty() && source_sha256.size() == 64U;
    }
};

enum class WorkspaceStatus {
    invalid,
    read_only,
    editable_clean,
    editable_dirty,
};

[[nodiscard]] constexpr std::string_view to_string(
    WorkspaceStatus status) noexcept {
    switch (status) {
    case WorkspaceStatus::invalid: return "invalid";
    case WorkspaceStatus::read_only: return "read-only";
    case WorkspaceStatus::editable_clean: return "editable-clean";
    case WorkspaceStatus::editable_dirty: return "editable-dirty";
    }
    return "invalid";
}

class ResourceWorkspaceSession final {
public:
    ResourceWorkspaceSession(
        gdspaces::ResourcePayload payload,
        const ToolRegistry& tools,
        const FormatIntegrationRegistry& formats,
        WorkspaceContext context = {});

    [[nodiscard]] bool valid() const noexcept;
    [[nodiscard]] WorkspaceStatus status() const noexcept;
    [[nodiscard]] const gdspaces::ResourceRef& resource() const noexcept;
    [[nodiscard]] const gdspaces::ResourcePayload& source_payload() const noexcept;
    [[nodiscard]] const WorkspaceContext& context() const noexcept;
    [[nodiscard]] const FormatIntegrationDescriptor* format() const noexcept;
    [[nodiscard]] const std::vector<ToolRoute>& routes() const noexcept;
    [[nodiscard]] const std::vector<gdspaces::Diagnostic>& diagnostics() const noexcept;
    [[nodiscard]] const WorkspaceEventJournal& events() const noexcept;

    [[nodiscard]] bool record_parser_completed(
        std::string parser_id,
        bool recognized,
        gdspaces::ToolTarget consumer);
    [[nodiscard]] const ParserValidationState*
        parser_validation() const noexcept;
    [[nodiscard]] bool add_parser_diagnostics(
        std::span<const formats::ParseDiagnostic> diagnostics);
    [[nodiscard]] bool attach_binary_document(binary::Document document);
    [[nodiscard]] const binary::Document* binary_document() const noexcept;
    [[nodiscard]] bool attach_executable_context(
        ExecutableResourceContext context);
    [[nodiscard]] const ExecutableResourceContext*
        executable_context() const noexcept;

    [[nodiscard]] bool link_evidence_record(
        const evidence::EvidenceRegistry& registry,
        std::string_view record_id);
    [[nodiscard]] std::size_t link_evidence_claim(
        const evidence::EvidenceRegistry& registry,
        std::string_view claim_id);
    [[nodiscard]] std::size_t link_format_evidence(
        const evidence::EvidenceRegistry& registry);
    [[nodiscard]] const std::vector<std::string>&
        evidence_record_ids() const noexcept;

    [[nodiscard]] bool attach_stage_bundle(const gdspaces::StageBundle& bundle);
    [[nodiscard]] const StageResourceContext* stage() const noexcept;

    [[nodiscard]] bool enable_working_copy();
    [[nodiscard]] const gdspaces::WorkingCopy* working_copy() const noexcept;
    [[nodiscard]] gdspaces::EditResult apply_edit(
        gdspaces::EditOperation operation,
        gdspaces::ToolTarget producer);
    [[nodiscard]] bool undo_last_edit(gdspaces::ToolTarget producer);
    [[nodiscard]] bool reset_working_copy(gdspaces::ToolTarget producer);

    [[nodiscard]] bool record_runtime_change_requested(
        std::string request_id,
        gdspaces::ToolTarget consumer,
        std::string message);
    [[nodiscard]] bool record_validation_plan_created(
        std::string plan_id,
        bool ready_for_execution,
        std::size_t requirement_count,
        std::size_t blocker_count);
    [[nodiscard]] bool record_patch_plan_compiled(
        std::string patch_plan_id,
        std::string message);
    [[nodiscard]] bool record_patch_copy_executed(
        std::string execution_id,
        std::string output_sha256,
        std::string rollback_plan_id);
    [[nodiscard]] bool request_validation(
        gdspaces::ToolTarget producer,
        std::string subject_id,
        std::string message);
    [[nodiscard]] bool record_manifest_exported(
        gdspaces::ToolTarget producer,
        std::string manifest_id);

private:
    [[nodiscard]] bool has_tool_route(gdspaces::ToolTarget target) const noexcept;
    void add_diagnostic(
        gdspaces::DiagnosticSeverity severity,
        std::string code,
        std::string message);

    gdspaces::ResourcePayload payload_;
    WorkspaceContext context_;
    std::optional<FormatIntegrationDescriptor> format_;
    std::vector<ToolRoute> routes_;
    std::vector<gdspaces::Diagnostic> diagnostics_;
    std::optional<ParserValidationState> parser_validation_;
    bool parser_error_diagnostics_{false};
    std::optional<binary::Document> binary_document_;
    std::optional<ExecutableResourceContext> executable_context_;
    std::vector<std::string> evidence_record_ids_;
    std::optional<StageResourceContext> stage_;
    std::optional<gdspaces::WorkingCopy> working_copy_;
    WorkspaceEventJournal events_;
};

} // namespace dmc::rengine::integration
