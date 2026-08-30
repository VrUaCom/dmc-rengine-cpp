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
    bool enclosing_stored_names_applied{false};
    std::optional<CompanionIndexCandidateKind> companion_kind;
    std::optional<gdspaces::ContainerNamingIdentitySnapshot> snapshot;
    std::vector<gdspaces::Diagnostic> diagnostics;

    [[nodiscard]] bool ok() const noexcept;
};

// One DMC3-specific Layer-1 naming entrypoint over an already materialized
// PAC/PNST expansion. The pipeline owns orchestration only; physical identity
// and bytes remain owned by the expansion/materialization layer.
//
// Authority domains remain separate:
// 1) optional enclosing_container: direct stored names for an exact descendant
//    expansion (currently the recovered effect-container convention);
// 2) explicit external_index, when supplied, is used exactly as provided;
// 3) otherwise companion_source may resolve one exact physical-path `.index`;
// 4) embedded aliases and DMC3 structural semantic format evidence are
//    reconciled by ContainerNamingReconciler;
// 5) ResourceNamingIdentityBuilder emits one read-only snapshot that retains
//    each evidence domain without laundering it into physical identity.
//
// Explicit external evidence intentionally outranks companion discovery so a
// retained extraction corpus may name an NBZ-materialized PAC even when the
// two authorities have different source_ids. Auto-discovery never crosses a
// source boundary. Enclosing stored names never act as `.index` candidates.
class Dmc3NamingPipeline final {
public:
    [[nodiscard]] static Dmc3NamingPipelineResult apply(
        gdspaces::ContainerExpansion& expansion,
        const gdspaces::ResourcePayload* external_index = nullptr,
        const gdspaces::ISource* companion_source = nullptr,
        const gdspaces::ResourcePayload* enclosing_container = nullptr);
};

} // namespace dmc::rengine::profiles::dmc3
