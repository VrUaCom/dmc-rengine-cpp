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

[[nodiscard]] bool canonical_filename_matches(
    std::uint32_t index,
    std::string_view filename) noexcept {
    std::array<char, 32> expected{};
    constexpr std::string_view prefix{"DMC3-"};
    constexpr std::string_view suffix{".nbz"};

    std::copy(prefix.begin(), prefix.end(), expected.begin());
    auto* number_begin = expected.data() +
        static_cast<std::ptrdiff_t>(prefix.size());
    auto* number_end = expected.data() +
        static_cast<std::ptrdiff_t>(expected.size() - suffix.size());
    const auto converted = std::to_chars(number_begin, number_end, index);
    if (converted.ec != std::errc{}) {
        return false;
    }

    std::copy(suffix.begin(), suffix.end(), converted.ptr);
    const auto length = static_cast<std::size_t>(
        converted.ptr - expected.data()) + suffix.size();
    return filename == std::string_view{expected.data(), length};
}

} // namespace

bool RuntimeArchiveVolume::valid() const noexcept {
    return canonical_filename_matches(index, filename);
}

bool VolumeBootstrapPlan::valid() const noexcept {
    if (!physical_root_registered_before_archives || !mount_list_is_prepend ||
        registered_archives.size() != archive_resolution_order.size() ||
        registered_archives.size() !=
            static_cast<std::size_t>(first_missing_index)) {
        return false;
    }

    for (std::size_t order = 0U; order < registered_archives.size(); ++order) {
        const auto& volume = registered_archives[order];
        if (!volume.valid() ||
            static_cast<std::size_t>(volume.index) != order ||
            volume.registration_order != order ||
            volume.resolution_priority != registered_archives.size() - order - 1U) {
            return false;
        }
        if (static_cast<std::size_t>(archive_resolution_order[order]) !=
            registered_archives.size() - order - 1U) {
            return false;
        }
    }

    std::uint32_t previous = first_missing_index;
    bool first = true;
    for (const auto index : present_after_first_gap) {
        if (index <= first_missing_index || (!first && index <= previous)) {
            return false;
        }
        previous = index;
        first = false;
    }

    return true;
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
            .resolution_priority =
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
