#include "dmc_rengine/profiles/dmc3/enemy_node_constraint_contract.hpp"

#include <cassert>

int main() {
    namespace ec = dmc::rengine::profiles::dmc3::enemy_node_constraint;
    static_assert(static_cast<std::uint32_t>(ec::ConstraintMode::HostWorld) == 1U);

    constexpr auto hair = ec::constraints_for("em028", 4U);
    static_assert(hair.has_value() && hair->body_slot == 1U);
    static_assert(hair->constraints.size() == 3U);
    static_assert(hair->constraints[0].host_node == 3U);

    constexpr auto dress = ec::constraints_for("em028", 5U);
    static_assert(dress.has_value() && dress->constraints[1].host_node == 14U);

    constexpr auto sleeves = ec::constraints_for("em028", 6U);
    static_assert(sleeves.has_value() && sleeves->constraints.size() == 5U);
    static_assert(!ec::constraints_for("em028", 9U).has_value());
    static_assert(!ec::constraints_for("em029", 4U).has_value());

    // Evidence addresses lie inside CEm028 init 0x140130480..0x140131520 and
    // increase in store order.
    std::uint64_t previous = 0x140130480ULL;
    for (const auto& record : ec::part_constraints) {
        for (const auto& constraint : record.constraints) {
            assert(constraint.evidence_va > previous && constraint.evidence_va < 0x140131520ULL);
            previous = constraint.evidence_va;
            assert(constraint.host_node < 23U);  // em028 body slot 1 has 23 nodes
        }
    }
    return 0;
}
