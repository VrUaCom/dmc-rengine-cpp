#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <string_view>

namespace dmc::rengine::profiles::dmc3::em034 {

inline constexpr std::uint8_t no_joint = 0xFFU;
inline constexpr std::uint8_t no_component = 0xFFU;

struct Vec3f final {
    float x{};
    float y{};
    float z{};
};

enum class EquipmentRole : std::uint8_t {
    launcher,
    handgun_a,
    handgun_b,
    bowgun,
    machine_gun,
};

enum class PlacementSemantic : std::uint8_t {
    stowed = 0U,
    active_held = 1U,
};

enum class ParentKind : std::uint8_t {
    body_joint,
    internal_runtime_transform,
};

enum class ControlDomain : std::uint8_t {
    body_constraint,
    independent_motion_script,
};

enum class DynamicRole : std::uint8_t {
    preserved_undecoded,
    missile_projectile,
    grapple_assembly,
    bowgun_projectile_domain,
};

struct PlacementRecord final {
    PlacementSemantic semantic{};
    ParentKind parent_kind{ParentKind::body_joint};

    // Serialized table joint. For component3 active placement the canonical
    // consumer bypasses this byte and uses runtime_parent_offset instead.
    std::uint8_t serialized_joint{no_joint};

    // Effective body joint when parent_kind == body_joint.
    std::uint8_t effective_joint{no_joint};

    // CEm034-relative transform owner when parent_kind is the internal runtime
    // transform path. Zero for ordinary body-joint placement.
    std::uint32_t runtime_parent_offset{};

    Vec3f translation{};
    Vec3f rotation_xyz_radians{};
};

struct PersistentComponentContract final {
    std::uint8_t component{};
    std::uint32_t model_slot{};
    EquipmentRole role{};

    std::uint32_t manager_offset{};
    std::uint32_t constraint_host_offset{};
    std::uint32_t auxiliary_offset{};

    std::array<PlacementRecord, 2> placements{};

    bool has_independent_motion_controller{};
    std::uint32_t motion_script_slot{};
    std::uint32_t motion_controller_offset{};
    std::uint32_t motion_control_gate_offset{};
};

inline constexpr std::array<PersistentComponentContract, 5>
    persistent_components{{
        {
            0U,
            20U,
            EquipmentRole::launcher,
            0x08E8U,
            0x4000U,
            0x4080U,
            {{
                {
                    PlacementSemantic::stowed,
                    ParentKind::body_joint,
                    3U,
                    3U,
                    0U,
                    {-2.0F, -20.0F, -17.0F},
                    {-1.57079625F, 0.0F, 1.08210409F},
                },
                {
                    PlacementSemantic::active_held,
                    ParentKind::body_joint,
                    9U,
                    9U,
                    0U,
                    {-8.4F, -1.0F, -1.3F},
                    {0.0F, 0.0F, 3.14159274F},
                },
            }},
            true,
            13U,
            0x52B0U,
            0x4020U,
        },
        {
            1U,
            21U,
            EquipmentRole::handgun_a,
            0x0900U,
            0x40C0U,
            0x4140U,
            {{
                {
                    PlacementSemantic::stowed,
                    ParentKind::body_joint,
                    14U,
                    14U,
                    0U,
                    {-1.0F, -4.0F, 13.0F},
                    {1.86750221F, 0.048869215F, 2.40855432F},
                },
                {
                    PlacementSemantic::active_held,
                    ParentKind::body_joint,
                    9U,
                    9U,
                    0U,
                    {-7.5F, -0.6F, -0.8F},
                    {0.0F, 0.0F, 0.0F},
                },
            }},
            false,
            0U,
            0U,
            0U,
        },
        {
            2U,
            22U,
            EquipmentRole::handgun_b,
            0x0908U,
            0x4180U,
            0x4200U,
            {{
                {
                    PlacementSemantic::stowed,
                    ParentKind::body_joint,
                    16U,
                    16U,
                    0U,
                    {-9.2F, -13.0F, -9.1F},
                    {0.0F, 0.0F, 1.65806270F},
                },
                {
                    PlacementSemantic::active_held,
                    ParentKind::body_joint,
                    13U,
                    13U,
                    0U,
                    {7.7F, -0.8F, 0.5F},
                    {0.0F, 0.0F, 3.14159274F},
                },
            }},
            false,
            0U,
            0U,
            0U,
        },
        {
            3U,
            23U,
            EquipmentRole::bowgun,
            0x0910U,
            0x4240U,
            0x42C0U,
            {{
                {
                    PlacementSemantic::stowed,
                    ParentKind::body_joint,
                    19U,
                    19U,
                    0U,
                    {10.0F, -15.0F, -2.5F},
                    {0.0F, 0.0F, -1.65806270F},
                },
                {
                    PlacementSemantic::active_held,
                    ParentKind::internal_runtime_transform,
                    13U,
                    no_joint,
                    0x43C0U,
                    {7.2F, -1.2F, 2.7F},
                    {0.0F, -0.17453292F, 0.0F},
                },
            }},
            false,
            0U,
            0U,
            0U,
        },
        {
            4U,
            24U,
            EquipmentRole::machine_gun,
            0x0930U,
            0x4300U,
            0x4380U,
            {{
                {
                    PlacementSemantic::stowed,
                    ParentKind::body_joint,
                    14U,
                    14U,
                    0U,
                    {17.0F, -5.0F, -16.0F},
                    {-1.22173047F, -0.23561944F, 0.62831849F},
                },
                {
                    PlacementSemantic::active_held,
                    ParentKind::body_joint,
                    13U,
                    13U,
                    0U,
                    {7.2F, -0.8F, -0.4F},
                    {0.0F, 0.0F, 3.14159274F},
                },
            }},
            false,
            0U,
            0U,
            0U,
        },
    }};

struct DynamicResourceBinding final {
    std::uint32_t model_slot{};
    std::uint32_t texture_slot{};
};

struct DynamicActorContract final {
    std::string_view class_name;
    std::uint64_t factory_va{};
    std::uint64_t deleting_destructor_va{};
    DynamicRole role{DynamicRole::preserved_undecoded};
    std::array<DynamicResourceBinding, 2> resources{};
    std::uint8_t resource_count{};
};

inline constexpr std::array<DynamicActorContract, 6> dynamic_actors{{
    {
        "CEm034Shl00",
        0x140172240ULL,
        0x1401721D0ULL,
        DynamicRole::preserved_undecoded,
        {},
        0U,
    },
    {
        "CEm034Shl01",
        0x1401729D0ULL,
        0x140172960ULL,
        DynamicRole::preserved_undecoded,
        {},
        0U,
    },
    {
        "CEm034Shl02",
        0x140173620ULL,
        0x1401735E0ULL,
        DynamicRole::missile_projectile,
        {{{25U, 19U}, {0U, 0U}}},
        1U,
    },
    {
        "CEm034Shl03",
        0x1401745F0ULL,
        0x140174010ULL,
        DynamicRole::grapple_assembly,
        {{{26U, 19U}, {30U, 29U}}},
        2U,
    },
    {
        "CEm034Shl04",
        0x140175210ULL,
        0x1401751A0ULL,
        DynamicRole::preserved_undecoded,
        {},
        0U,
    },
    {
        "CEm034Shl05",
        0x140175B10ULL,
        0x140175AA0ULL,
        DynamicRole::bowgun_projectile_domain,
        {},
        0U,
    },
}};

struct SignalPlacementTransition final {
    std::uint8_t state{};
    std::int16_t action{};  // -1 when this state is outside the closed bank4 map.
    std::uint8_t lane{};
    std::uint8_t channel{};
    std::uint8_t value{};
    std::uint8_t component{};
    PlacementSemantic placement{};
    ControlDomain control_domain{ControlDomain::body_constraint};
};

inline constexpr std::array<SignalPlacementTransition, 10>
    signal_placement_transitions{{
        {0x7BU, 40, 0U, 0U, 1U, 0U, PlacementSemantic::active_held,
         ControlDomain::body_constraint},
        {0x5CU, 9, 0U, 0U, 1U, 0U, PlacementSemantic::active_held,
         ControlDomain::body_constraint},
        {0x5DU, 10, 0U, 0U, 1U, 0U, PlacementSemantic::active_held,
         ControlDomain::body_constraint},
        {0x59U, 6, 0U, 0U, 1U, 0U, PlacementSemantic::stowed,
         ControlDomain::independent_motion_script},
        {0x61U, 14, 0U, 0U, 1U, 0U, PlacementSemantic::stowed,
         ControlDomain::independent_motion_script},
        {0x2AU, -1, 0U, 0U, 1U, 1U, PlacementSemantic::active_held,
         ControlDomain::body_constraint},
        {0x2AU, -1, 0U, 0U, 2U, 1U, PlacementSemantic::stowed,
         ControlDomain::body_constraint},
        {0x81U, 46, 1U, 2U, 1U, 3U, PlacementSemantic::stowed,
         ControlDomain::body_constraint},
        {0x85U, 50, 1U, 1U, 1U, 4U, PlacementSemantic::active_held,
         ControlDomain::body_constraint},
        {0x85U, 50, 1U, 1U, 2U, 4U, PlacementSemantic::stowed,
         ControlDomain::body_constraint},
    }};

struct StateEntryPlacementOverride final {
    std::uint8_t state{};
    std::int16_t action{};
    std::uint8_t component{};
    PlacementSemantic placement{};
    ControlDomain control_domain{ControlDomain::body_constraint};
};

inline constexpr std::array<StateEntryPlacementOverride, 13>
    state_entry_placement_overrides{{
        {0x60U, 13, 0U, PlacementSemantic::active_held, ControlDomain::body_constraint},
        {0x60U, 13, 4U, PlacementSemantic::active_held, ControlDomain::body_constraint},
        {0x63U, 16, 0U, PlacementSemantic::active_held, ControlDomain::body_constraint},
        {0x63U, 16, 4U, PlacementSemantic::active_held, ControlDomain::body_constraint},
        {0x7FU, 44, 1U, PlacementSemantic::active_held, ControlDomain::body_constraint},
        {0x7FU, 44, 2U, PlacementSemantic::active_held, ControlDomain::body_constraint},
        {0x81U, 46, 3U, PlacementSemantic::active_held, ControlDomain::body_constraint},
        {0x85U, 50, 4U, PlacementSemantic::stowed, ControlDomain::body_constraint},
        {0x7BU, 40, 0U, PlacementSemantic::stowed,
         ControlDomain::independent_motion_script},
        {0x7CU, 41, 0U, PlacementSemantic::active_held,
         ControlDomain::independent_motion_script},
        {0x7DU, 42, 0U, PlacementSemantic::active_held,
         ControlDomain::independent_motion_script},
        {0x5FU, 12, 0U, PlacementSemantic::active_held, ControlDomain::body_constraint},
        {0x62U, 15, 0U, PlacementSemantic::active_held, ControlDomain::body_constraint},
    }};

[[nodiscard]] constexpr const PersistentComponentContract*
component(std::uint8_t index) noexcept {
    return index < persistent_components.size()
        ? &persistent_components[index]
        : nullptr;
}

[[nodiscard]] constexpr const PersistentComponentContract*
component_for_slot(std::uint32_t slot) noexcept {
    for (const auto& candidate : persistent_components) {
        if (candidate.model_slot == slot) return &candidate;
    }
    return nullptr;
}

[[nodiscard]] constexpr const PlacementRecord*
placement(std::uint8_t component_index, PlacementSemantic semantic) noexcept {
    const auto* c = component(component_index);
    if (c == nullptr) return nullptr;
    const auto index = static_cast<std::size_t>(semantic);
    return index < c->placements.size() ? &c->placements[index] : nullptr;
}

[[nodiscard]] constexpr bool persistent_model_slot(std::uint32_t slot) noexcept {
    return component_for_slot(slot) != nullptr;
}

[[nodiscard]] constexpr bool dynamic_model_slot(std::uint32_t slot) noexcept {
    for (const auto& actor : dynamic_actors) {
        for (std::size_t i = 0U; i < actor.resource_count; ++i) {
            if (actor.resources[i].model_slot == slot) return true;
        }
    }
    return false;
}

[[nodiscard]] constexpr std::string_view to_string(EquipmentRole role) noexcept {
    switch (role) {
    case EquipmentRole::launcher: return "launcher";
    case EquipmentRole::handgun_a: return "handgun-a";
    case EquipmentRole::handgun_b: return "handgun-b";
    case EquipmentRole::bowgun: return "bowgun";
    case EquipmentRole::machine_gun: return "machine-gun";
    }
    return "unknown";
}

[[nodiscard]] constexpr std::string_view to_string(
    PlacementSemantic semantic) noexcept {
    switch (semantic) {
    case PlacementSemantic::stowed: return "stowed";
    case PlacementSemantic::active_held: return "active-held";
    }
    return "unknown";
}

} // namespace dmc::rengine::profiles::dmc3::em034
