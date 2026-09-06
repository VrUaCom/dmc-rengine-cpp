#pragma once

#include <cstddef>
#include <cstdint>
#include <span>

namespace dmc::rengine::formats::mod::runtime_postload {

inline constexpr std::uint64_t canonical_helper_va = 0x1402FE3B0ULL;
inline constexpr std::size_t header_object_count_offset = 0x10U;
inline constexpr std::size_t header_document_pointer_offset = 0x20U;
inline constexpr std::size_t first_object_offset = 0x40U;
inline constexpr std::size_t object_stride = 0x40U;
inline constexpr std::size_t object_mesh_count_offset = 0x00U;
inline constexpr std::size_t object_mesh_array_offset = 0x08U;
inline constexpr std::size_t mesh_stride = 0x50U;
inline constexpr std::size_t mesh_element_count_offset = 0x00U;
inline constexpr std::size_t mesh_packed_weights_topology_offset = 0x30U;
inline constexpr std::size_t mesh_generated_workspace_offset = 0x40U;
inline constexpr std::size_t mesh_generated_word_count_offset = 0x48U;

// The canonical MOD post-load helper mutates a serialized MOD allocation into
// a runtime image: selected relative offsets become host pointers, bit 15 in
// the packed weights/topology stream is consumed as a strip-break marker and
// cleared in-place, and a generated u16 topology-command stream is written to
// the mesh-relative workspace at +0x40. The resulting bytes are not a
// serializable MOD file and must not be fed to the structural parser as source.
enum class Status : std::uint8_t {
    ok,
    truncated_header,
    object_table_out_of_bounds,
    mesh_table_out_of_bounds,
    packed_stream_out_of_bounds,
    generated_workspace_out_of_bounds,
    pointer_overflow,
    generated_word_count_overflow,
};

struct Result final {
    Status status{Status::truncated_header};
    std::size_t object_count{};
    std::size_t mesh_count{};
    std::size_t generated_word_count{};

    [[nodiscard]] bool ok() const noexcept {
        return status == Status::ok;
    }
};

// Safe host reconstruction of canonical helper 0x1402FE3B0. The original
// helper assumes a valid typed MOD allocation. This implementation validates
// every range it dereferences and plans all writes before mutation, so malformed
// research fixtures fail without partial pointer relocation.
[[nodiscard]] Result apply_in_place(std::span<std::byte> bytes);

} // namespace dmc::rengine::formats::mod::runtime_postload
