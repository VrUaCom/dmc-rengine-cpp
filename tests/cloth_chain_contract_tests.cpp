#include "dmc_rengine/profiles/dmc3/cloth_chain_contract.hpp"

#include <cassert>

int main() {
    namespace cc = dmc::rengine::profiles::dmc3::cloth_chain;
    static_assert(cc::axis_names.size() == 6U && cc::axis_names[1] == "Y");
    static_assert(cc::defaults.max_speed == 50.0F && cc::defaults.damping == 0.99F);
    static_assert(cc::defaults.gravity[1] == -0.2F && cc::defaults.limit_length);
    static_assert(cc::solve_node_va < cc::defaults_va && cc::defaults_va < cc::parser_block_va);
    for (std::size_t i = 1U; i + 1U < cc::fields.size(); ++i) {
        assert(cc::fields[i - 1U].offset < cc::fields[i].offset);
    }
    for (const auto& b : cc::bindings) {
        if (b.archive_stem == "em000") assert(b.clt_slot + 1U == b.model_slot);
    }
    static_assert(cc::player_coat_capsules.size() == 6U);
    static_assert(cc::player_coat_capsules[0].body_joint == 3U &&
                  cc::player_coat_capsules[1].radius == 18.0F);
    return 0;
}
