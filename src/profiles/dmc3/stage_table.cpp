#include "dmc_rengine/profiles/dmc3/stage_table.hpp"

#include "dmc_rengine/profiles/dmc3/known_targets.hpp"

#include <algorithm>
#include <limits>
#include <set>
#include <utility>
#include <vector>

namespace dmc::rengine::profiles::dmc3 {
namespace {

void append_stage_id_range(
    std::vector<std::uint16_t>& ids,
    std::uint16_t first,
    std::uint16_t last) {
    for (std::uint32_t value = first; value <= last; ++value) {
        ids.push_back(static_cast<std::uint16_t>(value));
    }
}

[[nodiscard]] std::vector<std::uint16_t> bank_a_stage_ids() {
    std::vector<std::uint16_t> ids;
    ids.reserve(110U);
    append_stage_id_range(ids, 0U, 12U);
    append_stage_id_range(ids, 100U, 146U);
    append_stage_id_range(ids, 200U, 241U);
    append_stage_id_range(ids, 300U, 307U);
    return ids;
}

[[nodiscard]] std::vector<std::uint16_t> bank_b_stage_ids() {
    std::vector<std::uint16_t> ids;
    ids.reserve(79U);
    append_stage_id_range(ids, 308U, 313U);
    append_stage_id_range(ids, 400U, 446U);
    append_stage_id_range(ids, 448U, 449U);
    append_stage_id_range(ids, 600U, 612U);
    append_stage_id_range(ids, 900U, 910U);
    return ids;
}

[[nodiscard]] constexpr std::array<StageResourceRole, 4> columns() noexcept {
    return {
        StageResourceRole::script,
        StageResourceRole::room_config,
        StageResourceRole::room_effects,
        StageResourceRole::room_sound,
    };
}

[[nodiscard]] bool columns_match_promoted_layout(
    const StageResourceTableDescriptor& descriptor) noexcept {
    return descriptor.columns == columns();
}

[[nodiscard]] bool matches_exact_wave2_bank(
    const StageResourceTableDescriptor& descriptor) noexcept {
    if (!descriptor.valid() ||
        descriptor.evidence_packet_id != "dmc3-stage-wave2") {
        return false;
    }

    if (descriptor.va == 0x1405C4AA0ULL) {
        return descriptor.id == "dmc3-hdc-phase12-stage-resource-table" &&
            descriptor.file_offset == 0x005C30A0ULL &&
            descriptor.rva == 0x005C4AA0U &&
            descriptor.row_count == 110U;
    }
    if (descriptor.va == 0x1405C3080ULL) {
        return descriptor.id == "dmc3-stage-descriptor-bank-b" &&
            descriptor.file_offset == 0x005C1680ULL &&
            descriptor.rva == 0x005C3080U &&
            descriptor.row_count == 79U;
    }
    return false;
}

} // namespace

bool StageResourceTableDescriptor::valid() const noexcept {
    if (id.empty() || evidence_packet_id.empty() ||
        artifact_sha256.size() != 64U || artifact_size == 0U ||
        row_count == 0U || cell_stride != 0x10U ||
        kind16_offset != 0U || path_pointer_offset != 0x08U ||
        !columns_match_promoted_layout(*this) || row_size_bytes() != 0x40ULL ||
        entry_count() == 0U || file_offset >= artifact_size) {
        return false;
    }

    if (table_size_bytes() == 0U ||
        table_size_bytes() > artifact_size - file_offset) {
        return false;
    }

    constexpr std::uint64_t image_base = 0x140000000ULL;
    return va >= image_base && va - image_base == rva;
}

bool StageResourceTableDescriptor::is_promoted_wave2_authority() const noexcept {
    if (!matches_exact_wave2_bank(*this)) {
        return false;
    }

    const auto& target = phase12_canonical_target();
    return artifact_size == target.size &&
        target.matches_hash(artifact_sha256) &&
        target.image_base <= va && va - target.image_base == rva;
}

std::uint32_t StageResourceTableDescriptor::entry_count() const noexcept {
    constexpr auto column_count = 4U;
    if (row_count > std::numeric_limits<std::uint32_t>::max() / column_count) {
        return 0U;
    }
    return row_count * column_count;
}

std::uint64_t StageResourceTableDescriptor::row_size_bytes() const noexcept {
    constexpr auto column_count = 4ULL;
    if (cell_stride == 0U ||
        static_cast<std::uint64_t>(cell_stride) >
            std::numeric_limits<std::uint64_t>::max() / column_count) {
        return 0U;
    }
    return static_cast<std::uint64_t>(cell_stride) * column_count;
}

std::uint64_t StageResourceTableDescriptor::table_size_bytes() const noexcept {
    if (cell_stride == 0U) {
        return 0U;
    }
    const auto count = static_cast<std::uint64_t>(entry_count());
    if (count == 0U ||
        count > std::numeric_limits<std::uint64_t>::max() / cell_stride) {
        return 0U;
    }
    return count * cell_stride;
}

std::optional<StageResourceRole>
StageResourceTableDescriptor::role_for_column(
    std::size_t column) const noexcept {
    if (column >= columns.size()) {
        return std::nullopt;
    }
    return columns[column];
}

bool StageDescriptorBankMetadata::valid() const noexcept {
    if (id.empty() || va == 0U || row_count == 0U ||
        numeric_stage_ids.size() != row_count) {
        return false;
    }
    std::set<std::uint16_t> unique_ids(
        numeric_stage_ids.begin(), numeric_stage_ids.end());
    return unique_ids.size() == numeric_stage_ids.size();
}

std::uint32_t StageDescriptorUniverseMetadata::observed_descriptor_count() const noexcept {
    std::uint32_t count{};
    for (const auto& bank : banks) {
        if (bank.row_count > std::numeric_limits<std::uint32_t>::max() - count) {
            return 0U;
        }
        count += bank.row_count;
    }
    return count;
}

bool StageDescriptorUniverseMetadata::valid() const noexcept {
    if (evidence_packet_id != "dmc3-stage-wave2" ||
        selector_table_va != 0x1405C4440ULL || selector_entry_count != 193U ||
        group_base_table_va != 0x1405C4A50ULL || group_base_count != 10U ||
        observed_descriptor_count() != 189U ||
        !std::all_of(
            banks.begin(), banks.end(),
            [](const StageDescriptorBankMetadata& bank) { return bank.valid(); })) {
        return false;
    }

    std::set<std::uint16_t> all_ids;
    for (const auto& bank : banks) {
        all_ids.insert(bank.numeric_stage_ids.begin(), bank.numeric_stage_ids.end());
    }
    return all_ids.size() == observed_descriptor_count();
}

const StageResourceTableDescriptor& wave2_stage_resource_bank_a() noexcept {
    static const StageResourceTableDescriptor descriptor{
        .id = "dmc3-hdc-phase12-stage-resource-table",
        .evidence_packet_id = "dmc3-stage-wave2",
        .artifact_sha256 =
            "e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082",
        .artifact_size = 6356432ULL,
        .file_offset = 0x005C30A0ULL,
        .rva = 0x005C4AA0U,
        .va = 0x1405C4AA0ULL,
        .row_count = 110U,
        .cell_stride = 0x10U,
        .kind16_offset = 0U,
        .path_pointer_offset = 0x08U,
        .columns = columns(),
    };
    return descriptor;
}

const StageResourceTableDescriptor& wave2_stage_resource_bank_b() noexcept {
    static const StageResourceTableDescriptor descriptor{
        .id = "dmc3-stage-descriptor-bank-b",
        .evidence_packet_id = "dmc3-stage-wave2",
        .artifact_sha256 =
            "e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082",
        .artifact_size = 6356432ULL,
        .file_offset = 0x005C1680ULL,
        .rva = 0x005C3080U,
        .va = 0x1405C3080ULL,
        .row_count = 79U,
        .cell_stride = 0x10U,
        .kind16_offset = 0U,
        .path_pointer_offset = 0x08U,
        .columns = columns(),
    };
    return descriptor;
}

const std::array<StageResourceTableDescriptor, 2>&
wave2_stage_resource_banks() noexcept {
    static const std::array<StageResourceTableDescriptor, 2> banks{
        wave2_stage_resource_bank_a(),
        wave2_stage_resource_bank_b(),
    };
    return banks;
}

const StageResourceTableDescriptor& phase12_stage_resource_table() noexcept {
    return wave2_stage_resource_bank_a();
}

const StageDescriptorUniverseMetadata& wave2_stage_descriptor_universe() noexcept {
    static const StageDescriptorUniverseMetadata metadata{
        .evidence_packet_id = "dmc3-stage-wave2",
        .banks = {
            StageDescriptorBankMetadata{
                .id = "dmc3-stage-descriptor-bank-a",
                .va = 0x1405C4AA0ULL,
                .row_count = 110U,
                .numeric_stage_ids = bank_a_stage_ids(),
            },
            StageDescriptorBankMetadata{
                .id = "dmc3-stage-descriptor-bank-b",
                .va = 0x1405C3080ULL,
                .row_count = 79U,
                .numeric_stage_ids = bank_b_stage_ids(),
            },
        },
        .selector_table_va = 0x1405C4440ULL,
        .selector_entry_count = 193U,
        .group_base_table_va = 0x1405C4A50ULL,
        .group_base_count = 10U,
    };
    return metadata;
}

std::optional<std::uint16_t> runtime_stage_id_for_table_row(
    const StageResourceTableDescriptor& descriptor,
    std::uint32_t row_index) noexcept {
    if (!descriptor.is_promoted_wave2_authority() ||
        row_index >= descriptor.row_count) {
        return std::nullopt;
    }
    const auto& universe = wave2_stage_descriptor_universe();
    if (!universe.valid()) {
        return std::nullopt;
    }
    const auto bank_index = descriptor.va == universe.banks[0].va ? 0U :
        (descriptor.va == universe.banks[1].va ? 1U : 2U);
    if (bank_index >= universe.banks.size() ||
        row_index >= universe.banks[bank_index].numeric_stage_ids.size()) {
        return std::nullopt;
    }
    return universe.banks[bank_index].numeric_stage_ids[row_index];
}

} // namespace dmc::rengine::profiles::dmc3
