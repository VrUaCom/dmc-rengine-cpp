#include "dmc_rengine/gdspaces/source.hpp"

#include "dmc_rengine/gdspaces/resource_path_normalizer.hpp"

#include <string_view>
#include <utility>
#include <vector>

namespace dmc::rengine::gdspaces {

std::vector<ResourceRef> ISource::lookup(
    std::string_view provider_key,
    std::uint32_t normalization_flags) const {
    std::vector<ResourceRef> matches;

    // Product boundary: callers must supply one canonical, NUL-free provider
    // key. Treating a noncanonical key as a normal miss would hide a key-domain
    // mismatch between planner/provider layers.
    if (provider_key.empty() ||
        !ResourcePathNormalizer::c_string_compatible(provider_key) ||
        ResourcePathNormalizer::normalize(provider_key, normalization_flags) !=
            provider_key) {
        return matches;
    }

    auto resources = enumerate();
    for (auto& resource : resources) {
        if (!resource.valid() ||
            !ResourcePathNormalizer::c_string_compatible(
                resource.id.logical_path)) {
            continue;
        }

        if (ResourcePathNormalizer::normalize(
                resource.id.logical_path,
                normalization_flags) == provider_key) {
            matches.push_back(std::move(resource));
        }
    }
    return matches;
}

} // namespace dmc::rengine::gdspaces
