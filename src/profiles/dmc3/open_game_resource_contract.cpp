#include "dmc_rengine/profiles/dmc3/open_game_resource_contract.hpp"

#include "dmc_rengine/profiles/dmc3/resource_lookup_policy.hpp"

#include <algorithm>

namespace dmc::rengine::profiles::dmc3 {

// Every recovered constant is locked here rather than trusted to survive
// editing. A number in a header is a comment; a number a static_assert stands
// on cannot be changed quietly.

static_assert(OpenGameResourceContract::image_base == 0x140000000ULL);
static_assert(OpenGameResourceContract::canonical_target_sha256.size() == 64U);

static_assert(OpenGameResourceContract::rva_of(
    OpenGameResourceContract::open_game_resource_va) == 0x0002FCA0U);
static_assert(OpenGameResourceContract::rva_of(
    OpenGameResourceContract::bounded_join_va) == 0x003272C0U);
static_assert(OpenGameResourceContract::rva_of(
    OpenGameResourceContract::file_slot_release_va) == 0x00048DF0U);

static_assert(OpenGameResourceContract::direct_call_sites.size() == 3U);
static_assert(!OpenGameResourceContract::stored_function_pointer_observed);
static_assert(OpenGameResourceContract::direct_call_flags == 1U);

// Every recovered call site is inside the image the contract is bound to.
static_assert(std::ranges::all_of(
    OpenGameResourceContract::direct_call_sites,
    [](std::uint64_t site) noexcept {
        return site > OpenGameResourceContract::image_base;
    }));

static_assert(OpenGameResourceContract::archive_provider_mask == 1U);
static_assert(OpenGameResourceContract::physical_provider_mask == 2U);
static_assert(OpenGameResourceContract::provider_mask_for_pass(0U) == 1U);
static_assert(OpenGameResourceContract::provider_mask_for_pass(1U) == 2U);

static_assert(OpenGameResourceContract::candidate_buffer_bytes == 0x400U);
static_assert(OpenGameResourceContract::namespace_prefixes.size() == 6U);
static_assert(OpenGameResourceContract::attempts_per_request == 12U);
static_assert(OpenGameResourceContract::miss_return_value == -1);

// The first prefix tried is also the longest, which is why a single overflow
// check against it is equivalent to the recovered abort-on-first-overflow.
static_assert(
    OpenGameResourceContract::namespace_prefixes.front().size() ==
    OpenGameResourceContract::longest_prefix_bytes());
static_assert(OpenGameResourceContract::longest_prefix_bytes() == 14U);
static_assert(OpenGameResourceContract::max_basename_bytes() == 1009U);

// The last prefix is empty: the request is also tried under no namespace.
static_assert(OpenGameResourceContract::namespace_prefixes.back().empty());

// A candidate exactly at the destination size does not fit: the terminating
// NUL needs one of those bytes.
static_assert(OpenGameResourceContract::candidate_fits(
    std::string_view{"x"}));

static_assert(OpenGameResourceOverflowBehavior::releases_file_slot);
static_assert(OpenGameResourceOverflowBehavior::returns_miss);
static_assert(!OpenGameResourceOverflowBehavior::advances_prefix_index);
static_assert(!OpenGameResourceOverflowBehavior::enters_physical_pass);

// The product policy reads its bound from the contract rather than carrying a
// second copy of it. This assertion is what makes that true rather than
// currently-true.
static_assert(
    ResourceLookupPolicy::candidate_buffer_bytes ==
    OpenGameResourceContract::candidate_buffer_bytes);

} // namespace dmc::rengine::profiles::dmc3
