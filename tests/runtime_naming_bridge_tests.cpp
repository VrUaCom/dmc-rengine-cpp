#include "dmc_rengine/profiles/dmc3/runtime_naming_bridge.hpp"

#include <cassert>
#include <optional>
#include <string>

namespace {

[[nodiscard]] dmc::rengine::gdspaces::ResourceId parent_id(
    std::string source_id = "dmc3-0-nbz") {
    return dmc::rengine::gdspaces::ResourceId{
        .source_id = std::move(source_id),
        .logical_path = "GData.afs/scr/st001.pac",
        .container_chain = "NBZ[0]",
        .offset = 0x1200U,
        .size = 0x5000U,
    };
}

[[nodiscard]] dmc::rengine::profiles::dmc3::RuntimeResolutionReport
resolved_runtime(const dmc::rengine::gdspaces::ResourceId& id) {
    namespace dmc3 = dmc::rengine::profiles::dmc3;
    return dmc3::RuntimeResolutionReport{
        .request = "scr\\st001.pac",
        .status = dmc3::RuntimeResolutionStatus::resolved,
        .resolved = dmc::rengine::gdspaces::ResourceRef{
            .id = id,
            .display_name = "st001.pac",
            .format = "pac",
            .profile = "dmc3-hd",
            .synthetic_name = false,
            .container = true,
        },
        .ambiguous_matches = {},
        .probes = {},
        .detail = "fixture",
    };
}

[[nodiscard]] dmc::rengine::gdspaces::ContainerNamingIdentitySnapshot
naming_snapshot(const dmc::rengine::gdspaces::ResourceId& parent) {
    namespace gdspaces = dmc::rengine::gdspaces;
    return gdspaces::ContainerNamingIdentitySnapshot{
        .parent_resource = parent,
        .external_index_evidence = std::nullopt,
        .children = {
            gdspaces::ResourceNamingIdentity{
                .resource_id = gdspaces::ResourceId{
                    .source_id = parent.source_id,
                    .logical_path = parent.logical_path + "::PAC/slot-0005",
                    .container_chain = parent.container_chain + "/PAC[5]",
                    .offset = parent.offset + 0x100U,
                    .size = 0x80U,
                },
                .physical_slot_index = 5U,
                .populated = true,
                .extracted_ordinal = 0U,
                .external_index_raw_label = std::string{"st001_000.ukn"},
                .external_index_name = std::string{"st001_000.ukn"},
                .external_index_folder = false,
                .embedded_alias = std::string{"st001.sch"},
                .enclosing_container_stored_name = std::nullopt,
                .enclosing_container_stored_name_evidence = std::nullopt,
                .semantic_format = "hits",
                .canonical_extension = ".hits",
                .semantic_format_evidence = std::nullopt,
                .canonical_display_name = "st001_000.hits",
            },
        },
        .diagnostics = {},
    };
}

} // namespace

int main() {
    namespace dmc3 = dmc::rengine::profiles::dmc3;

    const auto physical = parent_id();
    const auto runtime = resolved_runtime(physical);
    const auto naming = naming_snapshot(physical);
    assert(naming.ok());

    const auto linked = dmc3::RuntimeNamingBridge::link(runtime, naming);
    assert(linked.ok());
    assert(linked.status == dmc3::RuntimeNamingBridgeStatus::linked);
    assert(linked.runtime_resource_id == linked.naming_parent_resource_id);
    assert(linked.named_child_count == 1U);

    // Same visible/logical path is not enough. A different source identity is a
    // different physical resource and must fail closed rather than falling back
    // to filename/path matching.
    const auto foreign_runtime = resolved_runtime(parent_id("different-source"));
    const auto mismatch = dmc3::RuntimeNamingBridge::link(foreign_runtime, naming);
    assert(!mismatch.ok());
    assert(
        mismatch.status ==
        dmc3::RuntimeNamingBridgeStatus::physical_identity_mismatch);

    auto unresolved = runtime;
    unresolved.status = dmc3::RuntimeResolutionStatus::not_found;
    unresolved.resolved.reset();
    const auto no_runtime = dmc3::RuntimeNamingBridge::link(unresolved, naming);
    assert(!no_runtime.ok());
    assert(
        no_runtime.status ==
        dmc3::RuntimeNamingBridgeStatus::runtime_unresolved);

    auto invalid_naming = naming;
    invalid_naming.parent_resource.logical_path.clear();
    const auto invalid = dmc3::RuntimeNamingBridge::link(runtime, invalid_naming);
    assert(!invalid.ok());
    assert(
        invalid.status ==
        dmc3::RuntimeNamingBridgeStatus::invalid_naming_snapshot);

    return 0;
}
