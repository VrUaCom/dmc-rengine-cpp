#include "dmc_rengine/profiles/dmc3/volume_bootstrap_policy.hpp"

#include <array>
#include <cassert>
#include <cstdint>
#include <span>
#include <string>
#include <vector>

int main() {
    using dmc::rengine::profiles::dmc3::VolumeBootstrapPolicy;

    assert(VolumeBootstrapPolicy::data_subdirectory() == "data/dmc3");
    assert(VolumeBootstrapPolicy::executable_data_suffix() == "\\data\\dmc3\\");
    assert(VolumeBootstrapPolicy::volume_format() == "%sDMC3-%d.nbz");
    assert(VolumeBootstrapPolicy::volume_filename(0U) == "DMC3-0.nbz");
    assert(VolumeBootstrapPolicy::volume_filename(12U) == "DMC3-12.nbz");

    const auto empty = VolumeBootstrapPolicy::plan(
        std::span<const std::uint32_t>{});
    assert(empty.valid());
    assert(empty.first_missing_index == 0U);
    assert(empty.registered_archives.empty());
    assert(empty.archive_resolution_order.empty());

    constexpr std::array<std::uint32_t, 4> contiguous_input{2U, 0U, 1U, 1U};
    const auto contiguous = VolumeBootstrapPolicy::plan(contiguous_input);
    assert(contiguous.valid());
    assert(contiguous.first_missing_index == 3U);
    assert(contiguous.registered_archives.size() == 3U);
    assert(contiguous.registered_archives[0].resolution_rank == 2U);
    assert(contiguous.registered_archives[1].resolution_rank == 1U);
    assert(contiguous.registered_archives[2].resolution_rank == 0U);
    assert((contiguous.archive_resolution_order ==
        std::vector<std::uint32_t>{2U, 1U, 0U}));

    auto wrong_filename = contiguous;
    wrong_filename.registered_archives[1].filename = "DMC3-9.nbz";
    assert(!wrong_filename.valid());

    constexpr std::array<std::uint32_t, 4> gap_input{0U, 2U, 3U, 7U};
    const auto gap = VolumeBootstrapPolicy::plan(gap_input);
    assert(gap.valid());
    assert(gap.first_missing_index == 1U);
    assert(gap.registered_archives.size() == 1U);
    assert((gap.archive_resolution_order == std::vector<std::uint32_t>{0U}));
    assert((gap.present_after_first_gap ==
        std::vector<std::uint32_t>{2U, 3U, 7U}));

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
    assert(missing_zero.registered_archives.empty());
    assert((missing_zero.present_after_first_gap ==
        std::vector<std::uint32_t>{1U, 2U, 9U}));

    constexpr std::array<std::uint32_t, 5> later_gap_input{4U, 1U, 0U, 5U, 9U};
    const auto later_gap = VolumeBootstrapPolicy::plan(later_gap_input);
    assert(later_gap.valid());
    assert(later_gap.first_missing_index == 2U);
    assert((later_gap.archive_resolution_order ==
        std::vector<std::uint32_t>{1U, 0U}));
    assert((later_gap.present_after_first_gap ==
        std::vector<std::uint32_t>{4U, 5U, 9U}));

    return 0;
}
