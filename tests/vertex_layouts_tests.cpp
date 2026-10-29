// DMC3 HD vertex formats (profiles/dmc3/vertex_layouts.hpp): every stride the
// engine stores is the one its FVF bits give with DIFFUSE as four floats.
#include "dmc_rengine/profiles/dmc3/vertex_layouts.hpp"

#include <cassert>

namespace layouts = dmc::rengine::profiles::dmc3::vertex_layouts;

int main() {
    for (const auto& format : layouts::formats) {
        assert(layouts::fvf_stride(format.fvf) == format.stride);
    }
    // A packed-dword DIFFUSE would make XYZ|DIFFUSE 16 bytes; the engine stores 0x1C.
    static_assert(layouts::fvf_stride(layouts::fvf_xyz | layouts::fvf_diffuse) == 0x1CU);
    static_assert(layouts::formats[2].shader_id == 0xD && layouts::formats[2].stride == 0x30U);
    return 0;
}
