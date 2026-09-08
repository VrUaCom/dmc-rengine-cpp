#pragma once

#include <array>

namespace dmc::rengine::formats::mod {

// Hash-bound retail corpus evidence from em000-extract.zip
// SHA-256 306130125f09824811289366324f4208c3c1aba880c5a7efa3953a88d566d07b.
//
// These versions all retain the currently recovered MOD structural grammar in
// that corpus: 0x40 document header, 0x40 object records, 0x50 mesh records,
// the same stream/node-domain layout and the same skin packing. This is a
// structural compatibility statement, not a global promise about every DMC3
// MOD revision or every higher-level semantic.
inline constexpr std::array<float, 4> corpus_confirmed_structural_versions{
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
