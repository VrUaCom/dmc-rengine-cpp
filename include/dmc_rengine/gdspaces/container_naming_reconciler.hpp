#pragma once

#include "dmc_rengine/gdspaces/container_expander.hpp"
#include "dmc_rengine/gdspaces/index_name_overlay.hpp"

#include <optional>
#include <string>
#include <vector>

namespace dmc::rengine::gdspaces {

// A byte/structure-backed profile interpretation that does not depend on an
// external extraction name. The resolver only describes what the materialized
// payload is; the reconciler remains the sole authority allowed to seal that
// observation into ResourceSemanticEvidence.
struct ResourceProfileSemantic final {
    std::string canonical_extension;
    std::string semantic_format;
};

using ResourceProfileSemanticResolver = std::optional<ResourceProfileSemantic> (*)(
    const ResourcePayload& child);

struct ContainerNamingReconcileResult final {
    bool reconciled{false};
    bool embedded_name_list_applied{false};
    bool magic_semantics_applied{false};
    bool profile_semantics_applied{false};
    bool external_index_applied{false};
    std::vector<Diagnostic> diagnostics;

    [[nodiscard]] bool ok() const noexcept;
};

// Canonical naming/semantic reconciliation for one already-materialized
// PAC/PNST expansion. The operation is transactional: physical identities and
// bytes are snapshotted, evidence is applied to a staged copy, and the caller's
// expansion is replaced only after all supplied authorities validate.
class ContainerNamingReconciler final {
public:
    [[nodiscard]] static ContainerNamingReconcileResult reconcile(
        ContainerExpansion& expansion,
        const ResourcePayload* external_index = nullptr,
        IndexProfileDisplayResolver profile_resolver = nullptr);

    // Adds profile-specific semantic evidence independently of `.index`.
    // This is deliberately a second phase: profile structure may identify a
    // texture bundle even when no historical extraction sidecar exists, but it
    // must never manufacture external-index authority or alter physical state.
    [[nodiscard]] static ContainerNamingReconcileResult apply_profile_semantics(
        ContainerExpansion& expansion,
        ResourceProfileSemanticResolver resolver);

private:
    // Kept as class members because ResourceSemanticEvidence is deliberately
    // non-forgeable outside this reconciler. Free helpers would not inherit the
    // class friendship granted by ResourceSemanticEvidence.
    [[nodiscard]] static bool persist_magic_semantics(
        ContainerExpansion& expansion,
        ContainerNamingReconcileResult& result);

    [[nodiscard]] static bool persist_overlay_semantics(
        ContainerExpansion& expansion,
        const IndexNameOverlay& overlay,
        ContainerNamingReconcileResult& result);
};

} // namespace dmc::rengine::gdspaces
