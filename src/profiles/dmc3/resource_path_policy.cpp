#include "dmc_rengine/profiles/dmc3/resource_path_policy.hpp"

#include "dmc_rengine/gdspaces/resource_path_normalizer.hpp"

namespace dmc::rengine::profiles::dmc3 {

std::string ResourcePathPolicy::physical(std::string_view path) {
    return gdspaces::ResourcePathNormalizer::normalize(path, physical_flags);
}

std::string ResourcePathPolicy::archive(std::string_view path) {
    return gdspaces::ResourcePathNormalizer::normalize(path, archive_flags);
}

} // namespace dmc::rengine::profiles::dmc3
