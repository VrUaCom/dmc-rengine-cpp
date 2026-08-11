#include "dmc_rengine/gdspaces/nbz_zip_source.hpp"

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <string>
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

struct FixtureEntry final {
    std::string name;
    std::uint16_t method{};
    std::vector<std::byte> stored_bytes;
    std::uint32_t uncompressed_size{};
    std::uint32_t crc32{};
    std::uint32_t local_offset{};
};

[[nodiscard]] std::vector<std::byte> make_zip_fixture() {
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
            .stored_bytes = {
                std::byte{0x11}, std::byte{0x22}, std::byte{0x33},
            },
            .uncompressed_size = 32U,
            .crc32 = 0xAABBCCDDU,
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
    using dmc::rengine::gdspaces::NbzZipSource;

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
    assert(resources[1].id.size == 32U);

    const auto stored = source.read(resources[0].id);
    assert(stored.has_value());
    assert(stored->readable());
    assert(stored->bytes.size() == 8U);
    assert(stored->resource.format == "pac");
    assert(stored->resource.container);
    assert(stored->byte_provenance.has_value());
    assert(stored->byte_provenance->valid());
    assert(stored->byte_provenance->kind == ByteOriginKind::direct_source_span);
    assert(stored->byte_provenance->transform == ByteTransform::none);
    assert(stored->byte_provenance->authority_id == "dmc3-nbz-0");
    assert(stored->byte_provenance->stored_size == 8U);
    assert(stored->byte_provenance->materialized_size == 8U);
    assert(stored->byte_provenance->crc32 == source.entries()[0].crc32);

    const auto deflated = source.read(resources[1].id);
    assert(deflated.has_value());
    assert(!deflated->readable());
    assert(deflated->bytes.empty());
    assert(deflated->byte_provenance.has_value());
    assert(deflated->byte_provenance->valid());
    assert(deflated->byte_provenance->kind ==
        ByteOriginKind::transformed_source_span);
    assert(deflated->byte_provenance->transform == ByteTransform::zip_deflate);
    assert(deflated->byte_provenance->stored_size == 3U);
    assert(deflated->byte_provenance->materialized_size == 32U);
    assert(has_code(
        deflated->diagnostics,
        "gdspaces.nbz.deflate-materialization-pending"));

    auto corrupt_crc = fixture;
    // First local/central CRC value is not used for source validity consistency,
    // so corrupt the stored entry data itself. The read path must catch it.
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

    auto zip64 = fixture;
    // EOCD central-directory offset field is 6 bytes before the final comment
    // length field in this zero-comment synthetic archive.
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
    std::filesystem::remove(zip64_path, error);
    return 0;
}
