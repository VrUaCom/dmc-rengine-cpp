#pragma once

#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <string_view>

namespace dmc::recovered::dmc3::runtime {

inline constexpr std::string_view kCanonicalDmc3Sha256 =
    "e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082";

enum class ResourceDomain : std::uint32_t {
    stage_script = 0U,
    stage_config = 1U,
    stage_effect = 2U,
    demo_event = 3U,
    localized_message = 4U,
    enemy_object = 5U,
    enemy_sound = 6U,
};

[[nodiscard]] constexpr std::string_view to_string(ResourceDomain domain) noexcept {
    switch (domain) {
    case ResourceDomain::stage_script: return "stage-script";
    case ResourceDomain::stage_config: return "stage-config";
    case ResourceDomain::stage_effect: return "stage-effect";
    case ResourceDomain::demo_event: return "demo-event";
    case ResourceDomain::localized_message: return "localized-message";
    case ResourceDomain::enemy_object: return "enemy-object";
    case ResourceDomain::enemy_sound: return "enemy-sound";
    }
    return "unknown";
}

struct StageResourceCellAbi final {
    std::uint32_t stride{};
    std::uint32_t kind16_offset{};
    std::uint32_t unresolved_begin{};
    std::uint32_t unresolved_end{};
    std::uint32_t path_pointer_offset{};

    [[nodiscard]] constexpr bool valid() const noexcept {
        return stride == 0x10U &&
            kind16_offset == 0x00U &&
            unresolved_begin == 0x02U &&
            unresolved_end == 0x07U &&
            path_pointer_offset == 0x08U;
    }
};

struct StageDescriptorBank final {
    std::uint64_t base_va{};
    std::uint32_t descriptor_count{};
};

struct StageIdRange final {
    std::uint16_t first{};
    std::uint16_t last{};

    [[nodiscard]] constexpr bool contains(std::uint16_t value) const noexcept {
        return value >= first && value <= last;
    }

    [[nodiscard]] constexpr std::uint32_t count() const noexcept {
        return static_cast<std::uint32_t>(last - first) + 1U;
    }
};

struct StageResourceAlias final {
    std::uint16_t stage_id{};
    std::uint16_t effect_stage_id{};
    std::uint16_t sound_stage_id{};
};

struct StageRuntimeEvidence final {
    StageResourceCellAbi cell;
    std::uint32_t descriptor_stride{};
    StageDescriptorBank bank_a;
    StageDescriptorBank bank_b;
    std::uint64_t selector_table_va{};
    std::uint32_t selector_count{};
    std::uint64_t group_base_table_va{};
    std::uint32_t group_base_count{};
    std::uint64_t numeric_resolver_code_va{};

    [[nodiscard]] constexpr bool valid() const noexcept {
        return cell.valid() &&
            descriptor_stride == 0x40U &&
            bank_a.base_va == 0x1405C4AA0ULL &&
            bank_a.descriptor_count == 110U &&
            bank_b.base_va == 0x1405C3080ULL &&
            bank_b.descriptor_count == 79U &&
            selector_table_va == 0x1405C4440ULL &&
            selector_count == 193U &&
            group_base_table_va == 0x1405C4A50ULL &&
            group_base_count == 10U &&
            numeric_resolver_code_va == 0x1401B985DULL;
    }
};

struct StageIdParts final {
    std::uint32_t hundreds{};
    std::uint32_t remainder{};
};

[[nodiscard]] constexpr StageIdParts split_numeric_stage_id(
    std::uint32_t stage_id) noexcept {
    return StageIdParts{
        .hundreds = stage_id / 100U,
        .remainder = stage_id % 100U,
    };
}

[[nodiscard]] inline const std::array<StageIdRange, 8>&
observed_stage_id_ranges() noexcept {
    static constexpr std::array<StageIdRange, 8> ranges{{
        {0U, 12U},
        {100U, 146U},
        {200U, 241U},
        {300U, 313U},
        {400U, 446U},
        {448U, 449U},
        {600U, 612U},
        {900U, 910U},
    }};
    return ranges;
}

[[nodiscard]] inline bool is_observed_stage_descriptor_id(
    std::uint16_t stage_id) noexcept {
    const auto& ranges = observed_stage_id_ranges();
    return std::any_of(
        ranges.begin(), ranges.end(),
        [stage_id](const StageIdRange& range) {
            return range.contains(stage_id);
        });
}

[[nodiscard]] inline const std::array<StageResourceAlias, 13>&
stage_resource_aliases() noexcept {
    static constexpr std::array<StageResourceAlias, 13> aliases{{
        {600U, 4U, 4U},
        {601U, 6U, 6U},
        {602U, 126U, 126U},
        {603U, 113U, 113U},
        {604U, 3U, 3U},
        {605U, 300U, 300U},
        {606U, 204U, 204U},
        {607U, 228U, 228U},
        {608U, 216U, 216U},
        {609U, 233U, 233U},
        {610U, 131U, 131U},
        {611U, 128U, 128U},
        {612U, 300U, 300U},
    }};
    return aliases;
}

struct ListManifestEvidence final {
    std::uint64_t cell_loader_va{};
    std::uint64_t primary_or_list_probe_va{};
    bool hash_comments{};
    bool dummy_token{};
    bool nested_lists{};
    bool rewrites_entries_to_pac{};
    bool recursive_loading{};

    [[nodiscard]] constexpr bool valid() const noexcept {
        return cell_loader_va == 0x1401B8CA0ULL &&
            primary_or_list_probe_va == 0x1401B79E0ULL &&
            hash_comments && dummy_token && nested_lists &&
            rewrites_entries_to_pac && recursive_loading;
    }
};

struct OwnerResourceCacheEvidence final {
    std::uint32_t observed_node_capacity{};
    std::uint32_t refcount_offset{};
    bool lookup_key_is_group_and_index{};
    bool has_ready_and_pending_lists{};
    bool release_decrements_refcount{};

    [[nodiscard]] constexpr bool valid() const noexcept {
        return observed_node_capacity == 32U &&
            refcount_offset == 0x2CU &&
            lookup_key_is_group_and_index &&
            has_ready_and_pending_lists &&
            release_decrements_refcount;
    }
};

struct ResourceDomainEvidence final {
    ResourceDomain domain{ResourceDomain::stage_script};
    std::string_view source;
};

[[nodiscard]] inline const std::array<ResourceDomainEvidence, 7>&
resource_domain_evidence() noexcept {
    static constexpr std::array<ResourceDomainEvidence, 7> domains{{
        {ResourceDomain::stage_script, "StageResourceDescriptor::script"},
        {ResourceDomain::stage_config, "StageResourceDescriptor::cfg"},
        {ResourceDomain::stage_effect, "StageResourceDescriptor::effect"},
        {ResourceDomain::demo_event, "demo/event resource table"},
        {ResourceDomain::localized_message, "language-indexed message cell"},
        {ResourceDomain::enemy_object, "enemy object/model PAC table"},
        {ResourceDomain::enemy_sound, "paired enemy sound PAC table"},
    }};
    return domains;
}

struct PlayerFactoryEntry final {
    std::uint32_t selector{};
    std::string_view class_name;
    std::uint32_t allocation_size{};
};

struct PlayerFactoryEvidence final {
    std::uint64_t factory_va{};
    std::array<PlayerFactoryEntry, 4> entries{};

    [[nodiscard]] bool valid() const noexcept {
        if (factory_va != 0x1401DE820ULL) {
            return false;
        }
        constexpr std::array<std::uint32_t, 4> expected_sizes{
            0xB8C0U, 0xB680U, 0x8280U, 0xB8C0U,
        };
        constexpr std::array<std::string_view, 4> expected_names{
            "CPlDante", "CPlVergil", "CPlLady", "CPlNewVergil",
        };
        for (std::size_t index = 0U; index < entries.size(); ++index) {
            if (entries[index].selector != index ||
                entries[index].allocation_size != expected_sizes[index] ||
                entries[index].class_name != expected_names[index]) {
                return false;
            }
        }
        return true;
    }
};

struct EnemySharedFactoryCase final {
    std::uint32_t first_selector{};
    std::uint32_t last_selector{};
    std::uint32_t constructor_selector{};
};

struct EnemyResourceExample final {
    std::uint32_t resource_set_selector{};
    std::string_view object_pac;
    std::string_view sound_pac;
};

struct EnemyFactoryEvidence final {
    std::uint64_t factory_va{};
    std::uint32_t selector_count{};
    std::uint32_t external_mapping_count{};
    std::array<std::uint32_t, 8> null_selectors{};
    std::array<EnemySharedFactoryCase, 2> shared_cases{};
    std::array<EnemyResourceExample, 5> resource_examples{};

    [[nodiscard]] bool valid() const noexcept {
        if (factory_va != 0x1401AC6D0ULL ||
            selector_count != 46U ||
            external_mapping_count != 64U ||
            null_selectors != std::array<std::uint32_t, 8>{
                9U, 15U, 24U, 26U, 38U, 41U, 42U, 43U,
            }) {
            return false;
        }
        if (shared_cases[0].first_selector != 17U ||
            shared_cases[0].last_selector != 20U ||
            shared_cases[0].constructor_selector != 17U ||
            shared_cases[1].first_selector != 29U ||
            shared_cases[1].last_selector != 31U ||
            shared_cases[1].constructor_selector != 28U) {
            return false;
        }
        return resource_examples[0].resource_set_selector == 12U &&
            resource_examples[0].object_pac == "obj\\em017.pac" &&
            resource_examples[0].sound_pac == "se\\snd_em17.pac" &&
            resource_examples[1].resource_set_selector == 20U &&
            resource_examples[1].object_pac == "obj\\em029.pac" &&
            resource_examples[2].resource_set_selector == 27U &&
            resource_examples[2].object_pac == "obj\\em037.pac" &&
            resource_examples[3].resource_set_selector == 28U &&
            resource_examples[3].object_pac == "obj\\em000.pac" &&
            resource_examples[3].sound_pac == "se\\snd_em00b.pac" &&
            resource_examples[4].resource_set_selector == 29U &&
            resource_examples[4].object_pac == "obj\\em000.pac" &&
            resource_examples[4].sound_pac == "se\\snd_emsr.pac";
    }
};

struct SceneFactoryEntry final {
    std::uint32_t id{};
    std::string_view class_name;
    std::uint32_t allocation_size{};
    std::uint64_t constructor_va{};
};

struct SceneRuntimeEvidence final {
    std::uint64_t scene_factory_dispatch_va{};
    std::uint64_t root_factory_global_va{};
    std::uint32_t root_factory_size{};
    std::uint32_t root_scene_manager_offset{};
    std::array<SceneFactoryEntry, 10> scenes{};
    bool nested_gameplay_scene_manager{};
    bool transition_calls_exit_destroy_create_enter{};

    [[nodiscard]] bool valid() const noexcept {
        if (scene_factory_dispatch_va != 0x140240090ULL ||
            root_factory_global_va != 0x140C8F970ULL ||
            root_factory_size != 0x14F0U ||
            root_scene_manager_offset != 0x1478U ||
            !nested_gameplay_scene_manager ||
            !transition_calls_exit_destroy_create_enter) {
            return false;
        }
        constexpr std::array<std::string_view, 10> names{
            "CSceneBoot", "CSceneOpening", "CSceneStartMenu", "CSceneMisSelect",
            "CSceneGame", "CSceneGameMain", "CSceneDemo", "CSceneMisStart",
            "CSceneResult", "CSceneEnding",
        };
        constexpr std::array<std::uint32_t, 10> sizes{
            0x38U, 0x28U, 0xC8U, 0xD0U, 0x12E0U,
            0x1BA30U, 0x24A30U, 0x500U, 0x358U, 0x18U,
        };
        constexpr std::array<std::uint64_t, 10> constructors{
            0ULL, 0ULL, 0x14023F430ULL, 0x14023F2B0ULL, 0x14023EDD0ULL,
            0x14023EEF0ULL, 0x14023ED40ULL, 0x14023F300ULL,
            0x14023F3A0ULL, 0ULL,
        };
        for (std::size_t index = 0U; index < scenes.size(); ++index) {
            if (scenes[index].id != index ||
                scenes[index].class_name != names[index] ||
                scenes[index].allocation_size != sizes[index] ||
                scenes[index].constructor_va != constructors[index]) {
                return false;
            }
        }
        return true;
    }
};

enum class DemoAssetType : std::uint32_t {
    mot = 0U,
    mcv = 1U,
    cam = 2U,
    hid = 3U,
    clt = 4U,
    tsc = 5U,
};

[[nodiscard]] constexpr std::string_view to_string(DemoAssetType type) noexcept {
    switch (type) {
    case DemoAssetType::mot: return "MOT";
    case DemoAssetType::mcv: return "MCV";
    case DemoAssetType::cam: return "CAM";
    case DemoAssetType::hid: return "HID";
    case DemoAssetType::clt: return "CLT";
    case DemoAssetType::tsc: return "TSC";
    }
    return "unknown";
}

struct AnimationRuntimeEvidence final {
    std::uint64_t demo_registry_va{};
    std::uint32_t demo_registry_capacity{};
    std::uint32_t motion_object_size{};
    std::uint32_t motion_channel_count{};
    std::uint32_t em029_motion_offset{};

    [[nodiscard]] constexpr bool valid() const noexcept {
        return demo_registry_va == 0x1402E01A0ULL &&
            demo_registry_capacity == 1024U &&
            motion_object_size == 0x220U &&
            motion_channel_count == 8U &&
            em029_motion_offset == 0x4D10U;
    }
};

struct GraphicsRuntimeEvidence final {
    std::uint64_t graphics_global_va{};
    std::uint32_t device_pointer_offset{};
    std::uint32_t context_pointer_offset{};
    std::uint32_t swapchain_pointer_offset{};
    std::uint32_t gfx_texture_size{};
    std::uint32_t gfx_texture_banks{};
    std::uint32_t gfx_textures_per_bank{};

    [[nodiscard]] constexpr std::uint32_t gfx_texture_count() const noexcept {
        return gfx_texture_banks * gfx_textures_per_bank;
    }

    [[nodiscard]] constexpr bool valid() const noexcept {
        return graphics_global_va == 0x140C0B410ULL &&
            device_pointer_offset == 0x08U &&
            context_pointer_offset == 0x10U &&
            swapchain_pointer_offset == 0x18U &&
            gfx_texture_size == 0x60U &&
            gfx_texture_banks == 2U &&
            gfx_textures_per_bank == 450U &&
            gfx_texture_count() == 900U;
    }
};

struct OggDescriptorAbi final {
    std::uint64_t table_va{};
    std::uint32_t descriptor_count{};
    std::uint32_t stride{};
    std::uint32_t filename_pointer_offset{};
    std::uint32_t loop_start_ms_offset{};
    std::uint32_t loop_end_ms_offset{};
    std::uint32_t no_loop_sentinel{};

    [[nodiscard]] constexpr bool valid() const noexcept {
        return table_va == 0x14055C610ULL &&
            descriptor_count == 154U &&
            stride == 0x10U &&
            filename_pointer_offset == 0x00U &&
            loop_start_ms_offset == 0x08U &&
            loop_end_ms_offset == 0x0CU &&
            no_loop_sentinel == 0xFFFFFFFFU;
    }
};

struct MediaRuntimeEvidence final {
    OggDescriptorAbi ogg;
    std::uint64_t ogg_lookup_va{};
    std::uint64_t loop_points_apply_va{};
    std::uint64_t sfd_to_wmv_translation_va{};
    std::array<std::string_view, 11> fmod_channel_groups{};
    std::string_view embedded_sample_magic;
    std::string_view codec_export;

    [[nodiscard]] bool valid() const noexcept {
        if (!ogg.valid() ||
            ogg_lookup_va != 0x140031D80ULL ||
            loop_points_apply_va != 0x140032A80ULL ||
            sfd_to_wmv_translation_va != 0x14032C10DULL ||
            embedded_sample_magic != "VAGp" ||
            codec_export != "FMODGetCodecDescription") {
            return false;
        }
        constexpr std::array<std::string_view, 11> expected_groups{
            "system", "common", "player-style", "weapon1", "weapon2",
            "weapon3", "weapon4", "enemy", "room", "music", "demo",
        };
        return fmod_channel_groups == expected_groups;
    }
};

struct MasterResourceCatalogEvidence final {
    std::uint64_t pointer_table_va{};
    std::uint32_t pointer_count{};
    std::uint32_t pac_count{};
    std::uint32_t txt_count{};
    std::uint32_t adx_count{};
    std::uint32_t bin_count{};
    std::uint32_t sfd_count{};

    [[nodiscard]] constexpr bool valid() const noexcept {
        return pointer_table_va == 0x140553050ULL &&
            pointer_count == 4039U &&
            pac_count == 3398U &&
            txt_count == 424U &&
            adx_count == 154U &&
            bin_count == 24U &&
            sfd_count == 19U;
    }
};

struct NumericResourceResolverEvidence final {
    std::uint64_t function_va{};
    std::uint32_t scan_begin{};
    std::uint32_t scan_end_exclusive{};
    std::uint32_t valid_id_count{};
    std::uint32_t valid_range_count{};

    [[nodiscard]] constexpr bool valid() const noexcept {
        return function_va == 0x1402C07F0ULL &&
            scan_begin == 0U &&
            scan_end_exclusive == 10001U &&
            valid_id_count == 932U &&
            valid_range_count == 145U;
    }
};

struct MemoryRuntimeEvidence final {
    std::uint64_t registry_lookup_va{};
    std::uint64_t central_arena_bytes{};
    std::array<std::uint64_t, 3> observed_subarena_bytes{};

    [[nodiscard]] constexpr bool valid() const noexcept {
        return registry_lookup_va == 0x1402C6150ULL &&
            central_arena_bytes == 256ULL * 1024ULL * 1024ULL &&
            observed_subarena_bytes ==
                std::array<std::uint64_t, 3>{
                    64ULL * 1024ULL * 1024ULL,
                    5ULL * 1024ULL * 1024ULL,
                    4ULL * 1024ULL * 1024ULL,
                };
    }
};

struct Wave3RuntimeEvidenceModel final {
    std::string_view artifact_sha256;
    std::string_view evidence_id;
    StageRuntimeEvidence stage;
    ListManifestEvidence list_manifest;
    OwnerResourceCacheEvidence owner_cache;
    PlayerFactoryEvidence player_factory;
    EnemyFactoryEvidence enemy_factory;
    SceneRuntimeEvidence scene;
    AnimationRuntimeEvidence animation;
    GraphicsRuntimeEvidence graphics;
    MediaRuntimeEvidence media;
    MasterResourceCatalogEvidence master_catalog;
    NumericResourceResolverEvidence numeric_resource_resolver;
    MemoryRuntimeEvidence memory;

    [[nodiscard]] bool valid() const noexcept {
        if (artifact_sha256 != kCanonicalDmc3Sha256 ||
            evidence_id != "dmc3-vanilla-wave3" ||
            !stage.valid() || !list_manifest.valid() || !owner_cache.valid() ||
            !player_factory.valid() || !enemy_factory.valid() || !scene.valid() ||
            !animation.valid() || !graphics.valid() || !media.valid() ||
            !master_catalog.valid() || !numeric_resource_resolver.valid() ||
            !memory.valid()) {
            return false;
        }
        std::uint32_t stage_count = 0U;
        for (const auto& range : observed_stage_id_ranges()) {
            stage_count += range.count();
        }
        return stage_count == 189U &&
            stage_resource_aliases().size() == 13U &&
            resource_domain_evidence().size() == 7U;
    }
};

[[nodiscard]] inline const Wave3RuntimeEvidenceModel&
wave3_runtime_evidence_model() noexcept {
    static const Wave3RuntimeEvidenceModel model{
        .artifact_sha256 = kCanonicalDmc3Sha256,
        .evidence_id = "dmc3-vanilla-wave3",
        .stage = StageRuntimeEvidence{
            .cell = StageResourceCellAbi{
                .stride = 0x10U,
                .kind16_offset = 0x00U,
                .unresolved_begin = 0x02U,
                .unresolved_end = 0x07U,
                .path_pointer_offset = 0x08U,
            },
            .descriptor_stride = 0x40U,
            .bank_a = StageDescriptorBank{.base_va = 0x1405C4AA0ULL, .descriptor_count = 110U},
            .bank_b = StageDescriptorBank{.base_va = 0x1405C3080ULL, .descriptor_count = 79U},
            .selector_table_va = 0x1405C4440ULL,
            .selector_count = 193U,
            .group_base_table_va = 0x1405C4A50ULL,
            .group_base_count = 10U,
            .numeric_resolver_code_va = 0x1401B985DULL,
        },
        .list_manifest = ListManifestEvidence{
            .cell_loader_va = 0x1401B8CA0ULL,
            .primary_or_list_probe_va = 0x1401B79E0ULL,
            .hash_comments = true,
            .dummy_token = true,
            .nested_lists = true,
            .rewrites_entries_to_pac = true,
            .recursive_loading = true,
        },
        .owner_cache = OwnerResourceCacheEvidence{
            .observed_node_capacity = 32U,
            .refcount_offset = 0x2CU,
            .lookup_key_is_group_and_index = true,
            .has_ready_and_pending_lists = true,
            .release_decrements_refcount = true,
        },
        .player_factory = PlayerFactoryEvidence{
            .factory_va = 0x1401DE820ULL,
            .entries = {{
                {0U, "CPlDante", 0xB8C0U},
                {1U, "CPlVergil", 0xB680U},
                {2U, "CPlLady", 0x8280U},
                {3U, "CPlNewVergil", 0xB8C0U},
            }},
        },
        .enemy_factory = EnemyFactoryEvidence{
            .factory_va = 0x1401AC6D0ULL,
            .selector_count = 46U,
            .external_mapping_count = 64U,
            .null_selectors = {{9U, 15U, 24U, 26U, 38U, 41U, 42U, 43U}},
            .shared_cases = {{{17U, 20U, 17U}, {29U, 31U, 28U}}},
            .resource_examples = {{
                {12U, "obj\\em017.pac", "se\\snd_em17.pac"},
                {20U, "obj\\em029.pac", "se\\snd_em29.pac"},
                {27U, "obj\\em037.pac", "se\\snd_em37.pac"},
                {28U, "obj\\em000.pac", "se\\snd_em00b.pac"},
                {29U, "obj\\em000.pac", "se\\snd_emsr.pac"},
            }},
        },
        .scene = SceneRuntimeEvidence{
            .scene_factory_dispatch_va = 0x140240090ULL,
            .root_factory_global_va = 0x140C8F970ULL,
            .root_factory_size = 0x14F0U,
            .root_scene_manager_offset = 0x1478U,
            .scenes = {{
                {0U, "CSceneBoot", 0x38U, 0ULL},
                {1U, "CSceneOpening", 0x28U, 0ULL},
                {2U, "CSceneStartMenu", 0xC8U, 0x14023F430ULL},
                {3U, "CSceneMisSelect", 0xD0U, 0x14023F2B0ULL},
                {4U, "CSceneGame", 0x12E0U, 0x14023EDD0ULL},
                {5U, "CSceneGameMain", 0x1BA30U, 0x14023EEF0ULL},
                {6U, "CSceneDemo", 0x24A30U, 0x14023ED40ULL},
                {7U, "CSceneMisStart", 0x500U, 0x14023F300ULL},
                {8U, "CSceneResult", 0x358U, 0x14023F3A0ULL},
                {9U, "CSceneEnding", 0x18U, 0ULL},
            }},
            .nested_gameplay_scene_manager = true,
            .transition_calls_exit_destroy_create_enter = true,
        },
        .animation = AnimationRuntimeEvidence{
            .demo_registry_va = 0x1402E01A0ULL,
            .demo_registry_capacity = 1024U,
            .motion_object_size = 0x220U,
            .motion_channel_count = 8U,
            .em029_motion_offset = 0x4D10U,
        },
        .graphics = GraphicsRuntimeEvidence{
            .graphics_global_va = 0x140C0B410ULL,
            .device_pointer_offset = 0x08U,
            .context_pointer_offset = 0x10U,
            .swapchain_pointer_offset = 0x18U,
            .gfx_texture_size = 0x60U,
            .gfx_texture_banks = 2U,
            .gfx_textures_per_bank = 450U,
        },
        .media = MediaRuntimeEvidence{
            .ogg = OggDescriptorAbi{
                .table_va = 0x14055C610ULL,
                .descriptor_count = 154U,
                .stride = 0x10U,
                .filename_pointer_offset = 0x00U,
                .loop_start_ms_offset = 0x08U,
                .loop_end_ms_offset = 0x0CU,
                .no_loop_sentinel = 0xFFFFFFFFU,
            },
            .ogg_lookup_va = 0x140031D80ULL,
            .loop_points_apply_va = 0x140032A80ULL,
            .sfd_to_wmv_translation_va = 0x14032C10DULL,
            .fmod_channel_groups = {{
                "system", "common", "player-style", "weapon1", "weapon2",
                "weapon3", "weapon4", "enemy", "room", "music", "demo",
            }},
            .embedded_sample_magic = "VAGp",
            .codec_export = "FMODGetCodecDescription",
        },
        .master_catalog = MasterResourceCatalogEvidence{
            .pointer_table_va = 0x140553050ULL,
            .pointer_count = 4039U,
            .pac_count = 3398U,
            .txt_count = 424U,
            .adx_count = 154U,
            .bin_count = 24U,
            .sfd_count = 19U,
        },
        .numeric_resource_resolver = NumericResourceResolverEvidence{
            .function_va = 0x1402C07F0ULL,
            .scan_begin = 0U,
            .scan_end_exclusive = 10001U,
            .valid_id_count = 932U,
            .valid_range_count = 145U,
        },
        .memory = MemoryRuntimeEvidence{
            .registry_lookup_va = 0x1402C6150ULL,
            .central_arena_bytes = 256ULL * 1024ULL * 1024ULL,
            .observed_subarena_bytes = {{
                64ULL * 1024ULL * 1024ULL,
                5ULL * 1024ULL * 1024ULL,
                4ULL * 1024ULL * 1024ULL,
            }},
        },
    };
    return model;
}

} // namespace dmc::recovered::dmc3::runtime
