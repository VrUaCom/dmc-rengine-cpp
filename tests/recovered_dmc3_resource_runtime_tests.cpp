#include "runtime/resources/resource_lifecycle.hpp"
#include "runtime/wave3_runtime.hpp"

#include <array>
#include <cassert>
#include <cstdint>

int main() {
    using namespace dmc::recovered::dmc3::runtime;
    using namespace dmc::recovered::dmc3::runtime::resources;

    const auto& wave2 = wave2_resource_runtime_model();
    assert(wave2.valid());
    assert(wave2.artifact_sha256 ==
        "e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082");
    assert(wave2.evidence_packet_id == "dmc3-stage-wave2");

    const auto& pool = wave2.pool;
    assert(pool.valid());
    assert(pool.base_va == 0x140C99D30ULL);
    assert(pool.entry_count == 363U);
    assert(pool.entry.stride == 0x48U);
    assert(pool.entry.group_index_offset == 0x00U);
    assert(pool.entry.state_offset == 0x04U);
    assert(pool.entry.subtype_index_offset == 0x08U);
    assert(pool.entry.source_descriptor_offset == 0x18U);
    assert(pool.entry.loaded_payload_offset == 0x20U);
    assert(pool.entry.owned_state_offset == 0x28U);

    constexpr std::array<std::uint32_t, 7> counts{
        4U, 136U, 60U, 28U, 1U, 128U, 6U,
    };
    constexpr std::array<std::uint32_t, 7> offsets{
        0U, 4U, 140U, 200U, 228U, 229U, 357U,
    };
    for (std::uint32_t index = 0U; index < counts.size(); ++index) {
        const auto* group = pool.group(index);
        assert(group != nullptr);
        assert(group->first_entry == offsets[index]);
        assert(group->entry_count == counts[index]);
    }
    assert(pool.group(7U) == nullptr);
    assert(pool.groups.back().end_entry() == 363U);

    constexpr std::array<ResourceLoadState, 4> expected_state_chain{
        ResourceLoadState::free_or_unstarted,
        ResourceLoadState::io_scheduled_or_loading,
        ResourceLoadState::io_complete_pending_postprocess,
        ResourceLoadState::ready_postprocessed,
    };
    assert(wave2.successful_state_chain == expected_state_chain);
    assert(to_string(ResourceLoadState::teardown_or_cancel_pending) ==
        "teardown-or-cancel-pending");
    assert(wave2.cleanup_transition.from ==
        ResourceLoadState::teardown_or_cancel_pending);
    assert(wave2.cleanup_transition.to ==
        ResourceLoadState::free_or_unstarted);
    assert(!wave2.teardown_source_state_domain_complete);

    const auto* mod = wave2.fixup(PostLoadFormat::mod);
    const auto* efm = wave2.fixup(PostLoadFormat::efm);
    const auto* scm = wave2.fixup(PostLoadFormat::scm);
    const auto* shw = wave2.fixup(PostLoadFormat::shw);
    assert(mod != nullptr && mod->function_va == 0x1402FE3B0ULL);
    assert(efm != nullptr && efm->function_va == 0x1402F7A90ULL);
    assert(scm != nullptr && scm->function_va == 0x1403051B0ULL);
    assert(shw != nullptr && shw->function_va == 0x1403204C0ULL);

    const auto& wave3 = wave3_runtime_evidence_model();
    assert(wave3.valid());
    assert(wave3.artifact_sha256 == kCanonicalDmc3Sha256);

    // Corrected Stage ABI and the complete descriptor universe.
    assert(wave3.stage.cell.kind16_offset == 0x00U);
    assert(wave3.stage.cell.path_pointer_offset == 0x08U);
    assert(wave3.stage.bank_a.descriptor_count == 110U);
    assert(wave3.stage.bank_b.descriptor_count == 79U);
    assert(wave3.stage.selector_count == 193U);
    assert(wave3.stage.group_base_count == 10U);
    assert(is_observed_stage_descriptor_id(0U));
    assert(is_observed_stage_descriptor_id(313U));
    assert(is_observed_stage_descriptor_id(612U));
    assert(is_observed_stage_descriptor_id(910U));
    assert(!is_observed_stage_descriptor_id(447U));
    assert(!is_observed_stage_descriptor_id(500U));
    assert(stage_resource_aliases().front().stage_id == 600U);
    assert(stage_resource_aliases().front().effect_stage_id == 4U);
    assert(stage_resource_aliases().back().stage_id == 612U);
    assert(stage_resource_aliases().back().effect_stage_id == 300U);

    const auto stage_parts = split_numeric_stage_id(612U);
    assert(stage_parts.hundreds == 6U);
    assert(stage_parts.remainder == 12U);

    // Global loader plus owner-local cache/refcount evidence.
    assert(wave3.owner_cache.observed_node_capacity == 32U);
    assert(wave3.owner_cache.refcount_offset == 0x2CU);
    assert(resource_domain_evidence()[0].domain == ResourceDomain::stage_script);
    assert(resource_domain_evidence()[5].domain == ResourceDomain::enemy_object);
    assert(resource_domain_evidence()[6].domain == ResourceDomain::enemy_sound);

    // .lst is a recursive manifest/fallback layer, not a filename alias.
    assert(wave3.list_manifest.primary_or_list_probe_va == 0x1401B79E0ULL);
    assert(wave3.list_manifest.nested_lists);
    assert(wave3.list_manifest.rewrites_entries_to_pac);

    // Concrete gameplay construction boundaries.
    assert(wave3.player_factory.entries[0].class_name == "CPlDante");
    assert(wave3.player_factory.entries[2].class_name == "CPlLady");
    assert(wave3.enemy_factory.selector_count == 46U);
    assert(wave3.enemy_factory.external_mapping_count == 64U);
    assert(wave3.enemy_factory.resource_examples[0].object_pac ==
        "obj\\em017.pac");

    // Root scene flow and nested gameplay flow.
    assert(wave3.scene.root_factory_global_va == 0x140C8F970ULL);
    assert(wave3.scene.root_factory_size == 0x14F0U);
    assert(wave3.scene.root_scene_manager_offset == 0x1478U);
    assert(wave3.scene.scenes[5].class_name == "CSceneGameMain");
    assert(wave3.scene.nested_gameplay_scene_manager);

    // Animation/demo registry and CMotion anchors.
    assert(wave3.animation.demo_registry_capacity == 1024U);
    assert(to_string(DemoAssetType::mot) == "MOT");
    assert(to_string(DemoAssetType::tsc) == "TSC");
    assert(wave3.animation.motion_channel_count == 8U);

    // D3D11 runtime and effect texture pool.
    assert(wave3.graphics.device_pointer_offset == 0x08U);
    assert(wave3.graphics.context_pointer_offset == 0x10U);
    assert(wave3.graphics.swapchain_pointer_offset == 0x18U);
    assert(wave3.graphics.gfx_texture_count() == 900U);

    // HD translation plus the legacy VAG sample path.
    assert(wave3.media.ogg.descriptor_count == 154U);
    assert(wave3.media.ogg.loop_start_ms_offset == 0x08U);
    assert(wave3.media.ogg.loop_end_ms_offset == 0x0CU);
    assert(wave3.media.fmod_channel_groups.size() == 11U);
    assert(wave3.media.embedded_sample_magic == "VAGp");
    assert(wave3.media.codec_export == "FMODGetCodecDescription");

    // String catalog and semantic numeric resolver remain separate layers.
    assert(wave3.master_catalog.pointer_count == 4039U);
    assert(wave3.master_catalog.pac_count == 3398U);
    assert(wave3.numeric_resource_resolver.function_va == 0x1402C07F0ULL);
    assert(wave3.numeric_resource_resolver.valid_id_count == 932U);

    // Process/runtime memory evidence.
    assert(wave3.memory.central_arena_bytes == 256ULL * 1024ULL * 1024ULL);
    assert(wave3.memory.registry_lookup_va == 0x1402C6150ULL);

    return 0;
}
