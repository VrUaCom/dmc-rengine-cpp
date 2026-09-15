#pragma once

#include "dmc_rengine/formats/mod.hpp"
#include "dmc_rengine/formats/mod/world_transform.hpp"

#include <cstdint>

namespace dmc::rengine::formats::mod::attachment {

// Cross-MOD attachment contract recovered from the DMC3 runtime. This API does
// not infer which resource owns another resource. Callers must supply an
// explicit child and an explicit candidate host. The child header +0x13 value
// is the EXE-confirmed default joint selector; the returned matrix is the
// candidate host's model-space currentWorld-equivalent joint matrix.
enum class ResolveStatus : std::uint8_t {
    resolved,
    host_spatial_hierarchy_unavailable,
    selector_out_of_range,
};

struct DefaultJointResolution final {
    ResolveStatus status{ResolveStatus::host_spatial_hierarchy_unavailable};
    std::uint8_t selector{};
    world_transform::Matrix4f host_joint_world = world_transform::identity_matrix();

    [[nodiscard]] constexpr bool ok() const noexcept {
        return status == ResolveStatus::resolved;
    }
};

// Resolve the child MOD default_joint_index against one explicit host MOD.
//
// Evidence boundary:
// - child.header.default_joint_index() is EXE_CONFIRMED as header +0x13;
// - host world matrices use the canonical MOD row-vector hierarchy contract;
// - this function makes no filename, load-order, actor-family, or ownership
//   inference. Ambiguity across several candidate hosts is a caller concern.
[[nodiscard]] DefaultJointResolution resolve_default_joint(
    const Document& child,
    const Document& host) noexcept;

} // namespace dmc::rengine::formats::mod::attachment
