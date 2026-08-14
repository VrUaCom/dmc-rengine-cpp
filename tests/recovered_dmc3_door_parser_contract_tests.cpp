#include "runtime/stage/door_parser_contract.hpp"

#include <array>
#include <cassert>
#include <string_view>

int main() {
    using namespace dmc::recovered::dmc3::runtime::stage;

    const auto& contract = door_parser_contract();
    assert(contract.valid());
    assert(contract.artifact_sha256 ==
        "e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082");
    assert(contract.logical_start_va == 0x1401A9DE0ULL);
    assert(contract.door_capacity == 12U);
    assert(contract.auto_path_va == 0x1401AB6D0ULL);
    assert(contract.observed_record_field_offset_c8 == 0xC8U);
    assert(contract.observed_record_field_offset_d4 == 0xD4U);
    assert(!contract.full_runtime_record_layout_complete);

    constexpr std::array<std::string_view, 7> expected_box_in{
        "x", "y", "z", "radiusX", "radiusY", "radiusZ", "yaw"};
    for (std::size_t index = 0U; index < contract.box_in_fields.size(); ++index) {
        assert(to_string(contract.box_in_fields[index]) == expected_box_in[index]);
    }

    constexpr std::array<std::string_view, 10> expected_box_load{
        "x0", "z0", "x1", "z1", "x2", "z2", "x3", "z3",
        "yMin", "yMax"};
    for (std::size_t index = 0U; index < contract.box_load_fields.size(); ++index) {
        assert(to_string(contract.box_load_fields[index]) ==
            expected_box_load[index]);
    }

    return 0;
}
