// 0x14002AB40 (reverse/object_flag_228.hpp): bit 2 of the dword at +0x228
// follows `enable`; the other 31 bits never change. The cases mirror the
// foundation gate's 10 000 differential runs against the original bytes.
#include "dmc_rengine/reverse/object_flag_228.hpp"

#include <cassert>
#include <cstdint>

namespace rv = dmc::rengine::reverse;

int main() {
    std::uint32_t state = 0x12345678U;
    for (int i = 0; i < 10000; ++i) {
        state = state * 1664525U + 1013904223U;  // LCG: field values
        const auto enable = static_cast<std::uint8_t>(state >> 24);
        std::uint32_t field = state;
        rv::set_object_flag_228(field, enable);
        const std::uint32_t expected = enable != 0U ? (state | 4U) : (state & 0xFFFFFFFBU);
        assert(field == expected);
        assert((field & ~4U) == (state & ~4U));
    }
    std::uint32_t field = 0xF01255A0U;  // the gate's own self-test bytes A0 55 12 F0
    rv::set_object_flag_228(field, 9U);
    assert(field == 0xF01255A4U);
    rv::set_object_flag_228(field, 0U);
    assert(field == 0xF01255A0U);
    static_assert(rv::object_flag_228_va == 0x14002AB40ULL && rv::object_flag_228_offset == 0x228U);
    return 0;
}
