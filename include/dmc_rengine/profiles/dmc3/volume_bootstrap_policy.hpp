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
    std::size_t resolution_priority{};

    [[nodiscard]] bool valid() const noexcept;
};

struct VolumeBootstrapPlan final {
    // The original runtime registers the physical data root first. Archive
    // mounts are then prepended one by one to the same VFS mount list.
    bool physical_root_registered_before_archives{true};
    bool mount_list_is_prepend{true};

    // First index for which DMC3-%d.nbz was not present. The game stops probing
    // at this gap and does not inspect later numbered files.
    std::uint32_t first_missing_index{};

    // Runtime registration order is ascending: 0, 1, 2, ... until the gap.
    std::vector<RuntimeArchiveVolume> registered_archives;

    // Effective archive lookup order is the reverse because each mount is
    // inserted at the list head.
    std::vector<std::uint32_t> archive_resolution_order;

    // Files discovered by the product after the first runtime gap. They may be
    // shown diagnostically/navigation-wise but are not runtime-equivalent mounts.
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

    // `present_indices` is a product-side discovery input. The returned plan
    // intentionally reproduces the executable's contiguous probe semantics
    // instead of mounting every discovered DMC3-*.nbz file.
    [[nodiscard]] static VolumeBootstrapPlan plan(
        std::span<const std::uint32_t> present_indices);
};

} // namespace dmc::rengine::profiles::dmc3
