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

    // Product discovery can preserve files after the first runtime gap as
    // diagnostics/navigation evidence without promoting them as runtime mounts.
    std::vector<std::uint32_t> present_after_first_gap;

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

    [[nodiscard]] static std::string volume_filename(std::uint32_t index);

    // present_indices is product-side discovery input. The result reproduces
    // contiguous first-gap-stop mount semantics instead of mounting every
    // discovered DMC3-*.nbz file.
    [[nodiscard]] static VolumeBootstrapPlan plan(
        std::span<const std::uint32_t> present_indices);
};

} // namespace dmc::rengine::profiles::dmc3
