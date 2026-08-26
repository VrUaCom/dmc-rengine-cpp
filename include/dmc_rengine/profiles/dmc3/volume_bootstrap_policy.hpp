#pragma once

#include <cstddef>
#include <cstdint>
#include <span>
#include <string>
#include <string_view>
#include <vector>

namespace dmc::rengine::profiles::dmc3 {

// One numbered filename that the original bootstrap discovers before the first
// missing DMC3-N.nbz. Discovery proves that registration is attempted; it does
// not prove that RegisterArchiveMount succeeded or inserted a mount node.
struct RuntimeArchiveMountAttempt final {
    std::uint32_t index{};
    std::string filename;
    std::size_t attempt_order{};

    [[nodiscard]] bool valid() const noexcept;
};

// One archive whose registration outcome is explicitly known to have succeeded.
// Zero resolution_rank means the archive is consulted first among successful
// archive mounts. Successful ascending registrations prepend to the original
// mount list, so higher successful indices have higher precedence.
struct RuntimeArchiveVolume final {
    std::uint32_t index{};
    std::string filename;
    std::size_t registration_order{};
    std::size_t resolution_rank{};

    [[nodiscard]] bool valid() const noexcept;
};

// Filename-discovery/attempt plan only. No field in this type claims that the
// physical or archive registration attempts succeeded.
struct VolumeBootstrapPlan final {
    bool physical_root_registration_attempt_precedes_archives{true};
    bool mount_list_is_prepend{true};
    std::uint32_t first_missing_index{};
    std::vector<RuntimeArchiveMountAttempt> discovered_archives;

    // Product discovery can preserve in-domain files after the first runtime
    // gap as diagnostics/navigation evidence without promoting them as mounts.
    std::vector<std::uint32_t> present_after_first_gap;

    // `%d` in the recovered filename formatter is a signed-decimal domain.
    // Product discovery may observe larger numeric suffixes, but they are not
    // runtime-equivalent numbered-volume indices and stay diagnostic-only.
    std::vector<std::uint32_t> present_outside_runtime_index_domain;

    [[nodiscard]] bool valid() const noexcept;
    [[nodiscard]] bool discovered(std::uint32_t index) const noexcept;
};

// Actual successful runtime topology. This type can only be derived from an
// explicit physical registration outcome plus explicit successful archive
// registration outcomes. It never infers success from filename discovery.
struct RuntimeMountTopology final {
    bool physical_root_mounted{};
    bool mount_list_is_prepend{true};
    std::vector<RuntimeArchiveVolume> mounted_archives;
    std::vector<std::uint32_t> archive_resolution_order;

    [[nodiscard]] bool valid_for(const VolumeBootstrapPlan& discovery) const noexcept;
};

class VolumeBootstrapPolicy final {
public:
    [[nodiscard]] static constexpr std::string_view data_subdirectory() noexcept {
        return "data/dmc3";
    }

    [[nodiscard]] static constexpr std::string_view executable_data_suffix() noexcept {
        return "\\data\\dmc3\\";
    }

    [[nodiscard]] static constexpr std::string_view volume_format() noexcept {
        return "%sDMC3-%d.nbz";
    }

    [[nodiscard]] static constexpr std::uint32_t runtime_index_max() noexcept {
        return 0x7FFFFFFFU;
    }

    [[nodiscard]] static constexpr bool runtime_index_valid(
        std::uint32_t index) noexcept {
        return index <= runtime_index_max();
    }

    // Returns an empty string for a product-discovered numeric suffix outside
    // the recovered non-negative signed `%d` runtime namespace.
    [[nodiscard]] static std::string volume_filename(std::uint32_t index);

    // present_indices is product-side filename discovery input. The returned
    // plan stops discovery at the first missing numbered filename and records
    // every registration attempt before that gap, without claiming success.
    [[nodiscard]] static VolumeBootstrapPlan plan(
        std::span<const std::uint32_t> present_indices);

    // Build actual resolver topology only from explicit registration outcomes.
    // Every successful archive index must have been discovered pre-gap. Order
    // of successful_archive_indices is irrelevant; original precedence is
    // derived from successful ascending registration attempts + prepend.
    [[nodiscard]] static RuntimeMountTopology mount_topology(
        const VolumeBootstrapPlan& discovery,
        bool physical_root_mounted,
        std::span<const std::uint32_t> successful_archive_indices);
};

} // namespace dmc::rengine::profiles::dmc3
