#pragma once

#include "dmc_rengine/gdspaces/resource_id.hpp"

#include <string>

namespace dmc::rengine::gdspaces {

struct ResourceRef final {
    ResourceId id;
    std::string display_name;
    std::string format;
    std::string profile;
    bool synthetic_name{false};
    bool container{false};

    [[nodiscard]] bool valid() const noexcept {
        return id.valid() && !display_name.empty();
    }

    friend bool operator==(const ResourceRef&, const ResourceRef&) = default;
};

} // namespace dmc::rengine::gdspaces
