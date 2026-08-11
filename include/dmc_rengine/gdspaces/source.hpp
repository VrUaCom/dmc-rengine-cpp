#pragma once

#include "dmc_rengine/gdspaces/resource_payload.hpp"
#include "dmc_rengine/gdspaces/resource_ref.hpp"

#include <cstdint>
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

    // Return every source resource whose logical path normalizes to the
    // already-normalized provider key under the caller-supplied runtime flags.
    // This intentionally does not choose among duplicate normalized keys.
    [[nodiscard]] virtual std::vector<ResourceRef> lookup(
        std::string_view provider_key,
        std::uint32_t normalization_flags) const;

    [[nodiscard]] virtual std::optional<ResourcePayload> read(
        const ResourceId& resource) const = 0;
};

} // namespace dmc::rengine::gdspaces
