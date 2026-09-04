#include "dmc_rengine/profiles/dmc3/resource_bootstrap_contract.hpp"

#include "dmc_rengine/profiles/dmc3/volume_bootstrap_policy.hpp"

#include <limits>

namespace dmc::rengine::profiles::dmc3 {

static_assert(Dmc3ResourceBootstrapContract::image_base == 0x140000000ULL);
static_assert(
    Dmc3ResourceBootstrapContract::canonical_target_sha256.size() == 64U);
static_assert(Dmc3ResourceBootstrapContract::rva_of(
    Dmc3ResourceBootstrapContract::bootstrap_va) == 0x0002E930U);

static_assert(Dmc3ResourceBootstrapContract::first_probe_index == 0U);
static_assert(Dmc3ResourceBootstrapContract::stops_at_first_gap);
static_assert(Dmc3ResourceBootstrapContract::physical_root_registered_first);
static_assert(Dmc3ResourceBootstrapContract::mount_list_is_prepend);

// The signed-decimal domain, stated as the relationship it actually is rather
// than as a magic constant that happens to equal it.
static_assert(
    Dmc3ResourceBootstrapContract::runtime_index_max ==
    static_cast<std::uint32_t>(std::numeric_limits<std::int32_t>::max()));
static_assert(Dmc3ResourceBootstrapContract::in_runtime_index_domain(0U));
static_assert(Dmc3ResourceBootstrapContract::in_runtime_index_domain(
    Dmc3ResourceBootstrapContract::runtime_index_max));
static_assert(!Dmc3ResourceBootstrapContract::in_runtime_index_domain(
    Dmc3ResourceBootstrapContract::runtime_index_max + 1U));
static_assert(!Dmc3ResourceBootstrapContract::in_runtime_index_domain(
    std::numeric_limits<std::uint32_t>::max()));

// Prepending inverts registration: with three contiguous volumes the last
// registered is consulted first.
static_assert(Dmc3ResourceBootstrapContract::resolution_rank(2U, 3U) == 0U);
static_assert(Dmc3ResourceBootstrapContract::resolution_rank(1U, 3U) == 1U);
static_assert(Dmc3ResourceBootstrapContract::resolution_rank(0U, 3U) == 2U);
static_assert(Dmc3ResourceBootstrapContract::resolution_rank(0U, 1U) == 0U);

// The product policy reads these from the contract rather than restating them.
static_assert(
    VolumeBootstrapPolicy::executable_data_suffix() ==
    Dmc3ResourceBootstrapContract::executable_data_suffix);
static_assert(
    VolumeBootstrapPolicy::volume_format() ==
    Dmc3ResourceBootstrapContract::volume_format);
static_assert(
    VolumeBootstrapPolicy::runtime_index_max() ==
    Dmc3ResourceBootstrapContract::runtime_index_max);

} // namespace dmc::rengine::profiles::dmc3
