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
    // Production/research gate for the canonical executable. SHA-256
    // verification and PE parsing are performed from the same byte span before
    // executable VAs are interpreted.
    [[nodiscard]] static NumericStageResolution resolve_canonical(
        std::span<const std::byte> executable_bytes,
        std::uint16_t stage_id);

    // Low-level structural entry point for synthetic fixtures and bounded
    // reverse experiments. The caller must guarantee that `image` was parsed
    // from the exact `executable_bytes` span. Product paths must use
    // resolve_canonical().
    [[nodiscard]] static NumericStageResolution resolve_from_image(
        std::span<const std::byte> executable_bytes,
        const exe::PeImage& image,
        std::uint16_t stage_id);
};

} // namespace dmc::rengine::profiles::dmc3
