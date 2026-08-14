#include "dmc_rengine/profiles/dmc3/stageops_ingress.hpp"

#include "dmc_rengine/profiles/dmc3/stageops_assembler.hpp"

#include <algorithm>
#include <optional>
#include <set>
#include <string>
#include <utility>
#include <vector>

namespace dmc::rengine::profiles::dmc3 {
namespace {

void add_diagnostic(
    StageOpsIngressResult& result,
    gdspaces::DiagnosticSeverity severity,
    std::string code,
    std::string message,
    std::optional<gdspaces::ResourceId> resource = std::nullopt) {
    result.diagnostics.push_back(gdspaces::Diagnostic{
        .severity = severity,
        .code = std::move(code),
        .message = std::move(message),
        .resource = std::move(resource),
    });
}

[[nodiscard]] bool same_provenance(
    const std::optional<gdspaces::ByteProvenance>& left,
    const std::optional<gdspaces::ByteProvenance>& right) noexcept {
    if (left.has_value() != right.has_value()) {
        return false;
    }
    if (!left.has_value()) {
        return true;
    }
    return left->kind == right->kind &&
        left->authority_id == right->authority_id &&
        left->offset == right->offset &&
        left->stored_size == right->stored_size &&
        left->materialized_size == right->materialized_size &&
        left->transform == right->transform &&
        left->crc32 == right->crc32;
}

[[nodiscard]] bool same_immutable_payload(
    const gdspaces::ResourcePayload& left,
    const gdspaces::ResourcePayload& right) noexcept {
    return left.resource == right.resource &&
        left.bytes == right.bytes &&
        same_provenance(left.byte_provenance, right.byte_provenance);
}

[[nodiscard]] bool has_error(
    const std::vector<gdspaces::Diagnostic>& diagnostics) noexcept {
    return std::any_of(
        diagnostics.begin(), diagnostics.end(),
        [](const gdspaces::Diagnostic& diagnostic) {
            return diagnostic.severity == gdspaces::DiagnosticSeverity::error;
        });
}

} // namespace

bool StageOpsIngressResult::valid() const noexcept {
    return assembly.valid() && !has_error(diagnostics);
}

StageOpsIngressResult StageOpsIngress::attach(
    StageRuntimeLoadReport report,
    integration::ProjectWorkspace& project) {
    StageOpsIngressResult result;

    // Build the Stage Ops authority before payload ownership is moved out of the
    // runtime report. This consumes only already-resolved/materialized facts.
    result.assembly = StageOpsAssembler::assemble(report);
    result.diagnostics = result.assembly.diagnostics;
    if (!result.assembly.valid()) {
        add_diagnostic(
            result,
            gdspaces::DiagnosticSeverity::error,
            "stageops.ingress.invalid-assembly",
            "The DMC3 runtime report could not produce a valid StageAssemblyWorkspace.");
        return result;
    }

    std::set<std::string, std::less<>> processed;
    std::vector<gdspaces::ResourceId> ingress_resources;

    const auto ingest = [&](gdspaces::ResourcePayload payload) mutable {
        if (!payload.resource.valid()) {
            add_diagnostic(
                result,
                gdspaces::DiagnosticSeverity::error,
                "stageops.ingress.invalid-resource",
                "A materialized Stage runtime payload has an invalid canonical resource identity.");
            return;
        }

        const auto canonical = payload.resource.id.canonical();
        if (!processed.insert(canonical).second) {
            const auto* existing = project.find_session(payload.resource.id);
            if (existing != nullptr &&
                !same_immutable_payload(existing->source_payload(), payload)) {
                add_diagnostic(
                    result,
                    gdspaces::DiagnosticSeverity::error,
                    "stageops.ingress.shared-resource-payload-mismatch",
                    "The same canonical Stage ResourceId was observed with different immutable payload bytes/provenance.",
                    payload.resource.id);
            }
            return;
        }

        if (!payload.readable() || !payload.byte_provenance.has_value() ||
            !payload.byte_provenance->valid()) {
            add_diagnostic(
                result,
                gdspaces::DiagnosticSeverity::error,
                "stageops.ingress.payload-not-canonical-materialized",
                "A Stage Ops materialized resource cannot enter ProjectWorkspace without readable bytes and valid ByteProvenance.",
                payload.resource.id);
            return;
        }

        const auto resource_id = payload.resource.id;
        const auto* existing = project.find_session(resource_id);
        if (existing != nullptr) {
            if (!same_immutable_payload(existing->source_payload(), payload)) {
                add_diagnostic(
                    result,
                    gdspaces::DiagnosticSeverity::error,
                    "stageops.ingress.existing-session-payload-mismatch",
                    "ProjectWorkspace already owns this ResourceId, but its immutable payload disagrees with the Stage runtime report.",
                    resource_id);
                return;
            }
            ++result.sessions_reused;
            ingress_resources.push_back(resource_id);
            return;
        }

        if (!project.create_session(
                std::move(payload),
                integration::WorkspaceContext{
                    .stage_context = true,
                    .menu_context = false,
                    .evidence_context = true,
                })) {
            add_diagnostic(
                result,
                gdspaces::DiagnosticSeverity::error,
                "stageops.ingress.session-create-failed",
                "ProjectWorkspace could not create a canonical session for a Stage Ops materialized resource.",
                resource_id);
            return;
        }

        ++result.sessions_created;
        ingress_resources.push_back(resource_id);
    };

    for (auto& loaded : report.resources) {
        if (loaded.payload.has_value()) {
            ingest(std::move(*loaded.payload));
            loaded.payload.reset();
        }
        if (!loaded.expansion.has_value()) {
            continue;
        }
        for (auto& expansion : loaded.expansion->expansions) {
            for (auto& child : expansion.children) {
                if (!child.entry.populated || !child.payload.resource.valid()) {
                    continue;
                }
                ingest(std::move(child.payload));
            }
        }
    }

    // Execute each implemented canonical parser at most once per ingress when a
    // retained typed result is not already available. Unknown/read-only formats
    // remain valid Stage Ops resources without invented parsing semantics.
    for (const auto& resource_id : ingress_resources) {
        const auto* session = project.find_session(resource_id);
        if (session == nullptr) {
            continue;
        }
        const auto* format = session->format();
        if (format == nullptr || format->parser_id.empty() ||
            session->parsed_resource() != nullptr) {
            continue;
        }

        auto analysis = integration::ResourceAnalyzer::analyze(
            project, resource_id);
        for (const auto& diagnostic : analysis.diagnostics) {
            if (diagnostic.severity == gdspaces::DiagnosticSeverity::error) {
                result.diagnostics.push_back(diagnostic);
            }
        }
        result.analyses.push_back(std::move(analysis));
    }

    // Assembly truth is stronger than ingestion convenience: every resource the
    // report marked materialized must now be backed by the shared project session
    // substrate. Partial/unresolved resources are allowed to remain sessionless.
    for (const auto& resource : result.assembly.resources) {
        if (!resource.materialized) {
            continue;
        }
        const auto* session = project.find_session(resource.resource.id);
        if (session == nullptr) {
            add_diagnostic(
                result,
                gdspaces::DiagnosticSeverity::error,
                "stageops.ingress.materialized-resource-session-missing",
                "A resource marked materialized in StageAssemblyWorkspace is missing from ProjectWorkspace after ingress.",
                resource.resource.id);
        }
    }

    return result;
}

} // namespace dmc::rengine::profiles::dmc3
