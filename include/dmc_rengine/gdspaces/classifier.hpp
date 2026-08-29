#pragma once

#include "dmc_rengine/gdspaces/profile.hpp"

#include <cstddef>
#include <span>
#include <string>
#include <string_view>

namespace dmc::rengine::gdspaces {

struct ResourceClassification final {
    std::string format{"unknown"};
    GameProfile profile{GameProfile::unknown};
    bool container{false};
    bool magic_confirmed{false};
    bool logical_namespace{false};
};

class ResourceClassifier final {
public:
    [[nodiscard]] static ResourceClassification classify(
        std::string_view logical_path,
        std::span<const std::byte> bytes = {});

    [[nodiscard]] static GameProfile profile_from_path(
        std::string_view logical_path);

    [[nodiscard]] static bool is_container_format(
        std::string_view format) noexcept;
};

} // namespace dmc::rengine::gdspaces
