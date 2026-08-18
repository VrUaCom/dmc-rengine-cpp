#include "dmc_rengine/profiles/dmc3/resource_lookup_policy.hpp"

#include "dmc_rengine/gdspaces/resource_path_normalizer.hpp"

#include <array>
#include <string>
#include <utility>

namespace dmc::rengine::profiles::dmc3 {
namespace {

constexpr std::array<std::string_view, 6> prefixes{
    "GDataX360.afs/",
    "GData.afs/",
    "Video/",
    "afs/sound/",
    "SAVEDATA/",
    "",
};

[[nodiscard]] bool candidate_fits(std::string_view value) noexcept {
    // OpenGameResource constructs candidates in a 0x400-byte destination.
    // Keep one byte available for the terminating NUL. Treating any overflow
    // as a complete-plan failure is conservative GDSpaces product behavior;
    // exact original per-candidate overflow continuation remains unclaimed.
    return value.size() < ResourceLookupPolicy::candidate_buffer_bytes;
}

[[nodiscard]] bool c_string_compatible(std::string_view value) noexcept {
    return gdspaces::ResourcePathNormalizer::c_string_compatible(value);
}

} // namespace

bool ResourceLookupAttempt::valid() const noexcept {
    if (attempt_index >= 12U || prefix_index >= prefixes.size() ||
        basename.empty() || candidate.empty() || !candidate_fits(candidate) ||
        !c_string_compatible(prefix) || !c_string_compatible(basename) ||
        !c_string_compatible(candidate)) {
        return false;
    }

    switch (provider) {
    case ResourceProviderClass::archive:
        return provider_mask == 1U;
    case ResourceProviderClass::physical:
        return provider_mask == 2U;
    }
    return false;
}

bool ResourceLookupPlan::valid() const noexcept {
    if (original_request.empty() || basename.empty() || attempts.size() != 12U ||
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
        const auto expected_mask = index < prefixes.size() ? 1U : 2U;
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

const std::array<std::string_view, 6>&
ResourceLookupPolicy::namespace_prefixes() noexcept {
    return prefixes;
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

    result.attempts.reserve(12U);
    std::size_t attempt_index = 0U;
    for (const auto provider : {
             ResourceProviderClass::archive,
             ResourceProviderClass::physical}) {
        const auto provider_mask =
            provider == ResourceProviderClass::archive ? 1U : 2U;
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
