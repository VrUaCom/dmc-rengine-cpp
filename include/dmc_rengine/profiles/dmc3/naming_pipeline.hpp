#pragma once

#include "dmc_rengine/gdspaces/container_naming_reconciler.hpp"
#include "dmc_rengine/gdspaces/resource_naming_identity.hpp"
#include "dmc_rengine/gdspaces/source.hpp"
#include "dmc_rengine/profiles/dmc3/companion_index_locator.hpp"

#include <optional>
#include <vector>

namespace dmc::rengine::profiles::dmc3 {

struct Dmc3NamingPipelineResult final {
    bool reconciled{false};
    bool explicit_external_index_used{false};
    bool companion_index_discovered{false};
    std::optional<CompanionIndexCandidateKind> companion_kind;
    std::optional<gdspaces::ContainerNamingIdentitySnapshot> snapshot;
    std::vector<gdspaces::Diagnostic> diagnostics;

    [[nodiscard]] bool ok() const noexcept;
};

// One DMC3-specific Layer-1 naming entrypoint over an already materialized
// PAC/PNST expansion. The pipeline owns orchestration only; physical identity
// and bytes remain owned by the expansion/materialization layer.
//
// Authority order:
// 1) an explicit external_index, when supplied, is used exactly as provided;
// 2) otherwise an optional companion_source may resolve one exact physical-path
//    companion .index via CompanionIndexLocator;
// 3) embedded aliases and DMC3 structural semantics are reconciled by the
//    canonical ContainerNamingReconciler;
// 4) a read-only unified naming snapshot is built from the reconciled result.
//
// Explicit external evidence intentionally outranks companion discovery so a
// retained extraction corpus may name an NBZ-materialized PAC even when the
// two authorities have different source_ids. Auto-discovery never crosses a
// source boundary.
class Dmc3NamingPipeline final {
public:
    [[nodiscard]] static Dmc3NamingPipelineResult apply(
        gdspaces::ContainerExpansion& expansion,
        const gdspaces::ResourcePayload* external_index = nullptr,
        const gdspaces::ISource* companion_source = nullptr);
};

} // namespace dmc::rengine::profiles::dmc3
