#include "dmc_rengine/integration/executable_evidence.hpp"

#include <string>
#include <string_view>

namespace dmc::rengine::integration {
namespace {

void add_diagnostic(
    ExecutableEvidenceResolutionReport& report,
    std::string code,
    std::string message) {
    report.diagnostics.push_back(exe::EvidenceAddressDiagnostic{
        .code = std::move(code),
        .message = std::move(message),
    });
}

} // namespace

ExecutableEvidenceResolutionReport resolve_executable_evidence(
    const ProjectWorkspace& project,
    const gdspaces::ResourceId& executable_resource,
    std::string_view evidence_record_id) {
    ExecutableEvidenceResolutionReport report{
        .executable_resource = executable_resource,
        .evidence_record_id = std::string(evidence_record_id),
        .locations = {},
        .diagnostics = {},
    };

    const auto* session = project.find_session(executable_resource);
    if (session == nullptr || session->executable_context() == nullptr) {
        add_diagnostic(
            report,
            "executable-evidence.context-missing",
            "The resource has no attached executable analysis context.");
        return report;
    }
    const auto* record = project.evidence().find(evidence_record_id);
    if (record == nullptr) {
        add_diagnostic(
            report,
            "executable-evidence.record-missing",
            "The requested Evidence Record is not loaded in ProjectWorkspace.");
        return report;
    }

    const auto& executable = *session->executable_context();
    bool matching_artifact_seen = false;
    for (const auto& location : record->locations) {
        const auto* artifact = project.artifacts().find(location.artifact_id);
        if (artifact == nullptr) {
            add_diagnostic(
                report,
                "executable-evidence.artifact-missing",
                "An EvidenceLocation references an artifact absent from ArtifactRegistry.");
            continue;
        }
        if (artifact->sha256 != executable.sha256) {
            continue;
        }
        matching_artifact_seen = true;
        auto resolution = exe::resolve_evidence_location(
            location,
            *artifact,
            executable,
            session->resource().id.size);
        if (resolution.location.has_value()) {
            report.locations.push_back(std::move(*resolution.location));
        }
        report.diagnostics.insert(
            report.diagnostics.end(),
            resolution.diagnostics.begin(),
            resolution.diagnostics.end());
    }

    if (!matching_artifact_seen) {
        add_diagnostic(
            report,
            "executable-evidence.no-matching-artifact",
            "The Evidence Record has no location for the executable artifact SHA-256.");
    } else if (report.locations.empty() && report.diagnostics.empty()) {
        add_diagnostic(
            report,
            "executable-evidence.no-resolved-location",
            "No patch-grade executable location could be resolved.");
    }
    return report;
}

} // namespace dmc::rengine::integration
