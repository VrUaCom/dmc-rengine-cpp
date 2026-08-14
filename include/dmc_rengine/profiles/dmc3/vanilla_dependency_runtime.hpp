#pragma once

#include "dmc_rengine/profiles/dmc3/vanilla_runtime.hpp"

#include <array>
#include <cstdint>
#include <string_view>

namespace dmc::rengine::profiles::dmc3::vanilla {

// Evidence-backed ordering recovered from the DMC3 stage-load path. This is a
// description/validation contract for the original game runtime, not a
// replacement scheduler for GDSpaces.
enum class StageLoadPhase : std::uint32_t {
    request_stage_config = 0U,
    wait_stage_config_ready = 1U,
    scan_config_enemy_ids = 2U,
    resolve_enemy_resource_sets = 3U,
    deduplicate_enemy_resource_sets = 4U,
    preload_enemy_objects = 5U,
    preload_enemy_sounds = 6U,
    request_stage_script = 7U,
    request_stage_effect = 8U,
    wait_pending_dependencies = 9U,
    stage_dependency_set_ready = 10U,
};

[[nodiscard]] constexpr std::string_view to_string(StageLoadPhase phase) noexcept {
    switch (phase) {
    case StageLoadPhase::request_stage_config: return "request-stage-config";
    case StageLoadPhase::wait_stage_config_ready: return "wait-stage-config-ready";
    case StageLoadPhase::scan_config_enemy_ids: return "scan-config-enemy-ids";
    case StageLoadPhase::resolve_enemy_resource_sets: return "resolve-enemy-resource-sets";
    case StageLoadPhase::deduplicate_enemy_resource_sets: return "deduplicate-enemy-resource-sets";
    case StageLoadPhase::preload_enemy_objects: return "preload-enemy-objects";
    case StageLoadPhase::preload_enemy_sounds: return "preload-enemy-sounds";
    case StageLoadPhase::request_stage_script: return "request-stage-script";
    case StageLoadPhase::request_stage_effect: return "request-stage-effect";
    case StageLoadPhase::wait_pending_dependencies: return "wait-pending-dependencies";
    case StageLoadPhase::stage_dependency_set_ready: return "stage-dependency-set-ready";
    }
    return "request-stage-config";
}

inline constexpr std::array<StageLoadPhase, 11> observed_stage_load_order{{
    StageLoadPhase::request_stage_config,
    StageLoadPhase::wait_stage_config_ready,
    StageLoadPhase::scan_config_enemy_ids,
    StageLoadPhase::resolve_enemy_resource_sets,
    StageLoadPhase::deduplicate_enemy_resource_sets,
    StageLoadPhase::preload_enemy_objects,
    StageLoadPhase::preload_enemy_sounds,
    StageLoadPhase::request_stage_script,
    StageLoadPhase::request_stage_effect,
    StageLoadPhase::wait_pending_dependencies,
    StageLoadPhase::stage_dependency_set_ready,
}};

struct EnemyDependencyResolutionModel final {
    std::uint32_t external_enemy_id_count;
    OwnerResourceDomain object_domain;
    OwnerResourceDomain sound_domain;
    bool class_selector_separate_from_resource_selector;
    bool deduplicate_resource_sets_before_preload;
};

inline constexpr EnemyDependencyResolutionModel enemy_dependency_model{
    .external_enemy_id_count = 64U,
    .object_domain = OwnerResourceDomain::enemy_object,
    .sound_domain = OwnerResourceDomain::enemy_sound,
    .class_selector_separate_from_resource_selector = true,
    .deduplicate_resource_sets_before_preload = true,
};

[[nodiscard]] bool observed_stage_load_transition(
    StageLoadPhase from,
    StageLoadPhase to) noexcept;

[[nodiscard]] constexpr bool stage_config_discovers_enemy_dependencies() noexcept {
    return true;
}

} // namespace dmc::rengine::profiles::dmc3::vanilla
