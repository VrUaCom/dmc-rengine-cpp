#pragma once

#include <array>
#include <cstddef>
#include <cstdint>

namespace dmc::recovered::dmc3::runtime::stage {

enum class StageResourceRole : std::uint32_t {
    script = 0U,
    config = 1U,
    effect = 2U,
    sound = 3U,
};

// Corrected executable ABI. The old 0x1405C4AA8 authority is the first path
// field; the actual first Bank-A descriptor begins at 0x1405C4AA0.
struct StageResourceCell final {
    std::uint16_t kind16{};                       // +0x00
    std::array<std::byte, 0x06> unresolved_02_07{}; // +0x02..+0x07
    std::uintptr_t path_pointer{};                // +0x08
};

static_assert(sizeof(std::uintptr_t) == 8U);
static_assert(sizeof(StageResourceCell) == 0x10U);
static_assert(offsetof(StageResourceCell, kind16) == 0x00U);
static_assert(offsetof(StageResourceCell, path_pointer) == 0x08U);

struct StageResourceDescriptor final {
    std::array<StageResourceCell, 4> cells{};

    [[nodiscard]] StageResourceCell& cell(StageResourceRole role) noexcept {
        return cells[static_cast<std::size_t>(role)];
    }

    [[nodiscard]] const StageResourceCell& cell(
        StageResourceRole role) const noexcept {
        return cells[static_cast<std::size_t>(role)];
    }
};

static_assert(sizeof(StageResourceDescriptor) == 0x40U);
static_assert(offsetof(StageResourceDescriptor, cells) == 0x00U);

} // namespace dmc::recovered::dmc3::runtime::stage
