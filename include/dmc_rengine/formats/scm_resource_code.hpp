#pragma once

#include <array>
#include <cstdint>

namespace dmc::rengine::formats::scm {

// Serialized SCM header +0x14 is retained by the canonical HD runtime at
// manager+0xE4. The hash-bound retail stage-PAC authority confirms the stable
// decimal decomposition:
//
//   raw = family_class * 100000 + model_set * 100 + sub_index
//
// The component names are intentionally structural. Across 51 SHA-distinct SCM
// payloads from st000.pac..st003.pac, observed family classes are 3, 4, 7 and 8.
// No gameplay/artistic label is assigned until a provenance-clean producer or
// typed downstream manager+0xE4 consumer is recovered. `model_set` is preferred
// over `stage` because observed values do not consistently equal the current
// stage number.
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

// Corpus-shape predicate only; this is not a file-validity rule.
//
// The observed domain widened repeatedly during the retail scan. Classes 3 and
// 8 are observed with sub_index 0; classes 4 and 7 contain both zero and
// non-zero child indices, so neither class gets an invented sub-index rule.
// A future code outside this predicate is preserved and warned about, never
// rejected as an invalid SCM.
inline constexpr std::array<std::uint16_t, 4> observed_family_classes{
    3U,
    4U,
    7U,
    8U,
};

[[nodiscard]] constexpr bool matches_observed_scm_resource_code_shape(
    const LegacyResourceCode& code) noexcept {
    if (code.family_class == 3U || code.family_class == 8U) {
        return code.sub_index == 0U;
    }
    return code.family_class == 4U || code.family_class == 7U;
}

// Compile-time evidence locks. These are deliberately examples of the
// structural domain rather than gameplay labels.
inline constexpr auto observed_resource_code_730507 =
    decode_legacy_resource_code(730507U);
static_assert(observed_resource_code_730507.family_class == 7U);
static_assert(observed_resource_code_730507.model_set == 305U);
static_assert(observed_resource_code_730507.sub_index == 7U);
static_assert(matches_observed_scm_resource_code_shape(
    observed_resource_code_730507));

inline constexpr auto observed_resource_code_813800 =
    decode_legacy_resource_code(813800U);
static_assert(observed_resource_code_813800.family_class == 8U);
static_assert(observed_resource_code_813800.model_set == 138U);
static_assert(observed_resource_code_813800.sub_index == 0U);
static_assert(matches_observed_scm_resource_code_shape(
    observed_resource_code_813800));

static_assert(!matches_observed_scm_resource_code_shape(
    decode_legacy_resource_code(300101U)));
static_assert(!matches_observed_scm_resource_code_shape(
    decode_legacy_resource_code(813801U)));
static_assert(matches_observed_scm_resource_code_shape(
    decode_legacy_resource_code(400115U)));
static_assert(matches_observed_scm_resource_code_shape(
    decode_legacy_resource_code(700115U)));
static_assert(!matches_observed_scm_resource_code_shape(
    decode_legacy_resource_code(900100U)));

} // namespace dmc::rengine::formats::scm
