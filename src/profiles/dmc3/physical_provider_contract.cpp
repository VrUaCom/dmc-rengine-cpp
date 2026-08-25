#include "dmc_rengine/profiles/dmc3/physical_provider_contract.hpp"

namespace dmc::rengine::profiles::dmc3 {
namespace {

static_assert(PhysicalProviderContract::normalization_flags == 0x0CU);
static_assert(PhysicalProviderContract::path_capacity == 0x400U);
static_assert(PhysicalProviderContract::desired_access == 0x80000000U);
static_assert(PhysicalProviderContract::share_mode == 1U);
static_assert(PhysicalProviderContract::creation_disposition == 3U);
static_assert(PhysicalProviderContract::flags_and_attributes == 0U);

static_assert(PhysicalProviderContract::is_open_miss_error(2U));
static_assert(PhysicalProviderContract::is_open_miss_error(3U));
static_assert(!PhysicalProviderContract::is_open_miss_error(18U));
static_assert(!PhysicalProviderContract::is_open_miss_error(5U));

static_assert(PhysicalProviderContract::is_exists_miss_error(2U));
static_assert(PhysicalProviderContract::is_exists_miss_error(3U));
static_assert(PhysicalProviderContract::is_exists_miss_error(18U));
static_assert(!PhysicalProviderContract::is_exists_miss_error(5U));

static_assert(PhysicalProviderContract::root_has_join_separator('/'));
static_assert(PhysicalProviderContract::root_has_join_separator(':'));
static_assert(PhysicalProviderContract::root_has_join_separator('\\'));
static_assert(!PhysicalProviderContract::root_has_join_separator('x'));

} // namespace
} // namespace dmc::rengine::profiles::dmc3
