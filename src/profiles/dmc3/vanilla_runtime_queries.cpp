#include "dmc_rengine/profiles/dmc3/vanilla_runtime_queries.hpp"

#include <algorithm>
#include <cctype>

namespace dmc::rengine::profiles::dmc3::vanilla {
namespace {

[[nodiscard]] std::string_view basename(std::string_view path) noexcept {
    const auto slash = path.find_last_of("/\\");
    return slash == std::string_view::npos ? path : path.substr(slash + 1U);
}

[[nodiscard]] std::string replace_extension(
    std::string_view value,
    std::string_view extension) {
    const auto slash = value.find_last_of("/\\");
    const auto dot = value.find_last_of('.');
    const auto has_extension = dot != std::string_view::npos &&
        (slash == std::string_view::npos || dot > slash);

    std::string result{
        has_extension ? value.substr(0U, dot) : value};
    result.append(extension);
    return result;
}

} // namespace

const StageRoleAlias* find_stage_role_alias(
    std::uint16_t stage_id) noexcept {
    const auto iterator = std::find_if(
        stage_600_aliases.begin(), stage_600_aliases.end(),
        [stage_id](const StageRoleAlias& alias) {
            return alias.stage_id == stage_id;
        });
    return iterator == stage_600_aliases.end() ? nullptr : &*iterator;
}

bool observed_resource_load_transition(
    ResourceLoadState from,
    ResourceLoadState to) noexcept {
    if (from == ResourceLoadState::free_or_unstarted &&
        to == ResourceLoadState::io_scheduled_or_loading) {
        return true;
    }
    if (from == ResourceLoadState::io_scheduled_or_loading &&
        to == ResourceLoadState::io_complete_pending_postprocess) {
        return true;
    }
    if (from == ResourceLoadState::io_complete_pending_postprocess &&
        to == ResourceLoadState::ready_postprocessed) {
        return true;
    }
    if ((from == ResourceLoadState::io_scheduled_or_loading ||
         from == ResourceLoadState::io_complete_pending_postprocess ||
         from == ResourceLoadState::ready_postprocessed) &&
        to == ResourceLoadState::teardown_or_cancel_pending) {
        return true;
    }
    return from == ResourceLoadState::teardown_or_cancel_pending &&
        to == ResourceLoadState::free_or_unstarted;
}

std::optional<OwnerResourceDomain> owner_resource_domain(
    std::uint32_t group_index) noexcept {
    if (group_index > static_cast<std::uint32_t>(OwnerResourceDomain::enemy_sound)) {
        return std::nullopt;
    }
    return static_cast<OwnerResourceDomain>(group_index);
}

const PostLoadFixup* find_post_load_fixup(
    std::string_view magic) noexcept {
    const auto iterator = std::find_if(
        post_load_fixups.begin(), post_load_fixups.end(),
        [magic](const PostLoadFixup& fixup) {
            return fixup.magic == magic;
        });
    return iterator == post_load_fixups.end() ? nullptr : &*iterator;
}

const SceneFactoryEntry* find_scene_factory_entry(
    SceneFactoryId id) noexcept {
    const auto iterator = std::find_if(
        scene_factory.begin(), scene_factory.end(),
        [id](const SceneFactoryEntry& entry) {
            return entry.id == id;
        });
    return iterator == scene_factory.end() ? nullptr : &*iterator;
}

const PlayerFactoryEntry* find_player_factory_entry(
    PlayerFactorySelector selector) noexcept {
    const auto iterator = std::find_if(
        player_factory.begin(), player_factory.end(),
        [selector](const PlayerFactoryEntry& entry) {
            return entry.selector == selector;
        });
    return iterator == player_factory.end() ? nullptr : &*iterator;
}

std::string hd_audio_lookup_key(std::string_view logical_path) {
    auto result = replace_extension(basename(logical_path), hd_audio_physical_extension);
    std::transform(
        result.begin(), result.end(), result.begin(),
        [](unsigned char value) {
            return static_cast<char>(std::tolower(value));
        });
    return result;
}

std::string hd_video_physical_path(std::string_view legacy_path) {
    return replace_extension(legacy_path, hd_video_physical_extension);
}

} // namespace dmc::rengine::profiles::dmc3::vanilla
