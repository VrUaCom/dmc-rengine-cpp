#include "dmc_rengine/profiles/dmc3/known_targets.hpp"

#include <cassert>
#include <string>

int main() {
    const auto& target = dmc::rengine::profiles::dmc3::phase12_canonical_target();
    assert(target.valid());
    assert(target.matches_hash(
        "e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082"));
    assert(target.matches_hash(
        "E454272ED0FB0247FCBCF300E5D55D7A3E96D50B89B9FFAFF81BB978DCBDD082"));
    assert(!target.matches_hash(
        "0000000000000000000000000000000000000000000000000000000000000000"));

    dmc::rengine::exe::PeImage image{
        .kind = dmc::rengine::exe::PeKind::pe32_plus,
        .machine = dmc::rengine::exe::PeMachine::amd64,
        .section_count = 1,
        .image_base = 0x140000000ULL,
        .entry_point_rva = 0x0034615CU,
        .size_of_image = 0x1000,
        .size_of_headers = 0x200,
        .subsystem = 2,
        .sections = {},
    };
    assert(target.matches_metadata(image));

    image.entry_point_rva = 0x1000;
    assert(!target.matches_metadata(image));

    return 0;
}
