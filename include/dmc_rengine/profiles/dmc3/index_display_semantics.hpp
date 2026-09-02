#pragma once

#include "dmc_rengine/gdspaces/container_naming_reconciler.hpp"
#include "dmc_rengine/gdspaces/index_name_overlay.hpp"

#include <optional>

namespace dmc::rengine::profiles::dmc3 {

// Profile-specific structural semantics for an already materialized DMC3
// payload. This is independent of `.index`: the same byte observation can be
// used when historical extraction metadata is missing.
[[nodiscard]] std::optional<gdspaces::ResourceProfileSemantic>
resolve_materialized_display_semantic(
    const gdspaces::ResourcePayload& child);

// Compatibility adapter for the historical `.index` overlay path. The index
// authority supplies the stem/ordinal; the semantic answer still comes only
// from the materialized payload bytes.
[[nodiscard]] std::optional<gdspaces::IndexProfileDisplaySemantic>
resolve_index_display_semantic(
    const gdspaces::ResourcePayload& child,
    const gdspaces::IndexSlotNameAuthority& authority);

} // namespace dmc::rengine::profiles::dmc3
