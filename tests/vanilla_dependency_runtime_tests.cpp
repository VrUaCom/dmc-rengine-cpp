#include "dmc_rengine/profiles/dmc3/vanilla_dependency_runtime.hpp"

#include <cassert>

int main() {
    namespace vanilla = dmc::rengine::profiles::dmc3::vanilla;

    assert(vanilla::observed_stage_load_order.size() == 11U);
    for (std::size_t index = 0U;
         index + 1U < vanilla::observed_stage_load_order.size();
         ++index) {
        assert(vanilla::observed_stage_load_transition(
            vanilla::observed_stage_load_order[index],
            vanilla::observed_stage_load_order[index + 1U]));
    }

    assert(!vanilla::observed_stage_load_transition(
        vanilla::StageLoadPhase::request_stage_config,
        vanilla::StageLoadPhase::request_stage_script));
    assert(vanilla::stage_config_discovers_enemy_dependencies());
    assert(vanilla::enemy_dependency_model.external_enemy_id_count == 64U);
    assert(vanilla::enemy_dependency_model.object_domain ==
        vanilla::OwnerResourceDomain::enemy_object);
    assert(vanilla::enemy_dependency_model.sound_domain ==
        vanilla::OwnerResourceDomain::enemy_sound);
    assert(vanilla::enemy_dependency_model.class_selector_separate_from_resource_selector);
    assert(vanilla::enemy_dependency_model.deduplicate_resource_sets_before_preload);

    return 0;
}
