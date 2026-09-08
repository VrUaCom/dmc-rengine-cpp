#pragma once

#include <cstddef>
#include <cstdint>
#include <optional>
#include <span>
#include <string>
#include <vector>

namespace dmc::rengine::formats::efm {

enum class ParseError : std::uint8_t {
    none,
    truncated_header,
    invalid_magic,
    object_table_out_of_bounds,
    mesh_table_out_of_bounds,
    aggregate_count_mismatch,
    stream_out_of_bounds,
    node_domain_out_of_bounds,
};

struct Mesh final {
    std::size_t record_offset{};
    std::uint16_t element_count{};
    std::uint16_t texture_slot{};
    std::uint64_t positions_offset{};
    std::uint64_t normals_offset{};
    std::uint64_t uv_offset{};
    std::uint64_t blend_indices_offset{};
    std::uint64_t topology_control_offset{};
    std::uint64_t color0_offset{};
    std::uint64_t generated_workspace_relative{};
    std::uint32_t generated_topology_count{};
};

struct Object final {
    std::size_t record_offset{};
    std::uint8_t mesh_count{};
    std::uint16_t aggregate_element_count{};
    std::uint64_t mesh_table_offset{};
    std::vector<Mesh> meshes;
};

struct Document final {
    std::size_t physical_size{};
    std::uint8_t object_count{};
    std::uint8_t node_domain_count{};
    std::uint8_t texture_slot_domain{};
    std::uint8_t runtime_mode_byte{};
    std::uint32_t runtime_metadata_u32{};
    std::uint64_t node_domain_offset{};
    std::vector<Object> objects;

    [[nodiscard]] bool valid() const noexcept;
};

struct ParseResult final {
    std::optional<Document> document;
    ParseError error{ParseError::none};
    std::string message;

    [[nodiscard]] bool ok() const noexcept {
        return document.has_value() && error == ParseError::none;
    }
};

// Structural EFM adapter. The parser materializes only the EXE/corpus-backed
// model-document shell and the six observed source streams. It intentionally
// leaves unknown fields and generated-workspace semantics unpromoted.
class Parser final {
public:
    static constexpr std::size_t k_header_size = 0x40U;
    static constexpr std::size_t k_object_size = 0x40U;
    static constexpr std::size_t k_mesh_size = 0x50U;

    [[nodiscard]] static ParseResult parse(std::span<const std::byte> bytes);
};

} // namespace dmc::rengine::formats::efm
