#pragma once

#include "dmc_rengine/gdspaces/resource_naming_identity.hpp"
#include "dmc_rengine/profiles/dmc3/runtime_resource_resolver.hpp"

#include <optional>
#include <string>
#include <string_view>

namespace dmc::rengine::profiles::dmc3 {

enum class RuntimeNamingBridgeStatus {
    linked,
    runtime_unresolved,
    invalid_naming_snapshot,
    physical_identity_mismatch,
};

[[nodiscard]] constexpr std::string_view to_string(
    RuntimeNamingBridgeStatus status) noexcept {
    switch (status) {
    case RuntimeNamingBridgeStatus::linked: return "linked";
    case RuntimeNamingBridgeStatus::runtime_unresolved:
        return "runtime-unresolved";
    case RuntimeNamingBridgeStatus::invalid_naming_snapshot:
        return "invalid-naming-snapshot";
    case RuntimeNamingBridgeStatus::physical_identity_mismatch:
        return "physical-identity-mismatch";
    }
    return "runtime-unresolved";
}

// Evidence-safe bridge between Layer 2 runtime resolution and the Layer 1
// naming snapshot for the resolved container. The join key is the exact
// physical ResourceId only. Runtime request/candidate/display strings,
// external .index labels, embedded aliases and canonical display names are
// never accepted as substitute identity keys.
struct RuntimeNamingBridgeReport final {
    RuntimeNamingBridgeStatus status{
        RuntimeNamingBridgeStatus::runtime_unresolved};
    std::string runtime_request;
    std::optional<gdspaces::ResourceId> runtime_resource_id;
    std::optional<gdspaces::ResourceId> naming_parent_resource_id;
    std::size_t named_child_count{};
    std::string detail;

    [[nodiscard]] bool ok() const noexcept {
        return status == RuntimeNamingBridgeStatus::linked &&
            runtime_resource_id.has_value() &&
            naming_parent_resource_id.has_value() &&
            *runtime_resource_id == *naming_parent_resource_id;
    }
};

class RuntimeNamingBridge final {
public:
    [[nodiscard]] static RuntimeNamingBridgeReport link(
        const RuntimeResolutionReport& runtime,
        const gdspaces::ContainerNamingIdentitySnapshot& naming);
};

} // namespace dmc::rengine::profiles::dmc3
