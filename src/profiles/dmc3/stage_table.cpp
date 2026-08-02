#include "dmc_rengine/profiles/dmc3/stage_table.hpp"

#include "dmc_rengine/profiles/dmc3/known_targets.hpp"

#include <limits>

namespace dmc::rengine::profiles::dmc3 {

bool StageResourceTableDescriptor::valid() const noexcept {
    if (id.empty() || evidence_packet_id.empty() ||
        artifact_sha256.size() != 64U || row_count == 0U) {
        return false;
    }

    const auto& target = phase12_canonical_target();
    return target.matches_hash(artifact_sha256) &&
        target.image_base <= va &&
        va - target.image_base == rva &&
        entry_count() == 440U;
}

std::uint32_t StageResourceTableDescriptor::entry_count() const noexcept {
    constexpr auto column_count = 4U;
    if (row_count > std::numeric_limits<std::uint32_t>::max() / column_count) {
        return 0U;
    }
    return row_count * column_count;
}

std::optional<StageResourceRole>
StageResourceTableDescriptor::role_for_column(
    std::size_t column) const noexcept {
    if (column >= columns.size()) {
        return std::nullopt;
    }
    return columns[column];
}

const StageResourceTableDescriptor&
phase12_stage_resource_table() noexcept {
    static const StageResourceTableDescriptor descriptor{
        .id = "dmc3-hdc-phase12-stage-resource-table",
        .evidence_packet_id = "dmc3-hdc-phase12-canonical-target",
        .artifact_sha256 =
            "e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082",
        .file_offset = 0x005C30A8ULL,
        .rva = 0x005C4AA8U,
        .va = 0x1405C4AA8ULL,
        .row_count = 110U,
        .columns = {
            StageResourceRole::script,
            StageResourceRole::room_config,
            StageResourceRole::room_effects,
            StageResourceRole::room_sound,
        },
    };
    return descriptor;
}

} // namespace dmc::rengine::profiles::dmc3
