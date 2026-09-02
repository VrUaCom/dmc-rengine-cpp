#pragma once

#include "dmc_rengine/gdspaces/container_expander.hpp"
#include "dmc_rengine/gdspaces/index_name_overlay.hpp"

#include <cstdint>
#include <optional>
#include <string>
#include <vector>

namespace dmc::rengine::profiles::dmc3 {
class Dmc3NamingPipeline;
}

namespace dmc::rengine::gdspaces {

enum class ResourceProfileSemanticKind : std::uint8_t {
    structural_format,
    runtime_content_tag,
    runtime_family_mask_tag,
};

// A byte/structure-backed profile interpretation that does not depend on an
// external extraction name. Constructing this value alone grants no authority:
// only the canonical DMC3 naming pipeline may ask the reconciler to seal it.
// The observation kind is retained so instruction-backed runtime evidence is
// not mislabeled as structural-parser proof, and so the three-byte registry
// probe remains distinct from the independent four-byte family-mask classifier.
struct ResourceProfileSemantic final {
    std::string canonical_extension;
    std::string semantic_format;
    ResourceProfileSemanticKind evidence_kind{
        ResourceProfileSemanticKind::structural_format};
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
    // Public reconciliation consumes only authorities already present in the
    // materialized bytes / exact external index. It deliberately accepts no
    // profile resolver: arbitrary callers must not be able to feed semantic
    // strings into the trusted sealing path.
    [[nodiscard]] static ContainerNamingReconcileResult reconcile(
        ContainerExpansion& expansion,
        const ResourcePayload* external_index = nullptr);

private:
    // Profile semantic sealing is deliberately not public. A public resolver
    // callback would let arbitrary callers ask this trusted reconciler to turn
    // an invented semantic string into sealed evidence. Only the canonical
    // DMC3 pipeline owns both profile transitions below.
    friend class ::dmc::rengine::profiles::dmc3::Dmc3NamingPipeline;

    [[nodiscard]] static ContainerNamingReconcileResult reconcile_profiled(
        ContainerExpansion& expansion,
        const ResourcePayload* external_index,
        IndexProfileDisplayResolver profile_resolver);

    [[nodiscard]] static ContainerNamingReconcileResult apply_profile_semantics(
        ContainerExpansion& expansion,
        ResourceProfileSemanticResolver resolver);

    [[nodiscard]] static bool persist_magic_semantics(
        ContainerExpansion& expansion,
        ContainerNamingReconcileResult& result);

    [[nodiscard]] static bool persist_overlay_semantics(
        ContainerExpansion& expansion,
        const IndexNameOverlay& overlay,
        ContainerNamingReconcileResult& result);
};

} // namespace dmc::rengine::gdspaces
