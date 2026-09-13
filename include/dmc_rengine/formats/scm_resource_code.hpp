#pragma once

#include <array>
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

// Corpus-shape predicate only; this is not a file-validity rule.
//
// The observed family-class domain has now widened twice in one day: the
// historical bounded corpus established 3/4, fresh st002.scm added 813800 =>
// class 8, and the direct st000..st003 container scan added 730507 => class 7,
// model_set 305, sub_index 7. That is the second correction to the same
// predicate in the same pass, which is itself the useful signal: the domain is
// being discovered rather than enumerated, so a payload outside it is a reason
// to widen this list, never a reason to doubt the file.
//
// The sub-index shape is therefore stated only where the corpus actually
// constrains it. Classes 3 and 8 are observed with sub_index 0 and no other
// value; class 4 and class 7 are observed with both zero and non-zero child
// indices, so neither carries a sub-index rule.
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

} // namespace dmc::rengine::formats::scm
