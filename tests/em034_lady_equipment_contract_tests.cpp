#include "dmc_rengine/profiles/dmc3/em034_lady_equipment_contract.hpp"

#include <array>
#include <cassert>
#include <cstdint>
#include <string_view>

int main() {
    namespace lady = dmc::rengine::profiles::dmc3::em034;

    static_assert(lady::persistent_components.size() == 5U);
    static_assert(lady::dynamic_actors.size() == 6U);
    static_assert(lady::signal_placement_transitions.size() == 10U);

    const std::array<std::uint32_t, 5> expected_slots{20U, 21U, 22U, 23U, 24U};
    for (std::size_t i = 0U; i < expected_slots.size(); ++i) {
        const auto* component = lady::component(static_cast<std::uint8_t>(i));
        assert(component != nullptr);
        assert(component->component == i);
        assert(component->model_slot == expected_slots[i]);
        assert(lady::persistent_model_slot(expected_slots[i]));
        assert(!lady::dynamic_model_slot(expected_slots[i]));

        const auto* stowed =
            lady::placement(static_cast<std::uint8_t>(i),
                            lady::PlacementSemantic::stowed);
        const auto* active =
            lady::placement(static_cast<std::uint8_t>(i),
                            lady::PlacementSemantic::active_held);
        assert(stowed != nullptr);
        assert(active != nullptr);
        assert(stowed->semantic == lady::PlacementSemantic::stowed);
        assert(active->semantic == lady::PlacementSemantic::active_held);
    }

    assert(lady::component(5U) == nullptr);
    assert(lady::component_for_slot(19U) == nullptr);
    assert(lady::component_for_slot(25U) == nullptr);

    const auto* launcher = lady::component(0U);
    assert(launcher != nullptr);
    assert(launcher->role == lady::EquipmentRole::launcher);
    assert(launcher->has_independent_motion_controller);
    assert(launcher->motion_script_slot == 13U);
    assert(launcher->motion_controller_offset == 0x52B0U);
    assert(launcher->motion_control_gate_offset == 0x4020U);
    assert(launcher->placements[0].effective_joint == 3U);
    assert(launcher->placements[1].effective_joint == 9U);

    const auto* handgun_a = lady::component(1U);
    const auto* handgun_b = lady::component(2U);
    assert(handgun_a != nullptr && handgun_b != nullptr);
    assert(handgun_a->role == lady::EquipmentRole::handgun_a);
    assert(handgun_b->role == lady::EquipmentRole::handgun_b);
    assert(handgun_a->placements[1].effective_joint == 9U);
    assert(handgun_b->placements[1].effective_joint == 13U);

    const auto* bowgun = lady::component(3U);
    assert(bowgun != nullptr);
    assert(bowgun->role == lady::EquipmentRole::bowgun);
    assert(bowgun->placements[0].effective_joint == 19U);
    assert(bowgun->placements[1].parent_kind ==
           lady::ParentKind::internal_runtime_transform);
    assert(bowgun->placements[1].serialized_joint == 13U);
    assert(bowgun->placements[1].effective_joint == lady::no_joint);
    assert(bowgun->placements[1].runtime_parent_offset == 0x43C0U);

    const auto* machine_gun = lady::component(4U);
    assert(machine_gun != nullptr);
    assert(machine_gun->role == lady::EquipmentRole::machine_gun);
    assert(machine_gun->placements[0].effective_joint == 14U);
    assert(machine_gun->placements[1].effective_joint == 13U);

    assert(lady::dynamic_model_slot(25U));
    assert(lady::dynamic_model_slot(26U));
    assert(lady::dynamic_model_slot(30U));
    assert(!lady::persistent_model_slot(25U));
    assert(!lady::persistent_model_slot(26U));
    assert(!lady::persistent_model_slot(30U));

    const auto& shl02 = lady::dynamic_actors[2];
    assert(shl02.class_name == std::string_view{"CEm034Shl02"});
    assert(shl02.role == lady::DynamicRole::missile_projectile);
    assert(shl02.resource_count == 1U);
    assert(shl02.resources[0].model_slot == 25U);
    assert(shl02.resources[0].texture_slot == 19U);

    const auto& shl03 = lady::dynamic_actors[3];
    assert(shl03.class_name == std::string_view{"CEm034Shl03"});
    assert(shl03.role == lady::DynamicRole::grapple_assembly);
    assert(shl03.resource_count == 2U);
    assert(shl03.resources[0].model_slot == 26U);
    assert(shl03.resources[0].texture_slot == 19U);
    assert(shl03.resources[1].model_slot == 30U);
    assert(shl03.resources[1].texture_slot == 29U);

    bool action50_to_active = false;
    bool action50_to_stowed = false;
    bool action50_channel0_placement = false;
    for (const auto& transition : lady::signal_placement_transitions) {
        if (transition.state != 0x85U) continue;
        if (transition.channel == 0U) action50_channel0_placement = true;
        if (transition.lane != 1U || transition.channel != 1U ||
            transition.component != 4U) {
            continue;
        }
        if (transition.value == 1U &&
            transition.placement == lady::PlacementSemantic::active_held) {
            action50_to_active = true;
        }
        if (transition.value == 2U &&
            transition.placement == lady::PlacementSemantic::stowed) {
            action50_to_stowed = true;
        }
    }
    assert(action50_to_active);
    assert(action50_to_stowed);
    assert(!action50_channel0_placement);

    bool action46_restore = false;
    for (const auto& transition : lady::signal_placement_transitions) {
        if (transition.state == 0x81U &&
            transition.action == 46 &&
            transition.lane == 1U &&
            transition.channel == 2U &&
            transition.value == 1U &&
            transition.component == 3U &&
            transition.placement == lady::PlacementSemantic::stowed) {
            action46_restore = true;
        }
    }
    assert(action46_restore);

    bool action44_handgun_a = false;
    bool action44_handgun_b = false;
    for (const auto& entry : lady::state_entry_placement_overrides) {
        if (entry.state != 0x7FU || entry.action != 44 ||
            entry.placement != lady::PlacementSemantic::active_held) {
            continue;
        }
        action44_handgun_a = action44_handgun_a || entry.component == 1U;
        action44_handgun_b = action44_handgun_b || entry.component == 2U;
    }
    assert(action44_handgun_a);
    assert(action44_handgun_b);

    assert(lady::to_string(lady::EquipmentRole::launcher) == "launcher");
    assert(lady::to_string(lady::EquipmentRole::bowgun) == "bowgun");
    assert(lady::to_string(lady::PlacementSemantic::stowed) == "stowed");
    assert(lady::to_string(lady::PlacementSemantic::active_held) == "active-held");

    return 0;
}
