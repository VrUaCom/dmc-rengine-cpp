#pragma once

#include <cstddef>
#include <cstdint>
#include <span>
#include <string>
#include <string_view>
#include <vector>

namespace dmc::rengine::profiles::dmc3 {

// One numbered filename reached by the original first-gap discovery loop.
// Discovery proves that the path existed when checked; it does not prove that
// archive registration/open/index initialization succeeded.
struct DiscoveredArchiveVolume final {
    std::uint32_t index{};
    std::string filename;
    std::size_t discovery_order{};

    [[nodiscard]] bool valid() const noexcept;
};

// One archive node that actually entered the runtime mount list after a
// successful registration attempt. The vector owning these values is kept in
// successful registration order (ascending bootstrap discovery order).
struct MountedArchiveVolume final {
    std::uint32_t index{};
    std::string filename;
    std::size_t registration_order{};

    // Zero means this successful archive is consulted first. Because every
    // successful registration prepends, later/higher successful indices have
    // lower resolution ranks.
    std::size_t resolution_rank{};

    [[nodiscard]] bool valid() const noexcept;
};

// Product-side reconstruction of the original bootstrap filename-discovery
// surface. It records attempts/order only; it deliberately does not claim that
// the physical root or any discovered archive mounted successfully.
struct VolumeBootstrapPlan final {
    bool physical_registration_attempted_before_archives{true};
    bool mount_list_is_prepend{true};
    std::uint32_t first_missing_index{};
    std::vector<DiscoveredArchiveVolume> discovered_archives;

    // Product discovery can preserve in-domain files after the first runtime
    // gap as diagnostics/navigation evidence without promoting them into the
    // original bootstrap discovery range.
    std::vector<std::uint32_t> present_after_first_gap;

    // `%d` in the recovered filename formatter is a signed-decimal domain.
    // Product discovery may observe larger numeric suffixes, but they are not
    // runtime-equivalent numbered-volume indices and stay diagnostic-only.
    std::vector<std::uint32_t> present_outside_runtime_index_domain;

    [[nodiscard]] bool valid() const noexcept;
};

// Effective successful runtime mount topology. This is a separate authority
// surface from filename discovery because both physical and archive registration
// helpers return failure without list insertion and bootstrap ignores those
// return values.
struct RuntimeMountTopology final {
    bool physical_registration_attempted_before_archives{true};
    bool physical_mount_succeeded{false};
    bool mount_list_is_prepend{true};
    std::vector<MountedArchiveVolume> mounted_archives;
    std::vector<std::uint32_t> archive_resolution_order;

    [[nodiscard]] bool structurally_valid() const noexcept;
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

    // present_indices is product-side filesystem discovery input. The result
    // reproduces only the original first-gap filename discovery range. It does
    // not infer successful mounts from path presence.
    [[nodiscard]] static VolumeBootstrapPlan plan(
        std::span<const std::uint32_t> present_indices);

    // Build one explicit successful topology from separately known registration
    // outcomes. successful_archive_indices must be unique, ascending and drawn
    // from discovery.discovered_archives. Invalid outcome input returns an
    // invalid/default topology rather than manufacturing success.
    [[nodiscard]] static RuntimeMountTopology mount_topology(
        const VolumeBootstrapPlan& discovery,
        bool physical_mount_succeeded,
        std::span<const std::uint32_t> successful_archive_indices);

    // Product/test convenience for the explicit clean-path assumption that the
    // physical registration and every discovered archive registration succeeded.
    // The name intentionally exposes that this is an assumption/result builder,
    // not evidence inferred from filename discovery.
    [[nodiscard]] static RuntimeMountTopology all_success_topology(
        const VolumeBootstrapPlan& discovery);
};

} // namespace dmc::rengine::profiles::dmc3
