#pragma once

#include "dmc_rengine/profiles/dmc3/known_targets.hpp"

#include <array>
#include <cstdint>
#include <optional>
#include <string_view>

namespace dmc::rengine::profiles::dmc3::hits_source1_query_evidence {

[[nodiscard]] inline bool matches_canonical_target(std::string_view sha256) noexcept {
    return phase12_canonical_target().matches_hash(sha256);
}

enum class Source1QueryRole : std::uint8_t {
    segment_correction_fec0,
    displacement_accumulator_601e0,
};

struct CanonicalFunctionBodyEvidence final {
    Source1QueryRole role{};
    std::uint64_t begin_va{};
    std::uint64_t end_va{};
    std::uint32_t size_bytes{};
    std::string_view sha256{};
};

struct Source1SegmentCorrectionEvidence final {
    std::uint64_t function_va{};
    std::uint8_t runtime_arg_index{};
    std::uint8_t mutable_point_arg_index{};
    std::uint8_t reference_point_arg_index{};
    std::uint32_t direct_static_caller_count{};
    std::uint32_t hard_rejected_raw_record_bit{};
    bool computes_reference_minus_mutable_direction{};
    bool xyz_length_controls_degenerate_return{};
    bool direction_normalizer_zeros_fourth_component{};
    bool nondegenerate_path_writes_mutable_point{};
    bool nondegenerate_path_returns_true_even_without_accepted_record{};
    bool return_value_is_hit_boolean{};
};

struct Source1DisplacementAccumulatorEvidence final {
    std::uint64_t function_va{};
    std::uint8_t runtime_arg_index{};
    std::uint8_t mutable_point_arg_index{};
    std::uint8_t reference_point_arg_index{};
    std::uint32_t direct_static_caller_count{};
    bool performs_initial_record_correction_pass{};
    bool performs_secondary_normal_accumulation_pass{};
    bool mutates_point_in_place{};
    bool return_reports_xyz_displacement_from_original{};
    bool xyz_norm_and_dot_exclude_fourth_component{};
    bool four_component_subtract_scale_add_is_used{};
    bool fourth_component_gameplay_semantics_resolved{};
};

namespace detail {

class EvidenceStore final {
public:
    [[nodiscard]] static std::optional<CanonicalFunctionBodyEvidence>
    body(std::string_view sha256, Source1QueryRole role) noexcept {
        if (!matches_canonical_target(sha256)) {
            return std::nullopt;
        }
        for (const auto& descriptor : k_bodies) {
            if (descriptor.role == role) {
                return descriptor;
            }
        }
        return std::nullopt;
    }

    [[nodiscard]] static std::optional<Source1SegmentCorrectionEvidence>
    segment_correction(std::string_view sha256) noexcept {
        if (!matches_canonical_target(sha256)) {
            return std::nullopt;
        }
        return k_segment_correction;
    }

    [[nodiscard]] static std::optional<Source1DisplacementAccumulatorEvidence>
    displacement_accumulator(std::string_view sha256) noexcept {
        if (!matches_canonical_target(sha256)) {
            return std::nullopt;
        }
        return k_displacement_accumulator;
    }

private:
    inline static constexpr std::array<CanonicalFunctionBodyEvidence, 2> k_bodies{{
        {Source1QueryRole::segment_correction_fec0,
         0x14005FEC0ULL,
         0x1400601D3ULL,
         787U,
         "cfaca9752adf3a581969875e99c6c1b9ffaa7f83d19d22fddbe8dec2d5cab09e"},
        {Source1QueryRole::displacement_accumulator_601e0,
         0x1400601E0ULL,
         0x14006078EULL,
         1454U,
         "7fb7fb7ce9447a5e200ba1471774eafe7e5f21877da71902f047563bd9f7597a"},
    }};

    inline static constexpr Source1SegmentCorrectionEvidence k_segment_correction{
        0x14005FEC0ULL,
        1U,
        2U,
        3U,
        1U,
        0x00080000U,
        true,
        true,
        true,
        true,
        true,
        false,
    };

    inline static constexpr Source1DisplacementAccumulatorEvidence k_displacement_accumulator{
        0x1400601E0ULL,
        1U,
        2U,
        3U,
        5U,
        true,
        true,
        true,
        true,
        true,
        true,
        false,
    };
};

} // namespace detail

[[nodiscard]] inline std::optional<CanonicalFunctionBodyEvidence>
canonical_function_body(std::string_view sha256, Source1QueryRole role) noexcept {
    return detail::EvidenceStore::body(sha256, role);
}

[[nodiscard]] inline std::optional<Source1SegmentCorrectionEvidence>
segment_correction_evidence(std::string_view sha256) noexcept {
    return detail::EvidenceStore::segment_correction(sha256);
}

[[nodiscard]] inline std::optional<Source1DisplacementAccumulatorEvidence>
displacement_accumulator_evidence(std::string_view sha256) noexcept {
    return detail::EvidenceStore::displacement_accumulator(sha256);
}

} // namespace dmc::rengine::profiles::dmc3::hits_source1_query_evidence
