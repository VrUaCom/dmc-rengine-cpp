#pragma once

#include "dmc_rengine/formats/hits.hpp"

#include <cstdint>
#include <string_view>

namespace dmc::rengine::hits::evidence {

enum class StaticSourceProfile : std::uint8_t {
    source_0_member_3 = 0,
    source_1_member_6 = 1,
    manual_or_unknown = 2,
};

struct FlagView final {
    std::uint32_t raw{};
    std::uint16_t upper_query_mask{};
    std::uint16_t lower_surface_value{};
};

struct SourceEvidenceProfile final {
    StaticSourceProfile source{StaticSourceProfile::manual_or_unknown};
    std::string_view label;
    std::string_view evidence_status;
    std::string_view structural_summary;
    std::string_view semantic_boundary;
};

[[nodiscard]] constexpr FlagView split_flags(std::uint32_t raw) noexcept {
    return FlagView{
        .raw = raw,
        .upper_query_mask = formats::hits::upper_flag_mask(raw),
        .lower_surface_value = formats::hits::lower_flag_value(raw),
    };
}

[[nodiscard]] constexpr SourceEvidenceProfile profile(
    StaticSourceProfile source) noexcept {
    switch (source) {
    case StaticSourceProfile::source_0_member_3:
        return SourceEvidenceProfile{
            .source = source,
            .label = "Source 0 / PAC member 3",
            .evidence_status = "EXE CONFIRMED + GAME VERIFIED",
            .structural_summary =
                "Default detailed stage-local HITS source with mixed raw flag values.",
            .semantic_boundary =
                "Exact gameplay names for individual flag combinations remain RESEARCH REQUIRED.",
        };
    case StaticSourceProfile::source_1_member_6:
        return SourceEvidenceProfile{
            .source = source,
            .label = "Source 1 / PAC member 6",
            .evidence_status = "EXE CONFIRMED + GAME VERIFIED",
            .structural_summary =
                "Optional independent coarse stage-local HITS source; 188/189 supplied records use raw flags 0.",
            .semantic_boundary =
                "Auxiliary specialized-query role is supported, but camera/navigation/boundary/player-only naming is RESEARCH REQUIRED.",
        };
    case StaticSourceProfile::manual_or_unknown:
    default:
        return SourceEvidenceProfile{
            .source = StaticSourceProfile::manual_or_unknown,
            .label = "Manual or unknown HITS source",
            .evidence_status = "RESEARCH REQUIRED",
            .structural_summary =
                "Valid HITS resource without confirmed stage PAC member provenance.",
            .semantic_boundary =
                "Do not infer source role from slot number or geometry alone.",
        };
    }
}

} // namespace dmc::rengine::hits::evidence
