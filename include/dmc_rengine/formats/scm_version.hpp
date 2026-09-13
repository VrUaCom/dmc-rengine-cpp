#pragma once

#include <array>

namespace dmc::rengine::formats::scm {

// Hash-bound retail stage-PAC authority, 2026-09-13.
//
// A direct structural scan of st000.pac through st003.pac found 51 valid SCM
// payloads. All 51 serialized extents have distinct SHA-256 digests. Their
// version distribution is:
//
//   0.83 x1   0.90 x16   1.00 x1   1.01 x33
//
// Every listed version uses the currently recovered SCM structural grammar:
// 0x40 document header, 0x40 object records, 0x50 mesh records, the same stream
// layout, scene-node domain and topology packing. Across the 51-payload census
// the preserved/undecoded header, object, mesh, scene and transform domains
// remain zero; topology bytes use only 0 and 2 over 107,233 vertices.
//
// This is a structural compatibility statement for the observed retail set,
// not a promise that every historical DMC3 SCM revision follows this grammar.
inline constexpr std::array<float, 4> corpus_confirmed_structural_versions{
    0.83F,
    0.90F,
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

} // namespace dmc::rengine::formats::scm
