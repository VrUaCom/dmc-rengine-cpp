#pragma once

#include "dmc_rengine/formats/scm.hpp"
#include "dmc_rengine/formats/scm_transform.hpp"

#include <optional>
#include <vector>

namespace dmc::rengine::formats::scm {

// Reconstruct the canonical runtime world-matrix propagation performed by
// 0x1402F9700. Scene parents are indexed by evaluation-order position while
// node_at_order_position maps that position to the actual scene-node index.
//
// In DMC3's recovered row-vector convention:
//   root:      world[node] = local[node] * root_base
//   non-root:  world[node] = local[node] * world[parent]
//
// Returns nullopt for malformed hierarchy state rather than inventing a
// fallback ordering or parent.
[[nodiscard]] std::optional<std::vector<Matrix4f>> build_world_matrices(
    const SceneNodeBlock& scene,
    const Matrix4f& root_base) noexcept;

[[nodiscard]] std::optional<std::vector<Matrix4f>> build_world_matrices(
    const SceneNodeBlock& scene) noexcept;

} // namespace dmc::rengine::formats::scm
