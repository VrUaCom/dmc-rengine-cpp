#include "dmc_rengine/stageops/runtime_links.hpp"

#include <algorithm>
#include <map>
#include <set>
#include <string>
#include <utility>

namespace dmc::rengine::stageops {
namespace {

[[nodiscard]] const StageDomainObject* find_domain(
    const StageDomainWorkspace& domains,
    std::string_view id) noexcept {
    const auto iterator = std::find_if(
        domains.objects.begin(), domains.objects.end(),
        [id](const StageDomainObject& object) {
            return object.id == id;
        });
    return iterator == domains.objects.end() ? nullptr : &*iterator;
}

} // namespace

bool StageRuntimeLinkWorkspace::valid() const noexcept {
    if (!identity.valid()) {
        return false;
    }

    std::set<std::string, std::less<>> ids;
    for (const auto& link : links) {
        if (!link.valid() || !ids.insert(link.id).second) {
            return false;
        }
    }
    return true;
}

std::vector<const StageRuntimeLink*> StageRuntimeLinkWorkspace::for_domain(
    std::string_view domain_object_id) const {
    std::vector<const StageRuntimeLink*> result;
    for (const auto& link : links) {
        if (link.domain_object_id == domain_object_id) {
            result.push_back(&link);
        }
    }
    return result;
}

StageRuntimeLinkWorkspace StageRuntimeLinkBuilder::build(
    const StageDomainWorkspace& domains,
    std::vector<StageRuntimeLink> explicit_links) {
    StageRuntimeLinkWorkspace workspace;
    if (!domains.valid()) {
        return workspace;
    }

    workspace.identity = domains.identity;
    workspace.source_stage_revision = domains.source_stage_revision;

    std::set<std::string, std::less<>> link_ids;
    for (auto& link : explicit_links) {
        if (!link.valid() ||
            find_domain(domains, link.domain_object_id) == nullptr ||
            !link_ids.insert(link.id).second) {
            // Fail closed: malformed links, links to foreign/nonexistent domain
            // objects, and duplicate identities are dropped instead of becoming
            // graph/runtime facts by convenience.
            continue;
        }
        workspace.links.push_back(std::move(link));
    }

    std::sort(
        workspace.links.begin(), workspace.links.end(),
        [](const StageRuntimeLink& left, const StageRuntimeLink& right) {
            return left.id < right.id;
        });
    return workspace;
}

} // namespace dmc::rengine::stageops
