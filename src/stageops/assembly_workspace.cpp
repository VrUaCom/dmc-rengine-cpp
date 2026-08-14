#include "dmc_rengine/stageops/assembly_workspace.hpp"

#include <algorithm>
#include <map>
#include <set>
#include <string>

namespace dmc::rengine::stageops {

bool StageAssemblyIdentity::valid() const noexcept {
    return stage.valid() &&
        !catalog_entry_id.empty() &&
        !source_table_id.empty() &&
        stage.resource_set_key() == catalog_entry_id;
}

bool StageAssemblyResource::valid() const noexcept {
    if (!resource.valid()) {
        return false;
    }
    if (materialized) {
        if (!byte_provenance.has_value() || !byte_provenance->valid()) {
            return false;
        }
    } else if (byte_provenance.has_value()) {
        // Provenance describes materialized bytes. Keeping provenance on a
        // resource declared non-materialized would create two conflicting facts.
        return false;
    }
    return true;
}

bool StageAssemblyMembership::valid() const noexcept {
    if (resource_id.empty() || role.empty()) {
        return false;
    }
    if (kind == StageAssemblyMembershipKind::nested_container_child) {
        return parent_resource_id.has_value() &&
            !parent_resource_id->empty() &&
            *parent_resource_id != resource_id &&
            container_slot.has_value();
    }
    return !parent_resource_id.has_value() && !container_slot.has_value();
}

StageAssemblyStatus StageAssemblyWorkspace::status() const noexcept {
    if (!valid()) {
        return StageAssemblyStatus::invalid;
    }
    if (product_materialization_complete) {
        return StageAssemblyStatus::product_materialized;
    }
    return StageAssemblyStatus::partial;
}

StageGameReadiness StageAssemblyWorkspace::game_readiness() const noexcept {
    return original_game_ready()
        ? StageGameReadiness::equivalent
        : StageGameReadiness::unproven;
}

bool StageAssemblyWorkspace::valid() const noexcept {
    if (!identity.valid()) {
        return false;
    }

    // Original-game readiness is strictly stronger than the product
    // materialization gate. A workspace may never claim the former alone.
    if (game_ready_equivalent && !product_materialization_complete) {
        return false;
    }

    std::map<std::string, const StageAssemblyResource*, std::less<>> by_id;
    for (const auto& resource : resources) {
        if (!resource.valid()) {
            return false;
        }
        const auto canonical = resource.resource.id.canonical();
        if (canonical.empty() || !by_id.emplace(canonical, &resource).second) {
            return false;
        }
    }

    std::set<std::string, std::less<>> resources_with_membership;
    std::set<std::string, std::less<>> root_memberships;
    std::set<std::string, std::less<>> nested_memberships;
    std::set<std::string, std::less<>> unique_membership_keys;

    for (const auto& membership : memberships) {
        if (!membership.valid()) {
            return false;
        }
        const auto resource_iterator = by_id.find(membership.resource_id);
        if (resource_iterator == by_id.end()) {
            return false;
        }

        if (membership.kind == StageAssemblyMembershipKind::nested_container_child) {
            if (!membership.parent_resource_id.has_value() ||
                by_id.find(*membership.parent_resource_id) == by_id.end()) {
                return false;
            }
            nested_memberships.insert(membership.resource_id);
        } else {
            root_memberships.insert(membership.resource_id);
        }

        const auto membership_key = membership.resource_id + "|" +
            std::to_string(static_cast<int>(membership.kind)) + "|" +
            membership.role + "|" +
            membership.parent_resource_id.value_or(std::string{}) + "|" +
            (membership.container_slot.has_value()
                ? std::to_string(*membership.container_slot)
                : std::string{});
        if (!unique_membership_keys.insert(membership_key).second) {
            return false;
        }
        resources_with_membership.insert(membership.resource_id);
    }

    // Every resource in the Stage Ops aggregate must be reachable through at
    // least one explicit membership, and the denormalized membership flags on
    // the resource must agree with those relationships.
    for (const auto& [canonical, resource] : by_id) {
        if (!resources_with_membership.contains(canonical)) {
            return false;
        }
        if (resource->descriptor_root != root_memberships.contains(canonical) ||
            resource->nested_container_child != nested_memberships.contains(canonical)) {
            return false;
        }
    }

    // A supplying load report may only advertise complete product
    // materialization when every ResourceId retained in the assembled workspace
    // has actual validated bytes/provenance. Partial workspaces remain valid.
    if (product_materialization_complete &&
        std::any_of(
            resources.begin(), resources.end(),
            [](const StageAssemblyResource& resource) {
                return !resource.materialized;
            })) {
        return false;
    }

    return true;
}

bool StageAssemblyWorkspace::product_ready() const noexcept {
    return valid() && product_materialization_complete;
}

bool StageAssemblyWorkspace::original_game_ready() const noexcept {
    return product_ready() && game_ready_equivalent;
}

const StageAssemblyResource* StageAssemblyWorkspace::find_resource(
    std::string_view canonical_resource_id) const noexcept {
    const auto iterator = std::find_if(
        resources.begin(), resources.end(),
        [canonical_resource_id](const StageAssemblyResource& resource) {
            return resource.resource.id.canonical() == canonical_resource_id;
        });
    return iterator == resources.end() ? nullptr : &*iterator;
}

std::vector<const StageAssemblyMembership*> StageAssemblyWorkspace::by_category(
    gdspaces::StageResourceCategory category) const {
    std::vector<const StageAssemblyMembership*> result;
    for (const auto& membership : memberships) {
        if (membership.category == category) {
            result.push_back(&membership);
        }
    }
    return result;
}

std::vector<const StageAssemblyMembership*>
StageAssemblyWorkspace::memberships_for(
    std::string_view canonical_resource_id) const {
    std::vector<const StageAssemblyMembership*> result;
    for (const auto& membership : memberships) {
        if (membership.resource_id == canonical_resource_id) {
            result.push_back(&membership);
        }
    }
    return result;
}

std::size_t StageAssemblyWorkspace::descriptor_root_count() const noexcept {
    return static_cast<std::size_t>(std::count_if(
        resources.begin(), resources.end(),
        [](const StageAssemblyResource& resource) {
            return resource.descriptor_root;
        }));
}

std::size_t StageAssemblyWorkspace::nested_resource_count() const noexcept {
    return static_cast<std::size_t>(std::count_if(
        resources.begin(), resources.end(),
        [](const StageAssemblyResource& resource) {
            return resource.nested_container_child;
        }));
}

} // namespace dmc::rengine::stageops
