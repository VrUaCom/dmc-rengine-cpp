#include "dmc_rengine/profiles/dmc3/runtime_naming_bridge.hpp"

#include <algorithm>

namespace dmc::rengine::profiles::dmc3 {

RuntimeNamingBridgeReport RuntimeNamingBridge::link(
    const RuntimeResolutionReport& runtime,
    const gdspaces::ContainerNamingIdentitySnapshot& naming) {
    RuntimeNamingBridgeReport report;
    report.runtime_request = runtime.request;

    if (!runtime.ok()) {
        report.status = RuntimeNamingBridgeStatus::runtime_unresolved;
        report.detail =
            "Runtime resolution must produce one exact ResourceRef before L2 can be linked to L1 naming.";
        return report;
    }

    report.runtime_resource_id = runtime.resolved->id;

    if (!naming.ok()) {
        report.status = RuntimeNamingBridgeStatus::invalid_naming_snapshot;
        report.detail =
            "L1 naming bridge requires a valid contradiction-free container naming snapshot.";
        return report;
    }

    report.naming_parent_resource_id = naming.parent_resource;

    if (runtime.resolved->id != naming.parent_resource) {
        report.status = RuntimeNamingBridgeStatus::physical_identity_mismatch;
        report.detail =
            "Runtime ResourceRef and L1 naming snapshot parent differ in exact physical ResourceId; no path/display fallback is permitted.";
        return report;
    }

    report.named_child_count = static_cast<std::size_t>(std::count_if(
        naming.children.begin(), naming.children.end(),
        [](const gdspaces::ResourceNamingIdentity& child) {
            return child.external_index_normalized_name().has_value() ||
                child.embedded_alias.has_value() ||
                child.enclosing_container_stored_name.has_value();
        }));
    report.status = RuntimeNamingBridgeStatus::linked;
    report.detail =
        "Runtime container identity is linked to the L1 naming snapshot by exact ResourceId only.";
    return report;
}

} // namespace dmc::rengine::profiles::dmc3
