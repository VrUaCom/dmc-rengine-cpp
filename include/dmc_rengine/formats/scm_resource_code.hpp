#pragma once

#include <cstdint>

namespace dmc::rengine::formats::scm {

// Serialized SCM header +0x14 is retained by the canonical HD runtime at
// manager +0xE4. A 77-copy corpus sweep shows a stable six-decimal-digit
// decomposition for every SCM sample:
//
//   raw = family_class * 100000 + model_set * 100 + sub_index
//
// The component names are intentionally structural. In particular, family
// classes 3/4 are not assigned gameplay names until a producer/lookup is
// recovered. `model_set` is preferred over `stage` because st445 resources
// carry model_set 115, proving this component is not the current stage number.
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

// Corpus-shape predicate only; this is not a file-validity rule. All 77 SCM
// copies in the preserved corpus use class 3 or 4. Class 3 always has
// sub_index 0; class 4 uses 0 and non-zero child indices.
[[nodiscard]] constexpr bool matches_observed_scm_resource_code_shape(
    const LegacyResourceCode& code) noexcept {
    if (code.family_class == 3U) return code.sub_index == 0U;
    return code.family_class == 4U;
}

} // namespace dmc::rengine::formats::scm
