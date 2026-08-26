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

[[nodiscard]] bool filename_matches_index(
    std::string_view filename,
    std::uint32_t index) noexcept {
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

bool RuntimeArchiveMountAttempt::valid() const noexcept {
    return filename_matches_index(filename, index);
}

bool RuntimeArchiveVolume::valid() const noexcept {
    return filename_matches_index(filename, index);
}

bool VolumeBootstrapPlan::valid() const noexcept {
    if (!physical_root_registration_attempt_precedes_archives ||
        !mount_list_is_prepend) {
        return false;
    }

    if (static_cast<std::size_t>(first_missing_index) != discovered_archives.size()) {
        return false;
    }

    for (std::size_t order = 0U; order < discovered_archives.size(); ++order) {
        const auto& attempt = discovered_archives[order];
        if (!attempt.valid() ||
            attempt.index != static_cast<std::uint32_t>(order) ||
            attempt.attempt_order != order) {
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

bool VolumeBootstrapPlan::discovered(std::uint32_t index) const noexcept {
    return index < first_missing_index &&
        static_cast<std::size_t>(index) < discovered_archives.size() &&
        discovered_archives[static_cast<std::size_t>(index)].index == index;
}

bool RuntimeMountTopology::valid_for(
    const VolumeBootstrapPlan& discovery) const noexcept {
    if (!discovery.valid() || !mount_list_is_prepend ||
        mounted_archives.size() != archive_resolution_order.size()) {
        return false;
    }

    for (std::size_t order = 0U; order < mounted_archives.size(); ++order) {
        const auto& volume = mounted_archives[order];
        if (!volume.valid() || !discovery.discovered(volume.index) ||
            volume.registration_order != order ||
            volume.resolution_rank != mounted_archives.size() - order - 1U) {
            return false;
        }
        if (order > 0U && mounted_archives[order - 1U].index >= volume.index) {
            return false;
        }
    }

    for (std::size_t rank = 0U; rank < archive_resolution_order.size(); ++rank) {
        const auto expected = mounted_archives[mounted_archives.size() - rank - 1U].index;
        if (archive_resolution_order[rank] != expected) {
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
        .physical_root_registration_attempt_precedes_archives = true,
        .mount_list_is_prepend = true,
        .first_missing_index = first_missing,
        .discovered_archives = {},
        .present_after_first_gap = {},
        .present_outside_runtime_index_domain = std::move(outside_runtime),
    };

    result.discovered_archives.reserve(first_missing);
    for (std::uint32_t index = 0U; index < first_missing; ++index) {
        result.discovered_archives.push_back(RuntimeArchiveMountAttempt{
            .index = index,
            .filename = volume_filename(index),
            .attempt_order = static_cast<std::size_t>(index),
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
    bool physical_root_mounted,
    std::span<const std::uint32_t> successful_archive_indices) {
    RuntimeMountTopology result{
        .physical_root_mounted = physical_root_mounted,
        .mount_list_is_prepend = true,
        .mounted_archives = {},
        .archive_resolution_order = {},
    };
    if (!discovery.valid()) {
        return result;
    }

    std::vector<std::uint32_t> successful{
        successful_archive_indices.begin(), successful_archive_indices.end()};
    std::sort(successful.begin(), successful.end());
    successful.erase(std::unique(successful.begin(), successful.end()), successful.end());

    for (const auto index : successful) {
        if (!discovery.discovered(index)) {
            result.mount_list_is_prepend = false;
            return result;
        }
    }

    result.mounted_archives.reserve(successful.size());
    for (std::size_t order = 0U; order < successful.size(); ++order) {
        const auto index = successful[order];
        result.mounted_archives.push_back(RuntimeArchiveVolume{
            .index = index,
            .filename = volume_filename(index),
            .registration_order = order,
            .resolution_rank = successful.size() - order - 1U,
        });
    }

    result.archive_resolution_order.reserve(successful.size());
    for (auto iterator = successful.rbegin(); iterator != successful.rend(); ++iterator) {
        result.archive_resolution_order.push_back(*iterator);
    }
    return result;
}

} // namespace dmc::rengine::profiles::dmc3
