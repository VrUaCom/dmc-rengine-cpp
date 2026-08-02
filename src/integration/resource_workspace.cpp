#include "dmc_rengine/integration/resource_workspace.hpp"

#include <algorithm>
#include <utility>

namespace dmc::rengine::integration {
namespace {

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

} // namespace

ResourceWorkspaceSession::ResourceWorkspaceSession(
    gdspaces::ResourcePayload payload,
    const ToolRegistry& tools,
    const FormatIntegrationRegistry& formats,
    WorkspaceContext context)
    : payload_(std::move(payload)),
      context_(context),
      diagnostics_(payload_.diagnostics) {
    const auto* descriptor = formats.find(payload_.resource.format);
    if (descriptor != nullptr) {
        format_ = *descriptor;
    } else {
        add_diagnostic(
            gdspaces::DiagnosticSeverity::warning,
            "workspace.format-unregistered",
            "No Format Integration Registry descriptor exists for this resource format.");
    }

    routes_ = tools.routes_for(
        payload_.resource,
        context_.stage_context,
        context_.menu_context,
        context_.evidence_context);

    if (!payload_.readable()) {
        add_diagnostic(
            gdspaces::DiagnosticSeverity::error,
            "workspace.source-unreadable",
            "The GDSpaces source payload is not readable; the workspace is invalid.");
    }
    if (payload_.resource.id.size != payload_.bytes.size()) {
        add_diagnostic(
            gdspaces::DiagnosticSeverity::error,
            "workspace.size-mismatch",
            "Resource identity size does not match the supplied source byte count.");
    }
    if (routes_.empty()) {
        add_diagnostic(
            gdspaces::DiagnosticSeverity::error,
            "workspace.no-tool-routes",
            "No tool route could be produced for the resource.");
    }
}

bool ResourceWorkspaceSession::valid() const noexcept {
    if (!payload_.readable() ||
        payload_.resource.id.size != payload_.bytes.size() ||
        routes_.empty()) {
        return false;
    }

    return std::none_of(
        diagnostics_.begin(), diagnostics_.end(),
        [](const gdspaces::Diagnostic& diagnostic) {
            return diagnostic.severity == gdspaces::DiagnosticSeverity::error &&
                   (diagnostic.code == "workspace.source-unreadable" ||
                    diagnostic.code == "workspace.size-mismatch" ||
                    diagnostic.code == "workspace.no-tool-routes");
        });
}

WorkspaceStatus ResourceWorkspaceSession::status() const noexcept {
    if (!valid()) {
        return WorkspaceStatus::invalid;
    }
    if (!working_copy_.has_value()) {
        return WorkspaceStatus::read_only;
    }
    return working_copy_->dirty()
        ? WorkspaceStatus::editable_dirty
        : WorkspaceStatus::editable_clean;
}

const gdspaces::ResourceRef&
ResourceWorkspaceSession::resource() const noexcept {
    return payload_.resource;
}

const gdspaces::ResourcePayload&
ResourceWorkspaceSession::source_payload() const noexcept {
    return payload_;
}

const WorkspaceContext& ResourceWorkspaceSession::context() const noexcept {
    return context_;
}

const FormatIntegrationDescriptor*
ResourceWorkspaceSession::format() const noexcept {
    return format_.has_value() ? &*format_ : nullptr;
}

const std::vector<ToolRoute>&
ResourceWorkspaceSession::routes() const noexcept {
    return routes_;
}

const std::vector<gdspaces::Diagnostic>&
ResourceWorkspaceSession::diagnostics() const noexcept {
    return diagnostics_;
}

bool ResourceWorkspaceSession::add_parser_diagnostics(
    std::span<const formats::ParseDiagnostic> diagnostics) {
    for (const auto& diagnostic : diagnostics) {
        if (!diagnostic.valid()) {
            add_diagnostic(
                gdspaces::DiagnosticSeverity::error,
                "workspace.invalid-parser-diagnostic",
                "A parser returned an invalid diagnostic record.");
            return false;
        }

        add_diagnostic(
            map_severity(diagnostic.severity),
            diagnostic.code,
            diagnostic.message + " [offset=" +
                std::to_string(diagnostic.offset) + "]");
    }
    return true;
}

bool ResourceWorkspaceSession::attach_binary_document(
    binary::Document document) {
    if (!valid()) {
        return false;
    }
    if (!format_.has_value() || !format_->binary_adapter) {
        add_diagnostic(
            gdspaces::DiagnosticSeverity::error,
            "workspace.binary-adapter-unregistered",
            "The format registry does not declare a Binary Inspector adapter for this resource.");
        return false;
    }
    if (document.resource().id != payload_.resource.id) {
        add_diagnostic(
            gdspaces::DiagnosticSeverity::error,
            "workspace.binary-resource-mismatch",
            "The Binary Inspector document belongs to a different canonical resource identity.");
        return false;
    }
    if (document.byte_size() != payload_.bytes.size()) {
        add_diagnostic(
            gdspaces::DiagnosticSeverity::error,
            "workspace.binary-size-mismatch",
            "The Binary Inspector document size does not match the source payload.");
        return false;
    }

    binary_document_ = std::move(document);
    return true;
}

const binary::Document*
ResourceWorkspaceSession::binary_document() const noexcept {
    return binary_document_.has_value() ? &*binary_document_ : nullptr;
}

bool ResourceWorkspaceSession::link_evidence_record(
    const evidence::EvidenceRegistry& registry,
    std::string_view record_id) {
    if (registry.find(record_id) == nullptr) {
        add_diagnostic(
            gdspaces::DiagnosticSeverity::warning,
            "workspace.evidence-record-missing",
            "Requested Evidence Registry record is not available: " +
                std::string(record_id));
        return false;
    }

    const auto existing = std::find(
        evidence_record_ids_.begin(),
        evidence_record_ids_.end(),
        record_id);
    if (existing == evidence_record_ids_.end()) {
        evidence_record_ids_.emplace_back(record_id);
        std::sort(evidence_record_ids_.begin(), evidence_record_ids_.end());
    }
    return true;
}

std::size_t ResourceWorkspaceSession::link_evidence_claim(
    const evidence::EvidenceRegistry& registry,
    std::string_view claim_id) {
    std::size_t linked = 0U;
    for (const auto* record : registry.by_claim(claim_id)) {
        if (record != nullptr && link_evidence_record(registry, record->id)) {
            ++linked;
        }
    }
    return linked;
}

std::size_t ResourceWorkspaceSession::link_format_evidence(
    const evidence::EvidenceRegistry& registry) {
    if (!format_.has_value()) {
        return 0U;
    }

    std::size_t linked = 0U;
    for (const auto& claim_id : format_->evidence_claim_ids) {
        linked += link_evidence_claim(registry, claim_id);
    }
    return linked;
}

const std::vector<std::string>&
ResourceWorkspaceSession::evidence_record_ids() const noexcept {
    return evidence_record_ids_;
}

bool ResourceWorkspaceSession::attach_stage_bundle(
    const gdspaces::StageBundle& bundle) {
    std::vector<const gdspaces::StageMember*> matches;
    for (const auto& member : bundle.all_members()) {
        if (member.resource.id == payload_.resource.id) {
            matches.push_back(&member);
        }
    }

    if (matches.size() != 1U) {
        add_diagnostic(
            gdspaces::DiagnosticSeverity::warning,
            matches.empty()
                ? "workspace.stage-member-missing"
                : "workspace.stage-member-ambiguous",
            matches.empty()
                ? "The resource is not a member of the supplied StageBundle."
                : "The resource appears more than once in the supplied StageBundle.");
        return false;
    }

    const auto* member = matches.front();
    if (format_.has_value() && format_->stage_category.has_value() &&
        *format_->stage_category != gdspaces::StageResourceCategory::unknown &&
        *format_->stage_category != member->category) {
        add_diagnostic(
            gdspaces::DiagnosticSeverity::error,
            "workspace.stage-category-mismatch",
            "StageBundle category conflicts with the Format Integration Registry.");
        return false;
    }

    stage_ = StageResourceContext{
        .identity = bundle.identity(),
        .category = member->category,
        .role = member->role,
    };
    return true;
}

const StageResourceContext*
ResourceWorkspaceSession::stage() const noexcept {
    return stage_.has_value() ? &*stage_ : nullptr;
}

bool ResourceWorkspaceSession::enable_working_copy() {
    if (!valid() || !format_.has_value() ||
        !format_->allows_working_copy()) {
        add_diagnostic(
            gdspaces::DiagnosticSeverity::warning,
            "workspace.working-copy-disallowed",
            "The current format integration policy is read-only.");
        return false;
    }
    if (!working_copy_.has_value()) {
        working_copy_.emplace(payload_);
    }
    return true;
}

gdspaces::WorkingCopy* ResourceWorkspaceSession::working_copy() noexcept {
    return working_copy_.has_value() ? &*working_copy_ : nullptr;
}

const gdspaces::WorkingCopy*
ResourceWorkspaceSession::working_copy() const noexcept {
    return working_copy_.has_value() ? &*working_copy_ : nullptr;
}

void ResourceWorkspaceSession::add_diagnostic(
    gdspaces::DiagnosticSeverity severity,
    std::string code,
    std::string message) {
    diagnostics_.push_back(gdspaces::Diagnostic{
        .severity = severity,
        .code = std::move(code),
        .message = std::move(message),
        .resource = payload_.resource.id,
    });
}

} // namespace dmc::rengine::integration
