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

    // Product lookup contract for an already-normalized provider key.
    // Implementations must preserve every matching physical identity and must
    // not invent a winner when multiple source identities normalize equally.
    // The default implementation is a correctness fallback; archive sources
    // may override it with an indexed implementation later.
    [[nodiscard]] virtual std::vector<ResourceRef> lookup(
        std::string_view provider_key,
        std::uint32_t normalization_flags) const;

    [[nodiscard]] virtual std::optional<ResourcePayload> read(
        const ResourceId& resource) const = 0;
};

} // namespace dmc::rengine::gdspaces
