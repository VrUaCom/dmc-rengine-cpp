#include "dmc_rengine/profiles/dmc3/stageops_assembler.hpp"

#include "dmc_rengine/gdspaces/stage_bundle_assembler.hpp"

#include <algorithm>
#include <map>
#include <set>
#include <string>
#include <tuple>
#include <utility>

namespace dmc::rengine::profiles::dmc3 {
namespace {

[[nodiscard]] gdspaces::StageResourceCategory category_for_role(
    StageResourceRole role) noexcept {
    switch (role) {
    case StageResourceRole::script:
        return gdspaces::StageResourceCategory::scripts;
    case StageResourceRole::room_config:
        return gdspaces::StageResourceCategory::unknown;
    case StageResourceRole::room_effects:
        return gdspaces::StageResourceCategory::effects;
    case StageResourceRole::room_sound:
        return gdspaces::StageResourceCategory::sounds;
    }
    return gdspaces::StageResourceCategory::unknown;
}

[[nodiscard]] gdspaces::StageIdentity stage_identity_for(
    const StageRuntimeLoadReport& report) {
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

[[nodiscard]] bool payload_materialized(
    const gdspaces::ResourcePayload& payload) noexcept {
    return payload.resource.valid() && payload.readable() &&
        payload.byte_provenance.has_value() &&
        payload.byte_provenance->valid() &&
        payload.byte_provenance->materialized_size ==
            static_cast<std::uint64_t>(payload.bytes.size());
}

[[nodiscard]] std::string membership_key(
    const stageops::StageAssemblyMembership& membership) {
    return membership.resource_id + "|" + membership.role + "|" +
        std::to_string(static_cast<int>(membership.kind)) + "|" +
        membership.parent_resource_id.value_or(std::string{}) + "|" +
        (membership.container_slot.has_value()
            ? std::to_string(*membership.container_slot)
            : std::string{});
}

void upsert_resource(
    std::map<std::string, stageops::StageAssemblyResource, std::less<>>& resources,
    const gdspaces::ResourceRef& reference,
    const std::optional<gdspaces::ByteProvenance>& provenance,
    bool materialized,
    bool descriptor_root,
    bool nested_child) {
    if (!reference.valid()) {
        return;
    }

    const auto key = reference.id.canonical();
    auto [iterator, inserted] = resources.try_emplace(
        key,
        stageops::StageAssemblyResource{
            .resource = reference,
            .byte_provenance = materialized ? provenance : std::nullopt,
            .materialized = materialized,
            .descriptor_root = descriptor_root,
            .nested_container_child = nested_child,
            .container_expansion_observed = false,
        });

    if (!inserted) {
        auto& existing = iterator->second;
        existing.materialized = existing.materialized || materialized;
        existing.descriptor_root = existing.descriptor_root || descriptor_root;
        existing.nested_container_child =
            existing.nested_container_child || nested_child;
        if (materialized) {
            existing.resource = reference;
            existing.byte_provenance = provenance;
        }
    }
}

void add_membership(
    std::vector<stageops::StageAssemblyMembership>& memberships,
    std::set<std::string, std::less<>>& seen,
    stageops::StageAssemblyMembership membership) {
    if (!membership.valid()) {
        return;
    }
    const auto key = membership_key(membership);
    if (seen.insert(key).second) {
        memberships.push_back(std::move(membership));
    }
}

void add_diagnostic(
    stageops::StageAssemblyWorkspace& workspace,
    gdspaces::DiagnosticSeverity severity,
    std::string code,
    std::string message) {
    workspace.diagnostics.push_back(gdspaces::Diagnostic{
        .severity = severity,
        .code = std::move(code),
        .message = std::move(message),
        .resource = std::nullopt,
    });
}

[[nodiscard]] bool bundle_identity_matches(
    const gdspaces::StageIdentity& expected,
    const gdspaces::StageIdentity& actual) noexcept {
    return actual.resource_set_key() == expected.resource_set_key() &&
        actual.profile == expected.profile &&
        actual.numeric_stage_id == expected.numeric_stage_id &&
        actual.semantic_stage_id == expected.semantic_stage_id &&
        actual.exe_evidence_id == expected.exe_evidence_id;
}

} // namespace

stageops::StageAssemblyWorkspace StageOpsAssembler::assemble(
    const StageRuntimeLoadReport& report) {
    stageops::StageAssemblyWorkspace workspace;
    const auto stage_identity = stage_identity_for(report);

    workspace.identity = stageops::StageAssemblyIdentity{
        .stage = stage_identity,
        .catalog_entry_id = report.resolution.catalog_entry_id,
        .global_catalog_row = report.resolution.table_row_index,
        .source_table_id = report.resolution.source_table_id,
        .source_row_index = report.resolution.source_row_index,
    };
    workspace.product_materialization_complete =
        report.materialization_complete();
    workspace.game_ready_equivalent = report.game_ready_equivalent();
    workspace.diagnostics = report.diagnostics;

    if (report.bundle.has_value() &&
        !bundle_identity_matches(stage_identity, report.bundle->identity())) {
        add_diagnostic(
            workspace,
            gdspaces::DiagnosticSeverity::error,
            "stageops.dmc3.bundle-identity-mismatch",
            "The supplied StageBundle identity disagrees with the DMC3 runtime-resolution identity; Stage Ops kept the executable/catalog identity as authority.");
        workspace.product_materialization_complete = false;
        workspace.game_ready_equivalent = false;
    }

    std::map<std::string, stageops::StageAssemblyResource, std::less<>>
        unique_resources;
    std::set<std::string, std::less<>> seen_memberships;

    // Preserve all four descriptor requirements even when lookup or
    // materialization failed. Missing roots remain explicit Stage Ops state.
    for (std::size_t index = 0U; index < report.resolution.resources.size(); ++index) {
        const auto& resolved = report.resolution.resources[index];
        const auto& loaded = report.resources[index];
        const auto role = std::string{to_string(resolved.reference.role)};
        const auto resource_id = resolved.runtime.resolved.has_value()
            ? std::optional<std::string>{
                resolved.runtime.resolved->id.canonical()}
            : std::nullopt;

        workspace.requirements.push_back(stageops::StageAssemblyRequirement{
            .requirement_id = "descriptor-root/" + role,
            .category = category_for_role(resolved.reference.role),
            .role = role,
            .requested_logical_path = resolved.reference.logical_path,
            .resource_id = resource_id,
            .materialized = loaded.payload_valid(),
        });
    }

    for (const auto& loaded : report.resources) {
        const auto& resolution = loaded.resolution;
        if (!resolution.runtime.resolved.has_value()) {
            continue;
        }

        const auto materialized = loaded.payload_valid();
        const auto& root_reference = materialized
            ? loaded.payload->resource
            : *resolution.runtime.resolved;
        const auto root_provenance = materialized
            ? loaded.payload->byte_provenance
            : std::nullopt;

        upsert_resource(
            unique_resources,
            root_reference,
            root_provenance,
            materialized,
            true,
            false);

        add_membership(
            workspace.memberships,
            seen_memberships,
            stageops::StageAssemblyMembership{
                .resource_id = root_reference.id.canonical(),
                .category = category_for_role(resolution.reference.role),
                .role = std::string{to_string(resolution.reference.role)},
                .kind = stageops::StageAssemblyMembershipKind::descriptor_root,
                .parent_resource_id = std::nullopt,
                .container_slot = std::nullopt,
            });

        if (!loaded.expansion.has_value()) {
            continue;
        }

        // First materialize all child ResourceIds. A later pass marks every
        // expansion parent, making this independent of expansion record order.
        for (const auto& expansion : loaded.expansion->expansions) {
            for (const auto& child : expansion.children) {
                if (!child.entry.populated || !child.payload.resource.valid()) {
                    continue;
                }

                const auto child_materialized = payload_materialized(child.payload);
                upsert_resource(
                    unique_resources,
                    child.payload.resource,
                    child_materialized
                        ? child.payload.byte_provenance
                        : std::nullopt,
                    child_materialized,
                    false,
                    true);

                add_membership(
                    workspace.memberships,
                    seen_memberships,
                    stageops::StageAssemblyMembership{
                        .resource_id = child.payload.resource.id.canonical(),
                        .category = gdspaces::StageBundleAssembler::infer_category(
                            child.payload.resource),
                        .role = "container-slot/" +
                            std::to_string(child.entry.slot_index),
                        .kind = stageops::StageAssemblyMembershipKind::nested_container_child,
                        .parent_resource_id = expansion.parent.id.canonical(),
                        .container_slot = child.entry.slot_index,
                    });
            }
        }

        for (const auto& expansion : loaded.expansion->expansions) {
            const auto parent = unique_resources.find(expansion.parent.id.canonical());
            if (parent != unique_resources.end()) {
                parent->second.container_expansion_observed = true;
            }
        }
    }

    workspace.resources.reserve(unique_resources.size());
    for (auto& [key, resource] : unique_resources) {
        (void)key;
        workspace.resources.push_back(std::move(resource));
    }

    std::sort(
        workspace.requirements.begin(), workspace.requirements.end(),
        [](const stageops::StageAssemblyRequirement& left,
           const stageops::StageAssemblyRequirement& right) {
            return left.requirement_id < right.requirement_id;
        });

    std::sort(
        workspace.memberships.begin(), workspace.memberships.end(),
        [](const stageops::StageAssemblyMembership& left,
           const stageops::StageAssemblyMembership& right) {
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

    if (!workspace.valid()) {
        workspace.product_materialization_complete = false;
        workspace.game_ready_equivalent = false;
    }
    return workspace;
}

} // namespace dmc::rengine::profiles::dmc3
