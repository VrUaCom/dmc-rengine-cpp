#include "dmc_rengine/formats/mod/attachment.hpp"

#include <cstddef>

namespace dmc::rengine::formats::mod::attachment {

DefaultJointResolution resolve_default_joint(
    const std::uint8_t selector,
    const std::span<const world_transform::Matrix4f> host_world) noexcept {
    DefaultJointResolution out;
    out.selector = selector;
    const auto index = static_cast<std::size_t>(selector);
    if (index >= host_world.size()) {
        out.status = ResolveStatus::selector_out_of_range;
        return out;
    }
    out.host_joint_world = host_world[index];
    out.status = ResolveStatus::resolved;
    return out;
}

DefaultJointResolution resolve_default_joint(
    const Document& child,
    const Document& host) noexcept {
    const auto host_world =
        world_transform::build_model_space_world_matrices(host.transform_domain);
    if (!host_world.has_value()) {
        DefaultJointResolution out;
        out.selector = child.header.default_joint_index();
        out.status = ResolveStatus::host_spatial_hierarchy_unavailable;
        return out;
    }
    return resolve_default_joint(
        child.header.default_joint_index(),
        std::span<const world_transform::Matrix4f>{*host_world});
}

} // namespace dmc::rengine::formats::mod::attachment
