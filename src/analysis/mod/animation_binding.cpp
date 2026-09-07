#include "dmc_rengine/analysis/mod/animation_binding.hpp"

#include <cstddef>
#include <vector>

namespace dmc::rengine::analysis::mod {
namespace {

namespace world = dmc::rengine::formats::mod::world_transform;

[[nodiscard]] bool binding_domain_valid(
    const dmc::rengine::formats::mod::transform_domain::ParseResult& domain) noexcept {
    const auto node_count = static_cast<std::size_t>(domain.raw_domain_count);
    return domain.ok() &&
        node_count != 0U &&
        domain.permutation_is_complete &&
        domain.hierarchy_is_topological &&
        domain.node_at_order_position.size() == node_count &&
        domain.parent_by_order_position.size() == node_count &&
        domain.adapter_table.size() == node_count;
}

} // namespace

std::optional<AnimationBindingProjection> project_animation_binding(
    const dmc::rengine::formats::mod::transform_domain::ParseResult& domain) {
    if (!binding_domain_valid(domain))
        return std::nullopt;

    const auto groups = project_motion_groups(domain);
    if (!groups.has_value())
        return std::nullopt;

    const auto node_count = static_cast<std::size_t>(domain.raw_domain_count);
    AnimationBindingProjection out;
    out.by_node_index.resize(node_count);
    out.node_at_order_position = domain.node_at_order_position;

    std::vector<bool> seen(node_count, false);
    for (std::size_t order_position = 0U;
         order_position < node_count;
         ++order_position) {
        const auto node = static_cast<std::size_t>(
            domain.node_at_order_position[order_position]);
        if (node >= node_count || seen[node])
            return std::nullopt;

        const auto parent = domain.parent_by_order_position[order_position];
        if (order_position == 0U) {
            if (parent != -1)
                return std::nullopt;
        } else {
            if (parent < 0)
                return std::nullopt;
            const auto parent_node = static_cast<std::size_t>(parent);
            if (parent_node >= node_count || !seen[parent_node])
                return std::nullopt;
        }

        out.by_node_index[node] = AnimationJointBinding{
            .node_index = static_cast<std::uint8_t>(node),
            .parent_node_index = parent,
            .motion_group = groups->by_order_position[order_position],
            .order_position = order_position,
        };
        seen[node] = true;
    }

    return out;
}

std::optional<std::vector<AnimationMatrix4f>> build_animated_world_matrices(
    const dmc::rengine::formats::mod::transform_domain::ParseResult& domain,
    const std::span<const AnimationMatrix4f> animated_local_by_node,
    const AnimationMatrix4f& root_base) noexcept {
    const auto binding = project_animation_binding(domain);
    if (!binding.has_value())
        return std::nullopt;

    const auto node_count = binding->by_node_index.size();
    if (animated_local_by_node.size() != node_count)
        return std::nullopt;

    std::vector<AnimationMatrix4f> current_world_by_node(node_count);
    for (std::size_t order_position = 0U;
         order_position < node_count;
         ++order_position) {
        const auto node = static_cast<std::size_t>(
            binding->node_at_order_position[order_position]);
        if (node >= node_count)
            return std::nullopt;

        const auto& joint = binding->by_node_index[node];
        if (joint.order_position != order_position)
            return std::nullopt;

        if (joint.parent_node_index < 0) {
            if (order_position != 0U)
                return std::nullopt;
            current_world_by_node[node] = world::multiply_dmc3_matrices(
                animated_local_by_node[node], root_base);
        } else {
            const auto parent = static_cast<std::size_t>(joint.parent_node_index);
            if (parent >= node_count)
                return std::nullopt;
            current_world_by_node[node] = world::multiply_dmc3_matrices(
                animated_local_by_node[node], current_world_by_node[parent]);
        }
    }

    return current_world_by_node;
}

std::optional<std::vector<AnimationMatrix4f>> build_animated_skin_palette(
    const dmc::rengine::formats::mod::transform_domain::ParseResult& domain,
    const std::span<const AnimationMatrix4f> animated_local_by_node,
    const AnimationMatrix4f& root_base) noexcept {
    const auto current_world = build_animated_world_matrices(
        domain, animated_local_by_node, root_base);
    if (!current_world.has_value())
        return std::nullopt;

    return world::build_skin_palette(domain, *current_world);
}

} // namespace dmc::rengine::analysis::mod
