#include "dmc_rengine/stageops/invalidation.hpp"

#include <algorithm>
#include <map>
#include <set>
#include <string>
#include <utility>

namespace dmc::rengine::stageops {

StageInvalidationReport StageInvalidationAnalyzer::analyze(
    const StageDomainKnowledgeWorkspace& knowledge,
    std::vector<std::string> changed_resource_ids) {
    StageInvalidationReport report;
    if (!knowledge.valid()) {
        return report;
    }

    report.identity = knowledge.identity();
    report.source_stage_revision = knowledge.source_stage_revision();

    std::sort(changed_resource_ids.begin(), changed_resource_ids.end());
    changed_resource_ids.erase(
        std::unique(changed_resource_ids.begin(), changed_resource_ids.end()),
        changed_resource_ids.end());

    for (const auto& resource_id : changed_resource_ids) {
        if (resource_id.empty()) {
            continue;
        }

        StageResourceInvalidationImpact impact{
            .resource_id = resource_id,
            .domain_object_ids = {},
            .domain_relation_ids = {},
            .runtime_link_ids = {},
        };

        std::set<std::string, std::less<>> affected_domains;
        for (const auto& object : knowledge.domains.objects) {
            if (object.resource_id == resource_id) {
                affected_domains.insert(object.id);
            }
        }
        impact.domain_object_ids.assign(
            affected_domains.begin(), affected_domains.end());

        for (const auto& relation : knowledge.relations.relations) {
            if (affected_domains.contains(relation.from_domain_id) ||
                affected_domains.contains(relation.to_domain_id)) {
                impact.domain_relation_ids.push_back(relation.id);
            }
        }
        for (const auto& link : knowledge.runtime_links.links) {
            if (affected_domains.contains(link.domain_object_id)) {
                impact.runtime_link_ids.push_back(link.id);
            }
        }

        std::sort(
            impact.domain_relation_ids.begin(),
            impact.domain_relation_ids.end());
        impact.domain_relation_ids.erase(
            std::unique(
                impact.domain_relation_ids.begin(),
                impact.domain_relation_ids.end()),
            impact.domain_relation_ids.end());
        std::sort(
            impact.runtime_link_ids.begin(),
            impact.runtime_link_ids.end());
        impact.runtime_link_ids.erase(
            std::unique(
                impact.runtime_link_ids.begin(),
                impact.runtime_link_ids.end()),
            impact.runtime_link_ids.end());

        report.affected_domain_object_count += impact.domain_object_ids.size();
        report.affected_domain_relation_count += impact.domain_relation_ids.size();
        report.affected_runtime_link_count += impact.runtime_link_ids.size();
        report.resources.push_back(std::move(impact));
    }

    return report;
}

} // namespace dmc::rengine::stageops
