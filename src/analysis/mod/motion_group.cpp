#include "dmc_rengine/analysis/mod/motion_group.hpp"

#include <array>
#include <cstddef>
#include <cstdint>

namespace dmc::rengine::analysis::mod {

std::optional<MotionGroupProjection> project_motion_groups(
    const dmc::rengine::formats::mod::transform_domain::ParseResult& domain) {
    const auto node_count = static_cast<std::size_t>(domain.raw_domain_count);
    if (!domain.permutation_is_complete ||
        domain.node_at_order_position.size() != node_count ||
        domain.adapter_table.size() != node_count) {
        return std::nullopt;
    }

    MotionGroupProjection out;
    out.by_order_position = domain.adapter_table;
    out.by_node_index.resize(node_count);

    for (std::size_t position = 0U; position < node_count; ++position) {
        const auto node = domain.node_at_order_position[position];
        if (static_cast<std::size_t>(node) >= node_count) return std::nullopt;
        const auto group = domain.adapter_table[position];
        out.by_node_index[node] = group;
        if (group > out.max_group_index) out.max_group_index = group;
    }
    return out;
}

namespace {
constexpr std::array<std::uint8_t, 6> order{0U, 1U, 2U, 5U, 3U, 4U};
constexpr std::array<std::uint8_t, 6> groups{0U, 0U, 1U, 2U, 0U, 1U};
static_assert(motion_group_for_node(order, groups, 0U).value() == 0U);
static_assert(motion_group_for_node(order, groups, 2U).value() == 1U);
static_assert(motion_group_for_node(order, groups, 5U).value() == 2U);
static_assert(motion_group_for_node(order, groups, 4U).value() == 1U);
constexpr std::array<std::uint8_t, 2> duplicate_order{0U, 0U};
constexpr std::array<std::uint8_t, 2> duplicate_groups{1U, 2U};
static_assert(!motion_group_for_node(duplicate_order, duplicate_groups, 0U));
} // namespace

} // namespace dmc::rengine::analysis::mod
