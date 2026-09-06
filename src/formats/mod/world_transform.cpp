#include "dmc_rengine/formats/mod/world_transform.hpp"

#include <cmath>

namespace dmc::rengine::formats::mod::world_transform {

bool supports_spatial_hierarchy(
    const transform_domain::ParseResult& domain) noexcept {
    const auto node_count = static_cast<std::size_t>(domain.raw_domain_count);
    return domain.ok() &&
        node_count != 0U &&
        domain.permutation_is_complete &&
        domain.hierarchy_is_topological &&
        domain.transform_records_complete &&
        domain.transform_records_finite &&
        domain.node_at_order_position.size() == node_count &&
        domain.parent_by_order_position.size() == node_count &&
        domain.local_transform_records_by_node_index.size() == node_count;
}

Matrix4f multiply_dmc3_matrices(
    const Matrix4f& left,
    const Matrix4f& right) noexcept {
    Matrix4f result{};
    for (std::size_t row = 0U; row < 4U; ++row) {
        for (std::size_t column = 0U; column < 4U; ++column) {
            float value = 0.0F;
            for (std::size_t k = 0U; k < 4U; ++k)
                value += left(row, k) * right(k, column);
            result.values[row * 4U + column] = value;
        }
    }
    return result;
}

Matrix4f build_local_matrix(
    const transform_domain::LocalTransformRecord& transform) noexcept {
    const auto cx = std::cos(transform.rotation_xyz_radians.x);
    const auto sx = std::sin(transform.rotation_xyz_radians.x);
    const auto cy = std::cos(transform.rotation_xyz_radians.y);
    const auto sy = std::sin(transform.rotation_xyz_radians.y);
    const auto cz = std::cos(transform.rotation_xyz_radians.z);
    const auto sz = std::sin(transform.rotation_xyz_radians.z);

    // Exact row-major expansion of Rz * Ry * Rx for the axis sequence used by
    // 0x140330450 in the canonical MOD/EFM initializer.
    Matrix4f result{{
        cy * cz,
        cx * sz + cz * sx * sy,
        -cx * cz * sy + sx * sz,
        0.0F,

        -cy * sz,
        cx * cz - sx * sy * sz,
        cx * sy * sz + cz * sx,
        0.0F,

        sy,
        -cy * sx,
        cx * cy,
        0.0F,

        0.0F,
        0.0F,
        0.0F,
        1.0F,
    }};

    // 0x140031200 adds serialized translation XYZ to row 3 while preserving
    // homogeneous W. The serialized translation-magnitude lane (+0x0C) is not
    // consumed as W by the matrix builder.
    result.values[12] += transform.translation.x;
    result.values[13] += transform.translation.y;
    result.values[14] += transform.translation.z;
    return result;
}

std::optional<std::vector<Matrix4f>> build_world_matrices(
    const transform_domain::ParseResult& domain,
    const Matrix4f& root_base) noexcept {
    if (!supports_spatial_hierarchy(domain))
        return std::nullopt;

    const auto node_count = static_cast<std::size_t>(domain.raw_domain_count);
    std::vector<Matrix4f> local(node_count);
    std::vector<Matrix4f> world(node_count);
    std::vector<bool> evaluated(node_count, false);

    for (std::size_t node = 0U; node < node_count; ++node)
        local[node] = build_local_matrix(
            domain.local_transform_records_by_node_index[node]);

    for (std::size_t order_position = 0U;
         order_position < node_count;
         ++order_position) {
        const auto node = static_cast<std::size_t>(
            domain.node_at_order_position[order_position]);
        if (node >= node_count || evaluated[node])
            return std::nullopt;

        const auto parent = domain.parent_by_order_position[order_position];
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
            world[node] = multiply_dmc3_matrices(
                local[node], world[parent_node]);
        }
        evaluated[node] = true;
    }

    return world;
}

std::optional<std::vector<Matrix4f>> build_model_space_world_matrices(
    const transform_domain::ParseResult& domain) noexcept {
    return build_world_matrices(domain, identity_matrix());
}

} // namespace dmc::rengine::formats::mod::world_transform
