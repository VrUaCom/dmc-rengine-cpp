#pragma once

#include "dmc_rengine/gdspaces/resource_payload.hpp"
#include "dmc_rengine/gdspaces/resource_ref.hpp"

#include <optional>
#include <string_view>
#include <vector>

namespace dmc::rengine::gdspaces {

class ISource {
public:
    virtual ~ISource() = default;

    [[nodiscard]] virtual std::string_view id() const noexcept = 0;
    [[nodiscard]] virtual std::string_view kind() const noexcept = 0;
    [[nodiscard]] virtual std::vector<ResourceRef> enumerate() const = 0;
    [[nodiscard]] virtual std::optional<ResourcePayload> read(
        const ResourceId& resource) const = 0;
};

} // namespace dmc::rengine::gdspaces
