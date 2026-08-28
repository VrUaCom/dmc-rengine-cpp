#pragma once

#include "dmc_rengine/gdspaces/container_expander.hpp"

#include <vector>

namespace dmc::rengine::gdspaces {

struct ContainerNamingReconcileResult final {
    bool reconciled{false};
    bool embedded_name_list_applied{false};
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
        const ResourcePayload* external_index = nullptr);
};

} // namespace dmc::rengine::gdspaces
