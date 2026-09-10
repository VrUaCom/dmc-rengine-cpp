#include "dmc_rengine/formats/scm.hpp"
#include "dmc_rengine/formats/scm_edit.hpp"
#include "dmc_rengine/formats/scm_layout.hpp"
#include "dmc_rengine/formats/scm_writer.hpp"
#include "../src/cli/scm_vertex_authoring_commands.hpp"

#include <array>
#include <bit>
#include <cassert>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <limits>
#include <span>
#include <string>
#include <vector>

namespace {

template <class T>
void put(std::vector<std::byte>& bytes, std::size_t offset, T value) {
    std::memcpy(bytes.data() + offset, &value, sizeof(T));
}

void put_f32_le(std::vector<std::byte>& bytes, std::size_t offset, float value) {
    const auto raw = std::bit_cast<std::uint32_t>(value);
    bytes[offset + 0U] =
        std::byte{static_cast<unsigned char>(raw & 0xFFU)};
    bytes[offset + 1U] =
        std::byte{static_cast<unsigned char>((raw >> 8U) & 0xFFU)};
    bytes[offset + 2U] =
        std::byte{static_cast<unsigned char>((raw >> 16U) & 0xFFU)};
    bytes[offset + 3U] =
        std::byte{static_cast<unsigned char>((raw >> 24U) & 0xFFU)};
}

std::vector<std::byte> fixture(float serialized_radius = 2.0F) {
    using namespace dmc::rengine::formats::scm;

    ObjectShape shape;
    shape.mesh_vertex_counts = {3U};
    const std::vector<ObjectShape> shapes{shape};
    const auto layout = build_serialized_layout(
        std::span<const ObjectShape>{shapes}, 1U);
    std::vector<std::byte> bytes(
        static_cast<std::size_t>(layout.file_size), std::byte{0});

    bytes[0] = std::byte{'S'};
    bytes[1] = std::byte{'C'};
    bytes[2] = std::byte{'M'};
    bytes[3] = std::byte{' '};
    put<float>(bytes, 0x04U, 1.01F);
    bytes[0x10U] = std::byte{1};
    bytes[0x11U] = std::byte{1};
    bytes[0x12U] = std::byte{1};
    put<std::uint32_t>(bytes, 0x14U, 300100U);
    put<std::uint64_t>(bytes, 0x20U, layout.scene.block_offset);

    const auto& object_layout = layout.objects[0];
    const auto object_offset =
        static_cast<std::size_t>(object_layout.record_offset);
    bytes[object_offset] = std::byte{1};
    bytes[object_offset + 1U] = std::byte{0x80};
    put<std::uint16_t>(bytes, object_offset + 0x02U, 3U);
    put<std::uint64_t>(
        bytes, object_offset + 0x08U, object_layout.mesh_table_offset);
    put<float>(bytes, object_offset + 0x3CU, serialized_radius);

    const auto& mesh_layout = object_layout.meshes[0];
    const auto mesh_offset =
        static_cast<std::size_t>(mesh_layout.record_offset);
    put<std::uint16_t>(bytes, mesh_offset + 0x00U, 3U);
    put<std::uint64_t>(bytes, mesh_offset + 0x10U, mesh_layout.positions_offset);
    put<std::uint64_t>(bytes, mesh_offset + 0x18U, mesh_layout.normals_offset);
    put<std::uint64_t>(bytes, mesh_offset + 0x20U, mesh_layout.uv_offset);
    put<std::uint64_t>(bytes, mesh_offset + 0x38U, mesh_layout.color_flags_offset);
    put<std::uint64_t>(
        bytes,
        mesh_offset + 0x40U,
        mesh_layout.index_workspace_offset - mesh_layout.record_offset);
    put<std::uint16_t>(
        bytes,
        static_cast<std::size_t>(mesh_layout.index_workspace_offset),
        index_workspace_sentinel);

    const auto positions = static_cast<std::size_t>(mesh_layout.positions_offset);
    put<float>(bytes, positions + 0U, 1.0F);
    put<float>(bytes, positions + 12U, 0.0F);
    put<float>(bytes, positions + 16U, 2.0F);
    put<float>(bytes, positions + 24U, 0.0F);
    put<float>(bytes, positions + 32U, 1.5F);

    const auto scene_offset =
        static_cast<std::size_t>(layout.scene.block_offset);
    put<std::uint32_t>(bytes, scene_offset + 0x00U, layout.scene.parent_rel);
    put<std::uint32_t>(bytes, scene_offset + 0x04U, layout.scene.order_rel);
    put<std::uint32_t>(
        bytes, scene_offset + 0x08U, layout.scene.object_binding_rel);
    put<std::uint32_t>(
        bytes, scene_offset + 0x0CU, layout.scene.transform_rel);
    bytes[scene_offset + layout.scene.parent_rel] = std::byte{0xFF};
    bytes[scene_offset + layout.scene.order_rel] = std::byte{0};
    bytes[scene_offset + layout.scene.object_binding_rel] = std::byte{0};
    return bytes;
}

void write_file(
    const std::filesystem::path& path,
    std::span<const std::byte> bytes) {
    std::ofstream stream(path, std::ios::binary | std::ios::trunc);
    assert(stream);
    if (!bytes.empty()) {
        stream.write(
            reinterpret_cast<const char*>(bytes.data()),
            static_cast<std::streamsize>(bytes.size()));
    }
    assert(stream.good());
}

int run_vertex_cli(
    const std::filesystem::path& input,
    const std::filesystem::path& output) {
    std::vector<std::string> storage{
        "dmc-rengine",
        "scm-set-vertex-position",
        input.string(),
        "0",
        "0",
        "0",
        "4",
        "0",
        "0",
        output.string(),
    };
    std::array<char*, 10> argv{};
    for (std::size_t index = 0U; index < storage.size(); ++index) {
        argv[index] = storage[index].data();
    }
    return dmc::rengine::cli::try_run_scm_vertex_authoring_command(
        static_cast<int>(argv.size()), argv.data());
}

} // namespace

int main() {
    using namespace dmc::rengine::formats::scm;

    const auto source = fixture();
    const auto parsed = Parser::parse(std::span<const std::byte>{source});
    assert(parsed.ok());
    assert(parsed.document.objects.size() == 1U);
    assert(parsed.document.objects[0].meshes.size() == 1U);
    assert(parsed.document.objects[0].meshes[0].positions.size() == 3U);
    assert(parsed.document.objects[0].bounding_radius == 2.0F);

    const auto source_position =
        parsed.document.objects[0].meshes[0].positions[0];
    assert(source_position.x == 1.0F);
    assert(source_position.y == 0.0F);
    assert(source_position.z == 0.0F);

    auto authored_document = parsed.document;
    const auto edit = set_vertex_position(
        authored_document, 0U, 0U, 0U, Vec3f{4.0F, 0.0F, 0.0F});
    assert(edit.ok());
    assert(edit.changed);

    const auto authored = Writer::write(
        authored_document, WriteMode::preserve_layout);
    assert(authored.ok());
    assert(authored.bytes.size() == source.size());

    const auto position_offset = static_cast<std::size_t>(
        parsed.document.objects[0].meshes[0].positions_offset);
    const auto radius_offset = static_cast<std::size_t>(
        parsed.document.objects[0].record_offset + 0x3CU);
    auto expected = source;
    put_f32_le(expected, position_offset + 0U, 4.0F);
    put_f32_le(expected, position_offset + 4U, 0.0F);
    put_f32_le(expected, position_offset + 8U, 0.0F);
    put_f32_le(expected, radius_offset, 4.0F);
    assert(authored.bytes == expected);

    bool position_byte_changed = false;
    bool radius_byte_changed = false;
    for (std::size_t index = 0U; index < source.size(); ++index) {
        if (source[index] == authored.bytes[index]) continue;
        const bool in_position =
            index >= position_offset && index < position_offset + 12U;
        const bool in_radius =
            index >= radius_offset && index < radius_offset + 4U;
        assert(in_position || in_radius);
        position_byte_changed = position_byte_changed || in_position;
        radius_byte_changed = radius_byte_changed || in_radius;
    }
    assert(position_byte_changed);
    assert(radius_byte_changed);

    const auto authored_parse = Parser::parse(
        std::span<const std::byte>{authored.bytes});
    assert(authored_parse.ok());
    assert(
        authored_parse.document.objects[0].meshes[0].positions[0].x == 4.0F);
    assert(
        authored_parse.document.objects[0].meshes[0].positions[0].y == 0.0F);
    assert(
        authored_parse.document.objects[0].meshes[0].positions[0].z == 0.0F);
    assert(authored_parse.document.objects[0].bounding_radius == 4.0F);

    auto inverse_document = authored_parse.document;
    const auto inverse_edit = set_vertex_position(
        inverse_document, 0U, 0U, 0U, source_position);
    assert(inverse_edit.ok());
    assert(inverse_edit.changed);
    const auto inverse = Writer::write(
        inverse_document, WriteMode::preserve_layout);
    assert(inverse.ok());
    assert(inverse.bytes == source);

    auto invalid_document = parsed.document;
    const auto nan = std::bit_cast<float>(0x7FC00000U);
    const auto invalid_edit = set_vertex_position(
        invalid_document, 0U, 0U, 0U, Vec3f{nan, 0.0F, 0.0F});
    assert(!invalid_edit.ok());
    assert(!invalid_edit.changed);

    const auto out_of_range = set_vertex_position(
        invalid_document,
        0U,
        0U,
        std::numeric_limits<std::size_t>::max(),
        Vec3f{1.0F, 2.0F, 3.0F});
    assert(!out_of_range.ok());
    assert(!out_of_range.changed);

    const auto test_root =
        std::filesystem::temp_directory_path() /
        "dmc-rengine-scm-vertex-authoring-tests";
    std::error_code cleanup_error;
    std::filesystem::remove_all(test_root, cleanup_error);
    assert(std::filesystem::create_directories(test_root));

    const auto mismatch_input = test_root / "radius-mismatch.scm";
    const auto mismatch_output = test_root / "radius-mismatch-authored.scm";
    const auto one_ulp_above_two = std::bit_cast<float>(
        std::bit_cast<std::uint32_t>(2.0F) + 1U);
    const auto mismatch_source = fixture(one_ulp_above_two);
    write_file(mismatch_input, std::span<const std::byte>{mismatch_source});
    assert(run_vertex_cli(mismatch_input, mismatch_output) == 6);
    assert(!std::filesystem::exists(mismatch_output));

    const auto valid_input = test_root / "valid.scm";
    const auto valid_output = test_root / "valid-authored.scm";
    write_file(valid_input, std::span<const std::byte>{source});
    assert(run_vertex_cli(valid_input, valid_output) == 0);

    std::vector<std::byte> cli_output;
    assert(dmc::rengine::cli::scm_authoring_detail::read_file(
        valid_output, cli_output));
    assert(cli_output == expected);

    const auto first_output = cli_output;
    assert(run_vertex_cli(valid_input, valid_output) == 17);
    cli_output.clear();
    assert(dmc::rengine::cli::scm_authoring_detail::read_file(
        valid_output, cli_output));
    assert(cli_output == first_output);

    std::filesystem::remove_all(test_root, cleanup_error);
    return 0;
}
