#include "dmc_rengine/stageops/assembly_workspace.hpp"

#include <algorithm>

namespace dmc::rengine::stageops {

StageAssemblyStatus StageAssemblyWorkspace::status() const noexcept {
    if (!valid()) {
        return StageAssemblyStatus::invalid;
    }
    if (game_ready_equivalent) {
        return StageAssemblyStatus::game_ready_equivalent;
    }
    if (product_materialization_complete) {
        return StageAssemblyStatus::product_materialized;
    }
    return StageAssemblyStatus::partial;
}

bool StageAssemblyWorkspace::valid() const noexcept {
    if (!identity.valid()) {
        return false;
    }
    return std::all_of(
        resources.begin(), resources.end(),
        [](const StageAssemblyResource& resource) {
            return resource.valid();
        }) &&
        std::all_of(
            memberships.begin(), memberships.end(),
            [](const StageAssemblyMembership& membership) {
                return membership.valid();
            });
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
