#include "scm_authoring_commands.hpp"

#include "dmc_rengine/formats/scm.hpp"
#include "dmc_rengine/formats/scm_layout.hpp"

#include <cassert>
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

    assert(run({
        "dmc-rengine",
        "scm-set-alpha-control",
        "source.scm",
        "0",
        "64",
        "edited.scm"}) == 0);

    const auto edited = read_file("edited.scm");
    assert(edited.size() == source.size());
    std::size_t changed_count = 0U;
    std::size_t changed_offset = 0U;
    for (std::size_t index = 0U; index < source.size(); ++index) {
        if (source[index] != edited[index]) {
            ++changed_count;
            changed_offset = index;
        }
    }
    assert(changed_count == 1U);
    assert(changed_offset == 0x41U);
    assert(source[0x41U] == std::byte{0x80});
    assert(edited[0x41U] == std::byte{0x40});

    const auto parsed = Parser::parse(std::span<const std::byte>{edited});
    assert(parsed.ok());
    assert(parsed.document.objects.size() == 1U);
    assert(parsed.document.objects[0].alpha_control == 0x40U);

    const auto before_repeat = edited;
    assert(run({
        "dmc-rengine",
        "scm-set-alpha-control",
        "source.scm",
        "0",
        "32",
        "edited.scm"}) != 0);
    assert(read_file("edited.scm") == before_repeat);

    assert(run({
        "dmc-rengine",
        "scm-set-alpha-control",
        "source.scm",
        "0",
        "128",
        "same.scm"}) != 0);
    assert(!std::filesystem::exists("same.scm"));

    assert(run({
        "dmc-rengine",
        "scm-set-alpha-control",
        "source.scm",
        "9",
        "64",
        "invalid.scm"}) != 0);
    assert(!std::filesystem::exists("invalid.scm"));

    std::filesystem::current_path(original_cwd);
    std::filesystem::remove_all(root, error);
    return 0;
}
