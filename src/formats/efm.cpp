#include "dmc_rengine/formats/efm.hpp"

#include <utility>

namespace dmc::rengine::formats::efm {
namespace {

[[nodiscard]] ParseResult fail(ParseError error, std::string message) {
    return ParseResult{
        .document = std::nullopt,
        .error = error,
        .message = std::move(message),
    };
}

[[nodiscard]] std::uint16_t read_u16(
    std::span<const std::byte> bytes, std::size_t offset) noexcept {
    return static_cast<std::uint16_t>(
        std::to_integer<std::uint16_t>(bytes[offset + 0U]) |
        static_cast<std::uint16_t>(
            std::to_integer<std::uint16_t>(bytes[offset + 1U]) << 8U));
}

[[nodiscard]] std::uint32_t read_u32(
    std::span<const std::byte> bytes, std::size_t offset) noexcept {
    return std::to_integer<std::uint32_t>(bytes[offset + 0U]) |
        (std::to_integer<std::uint32_t>(bytes[offset + 1U]) << 8U) |
        (std::to_integer<std::uint32_t>(bytes[offset + 2U]) << 16U) |
        (std::to_integer<std::uint32_t>(bytes[offset + 3U]) << 24U);
}

[[nodiscard]] std::uint64_t read_u64(
    std::span<const std::byte> bytes, std::size_t offset) noexcept {
    std::uint64_t value = 0U;
    for (std::size_t i = 0U; i < 8U; ++i) {
        value |= std::to_integer<std::uint64_t>(bytes[offset + i]) << (8U * i);
    }
    return value;
}

[[nodiscard]] bool range_ok(
    std::size_t file_size, std::uint64_t offset, std::uint64_t size) noexcept {
    const auto file = static_cast<std::uint64_t>(file_size);
    return offset <= file && size <= file - offset;
}

} // namespace

bool Document::valid() const noexcept {
    return physical_size >= Parser::k_header_size &&
        objects.size() == static_cast<std::size_t>(object_count) &&
        node_domain_offset < static_cast<std::uint64_t>(physical_size);
}

ParseResult Parser::parse(std::span<const std::byte> bytes) {
    if (bytes.size() < k_header_size) {
        return fail(ParseError::truncated_header, "EFM 0x40-byte header is truncated");
    }
    if (bytes[0U] != std::byte{'E'} || bytes[1U] != std::byte{'F'} ||
        bytes[2U] != std::byte{'M'} || bytes[3U] != std::byte{' '}) {
        return fail(ParseError::invalid_magic, "EFM<space> marker is absent");
    }

    Document document{
        .physical_size = bytes.size(),
        .object_count = std::to_integer<std::uint8_t>(bytes[0x10U]),
        .node_domain_count = std::to_integer<std::uint8_t>(bytes[0x11U]),
        .texture_slot_domain = std::to_integer<std::uint8_t>(bytes[0x12U]),
        .runtime_mode_byte = std::to_integer<std::uint8_t>(bytes[0x13U]),
        .runtime_metadata_u32 = read_u32(bytes, 0x14U),
        .node_domain_offset = read_u64(bytes, 0x20U),
    };

    const auto object_count = static_cast<std::size_t>(document.object_count);
    if (object_count > (bytes.size() - k_header_size) / k_object_size) {
        return fail(ParseError::object_table_out_of_bounds, "EFM object table crosses EOF");
    }
    if (document.node_domain_offset >= static_cast<std::uint64_t>(bytes.size())) {
        return fail(ParseError::node_domain_out_of_bounds, "EFM node-domain offset is outside the file");
    }

    document.objects.reserve(object_count);
    for (std::size_t object_index = 0U; object_index < object_count; ++object_index) {
        const auto object_offset = k_header_size + object_index * k_object_size;
        Object object{
            .record_offset = object_offset,
            .mesh_count = std::to_integer<std::uint8_t>(bytes[object_offset + 0x00U]),
            .aggregate_element_count = read_u16(bytes, object_offset + 0x02U),
            .mesh_table_offset = read_u64(bytes, object_offset + 0x08U),
        };

        const auto mesh_count = static_cast<std::size_t>(object.mesh_count);
        const auto mesh_table_size = static_cast<std::uint64_t>(mesh_count) * k_mesh_size;
        if (!range_ok(bytes.size(), object.mesh_table_offset, mesh_table_size)) {
            return fail(ParseError::mesh_table_out_of_bounds, "EFM mesh table crosses EOF");
        }

        std::uint32_t aggregate = 0U;
        object.meshes.reserve(mesh_count);
        for (std::size_t mesh_index = 0U; mesh_index < mesh_count; ++mesh_index) {
            const auto mesh_offset_u64 = object.mesh_table_offset +
                static_cast<std::uint64_t>(mesh_index * k_mesh_size);
            const auto mesh_offset = static_cast<std::size_t>(mesh_offset_u64);
            Mesh mesh{
                .record_offset = mesh_offset,
                .element_count = read_u16(bytes, mesh_offset + 0x00U),
                .texture_slot = read_u16(bytes, mesh_offset + 0x02U),
                .positions_offset = read_u64(bytes, mesh_offset + 0x10U),
                .normals_offset = read_u64(bytes, mesh_offset + 0x18U),
                .uv_offset = read_u64(bytes, mesh_offset + 0x20U),
                .blend_indices_offset = read_u64(bytes, mesh_offset + 0x28U),
                .topology_control_offset = read_u64(bytes, mesh_offset + 0x30U),
                .color0_offset = read_u64(bytes, mesh_offset + 0x38U),
                .generated_workspace_relative = read_u64(bytes, mesh_offset + 0x40U),
                .generated_topology_count = read_u32(bytes, mesh_offset + 0x48U),
            };

            const auto count = static_cast<std::uint64_t>(mesh.element_count);
            const bool streams_ok =
                range_ok(bytes.size(), mesh.positions_offset, count * 12U) &&
                range_ok(bytes.size(), mesh.normals_offset, count * 12U) &&
                range_ok(bytes.size(), mesh.uv_offset, count * 4U) &&
                range_ok(bytes.size(), mesh.blend_indices_offset, count * 4U) &&
                range_ok(bytes.size(), mesh.topology_control_offset, count * 2U) &&
                range_ok(bytes.size(), mesh.color0_offset, count * 4U);
            if (!streams_ok) {
                return fail(ParseError::stream_out_of_bounds, "EFM source stream crosses EOF");
            }

            aggregate += mesh.element_count;
            object.meshes.push_back(mesh);
        }

        if (aggregate != object.aggregate_element_count) {
            return fail(
                ParseError::aggregate_count_mismatch,
                "EFM object aggregate element count differs from child mesh sum");
        }
        document.objects.push_back(std::move(object));
    }

    return ParseResult{
        .document = std::move(document),
        .error = ParseError::none,
        .message = {},
    };
}

} // namespace dmc::rengine::formats::efm
