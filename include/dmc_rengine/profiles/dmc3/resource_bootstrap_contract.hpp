#pragma once

#include <cstdint>
#include <string_view>

namespace dmc::rengine::profiles::dmc3 {

// Instruction-backed contract for the recovered numbered-volume bootstrap.
//
// `Dmc3ResourceBootstrap` is what the original runtime does before any request
// is made: derive the data root beside the executable, probe numbered archives
// from zero, and register the mounts a later request resolves through. Bound to
// one exact image by `canonical_target_sha256`.
struct Dmc3ResourceBootstrapContract final {
    static constexpr std::string_view canonical_target_sha256 =
        "e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082";
    static constexpr std::uint64_t image_base = 0x140000000ULL;

    static constexpr std::uint64_t bootstrap_va = 0x14002E930ULL;

    // The data root is derived relative to the executable, with the recovered
    // backslash spelling the format string produces.
    static constexpr std::string_view executable_data_suffix = "\\data\\dmc3\\";
    static constexpr std::string_view volume_format = "%sDMC3-%d.nbz";

    static constexpr std::uint32_t first_probe_index = 0U;

    // `%d` is signed decimal, so the recovered numbered-volume namespace stops
    // at INT32_MAX. A larger numeric suffix on disk is a file the product may
    // legitimately discover; it is not a name this runtime could have produced,
    // and reporting it as a mount would widen a recovered namespace by
    // accident.
    static constexpr std::uint32_t runtime_index_max = 0x7FFFFFFFU;

    // Probing stops at the first missing index. A volume past that gap exists
    // as discovery evidence and is never answered from: the original runtime
    // could not reach it either.
    static constexpr bool stops_at_first_gap = true;

    // The physical data root is registered before any archive, and every mount
    // node is prepended to the list. Ascending registration therefore produces
    // descending resolution — 0,1,2 registers in that order and resolves 2,1,0
    // — with the physical root last among the archive-permitting providers.
    static constexpr bool physical_root_registered_first = true;
    static constexpr bool mount_list_is_prepend = true;

    [[nodiscard]] static consteval std::uint32_t rva_of(
        std::uint64_t virtual_address) noexcept {
        return static_cast<std::uint32_t>(virtual_address - image_base);
    }

    [[nodiscard]] static constexpr bool in_runtime_index_domain(
        std::uint32_t index) noexcept {
        return index <= runtime_index_max;
    }

    // Where a contiguous run of `count` volumes puts the volume at `index` in
    // resolution order. Prepending inverts registration, so the highest
    // contiguous volume is consulted first.
    [[nodiscard]] static constexpr std::uint32_t resolution_rank(
        std::uint32_t index,
        std::uint32_t count) noexcept {
        return index < count ? count - 1U - index : count;
    }
};

} // namespace dmc::rengine::profiles::dmc3
