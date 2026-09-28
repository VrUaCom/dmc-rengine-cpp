#include "relative_slot_commands.hpp"
#include "scm_reintegration_commands.hpp"

#include "dmc_rengine/formats/scm_layout.hpp"
#include "dmc_rengine/gdspaces/container_expander.hpp"
#include "dmc_rengine/gdspaces/resource_payload.hpp"
#include "dmc_rengine/profiles/dmc3/container_parsers.hpp"

#include <algorithm>
#include <array>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <span>
#include <string>
#include <string_view>
#include <vector>

namespace {

void put_u16(std::vector<std::byte>& bytes, std::size_t offset, std::uint16_t value) {
    bytes[offset + 0U] = static_cast<std::byte>(value & 0xFFU);
    bytes[offset + 1U] = static_cast<std::byte>((value >> 8U) & 0xFFU);
}

void put_u32(std::vector<std::byte>& bytes, std::size_t offset, std::uint32_t value) {
    bytes[offset + 0U] = static_cast<std::byte>(value & 0xFFU);
    bytes[offset + 1U] = static_cast<std::byte>((value >> 8U) & 0xFFU);
    bytes[offset + 2U] = static_cast<std::byte>((value >> 16U) & 0xFFU);
    bytes[offset + 3U] = static_cast<std::byte>((value >> 24U) & 0xFFU);
}

void put_u64(std::vector<std::byte>& bytes, std::size_t offset, std::uint64_t value) {
    for (std::size_t index = 0U; index < 8U; ++index) {
        bytes[offset + index] = static_cast<std::byte>(
            (value >> static_cast<unsigned>(index * 8U)) & 0xFFU);
    }
}

void put_f32(std::vector<std::byte>& bytes, std::size_t offset, float value) {
    static_assert(sizeof(float) == sizeof(std::uint32_t));
    std::uint32_t raw{};
    std::memcpy(&raw, &value, sizeof(raw));
    put_u32(bytes, offset, raw);
}

void write_file(const std::filesystem::path& path, std::span<const std::byte> bytes) {
    std::ofstream stream(path, std::ios::binary | std::ios::trunc);
    assert(stream);
    stream.write(reinterpret_cast<const char*>(bytes.data()), static_cast<std::streamsize>(bytes.size()));
    assert(stream.good());
}

[[nodiscard]] std::vector<std::byte> read_file(const std::filesystem::path& path) {
    std::ifstream stream(path, std::ios::binary | std::ios::ate);
    assert(stream);
    const auto end = stream.tellg();
    assert(end >= 0);
    std::vector<std::byte> bytes(static_cast<std::size_t>(end));
    stream.seekg(0, std::ios::beg);
    if (!bytes.empty()) {
        stream.read(reinterpret_cast<char*>(bytes.data()), static_cast<std::streamsize>(bytes.size()));
        assert(stream.good());
    }
    return bytes;
}

[[nodiscard]] std::vector<std::byte> parent_pac() {
    std::vector<std::byte> bytes(0x60U, std::byte{0});
    bytes[0] = std::byte{'P'};
    bytes[1] = std::byte{'A'};
    bytes[2] = std::byte{'C'};
    bytes[3] = std::byte{0};
    put_u32(bytes, 4U, 2U);
    put_u32(bytes, 8U, 0x20U);
    put_u32(bytes, 12U, 0x40U);
    for (std::size_t index = 0x10U; index < 0x20U; ++index) {
        bytes[index] = static_cast<std::byte>(0xA0U + index - 0x10U);
    }
    for (std::size_t index = 0x20U; index < 0x40U; ++index) {
        bytes[index] = static_cast<std::byte>(0x20U ^ index);
    }
    for (std::size_t index = 0x40U; index < bytes.size(); ++index) {
        bytes[index] = static_cast<std::byte>(0xC0U ^ index);
    }
    return bytes;
}

[[nodiscard]] std::vector<std::byte> inner_pnst() {
    std::vector<std::byte> bytes(0x50U, std::byte{0});
    bytes[0] = std::byte{'P'};
    bytes[1] = std::byte{'N'};
    bytes[2] = std::byte{'S'};
    bytes[3] = std::byte{'T'};
    put_u32(bytes, 4U, 2U);
    put_u32(bytes, 8U, 0x20U);
    put_u32(bytes, 12U, 0x30U);
    for (std::size_t index = 0x10U; index < 0x20U; ++index) {
        bytes[index] = static_cast<std::byte>(0x71U + index);
    }
    for (std::size_t index = 0x20U; index < 0x30U; ++index) {
        bytes[index] = static_cast<std::byte>(0x33U ^ index);
    }
    for (std::size_t index = 0x30U; index < bytes.size(); ++index) {
        bytes[index] = static_cast<std::byte>(0x55U ^ index);
    }
    return bytes;
}

[[nodiscard]] std::vector<std::byte> nested_pac() {
    const auto inner = inner_pnst();
    std::vector<std::byte> bytes(0x90U, std::byte{0});
    bytes[0] = std::byte{'P'};
    bytes[1] = std::byte{'A'};
    bytes[2] = std::byte{'C'};
    bytes[3] = std::byte{0};
    put_u32(bytes, 4U, 2U);
    put_u32(bytes, 8U, 0x20U);
    put_u32(bytes, 12U, 0x70U);
    for (std::size_t index = 0x10U; index < 0x20U; ++index) {
        bytes[index] = static_cast<std::byte>(0x91U + index);
    }
    std::copy(inner.begin(), inner.end(), bytes.begin() + 0x20U);
    for (std::size_t index = 0x70U; index < bytes.size(); ++index) {
        bytes[index] = static_cast<std::byte>(0xAAU ^ index);
    }
    return bytes;
}

[[nodiscard]] std::vector<std::byte> minimal_scm() {
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
    put_f32(bytes, 0x04U, 1.01F);
    bytes[0x10U] = std::byte{1};
    bytes[0x11U] = std::byte{1};
    bytes[0x12U] = std::byte{1};
    put_u64(bytes, 0x20U, layout.scene.block_offset);

    const auto& object_layout = layout.objects[0];
    const auto object_offset = static_cast<std::size_t>(object_layout.record_offset);
    bytes[object_offset + 0U] = std::byte{1};
    bytes[object_offset + 1U] = std::byte{0x80};
    put_u16(bytes, object_offset + 0x02U, 3U);
    put_u64(bytes, object_offset + 0x08U, object_layout.mesh_table_offset);

    const auto& mesh_layout = object_layout.meshes[0];
    const auto mesh_offset = static_cast<std::size_t>(mesh_layout.record_offset);
    put_u16(bytes, mesh_offset + 0x00U, 3U);
    put_u16(bytes, mesh_offset + 0x02U, 0U);
    put_u64(bytes, mesh_offset + 0x10U, mesh_layout.positions_offset);
    put_u64(bytes, mesh_offset + 0x18U, mesh_layout.normals_offset);
    put_u64(bytes, mesh_offset + 0x20U, mesh_layout.uv_offset);
    put_u64(bytes, mesh_offset + 0x28U, 0U);
    put_u64(bytes, mesh_offset + 0x38U, mesh_layout.color_flags_offset);
    put_u64(
        bytes,
        mesh_offset + 0x40U,
        mesh_layout.index_workspace_offset - mesh_layout.record_offset);
    put_u16(
        bytes,
        static_cast<std::size_t>(mesh_layout.index_workspace_offset),
        index_workspace_sentinel);

    const auto scene_offset = static_cast<std::size_t>(layout.scene.block_offset);
    put_u32(bytes, scene_offset + 0x00U, layout.scene.parent_rel);
    put_u32(bytes, scene_offset + 0x04U, layout.scene.order_rel);
    put_u32(bytes, scene_offset + 0x08U, layout.scene.object_binding_rel);
    put_u32(bytes, scene_offset + 0x0CU, layout.scene.transform_rel);
    bytes[scene_offset + layout.scene.parent_rel] = std::byte{0xFF};
    bytes[scene_offset + layout.scene.order_rel] = std::byte{0};
    bytes[scene_offset + layout.scene.object_binding_rel] = std::byte{0};

    const auto parsed = Parser::parse(std::span<const std::byte>{bytes});
    assert(parsed.ok());
    return bytes;
}

[[nodiscard]] std::vector<std::byte> scm_parent_pac(
    std::span<const std::byte> scm) {
    assert(!scm.empty());
    const auto second_offset = 0x20U + scm.size();
    assert(second_offset <= static_cast<std::size_t>(UINT32_MAX));
    std::vector<std::byte> bytes(second_offset + 0x20U, std::byte{0});
    bytes[0] = std::byte{'P'};
    bytes[1] = std::byte{'A'};
    bytes[2] = std::byte{'C'};
    bytes[3] = std::byte{0};
    put_u32(bytes, 4U, 2U);
    put_u32(bytes, 8U, 0x20U);
    put_u32(bytes, 12U, static_cast<std::uint32_t>(second_offset));
    for (std::size_t index = 0x10U; index < 0x20U; ++index) {
        bytes[index] = static_cast<std::byte>(0xD0U + index - 0x10U);
    }
    std::copy(scm.begin(), scm.end(), bytes.begin() + 0x20U);
    for (std::size_t index = second_offset; index < bytes.size(); ++index) {
        bytes[index] = static_cast<std::byte>(0xA5U ^ index);
    }
    return bytes;
}

[[nodiscard]] int run_scm_reintegration(
    const std::filesystem::path& source_scm,
    const std::filesystem::path& authored_scm,
    const std::filesystem::path& parent,
    unsigned int slot,
    const std::filesystem::path& output) {
    std::array<std::string, 7U> storage{
        "dmc-rengine",
        "verify-scm-reintegration",
        source_scm.string(),
        authored_scm.string(),
        parent.string(),
        std::to_string(slot),
        output.string(),
    };
    std::array<char*, 7U> argv{};
    for (std::size_t index = 0U; index < storage.size(); ++index) {
        argv[index] = storage[index].data();
    }
    return dmc::rengine::cli::try_run_scm_reintegration_command(
        static_cast<int>(argv.size()), argv.data());
}

[[nodiscard]] dmc::rengine::gdspaces::ResourcePayload payload_of(
    std::vector<std::byte> bytes,
    std::string_view logical_path) {
    dmc::rengine::gdspaces::ResourceId id{
        .source_id = "cli-nested-validation",
        .logical_path = std::string{logical_path},
        .container_chain = {},
        .offset = 0U,
        .size = static_cast<std::uint64_t>(bytes.size()),
    };
    return dmc::rengine::gdspaces::ResourcePayload{
        .resource = dmc::rengine::gdspaces::ResourceRef{
            .id = id,
            .display_name = std::string{logical_path},
            .format = "PAC",
            .profile = "DMC3",
            .synthetic_name = false,
            .container = true,
        },
        .bytes = std::move(bytes),
        .diagnostics = {},
        .byte_provenance = std::nullopt,
    };
}

[[nodiscard]] dmc::rengine::gdspaces::ContainerExpansion expand(
    const dmc::rengine::gdspaces::ResourcePayload& payload) {
    const auto registry = dmc::rengine::profiles::dmc3::make_container_parser_registry();
    const auto parsed = registry.parse(
        std::span<const std::byte>{payload.bytes.data(), payload.bytes.size()},
        payload.resource.id.logical_path);
    assert(parsed.ok());
    const auto expansion = dmc::rengine::gdspaces::ContainerExpander::expand(payload, parsed);
    assert(expansion.usable());
    return expansion;
}

} // namespace

int main() {
    namespace dmc3 = dmc::rengine::profiles::dmc3;

    const auto root = std::filesystem::temp_directory_path() / "dmc-rengine-relative-slot-cli";
    std::error_code error;
    std::filesystem::remove_all(root, error);
    std::filesystem::create_directories(root, error);
    assert(!error);

    const auto parent_path = root / "parent.pac";
    const auto replacement_path = root / "replacement.bin";
    const auto output_path = root / "rebuilt.pac";

    const auto parent = parent_pac();
    std::vector<std::byte> replacement(0x30U, std::byte{0x5AU});
    replacement[0] = std::byte{'R'};
    replacement[1] = std::byte{'E'};
    replacement[2] = std::byte{'P'};
    replacement[3] = std::byte{'L'};
    write_file(parent_path, parent);
    write_file(replacement_path, replacement);

    assert(dmc::rengine::cli::run_rebuild_relative_slot(parent_path, 0U, replacement_path, output_path) == 0);
    assert(std::filesystem::is_regular_file(output_path));

    const auto rebuilt = read_file(output_path);
    assert(rebuilt.size() == 0x70U);
    const auto registry = dmc3::make_container_parser_registry();
    const auto parsed = registry.parse(std::span<const std::byte>{rebuilt.data(), rebuilt.size()}, "rebuilt.pac");
    assert(parsed.ok());
    assert(parsed.document.format == "PAC");
    assert(parsed.document.entries.size() == 2U);
    assert(parsed.document.entries[0].offset == 0x20U);
    assert(parsed.document.entries[0].size == replacement.size());
    assert(parsed.document.entries[1].offset == 0x50U);
    assert(parsed.document.entries[1].size == 0x20U);
    assert(std::equal(replacement.begin(), replacement.end(), rebuilt.begin() + 0x20));
    assert(std::equal(parent.begin() + 0x40, parent.end(), rebuilt.begin() + 0x50));

    const auto before_repeat = rebuilt;
    assert(dmc::rengine::cli::run_rebuild_relative_slot(parent_path, 0U, replacement_path, output_path) != 0);
    assert(read_file(output_path) == before_repeat);
    assert(dmc::rengine::cli::run_rebuild_relative_slot(parent_path, 9U, replacement_path, root / "invalid.pac") != 0);

    // Regression: a basename-only output is a valid no-replace destination in
    // the current directory. This is the path form used by ordinary CLI calls.
    const auto original_cwd = std::filesystem::current_path();
    std::filesystem::current_path(root);
    const std::filesystem::path relative_output{"relative-rebuilt.pac"};
    assert(dmc::rengine::cli::run_rebuild_relative_slot(
        std::filesystem::path{"parent.pac"},
        0U,
        std::filesystem::path{"replacement.bin"},
        relative_output) == 0);
    assert(std::filesystem::is_regular_file(relative_output));
    assert(read_file(relative_output) == rebuilt);
    std::filesystem::current_path(original_cwd);

    const auto nested_source_path = root / "nested.pac";
    const auto nested_replacement_path = root / "nested-replacement.bin";
    const auto nested_output_path = root / "nested-rebuilt.pac";
    const auto nested_source = nested_pac();
    std::vector<std::byte> nested_replacement(0x28U, std::byte{0x6BU});
    nested_replacement[0] = std::byte{'N'};
    nested_replacement[1] = std::byte{'E'};
    nested_replacement[2] = std::byte{'S'};
    nested_replacement[3] = std::byte{'T'};
    write_file(nested_source_path, nested_source);
    write_file(nested_replacement_path, nested_replacement);
    const std::array<unsigned int, 2U> nested_path{0U, 0U};
    assert(dmc::rengine::cli::run_rebuild_relative_slot_path(
        nested_source_path,
        nested_path,
        nested_replacement_path,
        nested_output_path) == 0);

    auto nested_payload = payload_of(read_file(nested_output_path), "nested-rebuilt.pac");
    const auto outer = expand(nested_payload);
    assert(outer.children.size() == 2U);
    const auto inner = expand(outer.children[0].payload);
    assert(inner.children.size() == 2U);
    assert(inner.children[0].payload.bytes == nested_replacement);
    assert(std::equal(
        nested_source.begin() + 0x70U,
        nested_source.end(),
        outer.children[1].payload.bytes.begin()));

    const auto nested_before_repeat = read_file(nested_output_path);
    assert(dmc::rengine::cli::run_rebuild_relative_slot_path(
        nested_source_path,
        nested_path,
        nested_replacement_path,
        nested_output_path) != 0);
    assert(read_file(nested_output_path) == nested_before_repeat);

    // SCM reintegration regression: prove the parent slot is bound to the
    // exact source SCM, the authored child reparses after rematerialization,
    // and every non-target physical slot remains byte-identical.
    const auto source_scm_path = root / "source.scm";
    const auto authored_scm_path = root / "authored.scm";
    const auto scm_parent_path = root / "scm-parent.pac";
    const auto scm_output_path = root / "scm-rebuilt.pac";
    const auto source_scm = minimal_scm();
    auto authored_scm = source_scm;
    const auto alpha_offset = static_cast<std::size_t>(
        dmc::rengine::formats::scm::header_size + 1U);
    assert(authored_scm[alpha_offset] == std::byte{0x80});
    authored_scm[alpha_offset] = std::byte{0x40};
    const auto scm_parent = scm_parent_pac(source_scm);
    write_file(source_scm_path, source_scm);
    write_file(authored_scm_path, authored_scm);
    write_file(scm_parent_path, scm_parent);

    assert(run_scm_reintegration(
        source_scm_path,
        authored_scm_path,
        scm_parent_path,
        0U,
        scm_output_path) == 0);
    assert(std::filesystem::is_regular_file(scm_output_path));

    auto scm_rebuilt_payload = payload_of(
        read_file(scm_output_path), "scm-rebuilt.pac");
    const auto scm_rebuilt_expansion = expand(scm_rebuilt_payload);
    assert(scm_rebuilt_expansion.children.size() == 2U);
    assert(scm_rebuilt_expansion.children[0].payload.bytes == authored_scm);
    const auto source_parent_expansion = expand(
        payload_of(scm_parent, "scm-parent.pac"));
    assert(source_parent_expansion.children.size() == 2U);
    assert(
        scm_rebuilt_expansion.children[1].payload.bytes ==
        source_parent_expansion.children[1].payload.bytes);
    const auto reparsed_authored = dmc::rengine::formats::scm::Parser::parse(
        std::span<const std::byte>{
            scm_rebuilt_expansion.children[0].payload.bytes.data(),
            scm_rebuilt_expansion.children[0].payload.bytes.size()});
    assert(reparsed_authored.ok());
    assert(reparsed_authored.document.objects[0].alpha_control == 0x40U);

    // No-op authored evidence and an already-existing output both fail closed.
    assert(run_scm_reintegration(
        source_scm_path,
        source_scm_path,
        scm_parent_path,
        0U,
        root / "scm-noop.pac") != 0);
    const auto scm_before_repeat = read_file(scm_output_path);
    assert(run_scm_reintegration(
        source_scm_path,
        authored_scm_path,
        scm_parent_path,
        0U,
        scm_output_path) != 0);
    assert(read_file(scm_output_path) == scm_before_repeat);

    std::filesystem::remove_all(root, error);
    return 0;
}