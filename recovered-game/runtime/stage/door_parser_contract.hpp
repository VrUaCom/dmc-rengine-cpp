#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <string_view>

namespace dmc::recovered::dmc3::runtime::stage {

// This unit captures only the directly reconciled vanilla parser contract.
// It deliberately does not define the complete Door runtime record/class ABI:
// the current evidence closes parser grammar/capacity and selected record-field
// offsets, not every field, constructor, owner or lifetime edge.
enum class DoorBoxInField : std::uint32_t {
    x = 0U,
    y,
    z,
    radius_x,
    radius_y,
    radius_z,
    yaw,
};

enum class DoorBoxLoadField : std::uint32_t {
    x0 = 0U,
    z0,
    x1,
    z1,
    x2,
    z2,
    x3,
    z3,
    y_min,
    y_max,
};

struct DoorParserContract final {
    std::string_view artifact_sha256;

    // Stable repo-local evidence records supporting this recovered contract.
    std::array<std::string_view, 4> evidence_ids{};

    // Corrected logical function start. The older 0x1401A9E3B anchor is an
    // internal loop location and must not be treated as the function start.
    std::uint64_t logical_start_va{};

    // Directly confirmed vanilla table capacity.
    std::uint32_t door_capacity{};

    // BoxIn grammar:
    //   x y z radiusX radiusY radiusZ yaw
    std::array<DoorBoxInField, 7> box_in_fields{};

    // BoxLoad grammar:
    //   (x0,z0) (x1,z1) (x2,z2) (x3,z3) yMin yMax
    std::array<DoorBoxLoadField, 10> box_load_fields{};

    // Confirmed AUTO path anchor.
    std::uint64_t auto_path_va{};

    // Corrected selected runtime-record writes observed downstream of parsing.
    // Their semantic field names remain intentionally unspecified here.
    std::uint32_t observed_record_field_offset_c8{};
    std::uint32_t observed_record_field_offset_d4{};

    // Explicit evidence boundary: grammar and selected writes are closed, but
    // the complete Door runtime record/object ABI is not.
    bool full_runtime_record_layout_complete{};

    [[nodiscard]] bool valid() const noexcept;
};

[[nodiscard]] constexpr std::string_view to_string(
    DoorBoxInField field) noexcept {
    switch (field) {
    case DoorBoxInField::x: return "x";
    case DoorBoxInField::y: return "y";
    case DoorBoxInField::z: return "z";
    case DoorBoxInField::radius_x: return "radiusX";
    case DoorBoxInField::radius_y: return "radiusY";
    case DoorBoxInField::radius_z: return "radiusZ";
    case DoorBoxInField::yaw: return "yaw";
    }
    return "unknown";
}

[[nodiscard]] constexpr std::string_view to_string(
    DoorBoxLoadField field) noexcept {
    switch (field) {
    case DoorBoxLoadField::x0: return "x0";
    case DoorBoxLoadField::z0: return "z0";
    case DoorBoxLoadField::x1: return "x1";
    case DoorBoxLoadField::z1: return "z1";
    case DoorBoxLoadField::x2: return "x2";
    case DoorBoxLoadField::z2: return "z2";
    case DoorBoxLoadField::x3: return "x3";
    case DoorBoxLoadField::z3: return "z3";
    case DoorBoxLoadField::y_min: return "yMin";
    case DoorBoxLoadField::y_max: return "yMax";
    }
    return "unknown";
}

// Canonical reconciled parser contract for target e454... . This is recovered
// game-source metadata, not a Stage Ops grammar inference and not a substitute
// for the still-unrecovered full Door runtime object/lifetime implementation.
[[nodiscard]] const DoorParserContract& door_parser_contract() noexcept;

} // namespace dmc::recovered::dmc3::runtime::stage
