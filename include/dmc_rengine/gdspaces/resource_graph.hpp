#pragma once

#include "dmc_rengine/gdspaces/resource_ref.hpp"

#include <cstddef>
#include <map>
#include <string>
#include <string_view>
#include <vector>

namespace dmc::rengine::gdspaces {

enum class ResourceRelation {
    contains,
    depends_on,
    stage_member,
    evidence_for,
    opens_with,
};

[[nodiscard]] constexpr std::string_view to_string(
    ResourceRelation relation) noexcept {
    switch (relation) {
    case ResourceRelation::contains: return "contains";
    case ResourceRelation::depends_on: return "depends-on";
    case ResourceRelation::stage_member: return "stage-member";
    case ResourceRelation::evidence_for: return "evidence-for";
    case ResourceRelation::opens_with: return "opens-with";
    }
    return "depends-on";
}

struct ResourceEdge final {
    std::string from;
    std::string to;
    ResourceRelation relation{ResourceRelation::depends_on};

    friend bool operator==(const ResourceEdge&, const ResourceEdge&) = default;
};

class ResourceGraph final {
public:
    [[nodiscard]] bool add(ResourceRef resource);
    [[nodiscard]] bool connect(
        const ResourceId& from,
        const ResourceId& to,
        ResourceRelation relation);
    [[nodiscard]] const ResourceRef* find(const ResourceId& resource) const;
    [[nodiscard]] const ResourceRef* find(std::string_view canonical_id) const;
    [[nodiscard]] std::vector<const ResourceRef*> related(
        const ResourceId& resource,
        ResourceRelation relation) const;
    [[nodiscard]] std::size_t resource_count() const noexcept;
    [[nodiscard]] std::size_t edge_count() const noexcept;

private:
    std::map<std::string, ResourceRef, std::less<>> resources_;
    std::vector<ResourceEdge> edges_;
};

} // namespace dmc::rengine::gdspaces
