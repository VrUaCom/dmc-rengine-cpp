#include "dmc_rengine/stageops/domain_knowledge.hpp"

#include <set>
#include <string>
#include <utility>

namespace dmc::rengine::stageops {
namespace {

[[nodiscard]] bool same_identity(
    const StageAssemblyIdentity& left,
    const StageAssemblyIdentity& right) noexcept {
    return left.catalog_entry_id == right.catalog_entry_id &&
        left.global_catalog_row == right.global_catalog_row &&
        left.source_table_id == right.source_table_id &&
        left.source_row_index == right.source_row_index &&
        left.stage.profile == right.stage.profile &&
        left.stage.resource_set_key() == right.stage.resource_set_key() &&
        left.stage.numeric_stage_id == right.stage.numeric_stage_id &&
        left.stage.semantic_stage_id == right.stage.semantic_stage_id &&
        left.stage.exe_evidence_id == right.stage.exe_evidence_id;
}

} // namespace

bool StageDomainKnowledgeWorkspace::valid() const noexcept {
    if (!domains.valid() || !relations.valid() || !runtime_links.valid()) {
        return false;
    }
    if (!same_identity(domains.identity, relations.identity) ||
        !same_identity(domains.identity, runtime_links.identity)) {
        return false;
    }
    if (domains.source_stage_revision != relations.source_stage_revision ||
        domains.source_stage_revision != runtime_links.source_stage_revision) {
        return false;
    }

    std::set<std::string, std::less<>> domain_ids;
    for (const auto& object : domains.objects) {
        if (!object.valid() || !domain_ids.insert(object.id).second) {
            return false;
        }
    }

    for (const auto& relation : relations.relations) {
        if (!domain_ids.contains(relation.from_domain_id) ||
            !domain_ids.contains(relation.to_domain_id)) {
            return false;
        }
    }
    for (const auto& link : runtime_links.links) {
        if (!domain_ids.contains(link.domain_object_id)) {
            return false;
        }
    }
    return true;
}

StageDomainKnowledgeWorkspace StageDomainKnowledgeBuilder::build(
    StageDomainWorkspace domains,
    std::vector<StageRuntimeLink> explicit_runtime_links) {
    StageDomainKnowledgeWorkspace workspace;
    if (!domains.valid()) {
        return workspace;
    }

    workspace.relations = StageDomainRelationAssembler::assemble(domains);
    workspace.runtime_links = StageRuntimeLinkBuilder::build(
        domains,
        std::move(explicit_runtime_links));
    workspace.domains = std::move(domains);

    return workspace.valid()
        ? workspace
        : StageDomainKnowledgeWorkspace{};
}

} // namespace dmc::rengine::stageops
