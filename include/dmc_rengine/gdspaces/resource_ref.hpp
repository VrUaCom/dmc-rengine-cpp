#pragma once

#include "dmc_rengine/gdspaces/resource_id.hpp"

#include <cstdint>
#include <string>

namespace dmc::rengine::gdspaces {

struct ResourceRef final {
    ResourceId id;
    std::string display_name;
    std::string format;
    std::string profile;
    bool synthetic_name{false};
    bool container{false};

    // The animation registry's type code for this resource, or -1, and whether
    // this project can read that kind rather than only name it.
    //
    // Carried on the ref rather than recomputed downstream because the
    // classification that produced it saw the bytes; a consumer holding only a
    // name would answer differently for the one kind that is recognized
    // structurally.
    std::int32_t animation_type{-1};
    bool animation_structure_recovered{false};

    [[nodiscard]] bool valid() const noexcept {
        return id.valid() && !display_name.empty();
    }

    friend bool operator==(const ResourceRef&, const ResourceRef&) = default;
};

} // namespace dmc::rengine::gdspaces
