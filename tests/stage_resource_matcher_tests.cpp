#include "dmc_rengine/gdspaces/stage_bundle_assembler.hpp"
#include "dmc_rengine/profiles/dmc3/stage_resources.hpp"

#include <cassert>
#include <span>
#include <string>
#include <utility>
#include <vector>

namespace {

[[nodiscard]] dmc::rengine::gdspaces::ResourceRef resource(
    std::string source,
    std::string path) {
    return dmc::rengine::gdspaces::ResourceRef{
        .id = dmc::rengine::gdspaces::ResourceId{
            .source_id = std::move(source),
            .logical_path = std::move(path),
            .container_chain = {},
            .offset = 0,
            .size = 16,
        },
        .display_name = "stage-parent.pac",
        .format = "pac",
        .profile = "dmc3-hd",
        .synthetic_name = false,
        .container = true,
    };
}

} // namespace

int main() {
    using dmc::rengine::gdspaces::StageBundleAssembler;
    using dmc::rengine::gdspaces::StageIdentity;
    using dmc::rengine::gdspaces::StageResourceCategory;
    using dmc::rengine::profiles::dmc3::StageResourceMatcher;
    using dmc::rengine::profiles::dmc3::phase12_st001_resource_plan;

    const auto& plan = phase12_st001_resource_plan();
    assert(plan.valid());
    assert(plan.stage_id == "st001");

    const std::vector<dmc::rengine::gdspaces::ResourceRef> resources{
        resource("game", "DMC3/SCR/ST001.PAC"),
        resource("game", "data\\room\\st001cfg.pac"),
        resource("game", "room/st001_effect.pac"),
        resource("game", "se/snd_r001.pac"),
        resource("game", "room/unrelated.pac"),
    };

    const auto report = StageResourceMatcher::match(
        plan, std::span<const dmc::rengine::gdspaces::ResourceRef>{resources});
    assert(report.complete());
    assert(report.roles.size() == 4U);
    assert(report.diagnostics.empty());

    const auto candidates = report.unique_candidates();
    assert(candidates.size() == 4U);
    assert(candidates[0].category == StageResourceCategory::scripts);
    assert(candidates[1].category == StageResourceCategory::unknown);
    assert(candidates[2].category == StageResourceCategory::effects);
    assert(candidates[3].category == StageResourceCategory::sounds);

    const auto bundle = StageBundleAssembler::assemble(
        StageIdentity{
            .profile = "dmc3-hd",
            .stage_id = plan.stage_id,
            .display_name = "Stage 001",
            .exe_evidence_id = plan.evidence_id,
        },
        std::span<const dmc::rengine::gdspaces::StageMemberCandidate>{candidates});
    assert(bundle.valid());
    assert(bundle.size() == 4U);
    assert(bundle.members(StageResourceCategory::scripts).size() == 1U);
    assert(bundle.members(StageResourceCategory::effects).size() == 1U);
    assert(bundle.members(StageResourceCategory::sounds).size() == 1U);

    auto ambiguous_resources = resources;
    ambiguous_resources.push_back(resource("second-source", "scr/st001.pac"));
    const auto ambiguous = StageResourceMatcher::match(
        plan,
        std::span<const dmc::rengine::gdspaces::ResourceRef>{ambiguous_resources});
    assert(!ambiguous.complete());
    assert(ambiguous.unique_candidates().size() == 3U);

    const std::vector<dmc::rengine::gdspaces::ResourceRef> partial_resources{
        resources[0],
        resources[2],
    };
    const auto partial = StageResourceMatcher::match(
        plan,
        std::span<const dmc::rengine::gdspaces::ResourceRef>{partial_resources});
    assert(!partial.complete());
    assert(partial.unique_candidates().size() == 2U);

    return 0;
}
