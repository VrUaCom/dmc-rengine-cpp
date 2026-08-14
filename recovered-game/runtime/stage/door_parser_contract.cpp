#include "runtime/stage/door_parser_contract.hpp"

#include <algorithm>

namespace dmc::recovered::dmc3::runtime::stage {

bool DoorParserContract::valid() const noexcept {
    if (artifact_sha256.empty() ||
        logical_start_va == 0U ||
        door_capacity == 0U ||
        auto_path_va == 0U ||
        observed_record_field_offset_c8 == 0U ||
        observed_record_field_offset_d4 == 0U ||
        full_runtime_record_layout_complete ||
        std::any_of(
            evidence_ids.begin(), evidence_ids.end(),
            [](std::string_view id) { return id.empty(); })) {
        return false;
    }

    constexpr std::array expected_box_in{
        DoorBoxInField::x,
        DoorBoxInField::y,
        DoorBoxInField::z,
        DoorBoxInField::radius_x,
        DoorBoxInField::radius_y,
        DoorBoxInField::radius_z,
        DoorBoxInField::yaw,
    };
    constexpr std::array expected_box_load{
        DoorBoxLoadField::x0,
        DoorBoxLoadField::z0,
        DoorBoxLoadField::x1,
        DoorBoxLoadField::z1,
        DoorBoxLoadField::x2,
        DoorBoxLoadField::z2,
        DoorBoxLoadField::x3,
        DoorBoxLoadField::z3,
        DoorBoxLoadField::y_min,
        DoorBoxLoadField::y_max,
    };

    return box_in_fields == expected_box_in &&
        box_load_fields == expected_box_load;
}

const DoorParserContract& door_parser_contract() noexcept {
    static constexpr DoorParserContract contract{
        .artifact_sha256 =
            "e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082",
        .evidence_ids = {
            "ev-dmc3-door-parser-logical-function",
            "ev-dmc3-door-table-capacity",
            "ev-dmc3-door-box-grammar",
            "ev-dmc3-door-selected-record-writes",
        },
        .logical_start_va = 0x1401A9DE0ULL,
        .door_capacity = 12U,
        .box_in_fields = {
            DoorBoxInField::x,
            DoorBoxInField::y,
            DoorBoxInField::z,
            DoorBoxInField::radius_x,
            DoorBoxInField::radius_y,
            DoorBoxInField::radius_z,
            DoorBoxInField::yaw,
        },
        .box_load_fields = {
            DoorBoxLoadField::x0,
            DoorBoxLoadField::z0,
            DoorBoxLoadField::x1,
            DoorBoxLoadField::z1,
            DoorBoxLoadField::x2,
            DoorBoxLoadField::z2,
            DoorBoxLoadField::x3,
            DoorBoxLoadField::z3,
            DoorBoxLoadField::y_min,
            DoorBoxLoadField::y_max,
        },
        .auto_path_va = 0x1401AB6D0ULL,
        .observed_record_field_offset_c8 = 0xC8U,
        .observed_record_field_offset_d4 = 0xD4U,
        .full_runtime_record_layout_complete = false,
    };
    return contract;
}

} // namespace dmc::recovered::dmc3::runtime::stage
