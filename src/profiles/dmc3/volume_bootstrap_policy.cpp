#include "dmc_rengine/profiles/dmc3/volume_bootstrap_policy.hpp"

#include <algorithm>
#include <string>
#include <vector>

namespace dmc::rengine::profiles::dmc3 {

bool RuntimeArchiveVolume::valid() const noexcept {
    return !filename.empty();
}

bool VolumeBootstrapPlan::valid() const noexcept {
    if (!physical_root_registered_before_archives || !mount_list_is_prepend ||
        registered_archives.size() != archive_resolution_order.size()) {
        return false;
    }

    const auto archive_count = registered_archives.size();
    for (std::size_t order = 0U; order < archive_count; ++order) {
        const auto& volume = registered_archives[order];
        if (!volume.valid() ||
            volume.index != static_cast<std::uint32_t>(order) ||
            volume.registration_order != order ||
            volume.resolution_rank != archive_count - order - 1U) {
            return false;
        }

        const auto expected_index = static_cast<std::uint32_t>(
            archive_count - order - 1U);
        if (archive_resolution_order[order] != expected_index) {
            return false;
        }
    }

    if (static_cast<std::size_t>(first_missing_index) != archive_count) {
        return false;
    }

    return std::all_of(
        present_after_first_gap.begin(),
        present_after_first_gap.end(),
        [this](std::uint32_t index) {
            return index > first_missing_index;
        });
}

std::string VolumeBootstrapPolicy::volume_filename(std::uint32_t index) {
    return "DMC3-" + std::to_string(index) + ".nbz";
}

VolumeBootstrapPlan VolumeBootstrapPolicy::plan(
    std::span<const std::uint32_t> present_indices) {
    std::vector<std::uint32_t> present{
        present_indices.begin(), present_indices.end()};
    std::sort(present.begin(), present.end());
    present.erase(std::unique(present.begin(), present.end()), present.end());

    std::uint32_t first_missing = 0U;
    std::size_t cursor = 0U;
    while (cursor < present.size() && present[cursor] == first_missing) {
        ++first_missing;
        ++cursor;
    }

    VolumeBootstrapPlan result{
        .physical_root_registered_before_archives = true,
        .mount_list_is_prepend = true,
        .first_missing_index = first_missing,
        .registered_archives = {},
        .archive_resolution_order = {},
        .present_after_first_gap = {},
    };

    result.registered_archives.reserve(first_missing);
    for (std::uint32_t index = 0U; index < first_missing; ++index) {
        const auto registration_order = static_cast<std::size_t>(index);
        result.registered_archives.push_back(RuntimeArchiveVolume{
            .index = index,
            .filename = volume_filename(index),
            .registration_order = registration_order,
            .resolution_rank =
                static_cast<std::size_t>(first_missing - index - 1U),
        });
    }

    result.archive_resolution_order.reserve(first_missing);
    for (std::uint32_t index = first_missing; index > 0U; --index) {
        result.archive_resolution_order.push_back(index - 1U);
    }

    for (; cursor < present.size(); ++cursor) {
        if (present[cursor] > first_missing) {
            result.present_after_first_gap.push_back(present[cursor]);
        }
    }

    return result;
}

} // namespace dmc::rengine::profiles::dmc3
