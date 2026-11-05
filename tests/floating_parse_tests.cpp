#include "dmc_rengine/core/floating_parse.hpp"

#include <cassert>

int main() {
    using dmc::rengine::core::parse_floating;

    double d = 0.0;
    assert(parse_floating("1.25e2", d) && d == 125.0);
    assert(parse_floating("-0.25", d) && d == -0.25);
    assert(parse_floating("0", d) && d == 0.0);
    assert(parse_floating("2.5E-3", d) && d == 0.0025);

    float f = 0.0F;
    assert(parse_floating("1.5", f) && f == 1.5F);
    assert(parse_floating("-8", f) && f == -8.0F);

    // Inputs std::from_chars(general) rejects must be rejected on every
    // standard library, including the stream fallback used on Apple libc++.
    assert(!parse_floating("", d));
    assert(!parse_floating(" 1", d));
    assert(!parse_floating("+1", d));
    assert(!parse_floating("1.5x", d));
    assert(!parse_floating("1 ", d));
    assert(!parse_floating("0x1p3", d));
    assert(!parse_floating("-", d));
    assert(!parse_floating("1e", d));
    assert(!parse_floating("1e400", d));
    assert(!parse_floating("1e40", f));
    return 0;
}
