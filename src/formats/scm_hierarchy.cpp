#include "dmc_rengine/formats/scm_hierarchy.hpp"

#include <cstddef>

namespace dmc::rengine::formats::scm {

std::optional<std::vector<Matrix4f>> build_world_matrices(
    const SceneNodeBlock& scene,
    const Matrix4f& root_base) noexcept {
    const auto node_count = scene.transform_by_node_index.size();
    if (node_count == 0U ||
        scene.parent_by_order_position.size() != node_count ||
        scene.node_at_order_position.size() != node_count ||
        scene.object_binding_by_node_index.size() != node_count) {
        return std::nullopt;
    }

    std::vector<Matrix4f> local(node_count);
    std::vector<Matrix4f> world(node_count);
    std::vector<bool> evaluated(node_count, false);

    for (std::size_t node = 0U; node < node_count; ++node)
        local[node] = build_local_transform(scene.transform_by_node_index[node]);

    for (std::size_t order_position = 0U;
         order_position < node_count;
         ++order_position) {
        const auto node = static_cast<std::size_t>(
            scene.node_at_order_position[order_position]);
        if (node >= node_count || evaluated[node])
            return std::nullopt;

        const auto parent = scene.parent_by_order_position[order_position];
        if (order_position == 0U) {
            if (parent != -1)
                return std::nullopt;
            world[node] = multiply_dmc3_matrices(local[node], root_base);
        } else {
            if (parent < 0)
                return std::nullopt;
            const auto parent_node = static_cast<std::size_t>(parent);
            if (parent_node >= node_count || !evaluated[parent_node])
                return std::nullopt;
            world[node] = multiply_dmc3_matrices(local[node], world[parent_node]);
        }
        evaluated[node] = true;
    }

    return world;
}

std::optional<std::vector<Matrix4f>> build_world_matrices(
    const SceneNodeBlock& scene) noexcept {
    return build_world_matrices(scene, identity_matrix());
}

} // namespace dmc::rengine::formats::scm
