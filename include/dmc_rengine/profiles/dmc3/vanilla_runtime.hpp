#pragma once

#include <array>
#include <cstdint>
#include <limits>
#include <string_view>

namespace dmc::rengine::profiles::dmc3::vanilla {

// This header is an evidence-backed C++ description of recovered vanilla DMC3
// runtime facts. It is intentionally not a replacement implementation of the
// game and does not move recovered game ownership into GDSpaces.

inline constexpr std::string_view canonical_executable_sha256 =
    "e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082";
inline constexpr std::string_view distribution_executable_sha256 =
    "81c7e61983564113b5105e931d9f185accc14e44ae147d27f720c2d50935c7d6";

struct StageResolverLayout final {
    std::uint64_t selector_table_va;
    std::uint32_t selector_count;
    std::uint64_t group_base_table_va;
    std::uint32_t group_count;
    std::uint64_t code_observation_va;
};

inline constexpr StageResolverLayout stage_resolver{
    .selector_table_va = 0x1405C4440ULL,
    .selector_count = 193U,
    .group_base_table_va = 0x1405C4A50ULL,
    .group_count = 10U,
    .code_observation_va = 0x1401B985DULL,
};

struct StageRoleAlias final {
    std::uint16_t stage_id;
    std::uint16_t effect_stage_id;
    std::uint16_t sound_stage_id;
};

inline constexpr std::array<StageRoleAlias, 13> stage_600_aliases{{
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

enum class ResourceLoadState : std::uint32_t {
    free_or_unstarted = 0U,
    io_scheduled_or_loading = 1U,
    io_complete_pending_postprocess = 2U,
    ready_postprocessed = 3U,
    teardown_or_cancel_pending = 4U,
};

enum class OwnerResourceDomain : std::uint32_t {
    stage_script = 0U,
    stage_config = 1U,
    stage_effect = 2U,
    demo_or_event = 3U,
    localized_message = 4U,
    enemy_object = 5U,
    enemy_sound = 6U,
};

[[nodiscard]] constexpr std::string_view to_string(
    OwnerResourceDomain domain) noexcept {
    switch (domain) {
    case OwnerResourceDomain::stage_script: return "stage-script";
    case OwnerResourceDomain::stage_config: return "stage-config";
    case OwnerResourceDomain::stage_effect: return "stage-effect";
    case OwnerResourceDomain::demo_or_event: return "demo-event";
    case OwnerResourceDomain::localized_message: return "localized-message";
    case OwnerResourceDomain::enemy_object: return "enemy-object";
    case OwnerResourceDomain::enemy_sound: return "enemy-sound";
    }
    return "stage-script";
}

struct ResourceLoadPoolLayout final {
    std::uint64_t base_va;
    std::uint32_t entry_count;
    std::uint32_t entry_stride;
    std::array<std::uint32_t, 7> group_counts;
    std::array<std::uint32_t, 8> cumulative_offsets;
};

inline constexpr ResourceLoadPoolLayout resource_load_pool{
    .base_va = 0x140C99D30ULL,
    .entry_count = 363U,
    .entry_stride = 0x48U,
    .group_counts = {4U, 136U, 60U, 28U, 1U, 128U, 6U},
    .cumulative_offsets = {0U, 4U, 140U, 200U, 228U, 229U, 357U, 363U},
};

struct ResourceLoadEntryFieldLayout final {
    std::uint32_t group_index_offset;
    std::uint32_t state_offset;
    std::uint32_t subtype_or_variant_offset;
    std::uint32_t source_descriptor_offset;
    std::uint32_t loaded_payload_offset;
    std::uint32_t owned_allocation_offset;
};

inline constexpr ResourceLoadEntryFieldLayout resource_load_entry_fields{
    .group_index_offset = 0x00U,
    .state_offset = 0x04U,
    .subtype_or_variant_offset = 0x08U,
    .source_descriptor_offset = 0x18U,
    .loaded_payload_offset = 0x20U,
    .owned_allocation_offset = 0x28U,
};

struct OwnerCacheLayout final {
    std::uint32_t observed_capacity;
    std::uint32_t refcount_offset;
};

// The owner-local cache is distinct from the 363-entry loader pool. Lookup is
// by (group,index), references are reusable, and +0x2C is the observed refcount.
inline constexpr OwnerCacheLayout owner_cache{
    .observed_capacity = 32U,
    .refcount_offset = 0x2CU,
};

enum class PostLoadFormat {
    mod,
    efm,
    scm,
    shw,
    pnst,
};

struct PostLoadFixup final {
    PostLoadFormat format;
    std::string_view magic;
    std::uint64_t helper_va;
    bool recursive_container;
};

inline constexpr std::array<PostLoadFixup, 5> post_load_fixups{{
    {PostLoadFormat::mod, "MOD", 0x1402FE3B0ULL, false},
    {PostLoadFormat::efm, "EFM", 0x1402F7A90ULL, false},
    {PostLoadFormat::scm, "SCM", 0x1403051B0ULL, false},
    {PostLoadFormat::shw, "SHW", 0x1403204C0ULL, false},
    {PostLoadFormat::pnst, "PNST", 0ULL, true},
}};

struct ListManifestRuntimeLayout final {
    std::uint64_t resource_loader_va;
    std::uint64_t list_helper_va;
    std::string_view fallback_extension;
    std::string_view packed_extension;
    std::string_view dummy_token;
    std::string_view nested_list_token;
};

inline constexpr ListManifestRuntimeLayout list_manifest{
    .resource_loader_va = 0x1401B8CA0ULL,
    .list_helper_va = 0x1401B79E0ULL,
    .fallback_extension = ".lst",
    .packed_extension = ".pac",
    .dummy_token = "dummy",
    .nested_list_token = "lst",
};

enum class SceneFactoryId : std::uint32_t {
    boot = 0U,
    opening = 1U,
    start_menu = 2U,
    mission_select = 3U,
    game = 4U,
    game_main = 5U,
    demo = 6U,
    mission_start = 7U,
    result = 8U,
    ending = 9U,
};

struct SceneFactoryEntry final {
    SceneFactoryId id;
    std::string_view class_name;
    std::uint32_t allocation_size;
    std::uint64_t constructor_va; // 0 means manual vtable initialization was observed.
};

inline constexpr std::array<SceneFactoryEntry, 10> scene_factory{{
    {SceneFactoryId::boot, "CSceneBoot", 0x38U, 0ULL},
    {SceneFactoryId::opening, "CSceneOpening", 0x28U, 0ULL},
    {SceneFactoryId::start_menu, "CSceneStartMenu", 0xC8U, 0x14023F430ULL},
    {SceneFactoryId::mission_select, "CSceneMisSelect", 0xD0U, 0x14023F2B0ULL},
    {SceneFactoryId::game, "CSceneGame", 0x12E0U, 0x14023EDD0ULL},
    {SceneFactoryId::game_main, "CSceneGameMain", 0x1BA30U, 0x14023EEF0ULL},
    {SceneFactoryId::demo, "CSceneDemo", 0x24A30U, 0x14023ED40ULL},
    {SceneFactoryId::mission_start, "CSceneMisStart", 0x500U, 0x14023F300ULL},
    {SceneFactoryId::result, "CSceneResult", 0x358U, 0x14023F3A0ULL},
    {SceneFactoryId::ending, "CSceneEnding", 0x18U, 0ULL},
}};

struct RootSceneFactoryLayout final {
    std::uint64_t global_va;
    std::uint32_t object_size;
    std::uint32_t root_scene_manager_offset;
};

inline constexpr RootSceneFactoryLayout root_scene_factory{
    .global_va = 0x140C8F970ULL,
    .object_size = 0x14F0U,
    .root_scene_manager_offset = 0x1478U,
};

enum class PlayerFactorySelector : std::uint32_t {
    dante = 0U,
    vergil = 1U,
    lady = 2U,
    new_vergil = 3U,
};

struct PlayerFactoryEntry final {
    PlayerFactorySelector selector;
    std::string_view class_name;
    std::uint32_t observed_allocation_size;
};

inline constexpr std::uint64_t player_factory_va = 0x1401DE820ULL;
inline constexpr std::array<PlayerFactoryEntry, 4> player_factory{{
    {PlayerFactorySelector::dante, "CPlDante", 0xB8C0U},
    {PlayerFactorySelector::vergil, "CPlVergil", 0xB680U},
    {PlayerFactorySelector::lady, "CPlLady", 0x8280U},
    {PlayerFactorySelector::new_vergil, "CPlNewVergil", 0xB8C0U},
}};

struct EnemyFactoryLayout final {
    std::uint64_t factory_va;
    std::uint32_t selector_count;
    std::uint32_t external_mapping_count;
    std::array<std::uint32_t, 8> null_selectors;
};

inline constexpr EnemyFactoryLayout enemy_factory{
    .factory_va = 0x1401AC6D0ULL,
    .selector_count = 46U,
    .external_mapping_count = 64U,
    .null_selectors = {9U, 15U, 24U, 26U, 38U, 41U, 42U, 43U},
};

struct NumericResourceResolverLayout final {
    std::uint64_t resolver_va;
    std::uint64_t master_name_catalog_va;
    std::uint32_t master_name_count;
    std::uint32_t locale0_valid_id_count;
    std::uint32_t locale0_range_count;
    std::uint32_t observed_unique_variant_paths;
};

inline constexpr NumericResourceResolverLayout numeric_resource_resolver{
    .resolver_va = 0x1402C07F0ULL,
    .master_name_catalog_va = 0x140553050ULL,
    .master_name_count = 4039U,
    .locale0_valid_id_count = 932U,
    .locale0_range_count = 145U,
    .observed_unique_variant_paths = 2494U,
};

struct MasterCatalogExtensionCounts final {
    std::uint32_t pac;
    std::uint32_t txt;
    std::uint32_t adx;
    std::uint32_t bin;
    std::uint32_t sfd;
};

inline constexpr MasterCatalogExtensionCounts master_catalog_counts{
    .pac = 3398U,
    .txt = 424U,
    .adx = 154U,
    .bin = 24U,
    .sfd = 19U,
};

enum class DemoAssetType : std::uint32_t {
    mot = 0U,
    mcv = 1U,
    cam = 2U,
    hid = 3U,
    clt = 4U,
    tsc = 5U,
};

inline constexpr std::uint64_t demo_asset_registry_va = 0x1402E01A0ULL;
inline constexpr std::uint32_t demo_asset_registry_capacity = 1024U;

struct MotionRuntimeLayout final {
    std::uint32_t object_size;
    std::uint32_t track_count;
    std::uint32_t em029_embedded_offset;
};

inline constexpr MotionRuntimeLayout motion_runtime{
    .object_size = 0x220U,
    .track_count = 8U,
    .em029_embedded_offset = 0x4D10U,
};

struct HdAudioDescriptorTableLayout final {
    std::uint64_t table_va;
    std::uint32_t record_count;
    std::uint32_t record_stride;
    std::uint64_t lookup_code_va;
    std::uint64_t loop_apply_code_va;
    std::uint32_t no_explicit_loop_sentinel;
};

inline constexpr HdAudioDescriptorTableLayout hd_audio{
    .table_va = 0x14055C610ULL,
    .record_count = 154U,
    .record_stride = 0x10U,
    .lookup_code_va = 0x140031D80ULL,
    .loop_apply_code_va = 0x140032A80ULL,
    .no_explicit_loop_sentinel = std::numeric_limits<std::uint32_t>::max(),
};

struct HdAudioDescriptorAbi final {
    std::uint32_t filename_pointer_offset;
    std::uint32_t loop_start_ms_offset;
    std::uint32_t loop_end_ms_offset;
};

inline constexpr HdAudioDescriptorAbi hd_audio_descriptor_abi{
    .filename_pointer_offset = 0x00U,
    .loop_start_ms_offset = 0x08U,
    .loop_end_ms_offset = 0x0CU,
};

inline constexpr std::uint64_t hd_video_extension_rewrite_code_va = 0x14032C10DULL;
inline constexpr std::string_view hd_audio_physical_extension = ".ogg";
inline constexpr std::string_view hd_video_physical_extension = ".wmv";

struct GraphicsRuntimeLayout final {
    std::uint64_t global_va;
    std::uint32_t d3d11_device_offset;
    std::uint32_t d3d11_context_offset;
    std::uint32_t dxgi_swap_chain_offset;
};

inline constexpr GraphicsRuntimeLayout graphics_runtime{
    .global_va = 0x140C0B410ULL,
    .d3d11_device_offset = 0x08U,
    .d3d11_context_offset = 0x10U,
    .dxgi_swap_chain_offset = 0x18U,
};

struct GfxTexturePoolLayout final {
    std::uint32_t bank_count;
    std::uint32_t textures_per_bank;
    std::uint32_t object_size;
};

inline constexpr GfxTexturePoolLayout gfx_texture_pool{
    .bank_count = 2U,
    .textures_per_bank = 450U,
    .object_size = 0x60U,
};

inline constexpr std::array<std::string_view, 11> audio_channel_groups{{
    "system",
    "common",
    "player-style",
    "weapon1",
    "weapon2",
    "weapon3",
    "weapon4",
    "enemy",
    "room",
    "music",
    "demo",
}};

struct MemoryArenaLayout final {
    std::uint64_t central_bytes;
    std::array<std::uint64_t, 3> observed_subdomain_bytes;
};

inline constexpr MemoryArenaLayout memory_arena{
    .central_bytes = 256ULL * 1024ULL * 1024ULL,
    .observed_subdomain_bytes = {
        64ULL * 1024ULL * 1024ULL,
        5ULL * 1024ULL * 1024ULL,
        4ULL * 1024ULL * 1024ULL,
    },
};

inline constexpr std::uint64_t central_type_registry_lookup_va = 0x1402C6150ULL;

struct StageSetAbiFamily final {
    std::string_view label;
    std::uint32_t observed_type_count;
    std::uint32_t stage_set_subobject_offset;
    bool exposes_stage_set_rtti_base;
};

inline constexpr std::array<StageSetAbiFamily, 4> stage_set_abi_families{{
    {"actor-collision", 24U, 0x110U, true},
    {"work-lightweight", 4U, 0x60U, true},
    {"non-player", 3U, 0x180U, true},
    {"stage-set-like-without-rtti-base", 2U, 0U, false},
}};

} // namespace dmc::rengine::profiles::dmc3::vanilla
