#include "dmc_rengine/profiles/dmc3/em000_family_contract.hpp"

#include <cassert>

int main() {
    namespace em = dmc::rengine::profiles::dmc3::em000_family;
    static_assert(em::classes.size() == 5U);
    static_assert(em::classes[0].body_slot == 1U && em::classes[0].cloth[0].body_joint == 14U);
    static_assert(em::classes[2].cloth_count == 2U && em::classes[2].cloth[0].body_joint == 8U);
    static_assert(em::classes[4].weapon_slot_variant01 == 34U);
    static_assert(em::weapon_body_joint == 9U);
    for (const auto& c : em::classes) {
        assert(c.class_name.starts_with("CEm00"));
        assert(c.init_va > c.vtable_va - 0x500000ULL);
        for (std::uint8_t k = 0U; k < c.cloth_count; ++k) {
            assert(c.cloth[k].clt_slot + 1U == c.cloth[k].model_slot);
        }
    }
    return 0;
}
