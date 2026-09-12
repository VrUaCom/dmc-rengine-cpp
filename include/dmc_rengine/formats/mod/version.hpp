#pragma once

#include <array>

namespace dmc::rengine::formats::mod {

// Hash-bound retail corpus evidence.
//
// The original em000 extraction confirms 0.82, 0.84, 1.00 and 1.01. A GData
// service-model slice supplied on 2026-09-11 adds three independent 0.80 MODs:
//   at000.mod afbaefa0c6414490dd2e00ff35b36d07b15a38d59230d6d720734343f57b797d
//   at002.mod 7f72c15557c980aa4d505de9057483575ac1af1de110b2a3ac06ff67087dfd18
//   at003.mod 9cb1ac281ae7992859b2576dd06dd54548cff9cd2d74e66f03b1c9b76cc65b6a
//
// Every listed version retains the currently recovered MOD structural grammar:
// 0x40 document header, 0x40 object records, 0x50 mesh records, the same
// stream/node-domain layout and the same skin/topology packing. This is a
// structural compatibility statement, not a global promise about every DMC3
// MOD revision or every higher-level semantic.
inline constexpr std::array<float, 5> corpus_confirmed_structural_versions{
    0.80F,
    0.82F,
    0.84F,
    1.00F,
    1.01F,
};

[[nodiscard]] constexpr bool is_corpus_confirmed_structural_version(
    float version,
    float epsilon = 0.0001F) noexcept {
    for (const float expected : corpus_confirmed_structural_versions) {
        const float delta = version >= expected
            ? version - expected
            : expected - version;
        if (delta <= epsilon) return true;
    }
    return false;
}

} // namespace dmc::rengine::formats::mod
