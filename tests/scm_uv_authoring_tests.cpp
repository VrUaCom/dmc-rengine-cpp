#include "dmc_rengine/formats/scm.hpp"
#include "dmc_rengine/formats/scm_edit.hpp"
#include "dmc_rengine/formats/scm_layout.hpp"
#include "dmc_rengine/formats/scm_writer.hpp"
#include "../src/cli/scm_uv_authoring_commands.hpp"

#include <array>
#include <cassert>
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

void put_i16_le(
    std::vector<std::byte>& bytes,
    std::size_t offset,
    std::int16_t value) {
    const auto raw = static_cast<std::uint16_t>(value);
    bytes[offset + 0U] = std::byte{static_cast<unsigned char>(raw & 0xFFU)};
    bytes[offset + 1U] =
        std::byte{static_cast<unsigned char>((raw >> 8U) & 0xFFU)};
}

std::vector<std::byte> fixture() {
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
    const auto object_offset = static_cast<std::size_t>(object_layout.record_offset);
    bytes[object_offset] = std::byte{1};
    bytes[object_offset + 1U] = std::byte{0x80};
    put<std::uint16_t>(bytes, object_offset + 0x02U, 3U);
    put<std::uint64_t>(bytes, object_offset + 0x08U, object_layout.mesh_table_offset);
    put<float>(bytes, object_offset + 0x3CU, 2.0F);

    const auto& mesh_layout = object_layout.meshes[0];
    const auto mesh_offset = static_cast<std::size_t>(mesh_layout.record_offset);
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

    const auto scene_offset = static_cast<std::size_t>(layout.scene.block_offset);
    put<std::uint32_t>(bytes, scene_offset + 0x00U, layout.scene.parent_rel);
    put<std::uint32_t>(bytes, scene_offset + 0x04U, layout.scene.order_rel);
    put<std::uint32_t>(bytes, scene_offset + 0x08U, layout.scene.object_binding_rel);
    put<std::uint32_t>(bytes, scene_offset + 0x0CU, layout.scene.transform_rel);
    bytes[scene_offset + layout.scene.parent_rel] = std::byte{0xFF};
    bytes[scene_offset + layout.scene.order_rel] = std::byte{0};
    bytes[scene_offset + layout.scene.object_binding_rel] = std::byte{0};
    return bytes;
}

void write_file(const std::filesystem::path& path, std::span<const std::byte> bytes) {
    std::ofstream stream(path, std::ios::binary | std::ios::trunc);
    assert(stream);
    if (!bytes.empty()) {
        stream.write(
            reinterpret_cast<const char*>(bytes.data()),
            static_cast<std::streamsize>(bytes.size()));
    }
    assert(stream.good());
}

int run_uv_cli(
    const std::filesystem::path& input,
    const std::filesystem::path& output,
    const std::string& u = "0.5",
    const std::string& v = "-0.25") {
    std::vector<std::string> storage{
        "dmc-rengine",
        "scm-set-vertex-uv",
        input.string(),
        "0",
        "0",
        "0",
        u,
        v,
        output.string(),
    };
    std::array<char*, 9> argv{};
    for (std::size_t index = 0U; index < storage.size(); ++index) {
        argv[index] = storage[index].data();
    }
    return dmc::rengine::cli::try_run_scm_uv_authoring_command(
        static_cast<int>(argv.size()), argv.data());
}

} // namespace

int main() {
    using namespace dmc::rengine::formats::scm;
    using dmc::rengine::cli::scm_uv_authoring_detail::encode_component_guard;

    // Independent exact-value and boundary checks for signed int16 / 4096.
    const auto positive_half_step = encode_component_guard(1.0F / 8192.0F);
    const auto negative_half_step = encode_component_guard(-1.0F / 8192.0F);
    const auto maximum = encode_component_guard(32767.0F / 4096.0F);
    const auto minimum = encode_component_guard(-8.0F);
    assert(positive_half_step.has_value() && *positive_half_step == 1);
    assert(negative_half_step.has_value() && *negative_half_step == -1);
    assert(maximum.has_value() && *maximum == 32767);
    assert(minimum.has_value() && *minimum == -32768);
    assert(!encode_component_guard(8.0F).has_value());
    assert(!encode_component_guard(-32769.0F / 4096.0F).has_value());

    const auto source = fixture();
    const auto parsed = Parser::parse(std::span<const std::byte>{source});
    assert(parsed.ok());
    const auto source_uv = parsed.document.objects[0].meshes[0].uvs[0];
    assert(source_uv.u == 0);
    assert(source_uv.v == 0);

    auto authored_document = parsed.document;
    const auto edit = set_uv(
        authored_document, 0U, 0U, 0U, 0.5F, -0.25F);
    assert(edit.ok());
    assert(edit.changed);
    const auto authored_raw = authored_document.objects[0].meshes[0].uvs[0];
    assert(authored_raw.u == 2048);
    assert(authored_raw.v == -1024);

    const auto authored = Writer::write(
        authored_document, WriteMode::preserve_layout);
    assert(authored.ok());
    assert(authored.bytes.size() == source.size());

    const auto uv_offset = static_cast<std::size_t>(
        parsed.document.objects[0].meshes[0].uv_offset);
    auto expected = source;
    put_i16_le(expected, uv_offset + 0U, 2048);
    put_i16_le(expected, uv_offset + 2U, -1024);
    assert(authored.bytes == expected);

    bool changed = false;
    for (std::size_t index = 0U; index < source.size(); ++index) {
        if (source[index] == authored.bytes[index]) continue;
        changed = true;
        assert(index >= uv_offset && index < uv_offset + 4U);
    }
    assert(changed);

    const auto authored_parse = Parser::parse(
        std::span<const std::byte>{authored.bytes});
    assert(authored_parse.ok());
    const auto reparsed_uv = authored_parse.document.objects[0].meshes[0].uvs[0];
    assert(reparsed_uv.u == 2048);
    assert(reparsed_uv.v == -1024);
    assert(authored_parse.document.objects[0].bounding_radius == 2.0F);

    auto inverse_document = authored_parse.document;
    const auto inverse_edit = set_uv_raw(
        inverse_document, 0U, 0U, 0U, source_uv);
    assert(inverse_edit.ok());
    assert(inverse_edit.changed);
    const auto inverse = Writer::write(
        inverse_document, WriteMode::preserve_layout);
    assert(inverse.ok());
    assert(inverse.bytes == source);

    auto boundary_document = parsed.document;
    const auto positive_round = set_uv(
        boundary_document, 0U, 0U, 0U, 1.0F / 8192.0F, -1.0F / 8192.0F);
    assert(positive_round.ok());
    assert(positive_round.changed);
    assert(boundary_document.objects[0].meshes[0].uvs[0].u == 1);
    assert(boundary_document.objects[0].meshes[0].uvs[0].v == -1);

    auto extremes_document = parsed.document;
    const auto extremes = set_uv(
        extremes_document,
        0U,
        0U,
        0U,
        32767.0F / 4096.0F,
        -8.0F);
    assert(extremes.ok());
    assert(extremes.changed);
    assert(extremes_document.objects[0].meshes[0].uvs[0].u == 32767);
    assert(extremes_document.objects[0].meshes[0].uvs[0].v == -32768);

    auto invalid_document = parsed.document;
    const auto positive_overflow = set_uv(
        invalid_document, 0U, 0U, 0U, 8.0F, 0.0F);
    assert(!positive_overflow.ok());
    assert(!positive_overflow.changed);
    const auto negative_overflow = set_uv(
        invalid_document, 0U, 0U, 0U, -32769.0F / 4096.0F, 0.0F);
    assert(!negative_overflow.ok());
    assert(!negative_overflow.changed);
    const auto out_of_range = set_uv(
        invalid_document,
        0U,
        0U,
        std::numeric_limits<std::size_t>::max(),
        0.5F,
        -0.25F);
    assert(!out_of_range.ok());
    assert(!out_of_range.changed);

    const auto test_root =
        std::filesystem::temp_directory_path() /
        "dmc-rengine-scm-uv-authoring-tests";
    std::error_code cleanup_error;
    std::filesystem::remove_all(test_root, cleanup_error);
    assert(std::filesystem::create_directories(test_root));

    const auto valid_input = test_root / "valid.scm";
    const auto valid_output = test_root / "valid-authored.scm";
    write_file(valid_input, std::span<const std::byte>{source});
    assert(run_uv_cli(valid_input, valid_output) == 0);

    std::vector<std::byte> cli_output;
    assert(dmc::rengine::cli::scm_authoring_detail::read_file(
        valid_output, cli_output));
    assert(cli_output == expected);

    // A representable value that quantizes back to raw (0, 0) is a no-op.
    const auto no_op_output = test_root / "noop.scm";
    assert(run_uv_cli(valid_input, no_op_output, "0", "0") == 7);
    assert(!std::filesystem::exists(no_op_output));

    const auto overflow_output = test_root / "overflow.scm";
    assert(run_uv_cli(valid_input, overflow_output, "8", "0") == 3);
    assert(!std::filesystem::exists(overflow_output));

    const auto first_output = cli_output;
    assert(run_uv_cli(valid_input, valid_output) == 17);
    cli_output.clear();
    assert(dmc::rengine::cli::scm_authoring_detail::read_file(
        valid_output, cli_output));
    assert(cli_output == first_output);

    std::filesystem::remove_all(test_root, cleanup_error);
    return 0;
}
