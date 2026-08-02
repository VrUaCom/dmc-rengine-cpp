#include "dmc_rengine/gdspaces/stage_bundle_assembler.hpp"

#include <cassert>
#include <optional>
#include <span>
#include <string>
#include <utility>
#include <vector>

namespace {

[[nodiscard]] dmc::rengine::gdspaces::ResourceRef resource(
    std::string path,
    std::string format) {
    return dmc::rengine::gdspaces::ResourceRef{
        .id = dmc::rengine::gdspaces::ResourceId{
            .source_id = "synthetic-stage",
            .logical_path = std::move(path),
            .container_chain = {},
            .offset = 0,
            .size = 16,
        },
        .display_name = "member",
        .format = std::move(format),
        .profile = "dmc3-hd",
        .synthetic_name = false,
        .container = false,
    };
}

} // namespace

int main() {
    using dmc::rengine::gdspaces::StageBundleAssembler;
    using dmc::rengine::gdspaces::StageIdentity;
    using dmc::rengine::gdspaces::StageMemberCandidate;
    using dmc::rengine::gdspaces::StageResourceCategory;

    const auto script = resource("scr/st001_004.txt", "txt");
    const auto model = resource("room/st001/model.scm", "scm");
    const auto texture = resource("room/st001/texture.dds", "dds");
    const auto collision = resource("room/st001/collision.ukn", "hits");
    const auto effect = resource("room/st001_effect_000.txt", "txt");
    const auto event = resource("room/st001cfg_008.ukn", "dca");
    const auto unknown = resource("room/st001/data.ukn", "ukn");

    const std::vector<StageMemberCandidate> candidates{
        {.resource = script, .category = std::nullopt, .role = "main-script"},
        {.resource = model, .category = std::nullopt, .role = {}},
        {.resource = texture, .category = std::nullopt, .role = {}},
        {.resource = collision, .category = std::nullopt, .role = {}},
        {.resource = effect, .category = std::nullopt, .role = {}},
        {.resource = event, .category = StageResourceCategory::events, .role = "event-data"},
        {.resource = unknown, .category = std::nullopt, .role = {}},
        {.resource = script, .category = std::nullopt, .role = "duplicate"},
    };

    const auto bundle = StageBundleAssembler::assemble(
        StageIdentity{
            .profile = "dmc3-hd",
            .stage_id = "st001",
            .display_name = "Stage 001",
            .exe_evidence_id = "claim-stage-table-layout",
        },
        std::span<const StageMemberCandidate>{candidates});

    assert(bundle.valid());
    assert(bundle.size() == 7U);
    assert(bundle.members(StageResourceCategory::scripts).size() == 1U);
    assert(bundle.members(StageResourceCategory::models).size() == 1U);
    assert(bundle.members(StageResourceCategory::textures).size() == 1U);
    assert(bundle.members(StageResourceCategory::collision).size() == 1U);
    assert(bundle.members(StageResourceCategory::effects).size() == 1U);
    assert(bundle.members(StageResourceCategory::events).size() == 1U);
    assert(bundle.members(StageResourceCategory::unknown).size() == 1U);
    assert(bundle.diagnostics().size() == 1U);

    const std::vector<StageMemberCandidate> invalid_candidates{
        StageMemberCandidate{
            .resource = {},
            .category = std::nullopt,
            .role = {},
        },
    };
    const auto invalid = StageBundleAssembler::assemble(
        StageIdentity{
            .profile = "dmc3-hd",
            .stage_id = "st001",
            .display_name = "Stage 001",
            .exe_evidence_id = {},
        },
        std::span<const StageMemberCandidate>{invalid_candidates});
    assert(!invalid.valid());

    return 0;
}
