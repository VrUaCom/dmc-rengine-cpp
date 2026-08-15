#include "dmc_rengine/stageops/runtime_link_validation.hpp"

#include <algorithm>
#include <set>
#include <string>

namespace dmc::rengine::stageops {
namespace {

void add_issue(
    StageRuntimeLinkValidationReport& report,
    std::string code,
    std::string link_id,
    std::string message) {
    report.issues.push_back(StageRuntimeLinkValidationIssue{
        .code = std::move(code),
        .link_id = std::move(link_id),
        .message = std::move(message),
    });
}

[[nodiscard]] bool domain_exists(
    const StageDomainWorkspace& domains,
    std::string_view id) noexcept {
    return std::any_of(
        domains.objects.begin(), domains.objects.end(),
        [id](const StageDomainObject& object) {
            return object.id == id;
        });
}

} // namespace

StageRuntimeLinkValidationReport StageRuntimeLinkValidator::validate(
    const StageDomainWorkspace& domains,
    const std::vector<StageRuntimeLink>& links) {
    StageRuntimeLinkValidationReport report;
    if (!domains.valid()) {
        add_issue(
            report,
            "stage-runtime-link.domain-workspace-invalid",
            {},
            "The Stage Ops domain workspace is invalid; recovered-runtime links cannot be validated.");
        return report;
    }

    std::set<std::string, std::less<>> ids;
    for (const auto& link : links) {
        bool accepted = true;
        if (!link.valid()) {
            add_issue(
                report,
                "stage-runtime-link.invalid",
                link.id,
                "The recovered-runtime link is structurally incomplete; domain identity, runtime identity, artifact/evidence and claim identity are all required.");
            accepted = false;
        }
        if (!link.id.empty() && !ids.insert(link.id).second) {
            add_issue(
                report,
                "stage-runtime-link.duplicate-id",
                link.id,
                "The recovered-runtime link ID is duplicated within the same Stage Ops domain revision.");
            accepted = false;
        }
        if (!link.domain_object_id.empty() &&
            !domain_exists(domains, link.domain_object_id)) {
            add_issue(
                report,
                "stage-runtime-link.domain-object-missing",
                link.id,
                "The recovered-runtime link references a domain object absent from this Stage Ops domain workspace/revision.");
            accepted = false;
        }
        if (accepted) {
            ++report.accepted_link_count;
        }
    }
    return report;
}

} // namespace dmc::rengine::stageops
