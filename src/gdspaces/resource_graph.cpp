#include "dmc_rengine/gdspaces/resource_graph.hpp"

#include <algorithm>
#include <utility>

namespace dmc::rengine::gdspaces {

bool ResourceGraph::add(ResourceRef resource) {
    if (!resource.valid()) {
        return false;
    }

    const auto key = resource.id.canonical();
    return resources_.emplace(key, std::move(resource)).second;
}

bool ResourceGraph::connect(
    const ResourceId& from,
    const ResourceId& to,
    ResourceRelation relation) {
    const auto from_key = from.canonical();
    const auto to_key = to.canonical();
    if (resources_.find(from_key) == resources_.end() ||
        resources_.find(to_key) == resources_.end()) {
        return false;
    }

    const ResourceEdge candidate{
        .from = from_key,
        .to = to_key,
        .relation = relation,
    };

    if (std::find(edges_.begin(), edges_.end(), candidate) != edges_.end()) {
        return false;
    }

    edges_.push_back(candidate);
    return true;
}

const ResourceRef* ResourceGraph::find(const ResourceId& resource) const {
    return find(resource.canonical());
}

const ResourceRef* ResourceGraph::find(std::string_view canonical_id) const {
    const auto iterator = resources_.find(canonical_id);
    return iterator == resources_.end() ? nullptr : &iterator->second;
}

std::vector<const ResourceRef*> ResourceGraph::related(
    const ResourceId& resource,
    ResourceRelation relation) const {
    std::vector<const ResourceRef*> result;
    const auto key = resource.canonical();

    for (const auto& edge : edges_) {
        if (edge.from == key && edge.relation == relation) {
            if (const auto* target = find(edge.to); target != nullptr) {
                result.push_back(target);
            }
        }
    }

    return result;
}

std::size_t ResourceGraph::resource_count() const noexcept {
    return resources_.size();
}

std::size_t ResourceGraph::edge_count() const noexcept {
    return edges_.size();
}

} // namespace dmc::rengine::gdspaces
