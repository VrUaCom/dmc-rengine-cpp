#include "dmc_rengine/profiles/dmc3/resource_lookup_policy.hpp"

#include <cassert>
#include <cstddef>
#include <string>

int main() {
    using dmc::rengine::profiles::dmc3::ResourceLookupPolicy;
    using dmc::rengine::profiles::dmc3::ResourceProviderClass;

    const auto& prefixes = ResourceLookupPolicy::namespace_prefixes();
    assert(prefixes.size() == 6U);
    assert(prefixes[0] == "GDataX360.afs/");
    assert(prefixes[1] == "GData.afs/");
    assert(prefixes[2] == "Video/");
    assert(prefixes[3] == "afs/sound/");
    assert(prefixes[4] == "SAVEDATA/");
    assert(prefixes[5].empty());

    assert(ResourceLookupPolicy::basename_of("scr\\st001.pac") == "st001.pac");
    assert(ResourceLookupPolicy::basename_of("room/st001cfg.pac") == "st001cfg.pac");
    assert(ResourceLookupPolicy::basename_of("st001.pac") == "st001.pac");
    assert(ResourceLookupPolicy::basename_of("room/").empty());

    const auto plan = ResourceLookupPolicy::plan("folder/sub\\st001.pac");
    assert(plan.valid());
    assert(plan.original_request == "folder/sub\\st001.pac");
    assert(plan.basename == "st001.pac");
    assert(plan.attempts.size() == 12U);

    for (std::size_t index = 0U; index < 6U; ++index) {
        const auto& attempt = plan.attempts[index];
        assert(attempt.attempt_index == index);
        assert(attempt.provider == ResourceProviderClass::archive);
        assert(attempt.provider_mask == 1U);
        assert(attempt.prefix_index == index);
        assert(attempt.prefix == prefixes[index]);
        assert(attempt.candidate == std::string{prefixes[index]} + "st001.pac");
    }
    for (std::size_t index = 0U; index < 6U; ++index) {
        const auto& attempt = plan.attempts[index + 6U];
        assert(attempt.attempt_index == index + 6U);
        assert(attempt.provider == ResourceProviderClass::physical);
        assert(attempt.provider_mask == 2U);
        assert(attempt.prefix_index == index);
        assert(attempt.prefix == prefixes[index]);
        assert(attempt.candidate == std::string{prefixes[index]} + "st001.pac");
    }

    // Candidate construction preserves request basename bytes. Provider key
    // normalization is a later stage, not part of this planner.
    const auto mixed_case = ResourceLookupPolicy::plan("ROOM\\St001CFG.PAC");
    assert(mixed_case.valid());
    assert(mixed_case.basename == "St001CFG.PAC");
    assert(mixed_case.attempts[0].candidate == "GDataX360.afs/St001CFG.PAC");

    const auto empty = ResourceLookupPolicy::plan("");
    assert(!empty.valid());
    assert(empty.attempts.empty());

    const std::string embedded_nul{"room/st001\0evil.pac", 19U};
    const auto nul_rejected = ResourceLookupPolicy::plan(embedded_nul);
    assert(!nul_rejected.valid());
    assert(nul_rejected.attempts.empty());
    assert(ResourceLookupPolicy::basename_of(embedded_nul).empty());

    // A stale/forged request cannot remain valid merely because the stored
    // attempts are still internally self-consistent.
    auto forged_request = plan;
    forged_request.original_request = "other/st001cfg.pac";
    assert(!forged_request.valid());

    const std::string too_long(
        ResourceLookupPolicy::candidate_buffer_bytes,
        'x');
    const auto rejected = ResourceLookupPolicy::plan(too_long);
    assert(!rejected.valid());
    assert(rejected.attempts.empty());

    // Complete-or-invalid overflow handling is a conservative GDSpaces product
    // rule. The 0x400 destination bound is recovered; exact original behavior
    // after one oversized candidate remains unclaimed.
    const std::string prefix_overflow(
        ResourceLookupPolicy::candidate_buffer_bytes - 4U,
        'y');
    const auto prefix_rejected = ResourceLookupPolicy::plan(prefix_overflow);
    assert(!prefix_rejected.valid());
    assert(prefix_rejected.attempts.empty());

    return 0;
}
