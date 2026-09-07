#pragma once

#include "dmc_rengine/analysis/mod/motion_group.hpp"
#include "dmc_rengine/formats/mod/transform_domain.hpp"
#include "dmc_rengine/formats/mod/world_transform.hpp"

#include <cstddef>
#include <cstdint>
#include <optional>
#include <span>
#include <vector>

namespace dmc::rengine::analysis::mod {

using AnimationMatrix4f =
    dmc::rengine::formats::mod::world_transform::Matrix4f;

// MOD-side projection of the model node -> CMotion joint relationship.
//
// This deliberately contains only model-owned semantics that are already
// EXE-confirmed independently: node identity/order/parentage plus the recovered
// motion-group selector. It does not parse MOT payload bytes and it does not
// reinterpret the serialized MOD rest-local transform as animated state.
struct AnimationJointBinding final {
    std::uint8_t node_index{};
    std::int16_t parent_node_index{-1};
    std::uint8_t motion_group{};
    std::size_t order_position{};
};

struct AnimationBindingProjection final {
    // Indexed by MOD node index, matching the runtime joint/node association.
    std::vector<AnimationJointBinding> by_node_index;

    // Exact hierarchy evaluation order retained for tooling and diagnostics.
    std::vector<std::uint8_t> node_at_order_position;
};

// Compose the canonical MOD hierarchy and EXE-confirmed motion-group projection
// into a CMotion-facing node binding. Fails closed if the hierarchy is not a
// complete topological permutation or if the motion-group domain cannot be
// projected exactly once per node.
[[nodiscard]] std::optional<AnimationBindingProjection>
project_animation_binding(
    const dmc::rengine::formats::mod::transform_domain::ParseResult& domain);

// Build currentWorld[] from caller-provided evaluated animated-local matrices.
// The matrices are node-indexed and represent animation-owned pose state; the
// serialized MOD rest-local records remain untouched.
//
// DMC3 row-vector composition:
//   root  = animatedLocal[root] * rootBase
//   child = animatedLocal[child] * currentWorld[parent]
[[nodiscard]] std::optional<std::vector<AnimationMatrix4f>>
build_animated_world_matrices(
    const dmc::rengine::formats::mod::transform_domain::ParseResult& domain,
    std::span<const AnimationMatrix4f> animated_local_by_node,
    const AnimationMatrix4f& root_base) noexcept;

// Complete the canonical animated skinning chain without decoding MOT here:
//   animatedLocal -> currentWorld -> inverseRestWorld * currentWorld
[[nodiscard]] std::optional<std::vector<AnimationMatrix4f>>
build_animated_skin_palette(
    const dmc::rengine::formats::mod::transform_domain::ParseResult& domain,
    std::span<const AnimationMatrix4f> animated_local_by_node,
    const AnimationMatrix4f& root_base) noexcept;

} // namespace dmc::rengine::analysis::mod
