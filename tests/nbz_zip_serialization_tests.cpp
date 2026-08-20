#include "dmc_rengine/gdspaces/nbz_zip_serialization.hpp"

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <span>
#include <string_view>
#include <vector>

namespace {

void u16(std::vector<std::byte>& out, std::uint16_t value) {
    out.push_back(static_cast<std::byte>(value & 0xFFU));
    out.push_back(static_cast<std::byte>((value >> 8U) & 0xFFU));
}

void u32(std::vector<std::byte>& out, std::uint32_t value) {
    for (unsigned shift = 0U; shift < 32U; shift += 8U) {
        out.push_back(static_cast<std::byte>((value >> shift) & 0xFFU));
    }
}

void append(std::vector<std::byte>& out, std::string_view text) {
    for (const auto value : text) {
        out.push_back(static_cast<std::byte>(value));
    }
}

void append(
    std::vector<std::byte>& out,
    std::span<const std::byte> bytes) {
    out.insert(out.end(), bytes.begin(), bytes.end());
}

std::vector<std::byte> make_fixture() {
    std::vector<std::byte> bytes;
    append(bytes, "SFX!");

    constexpr std::string_view name = "a.bin";
    const std::vector<std::byte> local_extra{
        std::byte{0xAA}, std::byte{0xBB}, std::byte{0xCC}, std::byte{0xDD}};
    const std::vector<std::byte> stored{
        std::byte{'A'}, std::byte{'B'}, std::byte{'C'}};

    const auto local_offset = static_cast<std::uint32_t>(bytes.size());
    u32(bytes, 0x04034B50U);
    u16(bytes, 20U);
    u16(bytes, 0x0008U); // data descriptor follows stored bytes
    u16(bytes, 0U);      // STORE
    u16(bytes, 0x1234U);
    u16(bytes, 0x5678U);
    u32(bytes, 0U); // descriptor owns CRC/sizes in the local stream
    u32(bytes, 0U);
    u32(bytes, 0U);
    u16(bytes, static_cast<std::uint16_t>(name.size()));
    u16(bytes, static_cast<std::uint16_t>(local_extra.size()));
    append(bytes, name);
    append(bytes, local_extra);
    append(bytes, stored);

    // Descriptor and unknown/padding tail remain opaque local-region bytes.
    u32(bytes, 0x08074B50U);
    u32(bytes, 0U);
    u32(bytes, 3U);
    u32(bytes, 3U);
    bytes.push_back(std::byte{0xDE});
    bytes.push_back(std::byte{0xAD});

    const auto central_start = static_cast<std::uint32_t>(bytes.size());
    const std::vector<std::byte> central_extra{
        std::byte{0x10}, std::byte{0x20}, std::byte{0x30}};
    constexpr std::string_view central_comment = "CM";

    u32(bytes, 0x02014B50U);
    u16(bytes, 0x033FU); // preserve version-made-by bytes
    u16(bytes, 20U);
    u16(bytes, 0x0008U);
    u16(bytes, 0U);
    u16(bytes, 0x1234U);
    u16(bytes, 0x5678U);
    u32(bytes, 0U);
    u32(bytes, 3U);
    u32(bytes, 3U);
    u16(bytes, static_cast<std::uint16_t>(name.size()));
    u16(bytes, static_cast<std::uint16_t>(central_extra.size()));
    u16(bytes, static_cast<std::uint16_t>(central_comment.size()));
    u16(bytes, 0U);
    u16(bytes, 0xCAFEU);
    u32(bytes, 0x11223344U);
    u32(bytes, local_offset);
    append(bytes, name);
    append(bytes, central_extra);
    append(bytes, central_comment);

    const auto central_size =
        static_cast<std::uint32_t>(bytes.size()) - central_start;
    u32(bytes, 0x06054B50U);
    u16(bytes, 0U);
    u16(bytes, 0U);
    u16(bytes, 1U);
    u16(bytes, 1U);
    u32(bytes, central_size);
    u32(bytes, central_start);
    u16(bytes, 4U);
    append(bytes, "NOTE");
    return bytes;
}

std::filesystem::path write_fixture(const std::vector<std::byte>& bytes) {
    const auto path = std::filesystem::temp_directory_path() /
        "dmc-rengine-nbz-serialization-test.nbz";
    std::ofstream stream(path, std::ios::binary | std::ios::trunc);
    stream.write(
        reinterpret_cast<const char*>(bytes.data()),
        static_cast<std::streamsize>(bytes.size()));
    assert(stream.good());
    return path;
}

bool has_code(
    const std::vector<dmc::rengine::gdspaces::Diagnostic>& diagnostics,
    std::string_view code) {
    for (const auto& diagnostic : diagnostics) {
        if (diagnostic.code == code) {
            return true;
        }
    }
    return false;
}

} // namespace

int main() {
    using namespace dmc::rengine::gdspaces;

    const auto fixture = make_fixture();
    const auto path = write_fixture(fixture);
    NbzZipSource source("retail-snapshot", path);
    assert(source.valid());
    assert(source.entries().size() == 1U);

    const auto result = NbzZipSerializationScanner::scan(source);
    assert(result.ok());
    assert(result.snapshot.has_value());
    const auto& snapshot = *result.snapshot;
    assert(snapshot.valid());
    assert(snapshot.source_id == "retail-snapshot");
    assert(snapshot.archive_size == fixture.size());
    assert(snapshot.prefix_size == 4U);
    assert(snapshot.entries.size() == 1U);
    assert(snapshot.eocd_bytes.size() == 26U);
    assert(snapshot.eocd_bytes[snapshot.eocd_bytes.size() - 4U] == std::byte{'N'});
    assert(snapshot.eocd_bytes.back() == std::byte{'E'});

    const auto& entry = snapshot.entries.front();
    assert(entry.central_index == 0U);
    assert(entry.central_record_offset == snapshot.computed_central_start);
    assert(entry.central_record_bytes.size() == 56U);
    assert(entry.local_record_offset == 4U);
    assert(entry.local_prefix_bytes.size() == 39U);
    assert(entry.data_offset == 43U);
    assert(entry.stored_data_size == 3U);
    assert(entry.local_region_size == snapshot.computed_central_start - 4U);
    assert(entry.uses_data_descriptor);

    // Raw framing preserves metadata that NbzZipEntry intentionally does not
    // model as materialization semantics.
    assert(entry.local_prefix_bytes[35U] == std::byte{0xAA});
    assert(entry.local_prefix_bytes[38U] == std::byte{0xDD});
    assert(entry.central_record_bytes[51U] == std::byte{0x10});
    assert(entry.central_record_bytes[53U] == std::byte{0x30});
    assert(entry.central_record_bytes[54U] == std::byte{'C'});
    assert(entry.central_record_bytes[55U] == std::byte{'M'});

    // Snapshot validity is bound to preserved raw framing.
    auto tampered = snapshot;
    tampered.entries.front().central_record_bytes[0U] = std::byte{0};
    assert(!tampered.valid());

    const auto budget_failure = NbzZipSerializationScanner::scan(
        source,
        NbzZipSerializationLimits{.max_metadata_bytes = 64U});
    assert(!budget_failure.ok());
    assert(has_code(
        budget_failure.diagnostics,
        "gdspaces.nbz.serialization.metadata-budget"));

    NbzZipSource invalid_source(
        "missing-retail-source",
        path.string() + ".does-not-exist");
    const auto invalid = NbzZipSerializationScanner::scan(invalid_source);
    assert(!invalid.ok());
    assert(has_code(
        invalid.diagnostics,
        "gdspaces.nbz.serialization.source-invalid"));

    std::error_code error;
    std::filesystem::remove(path, error);
    return 0;
}
