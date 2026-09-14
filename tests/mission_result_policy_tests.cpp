#include "dmc_rengine/save/mission_result_policy.hpp"

#include <cassert>
#include <cstddef>

using namespace dmc::rengine::save::mission_result;

int main() {
    static_assert(category_count == 5U);
    static_assert(thresholds_per_category == 4U);
    static_assert(policy_record_size == 0x84U);
    static_assert(policy_group_stride == 0x298U);

    static_assert(category_thresholds_offset == 0x00U);
    static_assert(category_thresholds_size == 0x50U);
    static_assert(aggregate_values_offset == 0x50U);
    static_assert(aggregate_values_size == 0x10U);
    static_assert(raw_values_060_offset == 0x60U);
    static_assert(raw_values_060_size == 0x14U);
    static_assert(unknown_tail_offset == 0x74U);
    static_assert(unknown_tail_size == 0x10U);

    MissionResultPolicyRecordLayout record{};
    MissionResultPolicyGroupLayout group{};

    assert(sizeof(record) == policy_record_size);
    assert(sizeof(group) == policy_group_stride);

    const auto* record_base = reinterpret_cast<const std::byte*>(&record);
    const auto* aggregate =
        reinterpret_cast<const std::byte*>(&record.aggregate_values);
    const auto* raw060 = reinterpret_cast<const std::byte*>(&record.raw_values_060);
    const auto* tail = reinterpret_cast<const std::byte*>(&record.unknown_tail);

    assert(static_cast<std::size_t>(aggregate - record_base) == 0x50U);
    assert(static_cast<std::size_t>(raw060 - record_base) == 0x60U);
    assert(static_cast<std::size_t>(tail - record_base) == 0x74U);

    const auto* group_base = reinterpret_cast<const std::byte*>(&group);
    const auto* group_tail = reinterpret_cast<const std::byte*>(&group.raw_group_tail);
    assert(static_cast<std::size_t>(group_tail - group_base) == 0x294U);

    return 0;
}
