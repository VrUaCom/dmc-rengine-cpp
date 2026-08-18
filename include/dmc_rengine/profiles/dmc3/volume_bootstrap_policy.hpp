#pragma once

#include <cstddef>
#include <cstdint>
#include <span>
#include <string>
#include <string_view>
#include <vector>

namespace dmc::rengine::profiles::dmc3 {

struct RuntimeArchiveVolume final {
    std::uint32_t index{};
    std::string filename;
    std::size_t registration_order{};

    // Zero means this archive is consulted first among the contiguous archive
    // mounts. Because the original mount list prepends each ascending
    // registration, the highest contiguous volume receives rank zero.
    std::size_t resolution_rank{};

    [[nodiscard]] bool valid() const noexcept;
};

struct VolumeBootstrapPlan final {
    bool physical_root_registered_before_archives{true};
    bool mount_list_is_prepend{true};
    std::uint32_t first_missing_index{};
    std::vector<RuntimeArchiveVolume> registered_archives;
    std::vector<std::uint32_t> archive_resolution_order;

    // Product discovery can preserve in-domain files after the first runtime
    // gap as diagnostics/navigation evidence without promoting them as mounts.
    std::vector<std::uint32_t> present_after_first_gap;

    // `%d` in the recovered filename formatter is a signed-decimal domain.
    // Product discovery may observe larger numeric suffixes, but they are not
    // runtime-equivalent numbered-volume indices and stay diagnostic-only.
    std::vector<std::uint32_t> present_outside_runtime_index_domain;

    [[nodiscard]] bool valid() const noexcept;
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

    // present_indices is product-side discovery input. The result reproduces
    // contiguous first-gap-stop mount semantics instead of mounting every
    // discovered DMC3-*.nbz file.
    [[nodiscard]] static VolumeBootstrapPlan plan(
        std::span<const std::uint32_t> present_indices);
};

} // namespace dmc::rengine::profiles::dmc3
