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

    // Return every physical resource whose logical path normalizes to an
    // already-normalized provider key under caller-supplied flags. The generic
    // fallback preserves duplicate physical identities and never chooses a
    // winner. Sources may override with an equivalent indexed implementation.
    [[nodiscard]] virtual std::vector<ResourceRef> lookup(
        std::string_view provider_key,
        std::uint32_t normalization_flags) const;

    [[nodiscard]] virtual std::optional<ResourcePayload> read(
        const ResourceId& resource) const = 0;
};

} // namespace dmc::rengine::gdspaces
