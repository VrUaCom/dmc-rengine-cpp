#include "dmc_rengine/profiles/dmc3/vanilla_runtime.hpp"

#include <cassert>
#include <cstdint>

int main() {
    namespace vanilla = dmc::rengine::profiles::dmc3::vanilla;

    assert(vanilla::stage_resolver.selector_count == 193U);
    assert(vanilla::stage_resolver.group_count == 10U);
    assert(vanilla::stage_600_aliases.size() == 13U);
    assert(vanilla::stage_600_aliases.front().stage_id == 600U);
    assert(vanilla::stage_600_aliases.front().effect_stage_id == 4U);
    assert(vanilla::stage_600_aliases.back().stage_id == 612U);
    assert(vanilla::stage_600_aliases.back().effect_stage_id == 300U);

    std::uint32_t pool_total{};
    for (const auto count : vanilla::resource_load_pool.group_counts) {
        pool_total += count;
    }
    assert(pool_total == vanilla::resource_load_pool.entry_count);
    assert(vanilla::resource_load_pool.entry_count == 363U);
    assert(vanilla::resource_load_pool.entry_stride == 0x48U);
    assert(vanilla::resource_load_pool.cumulative_offsets.back() == 363U);
    assert(vanilla::owner_cache.observed_capacity == 32U);
    assert(vanilla::owner_cache.refcount_offset == 0x2CU);

    assert(vanilla::to_string(vanilla::OwnerResourceDomain::stage_script) ==
        "stage-script");
    assert(vanilla::to_string(vanilla::OwnerResourceDomain::enemy_sound) ==
        "enemy-sound");

    assert(vanilla::post_load_fixups.size() == 5U);
    assert(vanilla::post_load_fixups[0].magic == "MOD");
    assert(vanilla::post_load_fixups[2].helper_va == 0x1403051B0ULL);
    assert(vanilla::post_load_fixups[4].recursive_container);

    assert(vanilla::scene_factory.size() == 10U);
    assert(vanilla::scene_factory[4].class_name == "CSceneGame");
    assert(vanilla::scene_factory[5].allocation_size == 0x1BA30U);
    assert(vanilla::root_scene_factory.global_va == 0x140C8F970ULL);
    assert(vanilla::root_scene_factory.root_scene_manager_offset == 0x1478U);

    assert(vanilla::player_factory.size() == 4U);
    assert(vanilla::player_factory[0].class_name == "CPlDante");
    assert(vanilla::enemy_factory.selector_count == 46U);
    assert(vanilla::enemy_factory.external_mapping_count == 64U);

    assert(vanilla::numeric_resource_resolver.master_name_count == 4039U);
    assert(vanilla::numeric_resource_resolver.locale0_valid_id_count == 932U);
    assert(vanilla::numeric_resource_resolver.locale0_range_count == 145U);
    assert(vanilla::master_catalog_counts.pac == 3398U);
    assert(vanilla::master_catalog_counts.adx == 154U);

    assert(vanilla::demo_asset_registry_capacity == 1024U);
    assert(vanilla::motion_runtime.object_size == 0x220U);
    assert(vanilla::motion_runtime.track_count == 8U);

    assert(vanilla::hd_audio.record_count == 154U);
    assert(vanilla::hd_audio.record_stride == 0x10U);
    assert(vanilla::hd_audio_descriptor_abi.loop_start_ms_offset == 0x08U);
    assert(vanilla::hd_audio_descriptor_abi.loop_end_ms_offset == 0x0CU);
    assert(vanilla::hd_audio_physical_extension == ".ogg");
    assert(vanilla::hd_video_physical_extension == ".wmv");

    assert(vanilla::graphics_runtime.d3d11_device_offset == 0x08U);
    assert(vanilla::graphics_runtime.d3d11_context_offset == 0x10U);
    assert(vanilla::graphics_runtime.dxgi_swap_chain_offset == 0x18U);
    assert(vanilla::gfx_texture_pool.bank_count == 2U);
    assert(vanilla::gfx_texture_pool.textures_per_bank == 450U);
    assert(vanilla::audio_channel_groups.size() == 11U);

    assert(vanilla::memory_arena.central_bytes == 256ULL * 1024ULL * 1024ULL);
    assert(vanilla::stage_set_abi_families.size() == 4U);
    assert(vanilla::stage_set_abi_families[0].stage_set_subobject_offset == 0x110U);
    assert(!vanilla::stage_set_abi_families[3].exposes_stage_set_rtti_base);

    return 0;
}
