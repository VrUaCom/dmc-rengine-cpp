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

    if (provider_key.empty() ||
        !ResourcePathNormalizer::c_string_compatible(provider_key) ||
        ResourcePathNormalizer::normalize(provider_key, normalization_flags) !=
            provider_key) {
        return matches;
    }

    auto resources = enumerate();
    for (auto& resource : resources) {
        const auto& logical_path = resource.id.logical_path;
        if (!resource.valid() ||
            !ResourcePathNormalizer::c_string_compatible(logical_path)) {
            continue;
        }

        if (ResourcePathNormalizer::normalize(
                logical_path, normalization_flags) == provider_key) {
            matches.push_back(std::move(resource));
        }
    }
    return matches;
}

} // namespace dmc::rengine::gdspaces
