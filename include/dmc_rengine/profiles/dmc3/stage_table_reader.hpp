#pragma once

#include "dmc_rengine/exe/pe_image.hpp"
#include "dmc_rengine/gdspaces/diagnostic.hpp"
#include "dmc_rengine/profiles/dmc3/stage_table.hpp"

#include <array>
#include <cstddef>
#include <cstdint>
#include <span>
#include <string>
#include <vector>

namespace dmc::rengine::profiles::dmc3 {

struct StageResourceTableCellObservation final {
    std::uint32_t row_index{};
    std::uint32_t column_index{};
    StageResourceRole role{StageResourceRole::script};
    std::uint64_t cell_file_offset{};
    std::uint32_t cell_rva{};
    std::uint64_t cell_va{};
    std::uint64_t path_pointer_va{};
    std::uint64_t path_file_offset{};
    std::string logical_path;

    [[nodiscard]] bool valid() const noexcept {
        return !logical_path.empty() && path_pointer_va != 0U;
    }
};

struct StageResourceTableRowObservation final {
    std::uint32_t row_index{};
    std::array<StageResourceTableCellObservation, 4> cells{};

    [[nodiscard]] bool complete() const noexcept;
    [[nodiscard]] std::array<std::string, 4> logical_paths() const;
};

struct StageResourceTableReadResult final {
    std::vector<StageResourceTableRowObservation> rows;
    std::vector<gdspaces::Diagnostic> diagnostics;

    [[nodiscard]] bool complete(
        const StageResourceTableDescriptor& descriptor) const noexcept;
};

class StageResourceTableReader final {
public:
    [[nodiscard]] static StageResourceTableReadResult read(
        std::span<const std::byte> executable_bytes,
        const exe::PeImage& image,
        const StageResourceTableDescriptor& descriptor,
        std::size_t max_path_bytes = 260U);
};

} // namespace dmc::rengine::profiles::dmc3
