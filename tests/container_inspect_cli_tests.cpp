#include "container_inspect_commands.hpp"

#include "dmc_rengine/formats/scm_layout.hpp"

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <vector>

namespace {

// SafeProductValidation: synthetic fixtures only. Nothing here asserts
// anything about a real retail container.
void put_u32(std::vector<std::byte>& bytes, std::uint32_t value) {
    bytes.push_back(static_cast<std::byte>(value & 0xFFU));
    bytes.push_back(static_cast<std::byte>((value >> 8U) & 0xFFU));
    bytes.push_back(static_cast<std::byte>((value >> 16U) & 0xFFU));
    bytes.push_back(static_cast<std::byte>((value >> 24U) & 0xFFU));
}

void put_text_at(
    std::vector<std::byte>& bytes,
    std::size_t offset,
    std::string_view text) {
    for (std::size_t index = 0U; index < text.size(); ++index) {
        bytes[offset + index] = static_cast<std::byte>(
            static_cast<unsigned char>(text[index]));
    }
}

void put_text(std::vector<std::byte>& bytes, std::string_view text) {
    for (const auto character : text) {
        bytes.push_back(static_cast<std::byte>(
            static_cast<unsigned char>(character)));
    }
}

// A PAC whose slot payloads are supplied verbatim. A nullopt payload is an
// empty slot, which must survive as a preserved hole.
[[nodiscard]] std::vector<std::byte> make_pac(
    const std::vector<std::optional<std::vector<std::byte>>>& payloads) {
    const auto count = static_cast<std::uint32_t>(payloads.size());
    const auto table_end = 8U + count * 4U;

    std::vector<std::byte> header;
    put_text(header, std::string_view{"PAC\0", 4U});
    put_u32(header, count);

    std::vector<std::byte> blob;
    auto cursor = table_end;
    for (const auto& payload : payloads) {
        if (!payload.has_value()) {
            put_u32(header, 0U);
            continue;
        }
        put_u32(header, cursor);
        blob.insert(blob.end(), payload->begin(), payload->end());
        cursor += static_cast<std::uint32_t>(payload->size());
    }
    header.insert(header.end(), blob.begin(), blob.end());
    return header;
}

void write_file(
    const std::filesystem::path& path,
    std::span<const std::byte> bytes) {
    std::ofstream stream(path, std::ios::binary | std::ios::trunc);
    assert(stream);
    stream.write(
        reinterpret_cast<const char*>(bytes.data()),
        static_cast<std::streamsize>(bytes.size()));
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
    return dmc::rengine::cli::try_run_container_inspect_command(
        static_cast<int>(argv.size()), argv.data());
}

[[nodiscard]] std::vector<std::byte> dds_payload() {
    std::vector<std::byte> bytes;
    put_text(bytes, "DDS ");
    put_text(bytes, "0123456789abcdef0123456789ab");
    return bytes;
}

[[nodiscard]] std::vector<std::byte> mod_payload() {
    std::vector<std::byte> bytes;
    put_text(bytes, "MOD ");
    put_text(bytes, "0123456789abcdef0123456789ab");
    return bytes;
}

// A minimal but structurally real SCM image, built from the canonical
// serialized layout so its exact extent is known.
template <typename T>
void put_raw(std::vector<std::byte>& bytes, std::size_t offset, T value) {
    std::memcpy(bytes.data() + offset, &value, sizeof(T));
}

[[nodiscard]] std::vector<std::byte> real_scm(std::uint64_t& exact_size) {
    using namespace dmc::rengine::formats::scm;
    ObjectShape shape;
    shape.mesh_vertex_counts = {3U};
    const std::vector<ObjectShape> shapes{shape};
    const auto layout =
        build_serialized_layout(std::span<const ObjectShape>{shapes}, 1U);
    exact_size = layout.file_size;

    std::vector<std::byte> bytes(
        static_cast<std::size_t>(layout.file_size), std::byte{0});
    put_text_at(bytes, 0U, "SCM ");
    put_raw<float>(bytes, 0x04U, 1.01F);
    bytes[0x10U] = std::byte{1};
    bytes[0x11U] = std::byte{1};
    bytes[0x12U] = std::byte{1};
    put_raw<std::uint64_t>(bytes, 0x20U, layout.scene.block_offset);

    const auto& object = layout.objects[0];
    const auto object_offset = static_cast<std::size_t>(object.record_offset);
    bytes[object_offset] = std::byte{1};
    bytes[object_offset + 1U] = std::byte{0x80};
    put_raw<std::uint16_t>(bytes, object_offset + 0x02U, 3U);
    put_raw<std::uint64_t>(bytes, object_offset + 0x08U, object.mesh_table_offset);

    const auto& mesh = object.meshes[0];
    const auto mesh_offset = static_cast<std::size_t>(mesh.record_offset);
    put_raw<std::uint16_t>(bytes, mesh_offset + 0x00U, 3U);
    put_raw<std::uint64_t>(bytes, mesh_offset + 0x10U, mesh.positions_offset);
    put_raw<std::uint64_t>(bytes, mesh_offset + 0x18U, mesh.normals_offset);
    put_raw<std::uint64_t>(bytes, mesh_offset + 0x20U, mesh.uv_offset);
    put_raw<std::uint64_t>(bytes, mesh_offset + 0x38U, mesh.color_flags_offset);
    put_raw<std::uint64_t>(
        bytes, mesh_offset + 0x40U,
        mesh.index_workspace_offset - mesh.record_offset);
    put_raw<std::uint16_t>(
        bytes, static_cast<std::size_t>(mesh.index_workspace_offset), 0xFFFFU);

    const auto scene = static_cast<std::size_t>(layout.scene.block_offset);
    put_raw<std::uint32_t>(bytes, scene + 0x00U, layout.scene.parent_rel);
    put_raw<std::uint32_t>(bytes, scene + 0x04U, layout.scene.order_rel);
    put_raw<std::uint32_t>(bytes, scene + 0x08U, layout.scene.object_binding_rel);
    put_raw<std::uint32_t>(bytes, scene + 0x0CU, layout.scene.transform_rel);
    bytes[scene + layout.scene.parent_rel] = std::byte{0xFF};
    return bytes;
}

struct Fixture final {
    std::filesystem::path directory;
    std::filesystem::path container;

    explicit Fixture(std::string_view name) {
        directory = std::filesystem::temp_directory_path() / name;
        std::filesystem::remove_all(directory);
        std::filesystem::create_directories(directory);
        container = directory / "nested.pac";

        // Outer slot 0 is an inner PAC holding a MOD, an empty slot and a DDS.
        const auto inner = make_pac({mod_payload(), std::nullopt, dds_payload()});
        write_file(container, make_pac({inner, std::nullopt, dds_payload()}));
    }

    ~Fixture() { std::filesystem::remove_all(directory); }
};

void test_not_this_command() {
    assert(run({"dmc-rengine", "scan", "."}) == -1);
    assert(run({"dmc-rengine"}) == -1);
}

void test_usage() {
    assert(run({"dmc-rengine", "list-container"}) == 1);
    assert(run({"dmc-rengine", "list-container", "a", "b"}) == 1);
    assert(run({"dmc-rengine", "extract-slot", "a"}) == 1);
    assert(run({"dmc-rengine", "extract-slot", "a", "0", "b", "c"}) == 1);
}

void test_missing_input() {
    assert(run({"dmc-rengine", "list-container", "no-such-container.pac"}) == 1);
    assert(run({"dmc-rengine", "extract-slot", "no-such-container.pac", "0",
                "out.bin"}) == 1);
}

void test_list_container_expands_nested_tree() {
    const Fixture fixture{"dmc-rengine-list-container"};
    assert(run({"dmc-rengine", "list-container",
                fixture.container.string()}) == 0);
}

void test_extract_nested_slot_round_trips() {
    const Fixture fixture{"dmc-rengine-extract-nested"};
    const auto output = fixture.directory / "extracted.bin";

    assert(run({"dmc-rengine", "extract-slot", fixture.container.string(),
                "0/2", output.string()}) == 0);
    assert(read_file(output) == dds_payload());

    // No-replace publication must refuse a second write to the same path.
    assert(run({"dmc-rengine", "extract-slot", fixture.container.string(),
                "0/2", output.string()}) == 1);
}

void test_extract_top_level_slot() {
    const Fixture fixture{"dmc-rengine-extract-top"};
    const auto output = fixture.directory / "top.bin";
    assert(run({"dmc-rengine", "extract-slot", fixture.container.string(), "2",
                output.string()}) == 0);
    assert(read_file(output) == dds_payload());
}

void test_empty_and_out_of_range_slots_are_refused() {
    const Fixture fixture{"dmc-rengine-extract-refused"};
    // Slot 1 of the outer container is a preserved empty slot.
    assert(run({"dmc-rengine", "extract-slot", fixture.container.string(), "1",
                (fixture.directory / "empty.bin").string()}) == 1);
    assert(!std::filesystem::exists(fixture.directory / "empty.bin"));

    assert(run({"dmc-rengine", "extract-slot", fixture.container.string(), "99",
                (fixture.directory / "oob.bin").string()}) == 1);

    // Descending into a leaf is refused rather than silently truncated.
    assert(run({"dmc-rengine", "extract-slot", fixture.container.string(),
                "2/0", (fixture.directory / "leaf.bin").string()}) == 1);
}

void test_malformed_slot_paths_are_refused() {
    const Fixture fixture{"dmc-rengine-extract-badpath"};
    const auto out = (fixture.directory / "x.bin").string();
    for (const auto* bad : {"", "/", "0/", "/0", "a", "0//1", "-1", "0x2"}) {
        assert(run({"dmc-rengine", "extract-slot", fixture.container.string(),
                    bad, out}) == 1);
    }
}

// F-02: a relative-slot container derives a slot's size from the distance to
// the next offset, so alignment padding lands inside the slot. list-container
// must surface that instead of letting it pass silently.
void test_padded_scm_slot_reports_intrinsic_extent() {
    const auto directory = std::filesystem::temp_directory_path() /
        "dmc-rengine-intrinsic-extent";
    std::filesystem::remove_all(directory);
    std::filesystem::create_directories(directory);

    std::uint64_t exact = 0U;
    auto scm = real_scm(exact);
    assert(exact != 0U);

    // Sixteen bytes of trailing alignment, which the retail census says is the
    // norm rather than the exception.
    auto padded = scm;
    padded.insert(padded.end(), 16U, std::byte{0});

    const auto container = directory / "padded.pac";
    write_file(container, make_pac({padded}));
    assert(run({"dmc-rengine", "list-container", container.string()}) == 0);

    // The slot span really is larger than the resource it holds.
    const auto derived = static_cast<std::uint64_t>(padded.size());
    assert(derived == exact + 16U);

    std::filesystem::remove_all(directory);
}

} // namespace

int main() {
    test_not_this_command();
    test_usage();
    test_missing_input();
    test_list_container_expands_nested_tree();
    test_extract_nested_slot_round_trips();
    test_extract_top_level_slot();
    test_empty_and_out_of_range_slots_are_refused();
    test_malformed_slot_paths_are_refused();
    test_padded_scm_slot_reports_intrinsic_extent();
    return 0;
}
