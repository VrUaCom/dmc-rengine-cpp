#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <string_view>

namespace dmc::rengine::profiles::dmc3 {

// What a container's own manifest calls a payload, and what the payload calls
// itself.
//
// A stage container stores a CRLF name list in its slot 0, and this project
// checks a manifest line against the payload by comparing the line's extension
// to the classified format as plain text. That comparison is right twice out of
// three times and wrong once, and the wrong one has been showing on screen as
// an unconfirmed name in red:
//
//     st001.ptx  ->  slot 1, texture pack, classified `ptx`   agrees
//     st001.scm  ->  slot 2, model,        classified `scm`   agrees
//     st001.sch  ->  slot 3, collision,    classified `hits`  "disagrees"
//
// `sch` and `hits` are the same thing under two names: the authoring extension
// and the tag the payload carries at offset zero. Both stage containers in the
// corpus place a `HITS` record at the slot their manifest calls `.sch`, so the
// pairing is corroborated by two files rather than assumed from one.
//
// The distinction this preserves: an equivalence listed here means "these two
// names were seen naming the same payload", not "this extension means this
// format". Nothing in the image maps an extension to a type — the runtime
// knows a resource by its name, and the type probe reads the payload. So a
// pairing that is not listed here is not corroborated, and it says so.
struct AuthoringExtensionContract final {
    static constexpr std::string_view canonical_target_sha256 =
        "e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082";

    struct Pairing final {
        std::string_view extension;
        std::string_view format;
        // Stage containers in which this pairing was seen at the same slot.
        std::size_t observed_files{};
        std::size_t manifest_slot{};
    };

    static constexpr std::array<Pairing, 3> pairings{
        Pairing{.extension = "ptx", .format = "ptx",
                .observed_files = 2U, .manifest_slot = 1U},
        Pairing{.extension = "scm", .format = "scm",
                .observed_files = 2U, .manifest_slot = 2U},
        // The one that is not a plain string match.
        Pairing{.extension = "sch", .format = "hits",
                .observed_files = 2U, .manifest_slot = 3U},
    };

    // True where the manifest's extension and the payload's own classified
    // format are the same resource under two names.
    [[nodiscard]] static constexpr bool names_the_same_resource(
        std::string_view extension, std::string_view format) noexcept {
        if (extension.empty() || format.empty()) {
            return false;
        }
        if (extension == format) {
            return true;
        }
        for (const auto& pairing : pairings) {
            if (pairing.extension == extension && pairing.format == format) {
                return true;
            }
        }
        return false;
    }
};

// How a model group container numbers its slots.
//
// `st001.pac` slot 5 declares 11 slots and populates 0 and 10. `st114.pac`
// slot 5 declares 21 and populates 0, 10 and 20. Every populated index is a
// multiple of ten, and the declared count is exactly enough to reach the last
// one.
//
// This matters for what an operator is shown. Nine consecutive rows reading
// "empty slot" look like damage; they are reserved identity space between one
// group and the next, and the recovered walk already proves a zero offset is a
// slot that carries nothing rather than a container that is broken.
//
// Two containers is a convention, not a law. The stride is recorded with the
// count of what was seen, and a container that does not follow it is described
// rather than refused.
struct ModelGroupNumberingContract final {
    static constexpr std::string_view canonical_target_sha256 =
        "e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082";

    static constexpr std::uint32_t observed_stride = 10U;
    static constexpr std::size_t observed_containers = 2U;
    static constexpr std::uint32_t st001_declared_slots = 11U;
    static constexpr std::uint32_t st114_declared_slots = 21U;
    static constexpr std::uint32_t st001_populated = 2U;
    static constexpr std::uint32_t st114_populated = 3U;

    // The declared count that holds `populated` groups at this stride.
    [[nodiscard]] static constexpr std::uint32_t declared_slots_for(
        std::uint32_t populated) noexcept {
        return populated == 0U ? 0U : (populated - 1U) * observed_stride + 1U;
    }

    [[nodiscard]] static constexpr bool index_is_on_stride(
        std::uint32_t slot_index) noexcept {
        return slot_index % observed_stride == 0U;
    }
};

} // namespace dmc::rengine::profiles::dmc3
