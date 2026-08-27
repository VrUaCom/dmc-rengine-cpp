#pragma once

#include "dmc_rengine/gdspaces/index_name_overlay.hpp"

#include <optional>

namespace dmc::rengine::profiles::dmc3 {

[[nodiscard]] std::optional<gdspaces::IndexProfileDisplaySemantic>
resolve_index_display_semantic(
    const gdspaces::ResourcePayload& child,
    const gdspaces::IndexSlotNameAuthority& authority);

} // namespace dmc::rengine::profiles::dmc3
