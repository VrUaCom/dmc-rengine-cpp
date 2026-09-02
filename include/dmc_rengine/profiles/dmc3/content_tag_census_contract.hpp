#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <string_view>

namespace dmc::rengine::profiles::dmc3 {

// Every payload tag the runtime compares, anywhere in the image.
//
// This project has been treating a four-byte tag at offset zero as what
// identifies a record, because the corpus is full of them: `HITS`, `LIG2`,
// `DCA`, `SEF`, `CAM`, `EVE`, `POS`, `ITM`, `STE`, `MOT`. A sweep of every
// byte-comparison chain and every immediate that spells ASCII settles what the
// runtime actually does with them: **nothing**.
//
// Five content tags are compared, at two sites, and the container magics are
// two more. Every other tag in the corpus is written by whoever made the file
// and read by nobody — the same thing already established for `MOT` alone, now
// established for the whole set.
//
// The consequence is what makes it worth recording. A tag this project reads
// is a fact about the file and not about the game, so a reader built on one is
// a reader of authoring conventions. That is legitimate and it is a different
// claim, and the two should not be printed identically.
struct ContentTagCensusContract final {
    static constexpr std::string_view canonical_target_sha256 =
        "e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082";

    // The dispatcher, which runs a handler for each of four.
    static constexpr std::uint64_t dispatcher_va = 0x1401B9FA0ULL;
    // The first registry's content probe, which runs when the extension chain
    // matched nothing, and which knows one tag the dispatcher does not.
    static constexpr std::uint64_t content_probe_va = 0x1402DB1F0ULL;

    struct ComparedTag final {
        std::string_view tag;
        std::uint64_t dispatcher_compare_va;  // zero where absent
        std::uint64_t probe_compare_va;       // zero where absent
    };

    static constexpr std::array<ComparedTag, 5> compared_tags{
        ComparedTag{"MOD", 0x1401B9FB4ULL, 0x1402DB1F3ULL},
        ComparedTag{"EFM", 0x1401B9FCBULL, 0x1402DB206ULL},
        ComparedTag{"SCM", 0x1401B9FE2ULL, 0x1402DB21CULL},
        ComparedTag{"SHW", 0x1401B9FFBULL, 0x1402DB248ULL},
        // Compared by the probe only. No handler dispatches it, and no file in
        // any supplied corpus carries it.
        ComparedTag{"MRP", 0ULL, 0x1402DB232ULL},
    };

    // The container magics, compared by the walks rather than by either of the
    // two type sites above.
    static constexpr std::array<std::string_view, 2> container_magics{
        "PAC", "PNST"};

    // Tags this project reads that the runtime compares nowhere. Listing them
    // is the point: each is an authoring convention, not a runtime fact.
    static constexpr std::array<std::string_view, 10> tags_read_but_never_compared{
        "HITS", "LIG2", "DCA", "SEF", "CAM",
        "EVE", "POS", "ITM", "STE", "MOT"};

    // `LIG2` is the one that comes closest: a constructor stores it as an
    // object's type field at `0x14023ECC9`. Stored, never compared.
    static constexpr std::string_view stored_but_not_compared = "LIG2";
    static constexpr std::uint64_t stored_but_not_compared_va = 0x14023ECC9ULL;

    [[nodiscard]] static constexpr bool runtime_compares(
        std::string_view tag) noexcept {
        for (const auto& entry : compared_tags) {
            if (entry.tag == tag) {
                return true;
            }
        }
        for (const auto magic : container_magics) {
            if (magic == tag) {
                return true;
            }
        }
        return false;
    }
};

} // namespace dmc::rengine::profiles::dmc3
