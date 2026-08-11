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
    if (provider_key.empty()) {
        return matches;
    }

    auto resources = enumerate();
    for (auto& resource : resources) {
        if (ResourcePathNormalizer::normalize(
                resource.id.logical_path,
                normalization_flags) == provider_key) {
            matches.push_back(std::move(resource));
        }
    }
    return matches;
}

} // namespace dmc::rengine::gdspaces
