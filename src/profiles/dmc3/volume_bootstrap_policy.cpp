#include "dmc_rengine/profiles/dmc3/volume_bootstrap_policy.hpp"

#include <algorithm>
#include <array>
#include <charconv>
#include <string>
#include <string_view>
#include <system_error>
#include <vector>

namespace dmc::rengine::profiles::dmc3 {
namespace {

[[nodiscard]] bool canonical_volume_identity(
    std::uint32_t index,
    std::string_view filename) noexcept {
    if (!VolumeBootstrapPolicy::runtime_index_valid(index)) {
        return false;
    }

    constexpr std::string_view prefix = "DMC3-";
    constexpr std::string_view suffix = ".nbz";

    std::array<char, 10U> digits{};
    const auto conversion = std::to_chars(
        digits.data(), digits.data() + digits.size(), index);
    if (conversion.ec != std::errc{}) {
        return false;
    }

    const std::string_view encoded_index{
        digits.data(),
        static_cast<std::size_t>(conversion.ptr - digits.data())};
    if (filename.size() != prefix.size() + encoded_index.size() + suffix.size() ||
        !filename.starts_with(prefix) || !filename.ends_with(suffix)) {
        return false;
    }

    return filename.substr(prefix.size(), encoded_index.size()) == encoded_index;
}

} // namespace

bool DiscoveredArchiveVolume::valid() const noexcept {
    return canonical_volume_identity(index, filename);
}

bool RuntimeArchiveVolume::valid() const noexcept {
    return canonical_volume_identity(index, filename);
}

bool VolumeBootstrapPlan::valid() const noexcept {
    if (!physical_registration_attempted_before_archives ||
        !archive_discovery_stops_at_first_missing ||
        discovered_archives.size() != static_cast<std::size_t>(first_missing_index)) {
        return false;
    }

    for (std::size_t order = 0U; order < discovered_archives.size(); ++order) {
        const auto& volume = discovered_archives[order];
        if (!volume.valid() ||
            volume.index != static_cast<std::uint32_t>(order) ||
            volume.discovery_order != order) {
            return false;
        }
    }

    std::uint32_t previous = first_missing_index;
    for (const auto index : present_after_first_gap) {
        if (!VolumeBootstrapPolicy::runtime_index_valid(index) ||
            index <= previous) {
            return false;
        }
        previous = index;
    }

    bool first_outside = true;
    std::uint32_t previous_outside = 0U;
    for (const auto index : present_outside_runtime_index_domain) {
        if (VolumeBootstrapPolicy::runtime_index_valid(index) ||
            (!first_outside && index <= previous_outside)) {
            return false;
        }
        first_outside = false;
        previous_outside = index;
    }
    return true;
}

bool RuntimeMountTopology::valid() const noexcept {
    if (!mount_list_is_prepend ||
        mounted_archives.size() != archive_resolution_order.size()) {
        return false;
    }

    const auto archive_count = mounted_archives.size();
    for (std::size_t order = 0U; order < archive_count; ++order) {
        const auto& volume = mounted_archives[order];
        if (!volume.valid() ||
            volume.index >= discovery_first_missing_index ||
            volume.successful_registration_order != order ||
            volume.resolution_rank != archive_count - order - 1U ||
            (order != 0U && mounted_archives[order - 1U].index >= volume.index)) {
            return false;
        }

        const auto& expected = mounted_archives[archive_count - order - 1U];
        if (archive_resolution_order[order] != expected.index) {
            return false;
        }
    }
    return true;
}

bool RuntimeMountTopology::valid_for(
    const VolumeBootstrapPlan& discovery) const noexcept {
    if (!valid() || !discovery.valid() ||
        discovery_first_missing_index != discovery.first_missing_index) {
        return false;
    }

    for (const auto& mounted : mounted_archives) {
        const auto iterator = std::find_if(
            discovery.discovered_archives.begin(),
            discovery.discovered_archives.end(),
            [&mounted](const DiscoveredArchiveVolume& discovered) {
                return discovered.index == mounted.index &&
                    discovered.filename == mounted.filename;
            });
        if (iterator == discovery.discovered_archives.end()) {
            return false;
        }
    }
    return true;
}

std::string VolumeBootstrapPolicy::volume_filename(std::uint32_t index) {
    if (!runtime_index_valid(index)) {
        return {};
    }
    return "DMC3-" + std::to_string(index) + ".nbz";
}

VolumeBootstrapPlan VolumeBootstrapPolicy::plan(
    std::span<const std::uint32_t> present_indices) {
    std::vector<std::uint32_t> present{
        present_indices.begin(), present_indices.end()};
    std::sort(present.begin(), present.end());
    present.erase(std::unique(present.begin(), present.end()), present.end());

    const auto runtime_end = std::upper_bound(
        present.begin(), present.end(), runtime_index_max());
    std::vector<std::uint32_t> runtime_present{present.begin(), runtime_end};
    std::vector<std::uint32_t> outside_runtime{runtime_end, present.end()};

    std::uint32_t first_missing = 0U;
    std::size_t cursor = 0U;
    while (cursor < runtime_present.size() &&
           runtime_present[cursor] == first_missing) {
        ++first_missing;
        ++cursor;
    }

    VolumeBootstrapPlan result{
        .physical_registration_attempted_before_archives = true,
        .archive_discovery_stops_at_first_missing = true,
        .first_missing_index = first_missing,
        .discovered_archives = {},
        .present_after_first_gap = {},
        .present_outside_runtime_index_domain = std::move(outside_runtime),
    };

    result.discovered_archives.reserve(first_missing);
    for (std::uint32_t index = 0U; index < first_missing; ++index) {
        result.discovered_archives.push_back(DiscoveredArchiveVolume{
            .index = index,
            .filename = volume_filename(index),
            .discovery_order = static_cast<std::size_t>(index),
        });
    }

    for (; cursor < runtime_present.size(); ++cursor) {
        if (runtime_present[cursor] > first_missing) {
            result.present_after_first_gap.push_back(runtime_present[cursor]);
        }
    }

    return result;
}

std::optional<RuntimeMountTopology> VolumeBootstrapPolicy::successful_mount_topology(
    const VolumeBootstrapPlan& discovery,
    bool physical_root_mounted,
    std::span<const std::uint32_t> successful_archive_indices) {
    if (!discovery.valid()) {
        return std::nullopt;
    }

    std::vector<std::uint32_t> successful{
        successful_archive_indices.begin(), successful_archive_indices.end()};
    std::sort(successful.begin(), successful.end());
    if (std::adjacent_find(successful.begin(), successful.end()) != successful.end()) {
        return std::nullopt;
    }
    if (std::any_of(
            successful.begin(), successful.end(),
            [&discovery](std::uint32_t index) {
                return index >= discovery.first_missing_index;
            })) {
        return std::nullopt;
    }

    RuntimeMountTopology topology{
        .mount_list_is_prepend = true,
        .physical_root_mounted = physical_root_mounted,
        .discovery_first_missing_index = discovery.first_missing_index,
        .mounted_archives = {},
        .archive_resolution_order = {},
    };

    topology.mounted_archives.reserve(successful.size());
    for (std::size_t order = 0U; order < successful.size(); ++order) {
        const auto index = successful[order];
        topology.mounted_archives.push_back(RuntimeArchiveVolume{
            .index = index,
            .filename = volume_filename(index),
            .successful_registration_order = order,
            .resolution_rank = successful.size() - order - 1U,
        });
    }

    topology.archive_resolution_order.reserve(successful.size());
    for (auto iterator = successful.rbegin(); iterator != successful.rend(); ++iterator) {
        topology.archive_resolution_order.push_back(*iterator);
    }

    if (!topology.valid_for(discovery)) {
        return std::nullopt;
    }
    return topology;
}

} // namespace dmc::rengine::profiles::dmc3
