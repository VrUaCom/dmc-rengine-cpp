#pragma once

#include "dmc_rengine/stageops/domain_knowledge.hpp"

#include <cstddef>
#include <cstdint>
#include <string>
#include <string_view>
#include <vector>

namespace dmc::rengine::stageops {

struct StageResourceInvalidationImpact final {
    std::string resource_id;
    std::vector<std::string> domain_object_ids;
    std::vector<std::string> domain_relation_ids;
    std::vector<std::string> runtime_link_ids;

    [[nodiscard]] bool valid() const noexcept {
        return !resource_id.empty();
    }
};

struct StageInvalidationReport final {
    StageAssemblyIdentity identity;
    std::uint64_t source_stage_revision{};
    std::vector<StageResourceInvalidationImpact> resources;
    std::size_t affected_domain_object_count{};
    std::size_t affected_domain_relation_count{};
    std::size_t affected_runtime_link_count{};

    [[nodiscard]] bool valid() const noexcept {
        return identity.valid();
    }
};

// Computes only dependencies already explicit in Stage Ops knowledge:
//
// resource -> domain object -> structural relation/runtime link.
//
// It does not infer cross-resource gameplay dependencies. Those are added later
// only when Stage Ops owns an evidence-backed dependency relation.
class StageInvalidationAnalyzer final {
public:
    [[nodiscard]] static StageInvalidationReport analyze(
        const StageDomainKnowledgeWorkspace& knowledge,
        std::vector<std::string> changed_resource_ids);
};

} // namespace dmc::rengine::stageops
