#pragma once

#include "dmc_rengine/exe/pe_image.hpp"
#include "dmc_rengine/gdspaces/diagnostic.hpp"

#include <cstddef>
#include <cstdint>
#include <optional>
#include <span>
#include <string>
#include <vector>

namespace dmc::rengine::profiles::dmc3 {

struct NumericStageResolution final {
    std::uint16_t requested_stage_id{};
    std::uint32_t group_index{};
    std::uint32_t remainder{};
    std::uint64_t selector_base_va{};
    std::uint64_t selector_slot_va{};
    std::uint64_t descriptor_va{};
    std::string source_table_id;
    std::uint32_t source_row_index{};
    std::optional<std::uint16_t> descriptor_primary_stage_id;
    std::vector<gdspaces::Diagnostic> diagnostics;

    [[nodiscard]] bool resolved() const noexcept;
    [[nodiscard]] bool aliases_another_primary_stage() const noexcept {
        return resolved() && descriptor_primary_stage_id.has_value() &&
            *descriptor_primary_stage_id != requested_stage_id;
    }
};

class StageNumericResolver final {
public:
    // Canonical authority path. SHA-256 verification and PE parsing are derived
    // from the same byte span before recovered executable VAs are interpreted.
    [[nodiscard]] static NumericStageResolution resolve_canonical(
        std::span<const std::byte> executable_bytes,
        std::uint16_t stage_id);

    // Low-level structural path for synthetic fixtures and bounded reverse
    // experiments. Caller guarantees image/byte coherence. Product paths must
    // use resolve_canonical().
    [[nodiscard]] static NumericStageResolution resolve_from_image(
        std::span<const std::byte> executable_bytes,
        const exe::PeImage& image,
        std::uint16_t stage_id);
};

} // namespace dmc::rengine::profiles::dmc3
