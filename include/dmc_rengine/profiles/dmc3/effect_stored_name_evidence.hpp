#pragma once

#include "dmc_rengine/gdspaces/container_expander.hpp"

#include <vector>

namespace dmc::rengine::profiles::dmc3 {

struct EffectStoredNameApplyResult final {
    bool applicable{false};
    bool applied{false};
    std::vector<gdspaces::Diagnostic> diagnostics;

    [[nodiscard]] bool ok() const noexcept;
};

// Bind names stored in a physical `*_effect.pac` enclosing container to the
// exact materialized children of its records PNST. Binding is transactional and
// fail-closed. It does not mutate ResourceId, bytes, slot indices, provenance,
// write targets, or topology.
class EffectStoredNameEvidenceBuilder final {
public:
    [[nodiscard]] static EffectStoredNameApplyResult apply(
        const gdspaces::ResourcePayload& enclosing_container,
        gdspaces::ContainerExpansion& records_expansion);
};

} // namespace dmc::rengine::profiles::dmc3
