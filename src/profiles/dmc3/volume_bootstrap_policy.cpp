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

[[nodiscard]] bool runtime_volume_filename_matches(
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

[[nodiscard]] RuntimeMountTopology invalid_topology() {
    RuntimeMountTopology result;
    result.physical_registration_attempted_before_archives = false;
    return result;
}

} // namespace

bool DiscoveredArchiveVolume::valid() const noexcept {
    return runtime_volume_filename_matches(index, filename);
}

bool MountedArchiveVolume::valid() const noexcept {
    return runtime_volume_filename_matches(index, filename);
}

bool VolumeBootstrapPlan::valid() const noexcept {
    if (!physical_registration_attempted_before_archives ||
        !mount_list_is_prepend ||
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

bool RuntimeMountTopology::structurally_valid() const noexcept {
    if (!physical_registration_attempted_before_archives ||
        !mount_list_is_prepend ||
        mounted_archives.size() != archive_resolution_order.size()) {
        return false;
    }

    const auto mounted_count = mounted_archives.size();
    for (std::size_t order = 0U; order < mounted_count; ++order) {
        const auto& volume = mounted_archives[order];
        if (!volume.valid() || volume.registration_order != order ||
            volume.resolution_rank != mounted_count - order - 1U) {
            return false;
        }
        if (order != 0U &&
            mounted_archives[order - 1U].index >= volume.index) {
            return false;
        }

        const auto expected_resolution_index =
            mounted_archives[mounted_count - order - 1U].index;
        if (archive_resolution_order[order] != expected_resolution_index) {
            return false;
        }
    }
    return true;
}

bool RuntimeMountTopology::valid_for(
    const VolumeBootstrapPlan& discovery) const noexcept {
    if (!discovery.valid() || !structurally_valid()) {
        return false;
    }

    for (const auto& volume : mounted_archives) {
        if (volume.index >= discovery.first_missing_index) {
            return false;
        }
        const auto& discovered = discovery.discovered_archives[volume.index];
        if (discovered.index != volume.index ||
            discovered.filename != volume.filename) {
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
        .mount_list_is_prepend = true,
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

RuntimeMountTopology VolumeBootstrapPolicy::mount_topology(
    const VolumeBootstrapPlan& discovery,
    bool physical_mount_succeeded,
    std::span<const std::uint32_t> successful_archive_indices) {
    if (!discovery.valid()) {
        return invalid_topology();
    }

    RuntimeMountTopology result{
        .physical_registration_attempted_before_archives = true,
        .physical_mount_succeeded = physical_mount_succeeded,
        .mount_list_is_prepend = true,
        .mounted_archives = {},
        .archive_resolution_order = {},
    };

    result.mounted_archives.reserve(successful_archive_indices.size());
    bool first = true;
    std::uint32_t previous = 0U;
    for (const auto index : successful_archive_indices) {
        if (index >= discovery.first_missing_index ||
            (!first && index <= previous)) {
            return invalid_topology();
        }
        first = false;
        previous = index;

        const auto& discovered = discovery.discovered_archives[index];
        result.mounted_archives.push_back(MountedArchiveVolume{
            .index = index,
            .filename = discovered.filename,
            .registration_order = result.mounted_archives.size(),
            .resolution_rank = 0U,
        });
    }

    const auto mounted_count = result.mounted_archives.size();
    result.archive_resolution_order.reserve(mounted_count);
    for (std::size_t order = 0U; order < mounted_count; ++order) {
        result.mounted_archives[order].resolution_rank =
            mounted_count - order - 1U;
    }
    for (std::size_t remaining = mounted_count; remaining > 0U; --remaining) {
        result.archive_resolution_order.push_back(
            result.mounted_archives[remaining - 1U].index);
    }

    return result.valid_for(discovery) ? result : invalid_topology();
}

RuntimeMountTopology VolumeBootstrapPolicy::all_success_topology(
    const VolumeBootstrapPlan& discovery) {
    if (!discovery.valid()) {
        return invalid_topology();
    }

    std::vector<std::uint32_t> successful;
    successful.reserve(discovery.discovered_archives.size());
    for (const auto& volume : discovery.discovered_archives) {
        successful.push_back(volume.index);
    }
    return mount_topology(discovery, true, successful);
}

} // namespace dmc::rengine::profiles::dmc3
