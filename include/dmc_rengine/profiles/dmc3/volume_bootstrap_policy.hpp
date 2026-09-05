#pragma once

#include <cstddef>
#include <cstdint>
#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <vector>

namespace dmc::rengine::profiles::dmc3 {

// One numbered archive filename proven present during the recovered ascending
// bootstrap discovery loop. Presence means the mount helper was attempted; it
// does not mean that archive initialization succeeded or that a type-1 node was
// linked into the runtime mount list.
struct DiscoveredArchiveVolume final {
    std::uint32_t index{};
    std::string filename;
    std::size_t discovery_order{};

    [[nodiscard]] bool valid() const noexcept;
};

// One archive whose registration helper is known to have succeeded. Successful
// ascending registrations prepend their type-1 nodes, so resolution rank is the
// reverse of successful registration order, not the reverse of filename
// discovery count.
struct RuntimeArchiveVolume final {
    std::uint32_t index{};
    std::string filename;
    std::size_t successful_registration_order{};
    std::size_t resolution_rank{};

    [[nodiscard]] bool valid() const noexcept;
};

// Discovery-only bootstrap evidence. The canonical bootstrap at 0x14002E930
// checks DMC3-N filenames in ascending order and stops at the first missing
// filename. It does not consume the return value of either physical registration
// or archive registration, so this type deliberately carries no successful-mount
// claim.
struct VolumeBootstrapPlan final {
    bool physical_registration_attempted_before_archives{true};
    bool archive_discovery_stops_at_first_missing{true};
    std::uint32_t first_missing_index{};
    std::vector<DiscoveredArchiveVolume> discovered_archives;

    // Product discovery can preserve in-domain files after the first runtime
    // gap as diagnostics/navigation evidence without promoting them as mount
    // attempts or successful mounts.
    std::vector<std::uint32_t> present_after_first_gap;

    // `%d` in the recovered filename formatter is a signed-decimal domain.
    // Product discovery may observe larger numeric suffixes, but they are not
    // runtime-equivalent numbered-volume indices and stay diagnostic-only.
    std::vector<std::uint32_t> present_outside_runtime_index_domain;

    [[nodiscard]] bool valid() const noexcept;
};

// Runtime topology contains only nodes whose recovered registration helpers are
// known to have completed successfully. It is bound to the discovery stop that
// produced the mount attempts, but a discovered archive may be absent here when
// its registration failed. Likewise the physical type-0 node may be absent.
struct RuntimeMountTopology final {
    bool mount_list_is_prepend{true};
    bool physical_root_mounted{};
    std::uint32_t discovery_first_missing_index{};
    std::vector<RuntimeArchiveVolume> mounted_archives;
    std::vector<std::uint32_t> archive_resolution_order;

    [[nodiscard]] bool valid() const noexcept;
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

    // present_indices is product-side filename discovery input only. The result
    // reproduces the recovered ascending first-gap-stop discovery loop without
    // laundering mount attempts into successful registrations.
    [[nodiscard]] static VolumeBootstrapPlan plan(
        std::span<const std::uint32_t> present_indices);

    // Builds the runtime linked-list topology from explicit registration
    // outcomes. successful_archive_indices are a set of archive registrations
    // known to have succeeded during this discovery run. They may be sparse.
    // Duplicates, undiscovered indices, or any index at/after the first missing
    // filename fail closed. Returned registration order is recovered ascending
    // attempt order; resolution order is the reverse successful-prepend order.
    [[nodiscard]] static std::optional<RuntimeMountTopology> successful_mount_topology(
        const VolumeBootstrapPlan& discovery,
        bool physical_root_mounted,
        std::span<const std::uint32_t> successful_archive_indices);
};

} // namespace dmc::rengine::profiles::dmc3
