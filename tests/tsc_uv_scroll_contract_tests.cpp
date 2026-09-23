#include "dmc_rengine/profiles/dmc3/tsc_uv_scroll_contract.hpp"

#include <cassert>

int main() {
    namespace t = dmc::rengine::profiles::dmc3::tsc_uv_scroll;
    static_assert(t::record_size == 0x80U && t::offset_scale == 4096.0F);
    static_assert(t::type_handlers[3] == 0x14030B780ULL && t::type_handlers[10] == 0x14030BB50ULL);
    static_assert(t::cdrawuv_vtable_va - t::idrawuv_vtable_va == 0x28U);
    for (std::size_t i = 1U; i < t::fields.size(); ++i) {
        assert(t::fields[i - 1U].offset < t::fields[i].offset);
    }
    for (const auto& b : t::bindings) assert(b.tsc_slot != b.model_slot);
    // Object flags 0x01020000 -> scroll 0, 0x02000000 -> scroll 1 (em028 slot 5).
    static_assert(((0x01020000U >> t::object_scroll_flag_shift) & 0xFU) - 1U == 0U);
    static_assert(((0x02000000U >> t::object_scroll_flag_shift) & 0xFU) - 1U == 1U);
    return 0;
}
