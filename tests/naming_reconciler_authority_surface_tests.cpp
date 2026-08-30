#include "dmc_rengine/gdspaces/container_naming_reconciler.hpp"

#include <concepts>

namespace {

namespace gdspaces = dmc::rengine::gdspaces;

template <typename Reconciler>
concept PublicResolverInjectionSurface = requires(
    gdspaces::ContainerExpansion& expansion,
    const gdspaces::ResourcePayload* index,
    gdspaces::IndexProfileDisplayResolver resolver) {
    {
        Reconciler::reconcile(expansion, index, resolver)
    } -> std::same_as<gdspaces::ContainerNamingReconcileResult>;
};

template <typename Reconciler>
concept PublicProfiledReconcileSurface = requires(
    gdspaces::ContainerExpansion& expansion,
    const gdspaces::ResourcePayload* index,
    gdspaces::IndexProfileDisplayResolver resolver) {
    {
        Reconciler::reconcile_profiled(expansion, index, resolver)
    } -> std::same_as<gdspaces::ContainerNamingReconcileResult>;
};

// Profile semantic callbacks are interpretation code, not evidence by
// themselves. Neither public entry point may accept one: only the friended
// Dmc3NamingPipeline may enter the private profiled reconciliation path.
static_assert(
    !PublicResolverInjectionSurface<gdspaces::ContainerNamingReconciler>);
static_assert(
    !PublicProfiledReconcileSurface<gdspaces::ContainerNamingReconciler>);

} // namespace

int main() {
    return 0;
}
