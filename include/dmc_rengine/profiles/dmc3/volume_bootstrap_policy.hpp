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
    // The recovered runtime registers the physical data root first. Numbered
    // archive mounts are then prepended to the same mount list.
    bool physical_root_registered_before_archives{true};
    bool mount_list_is_prepend{true};

    // First numbered DMC3-N.nbz that is absent. Original probing stops here.
    std::uint32_t first_missing_index{};

    // Registration order: 0,1,2,... up to but excluding first_missing_index.
    std::vector<RuntimeArchiveVolume> registered_archives;

    // Effective lookup order after prepend mounting: highest contiguous N -> 0.
    std::vector<std::uint32_t> archive_resolution_order;

    // Product discovery may observe files after the first gap. They are kept
    // only as diagnostics/navigation data and are not runtime-equivalent mounts.
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

    // `present_indices` is product-side discovery input. Duplicate/unsorted
    // indices are accepted; the returned plan canonicalizes them before
    // reproducing the recovered contiguous first-gap semantics.
    [[nodiscard]] static VolumeBootstrapPlan plan(
        std::span<const std::uint32_t> present_indices);
};

} // namespace dmc::rengine::profiles::dmc3
