#include "dmc_rengine/stageops/dmc3_stage_assembler.hpp"

#include "dmc_rengine/gdspaces/stage_bundle_assembler.hpp"

#include <algorithm>
#include <map>
#include <set>
#include <string>
#include <tuple>
#include <utility>

namespace dmc::rengine::stageops {
namespace {

[[nodiscard]] gdspaces::StageResourceCategory category_for_role(
    profiles::dmc3::StageResourceRole role) noexcept {
    switch (role) {
    case profiles::dmc3::StageResourceRole::script:
        return gdspaces::StageResourceCategory::scripts;
    case profiles::dmc3::StageResourceRole::room_config:
        return gdspaces::StageResourceCategory::unknown;
    case profiles::dmc3::StageResourceRole::room_effects:
        return gdspaces::StageResourceCategory::effects;
    case profiles::dmc3::StageResourceRole::room_sound:
        return gdspaces::StageResourceCategory::sounds;
    }
    return gdspaces::StageResourceCategory::unknown;
}

[[nodiscard]] gdspaces::StageIdentity fallback_identity(
    const profiles::dmc3::StageRuntimeLoadReport& report) {
    const auto semantic = report.resolution.semantic_stage_id.value_or(
        std::string{});
    return gdspaces::StageIdentity{
        .profile = "dmc3-hd",
        .stage_id = report.resolution.catalog_entry_id,
        .display_name = semantic.empty()
            ? "Stage resource set " + report.resolution.catalog_entry_id
            : "Stage " + semantic,
        .exe_evidence_id = report.resolution.plan.evidence_id,
        .resource_set_id = report.resolution.catalog_entry_id,
        .semantic_stage_id = semantic,
        .numeric_stage_id = report.resolution.numeric_stage_id,
    };
}

[[nodiscard]] std::string membership_key(
    const StageAssemblyMembership& membership) {
    return membership.resource_id + "|" + membership.role + "|" +
        std::to_string(static_cast<int>(membership.kind)) + "|" +
        membership.parent_resource_id.value_or(std::string{}) + "|" +
        (membership.container_slot.has_value()
            ? std::to_string(*membership.container_slot)
            : std::string{});
}

void upsert_resource(
    std::map<std::string, StageAssemblyResource, std::less<>>& resources,
    const gdspaces::ResourceRef& reference,
    bool materialized,
    bool descriptor_root,
    bool nested_child) {
    if (!reference.valid()) {
        return;
    }

    const auto key = reference.id.canonical();
    auto [iterator, inserted] = resources.try_emplace(
        key,
        StageAssemblyResource{
            .resource = reference,
            .materialized = materialized,
            .descriptor_root = descriptor_root,
            .nested_container_child = nested_child,
            .container_expansion_attempted = false,
            .container_expansion_complete = false,
            .game_ready = false,
        });

    if (!inserted) {
        auto& existing = iterator->second;
        existing.materialized = existing.materialized || materialized;
        existing.descriptor_root = existing.descriptor_root || descriptor_root;
        existing.nested_container_child =
            existing.nested_container_child || nested_child;
        // Prefer the materialized reference when the identity is the same.
        if (materialized) {
            existing.resource = reference;
        }
    }
}

void add_membership(
    std::vector<StageAssemblyMembership>& memberships,
    std::set<std::string, std::less<>>& seen,
    StageAssemblyMembership membership) {
    if (!membership.valid()) {
        return;
    }
    const auto key = membership_key(membership);
    if (seen.insert(key).second) {
        memberships.push_back(std::move(membership));
    }
}

} // namespace

StageAssemblyWorkspace DMC3StageAssembler::assemble(
    const profiles::dmc3::StageRuntimeLoadReport& report) {
    StageAssemblyWorkspace workspace;

    const auto stage_identity = report.bundle.has_value()
        ? report.bundle->identity()
        : fallback_identity(report);

    workspace.identity = StageAssemblyIdentity{
        .stage = stage_identity,
        .catalog_entry_id = report.resolution.catalog_entry_id,
        .global_catalog_row = report.resolution.table_row_index,
        .source_table_id = report.resolution.source_table_id,
        .source_row_index = report.resolution.source_row_index,
        .numeric_stage_id = report.resolution.numeric_stage_id,
        .semantic_stage_id = report.resolution.semantic_stage_id,
    };
    workspace.product_materialization_complete =
        report.materialization_complete();
    workspace.game_ready_equivalent = report.game_ready_equivalent();
    workspace.diagnostics = report.diagnostics;

    std::map<std::string, StageAssemblyResource, std::less<>> unique_resources;
    std::set<std::string, std::less<>> seen_memberships;

    for (const auto& loaded : report.resources) {
        const auto& resolution = loaded.resolution;
        if (!resolution.runtime.resolved.has_value()) {
            continue;
        }

        const auto materialized = loaded.payload_valid();
        const auto& root_reference = materialized
            ? loaded.payload->resource
            : *resolution.runtime.resolved;
        upsert_resource(
            unique_resources,
            root_reference,
            materialized,
            true,
            false);

        add_membership(
            workspace.memberships,
            seen_memberships,
            StageAssemblyMembership{
                .resource_id = root_reference.id.canonical(),
                .category = category_for_role(resolution.reference.role),
                .role = std::string{profiles::dmc3::to_string(
                    resolution.reference.role)},
                .kind = StageAssemblyMembershipKind::descriptor_root,
                .parent_resource_id = std::nullopt,
                .container_slot = std::nullopt,
            });

        auto root_iterator = unique_resources.find(root_reference.id.canonical());
        if (root_iterator != unique_resources.end()) {
            root_iterator->second.container_expansion_attempted =
                loaded.expansion.has_value();
            root_iterator->second.container_expansion_complete =
                loaded.expansion.has_value() && loaded.expansion->complete();
        }

        if (!loaded.expansion.has_value()) {
            continue;
        }

        for (const auto& expansion : loaded.expansion->expansions) {
            const auto parent_id = expansion.parent.id.canonical();
            auto parent_iterator = unique_resources.find(parent_id);
            if (parent_iterator != unique_resources.end()) {
                parent_iterator->second.container_expansion_attempted = true;
                parent_iterator->second.container_expansion_complete =
                    parent_iterator->second.container_expansion_complete ||
                    expansion.usable();
            }

            for (const auto& child : expansion.children) {
                if (!child.entry.populated || !child.payload.resource.valid()) {
                    continue;
                }

                const auto child_materialized = child.payload.readable();
                upsert_resource(
                    unique_resources,
                    child.payload.resource,
                    child_materialized,
                    false,
                    true);

                add_membership(
                    workspace.memberships,
                    seen_memberships,
                    StageAssemblyMembership{
                        .resource_id = child.payload.resource.id.canonical(),
                        .category = gdspaces::StageBundleAssembler::infer_category(
                            child.payload.resource),
                        .role = "container-slot/" +
                            std::to_string(child.entry.slot_index),
                        .kind = StageAssemblyMembershipKind::nested_container_child,
                        .parent_resource_id = parent_id,
                        .container_slot = child.entry.slot_index,
                    });
            }
        }
    }

    workspace.resources.reserve(unique_resources.size());
    for (auto& [key, resource] : unique_resources) {
        (void)key;
        resource.game_ready = workspace.game_ready_equivalent &&
            resource.materialized;
        workspace.resources.push_back(std::move(resource));
    }

    std::sort(
        workspace.memberships.begin(), workspace.memberships.end(),
        [](const StageAssemblyMembership& left,
           const StageAssemblyMembership& right) {
            return std::tie(
                       left.resource_id,
                       left.kind,
                       left.parent_resource_id,
                       left.container_slot,
                       left.role) <
                   std::tie(
                       right.resource_id,
                       right.kind,
                       right.parent_resource_id,
                       right.container_slot,
                       right.role);
        });

    return workspace;
}

} // namespace dmc::rengine::stageops
