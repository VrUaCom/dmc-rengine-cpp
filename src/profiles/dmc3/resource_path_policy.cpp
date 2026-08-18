#include "dmc_rengine/profiles/dmc3/resource_path_policy.hpp"

#include "dmc_rengine/gdspaces/resource_path_normalizer.hpp"

namespace dmc::rengine::profiles::dmc3 {

bool ResourcePathPolicy::valid_input(std::string_view path) noexcept {
    return gdspaces::ResourcePathNormalizer::c_string_compatible(path);
}

std::string ResourcePathPolicy::physical(std::string_view path) {
    if (!valid_input(path)) {
        return {};
    }
    return gdspaces::ResourcePathNormalizer::normalize(path, physical_flags);
}

std::string ResourcePathPolicy::archive(std::string_view path) {
    if (!valid_input(path)) {
        return {};
    }
    return gdspaces::ResourcePathNormalizer::normalize(path, archive_flags);
}

} // namespace dmc::rengine::profiles::dmc3
