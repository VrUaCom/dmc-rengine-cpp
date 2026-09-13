#pragma once

#include <cstdint>

namespace dmc::rengine::formats::scm {

// Serialized SCM header +0x14 is retained by the canonical HD runtime at
// manager +0xE4. The preserved retail corpus plus fresh hash-bound st001/st002/
// st003 specimens show a stable decimal decomposition:
//
//   raw = family_class * 100000 + model_set * 100 + sub_index
//
// The component names are intentionally structural. Observed family classes now
// include 3, 4 and 8; no gameplay/artistic labels are assigned until a
// provenance-clean producer or typed downstream manager+0xE4 consumer is
// recovered. `model_set` is preferred over `stage` because observed values do
// not consistently equal the current stage number.
struct LegacyResourceCode final {
    std::uint32_t raw{};
    std::uint16_t model_set{};
    std::uint8_t family_class{};
    std::uint8_t sub_index{};
};

[[nodiscard]] constexpr LegacyResourceCode decode_legacy_resource_code(
    std::uint32_t raw) noexcept {
    LegacyResourceCode out{};
    out.raw = raw;
    out.family_class = static_cast<std::uint8_t>(raw / 100000U);
    out.model_set = static_cast<std::uint16_t>((raw / 100U) % 1000U);
    out.sub_index = static_cast<std::uint8_t>(raw % 100U);
    return out;
}

[[nodiscard]] constexpr std::uint32_t encode_legacy_resource_code(
    std::uint8_t family_class,
    std::uint16_t model_set,
    std::uint8_t sub_index) noexcept {
    return static_cast<std::uint32_t>(family_class) * 100000U +
           static_cast<std::uint32_t>(model_set) * 100U +
           static_cast<std::uint32_t>(sub_index);
}

// Corpus-shape predicate only; this is not a file-validity rule. The historical
// bounded corpus established classes 3/4. Fresh hash-bound st002.scm extends
// the observed domain with 813800 => class 8, model_set 138, sub_index 0.
// Classes 3 and 8 are currently observed with sub_index 0; class 4 includes
// zero and non-zero child indices.
[[nodiscard]] constexpr bool matches_observed_scm_resource_code_shape(
    const LegacyResourceCode& code) noexcept {
    if (code.family_class == 3U || code.family_class == 8U) {
        return code.sub_index == 0U;
    }
    return code.family_class == 4U;
}

} // namespace dmc::rengine::formats::scm
