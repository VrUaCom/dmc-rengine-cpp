#pragma once

#include <cstdint>
#include <optional>

namespace dmc::recovered::dmc3::runtime::stage {

inline constexpr std::uint64_t stage_group_base_table_va = 0x1405C4A50ULL;
inline constexpr std::uint32_t stage_group_count = 10U;
inline constexpr std::uint32_t stage_descriptor_stride = 0x40U;

struct StageDescriptorBank final {
    std::uint64_t base_va{};
    std::uint32_t row_count{};
};

inline constexpr StageDescriptorBank stage_bank_a{
    .base_va = 0x1405C4AA0ULL,
    .row_count = 110U,
};

inline constexpr StageDescriptorBank stage_bank_b{
    .base_va = 0x1405C3080ULL,
    .row_count = 79U,
};

class IReadOnlyAddressSpace {
public:
    virtual ~IReadOnlyAddressSpace() = default;
    [[nodiscard]] virtual std::optional<std::uint64_t> read_u64(
        std::uint64_t virtual_address) const = 0;
};

enum class StageDescriptorBankId {
    bank_a,
    bank_b,
};

enum class NumericStageResolutionStatus {
    resolved,
    group_out_of_range,
    unreadable_group_base,
    null_group_base,
    unreadable_selector_slot,
    null_descriptor,
    descriptor_outside_recovered_banks,
};

struct NumericStageResolution final {
    NumericStageResolutionStatus status{
        NumericStageResolutionStatus::group_out_of_range};
    std::uint16_t requested_stage_id{};
    std::uint32_t group_index{};
    std::uint32_t remainder{};
    std::uint64_t selector_base_va{};
    std::uint64_t selector_slot_va{};
    std::uint64_t descriptor_va{};
    std::optional<StageDescriptorBankId> bank;
    std::optional<std::uint32_t> source_row;

    [[nodiscard]] bool resolved() const noexcept {
        return status == NumericStageResolutionStatus::resolved &&
            bank.has_value() && source_row.has_value();
    }
};

[[nodiscard]] inline std::optional<std::uint32_t> descriptor_row_in_bank(
    std::uint64_t descriptor_va,
    StageDescriptorBank bank) noexcept {
    if (descriptor_va < bank.base_va) {
        return std::nullopt;
    }

    const auto delta = descriptor_va - bank.base_va;
    if (delta % stage_descriptor_stride != 0U) {
        return std::nullopt;
    }

    const auto row64 = delta / stage_descriptor_stride;
    if (row64 >= bank.row_count) {
        return std::nullopt;
    }
    return static_cast<std::uint32_t>(row64);
}

// Direct reconstruction of the canonical code path around 0x1401B985D:
//   group = stageId / 100
//   remainder = stageId % 100
//   selectorBase = groupBase[group]
//   descriptor = selectorBase[remainder]
//
// The result identifies only the recovered descriptor bank/row. It deliberately
// assigns no gameplay/mission/room semantics to that identity.
class NumericStageResolver final {
public:
    [[nodiscard]] static NumericStageResolution resolve(
        std::uint16_t stage_id,
        const IReadOnlyAddressSpace& address_space) {
        NumericStageResolution result{
            .status = NumericStageResolutionStatus::group_out_of_range,
            .requested_stage_id = stage_id,
            .group_index = static_cast<std::uint32_t>(stage_id / 100U),
            .remainder = static_cast<std::uint32_t>(stage_id % 100U),
        };

        if (result.group_index >= stage_group_count) {
            return result;
        }

        const auto group_entry_va = stage_group_base_table_va +
            static_cast<std::uint64_t>(result.group_index) * 8ULL;
        const auto selector_base = address_space.read_u64(group_entry_va);
        if (!selector_base.has_value()) {
            result.status = NumericStageResolutionStatus::unreadable_group_base;
            return result;
        }
        if (*selector_base == 0U) {
            result.status = NumericStageResolutionStatus::null_group_base;
            return result;
        }

        result.selector_base_va = *selector_base;
        result.selector_slot_va = result.selector_base_va +
            static_cast<std::uint64_t>(result.remainder) * 8ULL;

        const auto descriptor = address_space.read_u64(result.selector_slot_va);
        if (!descriptor.has_value()) {
            result.status = NumericStageResolutionStatus::unreadable_selector_slot;
            return result;
        }
        if (*descriptor == 0U) {
            result.status = NumericStageResolutionStatus::null_descriptor;
            return result;
        }
        result.descriptor_va = *descriptor;

        if (const auto row = descriptor_row_in_bank(result.descriptor_va, stage_bank_a);
            row.has_value()) {
            result.bank = StageDescriptorBankId::bank_a;
            result.source_row = *row;
            result.status = NumericStageResolutionStatus::resolved;
            return result;
        }

        if (const auto row = descriptor_row_in_bank(result.descriptor_va, stage_bank_b);
            row.has_value()) {
            result.bank = StageDescriptorBankId::bank_b;
            result.source_row = *row;
            result.status = NumericStageResolutionStatus::resolved;
            return result;
        }

        result.status =
            NumericStageResolutionStatus::descriptor_outside_recovered_banks;
        return result;
    }
};

} // namespace dmc::recovered::dmc3::runtime::stage
