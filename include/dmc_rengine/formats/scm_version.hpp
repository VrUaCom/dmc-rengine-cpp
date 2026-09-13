#pragma once

#include <array>

namespace dmc::rengine::formats::scm {

// Hash-bound retail corpus evidence.
//
// The reverse was closed against a corpus that carried only 1.01, and the
// parser warned on anything else. A direct scan of the retail stage containers
// st000.pac through st003.pac — reading the object and mesh tables, the 0x50
// continuation chain, vertex sums, stream bounds, scene arrays and the
// index-workspace footprint, rather than matching the `SCM ` magic — found 51
// structurally valid occurrences carrying four distinct versions:
//
//   0.83 x1   0.90 x16   1.00 x1   1.01 x33
//
// Those are occurrence counts inside the containers, not counts of unique
// payloads: the same scene model appears more than once across a stage set and
// the SHA deduplication of these 51 is not finished. The distinction matters
// for any claim about how common a version is and not at all for this list,
// which only records that each version was observed at least once.
//
// Every listed version parses under the currently recovered SCM structural
// grammar: the 0x40 document header, 0x40 object records, 0x50 mesh records,
// the same stream layout and scene-node domain, and the same topology packing.
// The legacy versions did not revive any preservation-only domain — the header
// lanes, object +0x04 and +0x14..+0x2F, mesh +0x0C/+0x30/+0x48/+0x4C, scene
// +0x10..+0x1F, transform +0x1C and every GS CLAMP field are zero across all
// 51 — and their topology bytes use only 0 and 2 over 107,233 vertices, so no
// topology bit outside the confirmed 0x02 appears in them either.
//
// This is a structural compatibility statement about the observed set, not a
// promise about every DMC3 SCM revision or any higher-level semantic.
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
