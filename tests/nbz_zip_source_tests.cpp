#include "dmc_rengine/gdspaces/container_expander.hpp"
#include "dmc_rengine/gdspaces/nbz_zip_source.hpp"
#include "dmc_rengine/profiles/dmc3/container_parsers.hpp"

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <span>
#include <string>
#include <string_view>
#include <vector>

namespace {

void append_u16(std::vector<std::byte>& bytes, std::uint16_t value) {
    bytes.push_back(static_cast<std::byte>(value & 0xFFU));
    bytes.push_back(static_cast<std::byte>((value >> 8U) & 0xFFU));
}

void append_u32(std::vector<std::byte>& bytes, std::uint32_t value) {
    for (unsigned shift = 0U; shift < 32U; shift += 8U) {
        bytes.push_back(static_cast<std::byte>((value >> shift) & 0xFFU));
    }
}

void append_ascii(std::vector<std::byte>& bytes, const std::string& value) {
    for (const auto character : value) {
        bytes.push_back(static_cast<std::byte>(character));
    }
}

void write_u32(
    std::vector<std::byte>& bytes,
    std::size_t offset,
    std::uint32_t value) {
    for (unsigned shift = 0U; shift < 32U; shift += 8U) {
        bytes[offset + shift / 8U] =
            static_cast<std::byte>((value >> shift) & 0xFFU);
    }
}

void write_ascii(
    std::vector<std::byte>& bytes,
    std::size_t offset,
    std::string_view value) {
    for (std::size_t index = 0U; index < value.size(); ++index) {
        bytes[offset + index] = static_cast<std::byte>(value[index]);
    }
}

[[nodiscard]] std::vector<std::byte> from_hex(std::string_view hex) {
    assert((hex.size() % 2U) == 0U);
    const auto nibble = [](char value) -> std::uint8_t {
        if (value >= '0' && value <= '9') {
            return static_cast<std::uint8_t>(value - '0');
        }
        if (value >= 'a' && value <= 'f') {
            return static_cast<std::uint8_t>(10 + value - 'a');
        }
        assert(false);
        return 0U;
    };

    std::vector<std::byte> bytes;
    bytes.reserve(hex.size() / 2U);
    for (std::size_t index = 0U; index < hex.size(); index += 2U) {
        bytes.push_back(static_cast<std::byte>(
            static_cast<std::uint8_t>((nibble(hex[index]) << 4U) |
                nibble(hex[index + 1U]))));
    }
    return bytes;
}

[[nodiscard]] std::uint32_t crc32_of(const std::vector<std::byte>& bytes) {
    std::uint32_t crc = 0xFFFFFFFFU;
    for (const auto value : bytes) {
        crc ^= std::to_integer<std::uint8_t>(value);
        for (unsigned bit = 0U; bit < 8U; ++bit) {
            const auto mask = static_cast<std::uint32_t>(
                0U - static_cast<std::uint32_t>(crc & 1U));
            crc = (crc >> 1U) ^ (0xEDB88320U & mask);
        }
    }
    return ~crc;
}

[[nodiscard]] std::vector<std::byte> make_pac_payload() {
    std::vector<std::byte> bytes(80U, std::byte{0});
    write_ascii(bytes, 0U, std::string_view{"PAC\0", 4U});
    write_u32(bytes, 4U, 5U);
    write_u32(bytes, 8U, 32U);
    write_u32(bytes, 12U, 0U);
    write_u32(bytes, 16U, 48U);
    write_u32(bytes, 20U, 48U);
    write_u32(bytes, 24U, 64U);
    write_ascii(bytes, 32U, "DDS ");
    write_ascii(bytes, 48U, std::string_view{"SCM\0", 4U});
    write_ascii(bytes, 64U, std::string_view{"PAC\0", 4U});
    return bytes;
}

struct FixtureEntry final {
    std::string name;
    std::uint16_t method{};
    std::vector<std::byte> stored_bytes;
    std::uint32_t uncompressed_size{};
    std::uint32_t crc32{};
    std::uint32_t local_offset{};
};

[[nodiscard]] std::vector<std::byte> make_zip_fixture() {
    const auto pac_payload = make_pac_payload();
    std::vector<FixtureEntry> entries{
        FixtureEntry{
            .name = "scr/st001.pac",
            .method = 0U,
            .stored_bytes = {
                std::byte{'P'}, std::byte{'A'}, std::byte{'C'}, std::byte{0},
                std::byte{0}, std::byte{0}, std::byte{0}, std::byte{0},
            },
            .uncompressed_size = 8U,
            .crc32 = 0U,
            .local_offset = 0U,
        },
        FixtureEntry{
            .name = "room/st001cfg.pac",
            .method = 8U,
            .stored_bytes = from_hex(
                "0b707466606560605060800003287680f25d5c8261526010ecec8bcc650800ea470600"),
            .uncompressed_size = static_cast<std::uint32_t>(pac_payload.size()),
            .crc32 = crc32_of(pac_payload),
            .local_offset = 0U,
        },
    };
    entries[0].crc32 = crc32_of(entries[0].stored_bytes);

    std::vector<std::byte> bytes;
    for (auto& entry : entries) {
        entry.local_offset = static_cast<std::uint32_t>(bytes.size());
        append_u32(bytes, 0x04034B50U);
        append_u16(bytes, 20U);
        append_u16(bytes, 0U);
        append_u16(bytes, entry.method);
        append_u16(bytes, 0U);
        append_u16(bytes, 0U);
        append_u32(bytes, entry.crc32);
        append_u32(bytes, static_cast<std::uint32_t>(entry.stored_bytes.size()));
        append_u32(bytes, entry.uncompressed_size);
        append_u16(bytes, static_cast<std::uint16_t>(entry.name.size()));
        append_u16(bytes, 0U);
        append_ascii(bytes, entry.name);
        bytes.insert(bytes.end(), entry.stored_bytes.begin(), entry.stored_bytes.end());
    }

    const auto central_offset = static_cast<std::uint32_t>(bytes.size());
    for (const auto& entry : entries) {
        append_u32(bytes, 0x02014B50U);
        append_u16(bytes, 20U);
        append_u16(bytes, 20U);
        append_u16(bytes, 0U);
        append_u16(bytes, entry.method);
        append_u16(bytes, 0U);
        append_u16(bytes, 0U);
        append_u32(bytes, entry.crc32);
        append_u32(bytes, static_cast<std::uint32_t>(entry.stored_bytes.size()));
        append_u32(bytes, entry.uncompressed_size);
        append_u16(bytes, static_cast<std::uint16_t>(entry.name.size()));
        append_u16(bytes, 0U);
        append_u16(bytes, 0U);
        append_u16(bytes, 0U);
        append_u16(bytes, 0U);
        append_u32(bytes, 0U);
        append_u32(bytes, entry.local_offset);
        append_ascii(bytes, entry.name);
    }
    const auto central_size =
        static_cast<std::uint32_t>(bytes.size()) - central_offset;

    append_u32(bytes, 0x06054B50U);
    append_u16(bytes, 0U);
    append_u16(bytes, 0U);
    append_u16(bytes, static_cast<std::uint16_t>(entries.size()));
    append_u16(bytes, static_cast<std::uint16_t>(entries.size()));
    append_u32(bytes, central_size);
    append_u32(bytes, central_offset);
    append_u16(bytes, 0U);
    return bytes;
}

[[nodiscard]] bool has_code(
    const std::vector<dmc::rengine::gdspaces::Diagnostic>& diagnostics,
    const std::string& code) {
    return std::any_of(
        diagnostics.begin(), diagnostics.end(),
        [&code](const auto& diagnostic) {
            return diagnostic.code == code;
        });
}

[[nodiscard]] std::filesystem::path write_fixture(
    const std::vector<std::byte>& bytes,
    const std::string& suffix) {
    const auto path = std::filesystem::temp_directory_path() /
        ("dmc-rengine-nbz-" + suffix + ".nbz");
    std::ofstream stream(path, std::ios::binary | std::ios::trunc);
    assert(stream.good());
    stream.write(
        reinterpret_cast<const char*>(bytes.data()),
        static_cast<std::streamsize>(bytes.size()));
    assert(stream.good());
    return path;
}

} // namespace

int main() {
    using dmc::rengine::gdspaces::ByteOriginKind;
    using dmc::rengine::gdspaces::ByteTransform;
    using dmc::rengine::gdspaces::ContainerExpander;
    using dmc::rengine::gdspaces::NbzZipSource;
    using dmc::rengine::profiles::dmc3::make_container_parser_registry;

    const auto fixture = make_zip_fixture();
    const auto path = write_fixture(fixture, "valid");

    NbzZipSource source("dmc3-nbz-0", path);
    assert(source.valid());
    assert(source.kind() == "nbz-zip");
    assert(source.entries().size() == 2U);
    assert(source.entries()[0].logical_path == "scr/st001.pac");
    assert(source.entries()[0].compression_method == 0U);
    assert(source.entries()[1].logical_path == "room/st001cfg.pac");
    assert(source.entries()[1].compression_method == 8U);

    const auto resources = source.enumerate();
    assert(resources.size() == 2U);
    assert(resources[0].id.logical_path == "scr/st001.pac");
    assert(resources[0].id.container_chain == "nbz[0]");
    assert(resources[0].id.offset == 0U);
    assert(resources[0].id.size == 8U);
    assert(resources[1].id.container_chain == "nbz[1]");
    assert(resources[1].id.offset == 0U);
    assert(resources[1].id.size == 80U);

    const auto stored = source.read(resources[0].id);
    assert(stored.has_value());
    assert(stored->readable());
    assert(stored->bytes.size() == 8U);
    assert(stored->resource.format == "pac");
    assert(stored->resource.container);
    assert(stored->byte_provenance.has_value());
    assert(stored->byte_provenance->valid());
    assert(stored->byte_provenance->kind == ByteOriginKind::direct_source_span);
    assert(stored->byte_provenance->transform == ByteTransform::zip_stored);
    assert(stored->byte_provenance->authority_id == "dmc3-nbz-0");
    assert(stored->byte_provenance->stored_size == 8U);
    assert(stored->byte_provenance->materialized_size == 8U);
    assert(stored->byte_provenance->crc32 == source.entries()[0].crc32);

    const auto deflated = source.read(resources[1].id);
    assert(deflated.has_value());
    assert(deflated->readable());
    assert(deflated->bytes == make_pac_payload());
    assert(deflated->resource.format == "pac");
    assert(deflated->resource.container);
    assert(deflated->byte_provenance.has_value());
    assert(deflated->byte_provenance->valid());
    assert(deflated->byte_provenance->kind ==
        ByteOriginKind::transformed_source_span);
    assert(deflated->byte_provenance->transform == ByteTransform::zip_deflate);
    assert(deflated->byte_provenance->stored_size == 35U);
    assert(deflated->byte_provenance->materialized_size == 80U);
    assert(deflated->byte_provenance->crc32 == source.entries()[1].crc32);

    auto registry = make_container_parser_registry();
    const auto parsed = registry.parse(
        std::span<const std::byte>{deflated->bytes},
        deflated->resource.id.logical_path);
    assert(parsed.ok());
    assert(parsed.document.declared_slot_count == 5U);

    const auto expansion = ContainerExpander::expand(*deflated, parsed);
    assert(expansion.usable());
    assert(expansion.children.size() == 5U);
    assert(expansion.children[0].payload.resource.id.offset == 32U);
    assert(expansion.children[0].payload.resource.format == "dds");
    assert(expansion.children[0].payload.byte_provenance.has_value());
    assert(expansion.children[0].payload.byte_provenance->kind ==
        ByteOriginKind::materialized_parent_span);
    assert(expansion.children[0].payload.byte_provenance->offset == 32U);
    assert(expansion.children[0].payload.byte_provenance->authority_id ==
        deflated->resource.id.canonical());
    assert(expansion.children[0].payload.byte_provenance->offset !=
        source.entries()[1].data_offset + 32U);
    assert(expansion.children[1].payload.resource.format == "empty-slot");
    assert(expansion.children[2].payload.resource.format == "scm");
    assert(expansion.children[3].payload.resource.format == "scm");
    assert(expansion.children[4].payload.resource.format == "pac");
    assert(expansion.children[4].payload.resource.container);

    auto corrupt_crc = fixture;
    const auto first_data_offset = source.entries()[0].data_offset;
    corrupt_crc[static_cast<std::size_t>(first_data_offset)] ^= std::byte{0x01};
    const auto corrupt_path = write_fixture(corrupt_crc, "bad-crc");
    NbzZipSource bad_crc_source("dmc3-nbz-bad-crc", corrupt_path);
    assert(bad_crc_source.valid());
    const auto bad_resources = bad_crc_source.enumerate();
    const auto bad_payload = bad_crc_source.read(bad_resources[0].id);
    assert(bad_payload.has_value());
    assert(!bad_payload->readable());
    assert(has_code(bad_payload->diagnostics, "gdspaces.nbz.crc32-mismatch"));

    auto corrupt_deflate = fixture;
    const auto deflate_data_offset = source.entries()[1].data_offset;
    corrupt_deflate[static_cast<std::size_t>(deflate_data_offset)] ^= std::byte{0x80};
    const auto corrupt_deflate_path = write_fixture(
        corrupt_deflate, "bad-deflate");
    NbzZipSource bad_deflate_source(
        "dmc3-nbz-bad-deflate", corrupt_deflate_path);
    assert(bad_deflate_source.valid());
    const auto bad_deflate_resources = bad_deflate_source.enumerate();
    const auto bad_deflate_payload = bad_deflate_source.read(
        bad_deflate_resources[1].id);
    assert(bad_deflate_payload.has_value());
    assert(!bad_deflate_payload->readable());
    assert(has_code(bad_deflate_payload->diagnostics, "gdspaces.nbz.deflate-failed") ||
        has_code(bad_deflate_payload->diagnostics, "gdspaces.nbz.crc32-mismatch"));

    auto zip64 = fixture;
    const auto eocd_offset = zip64.size() - 22U;
    write_u32(zip64, eocd_offset + 16U, 0xFFFFFFFFU);
    const auto zip64_path = write_fixture(zip64, "zip64-sentinel");
    NbzZipSource zip64_source("dmc3-nbz-zip64", zip64_path);
    assert(!zip64_source.valid());
    assert(zip64_source.enumerate().empty());
    assert(has_code(zip64_source.diagnostics(), "gdspaces.nbz.zip64-unresolved"));

    std::error_code error;
    std::filesystem::remove(path, error);
    std::filesystem::remove(corrupt_path, error);
    std::filesystem::remove(corrupt_deflate_path, error);
    std::filesystem::remove(zip64_path, error);
    return 0;
}
