#include "relative_slot_commands.hpp"

#include "dmc_rengine/gdspaces/container_expander.hpp"
#include "dmc_rengine/gdspaces/resource_payload.hpp"
#include "dmc_rengine/profiles/dmc3/container_parsers.hpp"

#include <algorithm>
#include <array>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <span>
#include <string_view>
#include <vector>

namespace {

void put_u32(std::vector<std::byte>& bytes, std::size_t offset, std::uint32_t value) {
    bytes[offset + 0U] = static_cast<std::byte>(value & 0xFFU);
    bytes[offset + 1U] = static_cast<std::byte>((value >> 8U) & 0xFFU);
    bytes[offset + 2U] = static_cast<std::byte>((value >> 16U) & 0xFFU);
    bytes[offset + 3U] = static_cast<std::byte>((value >> 24U) & 0xFFU);
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

    std::filesystem::remove_all(root, error);
    return 0;
}
