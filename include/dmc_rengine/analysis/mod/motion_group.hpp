#pragma once

#include "dmc_rengine/formats/mod/transform_domain.hpp"

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <span>
#include <vector>

namespace dmc::rengine::analysis::mod {

// EXE-confirmed interpretation of the third serialized MOD node-domain table.
// The table itself is indexed by hierarchy order position. During CMotion
// binding (0x14030F850), its byte is copied to CMotionJoint +0xF8 for the node
// selected by nodeAtOrderPosition[position]. Multiple CMotion evaluators then
// process only joints whose +0xF8 equals the requested group index.
//
// This establishes a behavioral "motion group index" contract. It does not
// identify high-level meanings for group values 0/1/2 (body part, layer name,
// etc.); those labels remain evidence-gated.
struct MotionGroupProjection final {
    std::vector<std::uint8_t> by_order_position;
    std::vector<std::uint8_t> by_node_index;
    std::uint8_t max_group_index{};
};

// Resolve one node's motion-group byte from the two serialized order-position
// domains. The function fails closed unless exactly one order position maps to
// the requested node.
[[nodiscard]] constexpr std::optional<std::uint8_t> motion_group_for_node(
    std::span<const std::uint8_t> node_at_order_position,
    std::span<const std::uint8_t> motion_group_by_order_position,
    std::uint8_t node_index) noexcept {
    if (node_at_order_position.size() != motion_group_by_order_position.size())
        return std::nullopt;

    std::optional<std::uint8_t> result;
    for (std::size_t position = 0U;
         position < node_at_order_position.size();
         ++position) {
        if (node_at_order_position[position] != node_index) continue;
        if (result.has_value()) return std::nullopt;
        result = motion_group_by_order_position[position];
    }
    return result;
}

// Project the raw third table retained by formats::mod::transform_domain into
// a node-indexed CMotion view. The raw parser field remains untouched so binary
// parsing and runtime semantics stay separated under ADR-0003.
[[nodiscard]] std::optional<MotionGroupProjection> project_motion_groups(
    const dmc::rengine::formats::mod::transform_domain::ParseResult& domain);

} // namespace dmc::rengine::analysis::mod
