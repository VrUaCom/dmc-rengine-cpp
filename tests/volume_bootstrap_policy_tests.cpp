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
    using dmc::rengine::profiles::dmc3::RuntimeArchiveVolume;
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

    const DiscoveredArchiveVolume max_discovered{
        .index = VolumeBootstrapPolicy::runtime_index_max(),
        .filename = "DMC3-2147483647.nbz",
        .discovery_order = 0U,
    };
    assert(max_discovered.valid());

    const RuntimeArchiveVolume max_runtime_named{
        .index = VolumeBootstrapPolicy::runtime_index_max(),
        .filename = "DMC3-2147483647.nbz",
        .successful_registration_order = 0U,
        .resolution_rank = 0U,
    };
    assert(max_runtime_named.valid());

    const RuntimeArchiveVolume outside_runtime_named{
        .index = VolumeBootstrapPolicy::runtime_index_max() + 1U,
        .filename = "DMC3-2147483648.nbz",
        .successful_registration_order = 0U,
        .resolution_rank = 0U,
    };
    assert(!outside_runtime_named.valid());

    auto leading_zero = max_discovered;
    leading_zero.index = 7U;
    leading_zero.filename = "DMC3-07.nbz";
    assert(!leading_zero.valid());

    const auto empty = VolumeBootstrapPolicy::plan(
        std::span<const std::uint32_t>{});
    assert(empty.valid());
    assert(empty.first_missing_index == 0U);
    assert(empty.discovered_archives.empty());
    assert(empty.present_outside_runtime_index_domain.empty());

    const auto empty_topology = VolumeBootstrapPolicy::successful_mount_topology(
        empty, false, std::span<const std::uint32_t>{});
    assert(empty_topology.has_value());
    assert(empty_topology->valid_for(empty));
    assert(!empty_topology->physical_root_mounted);
    assert(empty_topology->mounted_archives.empty());
    assert(empty_topology->archive_resolution_order.empty());

    constexpr std::array<std::uint32_t, 4> contiguous_input{2U, 0U, 1U, 1U};
    const auto contiguous = VolumeBootstrapPolicy::plan(contiguous_input);
    assert(contiguous.valid());
    assert(contiguous.first_missing_index == 3U);
    assert(contiguous.discovered_archives.size() == 3U);
    assert(contiguous.discovered_archives[0].index == 0U);
    assert(contiguous.discovered_archives[1].index == 1U);
    assert(contiguous.discovered_archives[2].index == 2U);
    assert(contiguous.discovered_archives[2].discovery_order == 2U);
    assert(contiguous.present_outside_runtime_index_domain.empty());

    // Clean bootstrap success preserves the previously recovered observable
    // archive precedence: ascending registrations prepend, so 2 -> 1 -> 0.
    constexpr std::array<std::uint32_t, 3> all_success{0U, 1U, 2U};
    const auto clean_topology = VolumeBootstrapPolicy::successful_mount_topology(
        contiguous, true, all_success);
    assert(clean_topology.has_value());
    assert(clean_topology->valid_for(contiguous));
    assert(clean_topology->physical_root_mounted);
    assert(clean_topology->mounted_archives.size() == 3U);
    assert(clean_topology->mounted_archives[0].resolution_rank == 2U);
    assert(clean_topology->mounted_archives[1].resolution_rank == 1U);
    assert(clean_topology->mounted_archives[2].resolution_rank == 0U);
    assert((clean_topology->archive_resolution_order ==
        std::vector<std::uint32_t>{2U, 1U, 0U}));

    // Canonical EXE bootstrap ignores archive-registration return values. An
    // existing DMC3-1 may fail mount while DMC3-2 is still discovered and
    // successfully prepended. Successful topology is therefore sparse: 2 -> 0.
    constexpr std::array<std::uint32_t, 2> sparse_success{2U, 0U};
    const auto sparse_topology = VolumeBootstrapPolicy::successful_mount_topology(
        contiguous, true, sparse_success);
    assert(sparse_topology.has_value());
    assert(sparse_topology->valid_for(contiguous));
    assert(sparse_topology->mounted_archives.size() == 2U);
    assert(sparse_topology->mounted_archives[0].index == 0U);
    assert(sparse_topology->mounted_archives[0].successful_registration_order == 0U);
    assert(sparse_topology->mounted_archives[1].index == 2U);
    assert(sparse_topology->mounted_archives[1].successful_registration_order == 1U);
    assert((sparse_topology->archive_resolution_order ==
        std::vector<std::uint32_t>{2U, 0U}));

    // Physical registration return is ignored by bootstrap as well. A topology
    // with successful archives and no type-0 physical node is representable.
    const auto no_physical = VolumeBootstrapPolicy::successful_mount_topology(
        contiguous, false, sparse_success);
    assert(no_physical.has_value());
    assert(no_physical->valid_for(contiguous));
    assert(!no_physical->physical_root_mounted);
    assert((no_physical->archive_resolution_order ==
        std::vector<std::uint32_t>{2U, 0U}));

    constexpr std::array<std::uint32_t, 2> duplicate_success{0U, 0U};
    assert(!VolumeBootstrapPolicy::successful_mount_topology(
        contiguous, true, duplicate_success).has_value());
    constexpr std::array<std::uint32_t, 1> missing_index_claim{3U};
    assert(!VolumeBootstrapPolicy::successful_mount_topology(
        contiguous, true, missing_index_claim).has_value());

    auto wrong_filename = contiguous;
    wrong_filename.discovered_archives[1].filename = "DMC3-9.nbz";
    assert(!wrong_filename.valid());
    assert(!VolumeBootstrapPolicy::successful_mount_topology(
        wrong_filename, true, all_success).has_value());

    auto forged_sparse = *sparse_topology;
    forged_sparse.archive_resolution_order = {2U, 1U};
    assert(!forged_sparse.valid());
    auto wrong_discovery_binding = *sparse_topology;
    wrong_discovery_binding.discovery_first_missing_index = 2U;
    assert(!wrong_discovery_binding.valid());
    assert(!wrong_discovery_binding.valid_for(contiguous));

    constexpr std::array<std::uint32_t, 4> gap_input{0U, 2U, 3U, 7U};
    const auto gap = VolumeBootstrapPolicy::plan(gap_input);
    assert(gap.valid());
    assert(gap.first_missing_index == 1U);
    assert(gap.discovered_archives.size() == 1U);
    assert(gap.discovered_archives[0].index == 0U);
    assert((gap.present_after_first_gap ==
        std::vector<std::uint32_t>{2U, 3U, 7U}));
    assert(gap.present_outside_runtime_index_domain.empty());

    constexpr std::array<std::uint32_t, 1> gap_success{0U};
    const auto gap_topology = VolumeBootstrapPolicy::successful_mount_topology(
        gap, true, gap_success);
    assert(gap_topology.has_value());
    assert((gap_topology->archive_resolution_order ==
        std::vector<std::uint32_t>{0U}));
    constexpr std::array<std::uint32_t, 1> after_gap_not_attempted{2U};
    assert(!VolumeBootstrapPolicy::successful_mount_topology(
        gap, true, after_gap_not_attempted).has_value());

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
    constexpr std::array<std::uint32_t, 1> impossible_zero_success{1U};
    assert(!VolumeBootstrapPolicy::successful_mount_topology(
        missing_zero, true, impossible_zero_success).has_value());

    constexpr std::array<std::uint32_t, 5> later_gap_input{4U, 1U, 0U, 5U, 9U};
    const auto later_gap = VolumeBootstrapPolicy::plan(later_gap_input);
    assert(later_gap.valid());
    assert(later_gap.first_missing_index == 2U);
    assert(later_gap.discovered_archives.size() == 2U);
    assert((later_gap.present_after_first_gap ==
        std::vector<std::uint32_t>{4U, 5U, 9U}));

    // Product discovery may see numeric suffixes outside the recovered `%d`
    // non-negative signed domain. Keep them visible, but never turn them into a
    // runtime mount attempt or manufacture a runtime-equivalent filename.
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
