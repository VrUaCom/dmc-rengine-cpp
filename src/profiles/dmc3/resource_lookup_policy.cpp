#include "dmc_rengine/profiles/dmc3/resource_lookup_policy.hpp"

#include "dmc_rengine/gdspaces/resource_path_normalizer.hpp"
#include "dmc_rengine/profiles/dmc3/open_game_resource_contract.hpp"

#include <array>
#include <string>
#include <utility>

namespace dmc::rengine::profiles::dmc3 {
namespace {

// The recovered prefix table has one home, in the contract that recovered it.
constexpr const auto& prefixes = OpenGameResourceContract::namespace_prefixes;

[[nodiscard]] bool candidate_fits(std::string_view value) noexcept {
    // OpenGameResource builds the active candidate with the recovered bounded
    // join helper and a 0x400-byte destination. The helper keeps one byte for
    // the terminating NUL. On failure OpenGameResource destroys the newly
    // allocated file slot/object and returns -1 immediately; it does not skip
    // to a shorter prefix. Because GDataX360.afs/ is the first and longest
    // recovered prefix, plan-wide fail-closed validation is equivalent for the
    // canonical direct-call mode (all recovered direct callers pass flags=1).
    return value.size() < ResourceLookupPolicy::candidate_buffer_bytes;
}

[[nodiscard]] bool c_string_compatible(std::string_view value) noexcept {
    return gdspaces::ResourcePathNormalizer::c_string_compatible(value);
}

} // namespace

bool ResourceLookupAttempt::valid() const noexcept {
    if (attempt_index >= ResourceLookupPolicy::attempts_per_request ||
        prefix_index >= prefixes.size() ||
        basename.empty() || candidate.empty() || !candidate_fits(candidate) ||
        !c_string_compatible(prefix) || !c_string_compatible(basename) ||
        !c_string_compatible(candidate)) {
        return false;
    }

    switch (provider) {
    case ResourceProviderClass::archive:
        return provider_mask == OpenGameResourceContract::archive_provider_mask;
    case ResourceProviderClass::physical:
        return provider_mask == OpenGameResourceContract::physical_provider_mask;
    }
    return false;
}

bool ResourceLookupPlan::valid() const noexcept {
    if (original_request.empty() || basename.empty() ||
        attempts.size() != ResourceLookupPolicy::attempts_per_request ||
        !c_string_compatible(original_request) ||
        basename != ResourceLookupPolicy::basename_of(original_request)) {
        return false;
    }

    for (std::size_t index = 0U; index < attempts.size(); ++index) {
        if (!attempts[index].valid() || attempts[index].attempt_index != index) {
            return false;
        }

        const auto expected_provider = index < prefixes.size()
            ? ResourceProviderClass::archive
            : ResourceProviderClass::physical;
        const auto expected_mask =
            OpenGameResourceContract::provider_mask_for_pass(
                index / prefixes.size());
        const auto expected_prefix_index = index % prefixes.size();
        if (attempts[index].provider != expected_provider ||
            attempts[index].provider_mask != expected_mask ||
            attempts[index].prefix_index != expected_prefix_index ||
            attempts[index].prefix != prefixes[expected_prefix_index] ||
            attempts[index].basename != basename ||
            attempts[index].candidate != attempts[index].prefix + basename) {
            return false;
        }
    }
    return true;
}

std::string ResourceLookupPolicy::basename_of(std::string_view logical_path) {
    if (logical_path.empty() || !c_string_compatible(logical_path)) {
        return {};
    }

    const auto slash = logical_path.find_last_of("/\\");
    if (slash == std::string_view::npos) {
        return std::string{logical_path};
    }
    if (slash + 1U >= logical_path.size()) {
        return {};
    }
    return std::string{logical_path.substr(slash + 1U)};
}

ResourceLookupPlan ResourceLookupPolicy::plan(std::string_view logical_path) {
    ResourceLookupPlan result{
        .original_request = std::string{logical_path},
        .basename = basename_of(logical_path),
        .attempts = {},
    };

    if (result.basename.empty() || !c_string_compatible(logical_path)) {
        return result;
    }

    result.attempts.reserve(ResourceLookupPolicy::attempts_per_request);
    std::size_t attempt_index = 0U;
    // One complete pass per provider, in the recovered order: archive first,
    // then physical. The pass index is what selects the mask, so the two can
    // never disagree the way two parallel literals could.
    for (std::size_t pass = 0U;
         pass < OpenGameResourceContract::provider_passes;
         ++pass) {
        const auto provider = pass == 0U
            ? ResourceProviderClass::archive
            : ResourceProviderClass::physical;
        const auto provider_mask =
            OpenGameResourceContract::provider_mask_for_pass(pass);
        for (std::size_t prefix_index = 0U;
             prefix_index < prefixes.size();
             ++prefix_index) {
            std::string candidate{prefixes[prefix_index]};
            candidate += result.basename;
            if (!candidate_fits(candidate)) {
                result.attempts.clear();
                return result;
            }

            result.attempts.push_back(ResourceLookupAttempt{
                .attempt_index = attempt_index++,
                .provider = provider,
                .provider_mask = provider_mask,
                .prefix_index = prefix_index,
                .prefix = std::string{prefixes[prefix_index]},
                .basename = result.basename,
                .candidate = std::move(candidate),
            });
        }
    }
    return result;
}

} // namespace dmc::rengine::profiles::dmc3
