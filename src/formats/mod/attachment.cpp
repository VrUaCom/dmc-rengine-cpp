#include "dmc_rengine/formats/mod/attachment.hpp"

#include <cstddef>

namespace dmc::rengine::formats::mod::attachment {

DefaultJointResolution resolve_default_joint(
    const Document& child,
    const Document& host) noexcept {
    DefaultJointResolution out;
    out.selector = child.header.default_joint_index();

    const auto host_world =
        world_transform::build_model_space_world_matrices(host.transform_domain);
    if (!host_world.has_value()) {
        out.status = ResolveStatus::host_spatial_hierarchy_unavailable;
        return out;
    }

    const auto selector = static_cast<std::size_t>(out.selector);
    if (selector >= host_world->size()) {
        out.status = ResolveStatus::selector_out_of_range;
        return out;
    }

    out.host_joint_world = (*host_world)[selector];
    out.status = ResolveStatus::resolved;
    return out;
}

} // namespace dmc::rengine::formats::mod::attachment
