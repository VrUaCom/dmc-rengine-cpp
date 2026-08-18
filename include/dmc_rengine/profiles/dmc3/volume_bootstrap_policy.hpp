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
    // Direct executable evidence: physical root is registered before numbered
    // NBZ archives, and every subsequent mount node is prepended.
    bool physical_root_registered_before_archives{true};
    bool mount_list_is_prepend{true};

    // First DMC3-N.nbz index absent from the contiguous runtime probe. The
    // original bootstrap stops here and does not mount later numbered files.
    std::uint32_t first_missing_index{};

    // Original registration order: 0, 1, 2, ... until first_missing_index.
    std::vector<RuntimeArchiveVolume> registered_archives;

    // Effective archive lookup order after prepend insertion: highest
    // contiguous volume down to zero.
    std::vector<std::uint32_t> archive_resolution_order;

    // Product discovery may see numbered files after the first gap. They are
    // retained diagnostically but are not represented as runtime-equivalent
    // mounts by this plan.
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

    // present_indices is product-side discovery input. The result deliberately
    // reproduces the executable's contiguous first-gap-stop mount semantics
    // rather than mounting every discovered DMC3-*.nbz file.
    [[nodiscard]] static VolumeBootstrapPlan plan(
        std::span<const std::uint32_t> present_indices);
};

} // namespace dmc::rengine::profiles::dmc3
