#pragma once

#include "dmc_rengine/gdspaces/stage_bundle.hpp"

#include <optional>
#include <span>
#include <string>

namespace dmc::rengine::gdspaces {

struct StageMemberCandidate final {
    ResourceRef resource;
    std::optional<StageResourceCategory> category;
    std::string role;
};

class StageBundleAssembler final {
public:
    [[nodiscard]] static StageBundle assemble(
        StageIdentity identity,
        std::span<const StageMemberCandidate> candidates);

    [[nodiscard]] static StageResourceCategory infer_category(
        const ResourceRef& resource);
};

} // namespace dmc::rengine::gdspaces
