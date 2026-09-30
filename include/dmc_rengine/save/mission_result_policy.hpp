#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <type_traits>

namespace dmc::rengine::save::mission_result {

inline constexpr std::size_t category_count = 5U;
inline constexpr std::size_t thresholds_per_category = 4U;
inline constexpr std::size_t policy_record_size = 0x84U;
inline constexpr std::size_t policy_group_stride = 0x298U;

inline constexpr std::size_t category_thresholds_offset = 0x00U;
inline constexpr std::size_t category_thresholds_size = 0x50U;
inline constexpr std::size_t aggregate_values_offset = 0x50U;
inline constexpr std::size_t aggregate_values_size = 0x10U;
inline constexpr std::size_t raw_values_060_offset = 0x60U;
inline constexpr std::size_t raw_values_060_size = 0x14U;
inline constexpr std::size_t unknown_tail_offset = 0x74U;
inline constexpr std::size_t unknown_tail_size = 0x10U;

// Wide Pass 35/36 evidence-safe physical layout only. The five result
// categories, the raw +0x60 values, and the +0x74 tail remain semantically
// unresolved. Do not assign gameplay labels without a direct consumer edge.
struct MissionResultPolicyRecordLayout final {
    std::array<std::array<float, thresholds_per_category>, category_count>
        category_thresholds{};
    std::array<float, 4U> aggregate_values{};
    std::array<std::uint32_t, category_count> raw_values_060{};
    std::array<std::byte, unknown_tail_size> unknown_tail{};
};

static_assert(std::is_standard_layout_v<MissionResultPolicyRecordLayout>);
static_assert(sizeof(MissionResultPolicyRecordLayout) == policy_record_size);
static_assert(
    offsetof(MissionResultPolicyRecordLayout, category_thresholds) ==
    category_thresholds_offset);
static_assert(
    offsetof(MissionResultPolicyRecordLayout, aggregate_values) ==
    aggregate_values_offset);
static_assert(
    offsetof(MissionResultPolicyRecordLayout, raw_values_060) ==
    raw_values_060_offset);
static_assert(
    offsetof(MissionResultPolicyRecordLayout, unknown_tail) ==
    unknown_tail_offset);

// The observed group stride is four bytes larger than five records. The final
// dword is intentionally kept raw until ownership/consumer evidence exists.
struct MissionResultPolicyGroupLayout final {
    std::array<MissionResultPolicyRecordLayout, category_count> records{};
    std::uint32_t raw_group_tail{};
};

static_assert(std::is_standard_layout_v<MissionResultPolicyGroupLayout>);
static_assert(sizeof(MissionResultPolicyGroupLayout) == policy_group_stride);
static_assert(
    offsetof(MissionResultPolicyGroupLayout, raw_group_tail) ==
    category_count * policy_record_size);

} // namespace dmc::rengine::save::mission_result
