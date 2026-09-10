#include "scm_authoring_commands.hpp"

#include "dmc_rengine/formats/scm.hpp"
#include "dmc_rengine/formats/scm_layout.hpp"

#include <cassert>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <span>
#include <string>
#include <vector>

namespace {

template <class T>
void put(std::vector<std::byte>& bytes, std::size_t offset, T value) {
    std::memcpy(bytes.data() + offset, &value, sizeof(T));
}

[[nodiscard]] std::vector<std::byte> fixture() {
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
    put<std::uint64_t>(bytes, 0x20U, layout.scene.block_offset);

    const auto& object = layout.objects[0];
    const auto object_offset = static_cast<std::size_t>(object.record_offset);
    bytes[object_offset] = std::byte{1};
    bytes[object_offset + 1U] = std::byte{0x80};
    put<std::uint16_t>(bytes, object_offset + 0x02U, 3U);
    put<std::uint64_t>(bytes, object_offset + 0x08U, object.mesh_table_offset);

    const auto& mesh = object.meshes[0];
    const auto mesh_offset = static_cast<std::size_t>(mesh.record_offset);
    put<std::uint16_t>(bytes, mesh_offset + 0x00U, 3U);
    put<std::uint64_t>(bytes, mesh_offset + 0x10U, mesh.positions_offset);
    put<std::uint64_t>(bytes, mesh_offset + 0x18U, mesh.normals_offset);
    put<std::uint64_t>(bytes, mesh_offset + 0x20U, mesh.uv_offset);
    put<std::uint64_t>(bytes, mesh_offset + 0x38U, mesh.color_flags_offset);
    put<std::uint64_t>(
        bytes,
        mesh_offset + 0x40U,
        mesh.index_workspace_offset - mesh.record_offset);
    put<std::uint16_t>(
        bytes,
        static_cast<std::size_t>(mesh.index_workspace_offset),
        index_workspace_sentinel);

    const auto scene = static_cast<std::size_t>(layout.scene.block_offset);
    put<std::uint32_t>(bytes, scene + 0x00U, layout.scene.parent_rel);
    put<std::uint32_t>(bytes, scene + 0x04U, layout.scene.order_rel);
    put<std::uint32_t>(bytes, scene + 0x08U, layout.scene.object_binding_rel);
    put<std::uint32_t>(bytes, scene + 0x0CU, layout.scene.transform_rel);
    bytes[scene + layout.scene.parent_rel] = std::byte{0xFF};
    bytes[scene + layout.scene.order_rel] = std::byte{0};
    bytes[scene + layout.scene.object_binding_rel] = std::byte{0};

    const auto transform = scene + layout.scene.transform_rel;
    put<float>(bytes, transform + 0x00U, 0.0F);
    put<float>(bytes, transform + 0x04U, -27.5F);
    put<float>(bytes, transform + 0x08U, 0.0F);
    put<float>(bytes, transform + 0x0CU, 27.5F);
    put<float>(bytes, transform + 0x10U, 0.1F);
    put<float>(bytes, transform + 0x14U, 0.2F);
    put<float>(bytes, transform + 0x18U, 0.3F);
    put<float>(bytes, transform + 0x1CU, 0.125F);
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

[[nodiscard]] std::vector<std::byte> read_file(
    const std::filesystem::path& path) {
    std::ifstream stream(path, std::ios::binary | std::ios::ate);
    assert(stream);
    const auto end = stream.tellg();
    assert(end >= 0);
    std::vector<std::byte> bytes(static_cast<std::size_t>(end));
    stream.seekg(0, std::ios::beg);
    if (!bytes.empty()) {
        stream.read(
            reinterpret_cast<char*>(bytes.data()),
            static_cast<std::streamsize>(bytes.size()));
        assert(stream.good());
    }
    return bytes;
}

[[nodiscard]] int run(const std::vector<std::string>& arguments) {
    std::vector<char*> argv;
    argv.reserve(arguments.size());
    for (const auto& argument : arguments) {
        argv.push_back(const_cast<char*>(argument.c_str()));
    }
    return dmc::rengine::cli::try_run_scm_authoring_command(
        static_cast<int>(argv.size()), argv.data());
}

} // namespace

int main() {
    using dmc::rengine::formats::scm::Parser;
    using dmc::rengine::formats::scm::scene_transform_size;

    const auto root =
        std::filesystem::temp_directory_path() / "dmc-rengine-scm-authoring-cli";
    std::error_code error;
    std::filesystem::remove_all(root, error);
    std::filesystem::create_directories(root, error);
    assert(!error);

    const auto original_cwd = std::filesystem::current_path();
    std::filesystem::current_path(root);

    const auto source = fixture();
    write_file("source.scm", source);
    const auto source_parse = Parser::parse(std::span<const std::byte>{source});
    assert(source_parse.ok());
    const auto transform_offset = static_cast<std::size_t>(
        source_parse.document.scene_nodes.offset +
        source_parse.document.scene_nodes.transform_rel);
    assert(scene_transform_size == 0x20U);

    assert(run({
        "dmc-rengine",
        "scm-set-alpha-control",
        "source.scm",
        "0",
        "64",
        "alpha.scm"}) == 0);
    const auto alpha = read_file("alpha.scm");
    std::size_t alpha_changed = 0U;
    std::size_t alpha_offset = 0U;
    for (std::size_t index = 0U; index < source.size(); ++index) {
        if (source[index] != alpha[index]) {
            ++alpha_changed;
            alpha_offset = index;
        }
    }
    assert(alpha_changed == 1U);
    assert(alpha_offset == 0x41U);

    assert(run({
        "dmc-rengine",
        "scm-set-node-translation",
        "source.scm",
        "0",
        "1",
        "-27.5",
        "0",
        "translated.scm"}) == 0);
    const auto translated = read_file("translated.scm");
    assert(translated.size() == source.size());

    std::size_t translation_changed = 0U;
    for (std::size_t index = 0U; index < source.size(); ++index) {
        if (source[index] == translated[index]) continue;
        ++translation_changed;
        assert(index >= transform_offset);
        assert(index < transform_offset + 0x10U);
    }
    assert(translation_changed > 0U);

    const auto translated_parse =
        Parser::parse(std::span<const std::byte>{translated});
    assert(translated_parse.ok());
    const auto& transform =
        translated_parse.document.scene_nodes.transform_by_node_index[0];
    assert(transform.translation.x == 1.0F);
    assert(transform.translation.y == -27.5F);
    assert(transform.translation.z == 0.0F);
    assert(transform.translation_magnitude ==
           std::sqrt(1.0F + 27.5F * 27.5F));
    assert(transform.rotation_xyz_radians.x == 0.1F);
    assert(transform.rotation_xyz_radians.y == 0.2F);
    assert(transform.rotation_xyz_radians.z == 0.3F);
    assert(transform.reserved1c == 0.125F);

    const auto before_repeat = translated;
    assert(run({
        "dmc-rengine",
        "scm-set-node-translation",
        "source.scm",
        "0",
        "2",
        "-27.5",
        "0",
        "translated.scm"}) != 0);
    assert(read_file("translated.scm") == before_repeat);

    assert(run({
        "dmc-rengine",
        "scm-set-node-translation",
        "source.scm",
        "0",
        "nan",
        "0",
        "0",
        "invalid-nan.scm"}) != 0);
    assert(!std::filesystem::exists("invalid-nan.scm"));

    assert(run({
        "dmc-rengine",
        "scm-set-node-translation",
        "source.scm",
        "9",
        "1",
        "2",
        "3",
        "invalid-node.scm"}) != 0);
    assert(!std::filesystem::exists("invalid-node.scm"));

    std::filesystem::current_path(original_cwd);
    std::filesystem::remove_all(root, error);
    return 0;
}
