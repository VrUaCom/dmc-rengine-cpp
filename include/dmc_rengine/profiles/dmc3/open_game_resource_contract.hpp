#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <string_view>

namespace dmc::rengine::profiles::dmc3 {

// Instruction-backed contract for the recovered generic resource request path.
//
// `OpenGameResource` is the caller-side entry the original runtime uses to ask
// for a resource by name. Everything here was recovered from one exact image
// and is bound to it by `canonical_target_sha256`: an address without the
// identity of the binary it came from is not evidence, it is a number.
//
// This type states what the executable does. It does not execute anything, and
// it is not a claim that the product resolver is an emulation of that path —
// GDSpaces reproduces the recovered *policy* as portable code, and the
// separation is what lets both be checked against each other.
struct OpenGameResourceContract final {
    static constexpr std::string_view canonical_target_sha256 =
        "e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082";
    static constexpr std::uint64_t image_base = 0x140000000ULL;

    static constexpr std::uint64_t open_game_resource_va = 0x14002FCA0ULL;

    // Bounded join helper the request path builds each candidate with, and the
    // release the path takes when that helper refuses.
    static constexpr std::uint64_t bounded_join_va = 0x1403272C0ULL;
    static constexpr std::uint64_t file_slot_release_va = 0x140048DF0ULL;

    // A whole-image direct-call census found exactly these three call sites,
    // and no stored 64-bit pointer to the entry anywhere in the image. That
    // absence is the load-bearing half: with no function-table entry there is
    // no second caller mode to account for.
    static constexpr std::array<std::uint64_t, 3> direct_call_sites{
        0x14003340AULL,
        0x1403380C7ULL,
        0x1403381F7ULL,
    };
    static constexpr bool stored_function_pointer_observed = false;

    // All three call sites materialize EDX = 1 immediately before the call, so
    // this is the observed caller mode for the recovered direct-call path. The
    // other branches inside the function are implementation code that no
    // canonical caller selects; they must not be promoted to fallback policy
    // without an indirect-call or runtime receipt of their own.
    static constexpr std::uint32_t direct_call_flags = 1U;

    static constexpr std::uint32_t archive_provider_mask = 1U;
    static constexpr std::uint32_t physical_provider_mask = 2U;

    // The join helper's destination capacity, including the terminating NUL —
    // so candidate text must be strictly shorter than this.
    static constexpr std::size_t candidate_buffer_bytes = 0x400U;

    // The six logical namespace prefixes, in the exact recovered order. The
    // `.afs/` spellings are namespace strings and are not evidence for a
    // binary AFS backend.
    static constexpr std::array<std::string_view, 6> namespace_prefixes{
        "GDataX360.afs/",
        "GData.afs/",
        "Video/",
        "afs/sound/",
        "SAVEDATA/",
        "",
    };

    // One complete archive pass over all six prefixes, then one physical pass
    // over the same six.
    static constexpr std::size_t provider_passes = 2U;
    static constexpr std::size_t attempts_per_request =
        namespace_prefixes.size() * provider_passes;

    static constexpr std::int32_t miss_return_value = -1;

    [[nodiscard]] static consteval std::uint32_t rva_of(
        std::uint64_t virtual_address) noexcept {
        return static_cast<std::uint32_t>(virtual_address - image_base);
    }

    // The longest prefix is also the first one tried, and a candidate that
    // overflows aborts the whole request rather than advancing to a shorter
    // prefix. So the longest prefix alone decides the largest basename the
    // recovered path can ever accept.
    [[nodiscard]] static consteval std::size_t longest_prefix_bytes() noexcept {
        std::size_t longest = 0U;
        for (const auto prefix : namespace_prefixes) {
            if (prefix.size() > longest) {
                longest = prefix.size();
            }
        }
        return longest;
    }

    [[nodiscard]] static consteval std::size_t max_basename_bytes() noexcept {
        // One byte of the destination belongs to the terminating NUL.
        return candidate_buffer_bytes - 1U - longest_prefix_bytes();
    }

    [[nodiscard]] static constexpr bool candidate_fits(
        std::string_view candidate) noexcept {
        return candidate.size() < candidate_buffer_bytes;
    }

    [[nodiscard]] static constexpr std::uint32_t provider_mask_for_pass(
        std::size_t pass_index) noexcept {
        return pass_index == 0U ? archive_provider_mask : physical_provider_mask;
    }
};

// What the request path does when the join helper refuses a candidate.
//
// Named rather than described in a comment because every clause is a decision
// the product resolver could plausibly have made differently, and each one
// changes which requests resolve.
struct OpenGameResourceOverflowBehavior final {
    // The newly allocated file slot is released through the recovered release.
    static constexpr bool releases_file_slot = true;
    // The request returns the miss value immediately.
    static constexpr bool returns_miss = true;
    // It does not advance to the next prefix.
    static constexpr bool advances_prefix_index = false;
    // It does not fall through to the physical pass.
    static constexpr bool enters_physical_pass = false;
};

} // namespace dmc::rengine::profiles::dmc3
