#include "dmc_rengine/profiles/dmc3/vanilla_runtime_queries.hpp"

#include <cassert>

int main() {
    namespace vanilla = dmc::rengine::profiles::dmc3::vanilla;

    const auto* st600 = vanilla::find_stage_role_alias(600U);
    assert(st600 != nullptr);
    assert(st600->effect_stage_id == 4U);
    assert(st600->sound_stage_id == 4U);
    assert(vanilla::find_stage_role_alias(599U) == nullptr);

    assert(vanilla::observed_resource_load_transition(
        vanilla::ResourceLoadState::free_or_unstarted,
        vanilla::ResourceLoadState::io_scheduled_or_loading));
    assert(vanilla::observed_resource_load_transition(
        vanilla::ResourceLoadState::io_scheduled_or_loading,
        vanilla::ResourceLoadState::io_complete_pending_postprocess));
    assert(vanilla::observed_resource_load_transition(
        vanilla::ResourceLoadState::io_complete_pending_postprocess,
        vanilla::ResourceLoadState::ready_postprocessed));
    assert(vanilla::observed_resource_load_transition(
        vanilla::ResourceLoadState::ready_postprocessed,
        vanilla::ResourceLoadState::teardown_or_cancel_pending));
    assert(vanilla::observed_resource_load_transition(
        vanilla::ResourceLoadState::teardown_or_cancel_pending,
        vanilla::ResourceLoadState::free_or_unstarted));
    assert(!vanilla::observed_resource_load_transition(
        vanilla::ResourceLoadState::free_or_unstarted,
        vanilla::ResourceLoadState::ready_postprocessed));

    const auto enemy_sound = vanilla::owner_resource_domain(6U);
    assert(enemy_sound.has_value());
    assert(*enemy_sound == vanilla::OwnerResourceDomain::enemy_sound);
    assert(!vanilla::owner_resource_domain(7U).has_value());

    const auto* scm = vanilla::find_post_load_fixup("SCM");
    assert(scm != nullptr);
    assert(scm->helper_va == 0x1403051B0ULL);
    assert(vanilla::find_post_load_fixup("DDS") == nullptr);

    const auto* game = vanilla::find_scene_factory_entry(
        vanilla::SceneFactoryId::game);
    assert(game != nullptr);
    assert(game->class_name == "CSceneGame");

    const auto* dante = vanilla::find_player_factory_entry(
        vanilla::PlayerFactorySelector::dante);
    assert(dante != nullptr);
    assert(dante->class_name == "CPlDante");

    assert(vanilla::hd_audio_lookup_key("afs/sound/Battle_01.adx") ==
        "battle_01.ogg");
    assert(vanilla::hd_audio_lookup_key("Battle_00") == "battle_00.ogg");
    assert(vanilla::hd_video_physical_path("Video/m01_s00.sfd") ==
        "Video/m01_s00.wmv");
    assert(vanilla::hd_video_physical_path("LoopDemo") == "LoopDemo.wmv");

    return 0;
}
