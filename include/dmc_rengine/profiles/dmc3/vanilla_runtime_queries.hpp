#pragma once

#include "dmc_rengine/profiles/dmc3/vanilla_runtime.hpp"

#include <cstdint>
#include <optional>
#include <string>
#include <string_view>

namespace dmc::rengine::profiles::dmc3::vanilla {

// These helpers expose only observed/recovered facts. They do not turn the
// evidence model into a replacement game runtime.

[[nodiscard]] const StageRoleAlias* find_stage_role_alias(
    std::uint16_t stage_id) noexcept;

[[nodiscard]] bool observed_resource_load_transition(
    ResourceLoadState from,
    ResourceLoadState to) noexcept;

[[nodiscard]] std::optional<OwnerResourceDomain> owner_resource_domain(
    std::uint32_t group_index) noexcept;

[[nodiscard]] const PostLoadFixup* find_post_load_fixup(
    std::string_view magic) noexcept;

[[nodiscard]] const SceneFactoryEntry* find_scene_factory_entry(
    SceneFactoryId id) noexcept;

[[nodiscard]] const PlayerFactoryEntry* find_player_factory_entry(
    PlayerFactorySelector selector) noexcept;

// Reproduces the evidenced HD audio lookup key boundary: use the basename,
// replace the extension with .ogg and lowercase ASCII before descriptor-table
// lookup. It intentionally does not perform file I/O or FMOD calls.
[[nodiscard]] std::string hd_audio_lookup_key(std::string_view logical_path);

// Reproduces the evidenced media path rewrite boundary: preserve the supplied
// path and replace its final extension with .wmv.
[[nodiscard]] std::string hd_video_physical_path(std::string_view legacy_path);

} // namespace dmc::rengine::profiles::dmc3::vanilla
