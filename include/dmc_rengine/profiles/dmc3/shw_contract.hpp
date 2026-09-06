#pragma once

#include <cstddef>
#include <cstdint>
#include <string_view>

namespace dmc::rengine::profiles::dmc3 {

// Evidence-backed DMC3-HD SHW shadow-hull contract.
//
// Authority split:
// - identity / relocation / selector semantics: canonical dmc3.exe
// - exact serialized spans below: hash-bound real SHW payload
//   cb392ef2e874addb887d32bc44d409299a32a83a4845afcbdef31698283f2e7e
//
// See:
//   data/reverse/dmc3-real-mod-shw-payload-binding-20260901.json
//   docs/research/dmc3-real-mod-shw-payload-binding-2026-09-01.md
struct ShwContract final {
    static constexpr std::string_view canonical_target_sha256 =
        "e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082";
    static constexpr std::string_view bound_payload_sha256 =
        "cb392ef2e874addb887d32bc44d409299a32a83a4845afcbdef31698283f2e7e";

    static constexpr std::uint64_t image_base = 0x140000000ULL;
    static constexpr std::uint64_t relocate_va = 0x1403204C0ULL;
    static constexpr std::uint64_t runtime_builder_va = 0x14031FD30ULL;
    static constexpr std::uint64_t vertex_transform_loop_va = 0x1403202F0ULL;
    static constexpr std::uint64_t matrix_vector_helper_va = 0x140030A70ULL;

    static constexpr std::string_view magic = "SHW ";
    static constexpr float bound_version = 0.5F;

    static constexpr std::size_t header_size = 0x20U;
    static constexpr std::size_t version_offset = 0x04U;
    static constexpr std::size_t hull_count_offset = 0x10U;
    static constexpr std::size_t hull_table_offset = 0x20U;
    static constexpr std::size_t hull_record_size = 0x40U;

    static constexpr std::size_t vertex_count_offset = 0x00U;
    static constexpr std::size_t triangle_count_offset = 0x02U;
    static constexpr std::size_t triangle_pointer_offset = 0x10U;
    static constexpr std::size_t adjacency_pointer_offset = 0x18U;
    static constexpr std::size_t vertex_pointer_offset = 0x20U;
    static constexpr std::size_t selector_pointer_offset = 0x28U;

    static constexpr std::size_t triangle_record_size = 0x10U;
    static constexpr std::size_t adjacency_record_size = 0x08U;
    static constexpr std::size_t vertex_record_size = 0x10U;

    // The serialized count is one byte. This is a representational bound, not
    // a claim that every DMC3 SHW variant uses all 255 values.
    static constexpr std::size_t max_hull_count = 0xFFU;
};

} // namespace dmc::rengine::profiles::dmc3
