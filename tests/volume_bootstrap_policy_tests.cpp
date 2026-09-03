#include "dmc_rengine/profiles/dmc3/volume_bootstrap_policy.hpp"

#include <array>
#include <cassert>
#include <cstdint>
#include <limits>
#include <span>
#include <string>
#include <vector>

int main() {
    using dmc::rengine::profiles::dmc3::DiscoveredArchiveVolume;
    using dmc::rengine::profiles::dmc3::MountedArchiveVolume;
    using dmc::rengine::profiles::dmc3::VolumeBootstrapPolicy;

    assert(VolumeBootstrapPolicy::data_subdirectory() == "data/dmc3");
    assert(VolumeBootstrapPolicy::executable_data_suffix() == "\\data\\dmc3\\");
    assert(VolumeBootstrapPolicy::volume_format() == "%sDMC3-%d.nbz");
    assert(VolumeBootstrapPolicy::runtime_index_max() == 0x7FFFFFFFU);
    assert(VolumeBootstrapPolicy::runtime_index_valid(0U));
    assert(VolumeBootstrapPolicy::runtime_index_valid(
        VolumeBootstrapPolicy::runtime_index_max()));
    assert(!VolumeBootstrapPolicy::runtime_index_valid(
        VolumeBootstrapPolicy::runtime_index_max() + 1U));

    assert(VolumeBootstrapPolicy::volume_filename(0U) == "DMC3-0.nbz");
    assert(VolumeBootstrapPolicy::volume_filename(12U) == "DMC3-12.nbz");
    assert(VolumeBootstrapPolicy::volume_filename(
        VolumeBootstrapPolicy::runtime_index_max()) == "DMC3-2147483647.nbz");
    assert(VolumeBootstrapPolicy::volume_filename(
        VolumeBootstrapPolicy::runtime_index_max() + 1U).empty());
    assert(VolumeBootstrapPolicy::volume_filename(
        std::numeric_limits<std::uint32_t>::max()).empty());

    const DiscoveredArchiveVolume max_runtime_discovered{
        .index = VolumeBootstrapPolicy::runtime_index_max(),
        .filename = "DMC3-2147483647.nbz",
        .discovery_order = 0U,
    };
    assert(max_runtime_discovered.valid());

    const MountedArchiveVolume max_runtime_mounted{
        .index = VolumeBootstrapPolicy::runtime_index_max(),
        .filename = "DMC3-2147483647.nbz",
        .registration_order = 0U,
        .resolution_rank = 0U,
    };
    assert(max_runtime_mounted.valid());

    auto leading_zero = max_runtime_discovered;
    leading_zero.index = 7U;
    leading_zero.filename = "DMC3-07.nbz";
    assert(!leading_zero.valid());

    const auto empty = VolumeBootstrapPolicy::plan(
        std::span<const std::uint32_t>{});
    assert(empty.valid());
    assert(empty.physical_registration_attempted_before_archives);
    assert(empty.first_missing_index == 0U);
    assert(empty.discovered_archives.empty());
    assert(empty.present_outside_runtime_index_domain.empty());

    const auto empty_all_success = VolumeBootstrapPolicy::all_success_topology(empty);
    assert(empty_all_success.valid_for(empty));
    assert(empty_all_success.physical_mount_succeeded);
    assert(empty_all_success.mounted_archives.empty());
    assert(empty_all_success.archive_resolution_order.empty());

    const auto empty_physical_failure = VolumeBootstrapPolicy::mount_topology(
        empty, false, std::span<const std::uint32_t>{});
    assert(empty_physical_failure.valid_for(empty));
    assert(!empty_physical_failure.physical_mount_succeeded);

    constexpr std::array<std::uint32_t, 4> contiguous_input{2U, 0U, 1U, 1U};
    const auto contiguous = VolumeBootstrapPolicy::plan(contiguous_input);
    assert(contiguous.valid());
    assert(contiguous.first_missing_index == 3U);
    assert(contiguous.discovered_archives.size() == 3U);
    assert(contiguous.discovered_archives[0].index == 0U);
    assert(contiguous.discovered_archives[1].index == 1U);
    assert(contiguous.discovered_archives[2].index == 2U);
    assert(contiguous.present_outside_runtime_index_domain.empty());

    // Discovery itself carries no claim that any registration succeeded.
    auto wrong_filename = contiguous;
    wrong_filename.discovered_archives[1].filename = "DMC3-9.nbz";
    assert(!wrong_filename.valid());

    const auto clean = VolumeBootstrapPolicy::all_success_topology(contiguous);
    assert(clean.valid_for(contiguous));
    assert(clean.physical_mount_succeeded);
    assert(clean.mounted_archives.size() == 3U);
    assert(clean.mounted_archives[0].resolution_rank == 2U);
    assert(clean.mounted_archives[1].resolution_rank == 1U);
    assert(clean.mounted_archives[2].resolution_rank == 0U);
    assert((clean.archive_resolution_order ==
        std::vector<std::uint32_t>{2U, 1U, 0U}));

    // Exact reverse-driven sparse success case: archive 1 can fail registration
    // while discovery continues and archive 2 succeeds. Effective order is 2,0.
    constexpr std::array<std::uint32_t, 2> sparse_success{0U, 2U};
    const auto sparse = VolumeBootstrapPolicy::mount_topology(
        contiguous, true, sparse_success);
    assert(sparse.valid_for(contiguous));
    assert(sparse.physical_mount_succeeded);
    assert(sparse.mounted_archives.size() == 2U);
    assert(sparse.mounted_archives[0].index == 0U);
    assert(sparse.mounted_archives[1].index == 2U);
    assert((sparse.archive_resolution_order ==
        std::vector<std::uint32_t>{2U, 0U}));

    // Physical registration failure is independent from numbered discovery and
    // successful archive mounts.
    const auto sparse_without_physical = VolumeBootstrapPolicy::mount_topology(
        contiguous, false, sparse_success);
    assert(sparse_without_physical.valid_for(contiguous));
    assert(!sparse_without_physical.physical_mount_succeeded);
    assert((sparse_without_physical.archive_resolution_order ==
        std::vector<std::uint32_t>{2U, 0U}));

    constexpr std::array<std::uint32_t, 2> wrong_order{2U, 0U};
    const auto invalid_order = VolumeBootstrapPolicy::mount_topology(
        contiguous, true, wrong_order);
    assert(!invalid_order.valid_for(contiguous));

    constexpr std::array<std::uint32_t, 2> duplicate_success{0U, 0U};
    const auto invalid_duplicate = VolumeBootstrapPolicy::mount_topology(
        contiguous, true, duplicate_success);
    assert(!invalid_duplicate.valid_for(contiguous));

    constexpr std::array<std::uint32_t, 1> undiscovered_success{3U};
    const auto invalid_undiscovered = VolumeBootstrapPolicy::mount_topology(
        contiguous, true, undiscovered_success);
    assert(!invalid_undiscovered.valid_for(contiguous));

    constexpr std::array<std::uint32_t, 4> gap_input{0U, 2U, 3U, 7U};
    const auto gap = VolumeBootstrapPolicy::plan(gap_input);
    assert(gap.valid());
    assert(gap.first_missing_index == 1U);
    assert(gap.discovered_archives.size() == 1U);
    assert((gap.present_after_first_gap ==
        std::vector<std::uint32_t>{2U, 3U, 7U}));
    assert(gap.present_outside_runtime_index_domain.empty());

    auto duplicate_post_gap = gap;
    duplicate_post_gap.present_after_first_gap = {2U, 2U, 7U};
    assert(!duplicate_post_gap.valid());
    auto unsorted_post_gap = gap;
    unsorted_post_gap.present_after_first_gap = {3U, 2U, 7U};
    assert(!unsorted_post_gap.valid());

    constexpr std::array<std::uint32_t, 3> missing_zero_input{1U, 2U, 9U};
    const auto missing_zero = VolumeBootstrapPolicy::plan(missing_zero_input);
    assert(missing_zero.valid());
    assert(missing_zero.first_missing_index == 0U);
    assert(missing_zero.discovered_archives.empty());
    assert((missing_zero.present_after_first_gap ==
        std::vector<std::uint32_t>{1U, 2U, 9U}));

    constexpr std::array<std::uint32_t, 5> later_gap_input{4U, 1U, 0U, 5U, 9U};
    const auto later_gap = VolumeBootstrapPolicy::plan(later_gap_input);
    assert(later_gap.valid());
    assert(later_gap.first_missing_index == 2U);
    assert(later_gap.discovered_archives.size() == 2U);
    assert((later_gap.present_after_first_gap ==
        std::vector<std::uint32_t>{4U, 5U, 9U}));

    // Product discovery may see numeric suffixes outside the recovered `%d`
    // non-negative signed domain. Keep them visible, but never runtime-promote
    // them as discovered/bootstrap indices.
    const auto first_outside = VolumeBootstrapPolicy::runtime_index_max() + 1U;
    const auto uint32_max = std::numeric_limits<std::uint32_t>::max();
    const std::array<std::uint32_t, 5> outside_input{
        0U, 2U, first_outside, uint32_max, first_outside};
    const auto outside = VolumeBootstrapPolicy::plan(outside_input);
    assert(outside.valid());
    assert(outside.first_missing_index == 1U);
    assert(outside.discovered_archives.size() == 1U);
    assert((outside.present_after_first_gap == std::vector<std::uint32_t>{2U}));
    assert((outside.present_outside_runtime_index_domain ==
        std::vector<std::uint32_t>{first_outside, uint32_max}));

    auto bad_outside_domain = outside;
    bad_outside_domain.present_outside_runtime_index_domain = {7U, first_outside};
    assert(!bad_outside_domain.valid());
    auto duplicate_outside_domain = outside;
    duplicate_outside_domain.present_outside_runtime_index_domain = {
        first_outside, first_outside};
    assert(!duplicate_outside_domain.valid());

    return 0;
}
