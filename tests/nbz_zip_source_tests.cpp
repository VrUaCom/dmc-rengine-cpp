#include "dmc_rengine/gdspaces/container_expander.hpp"
#include "dmc_rengine/gdspaces/nbz_zip_source.hpp"
#include "dmc_rengine/profiles/dmc3/container_parsers.hpp"

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

struct EntrySpec final {
    std::string name;
    std::uint16_t method{};
    std::vector<std::byte> stored;
    std::vector<std::byte> materialized;
    bool wrong_crc{false};
};

struct ZipFixture final {
    std::vector<std::byte> bytes;
    std::size_t eocd_offset{};
    std::size_t central_start{};
    std::vector<std::size_t> local_name_offsets;
    std::vector<std::size_t> local_data_offsets;
    std::vector<std::size_t> central_record_offsets;
};

void u16(std::vector<std::byte>& out, std::uint16_t value) {
    out.push_back(static_cast<std::byte>(value & 0xFFU));
    out.push_back(static_cast<std::byte>((value >> 8U) & 0xFFU));
}
void u32(std::vector<std::byte>& out, std::uint32_t value) {
    for (unsigned shift = 0U; shift < 32U; shift += 8U) {
        out.push_back(static_cast<std::byte>((value >> shift) & 0xFFU));
    }
}
void patch_u16(std::vector<std::byte>& out, std::size_t offset, std::uint16_t value) {
    out[offset] = static_cast<std::byte>(value & 0xFFU);
    out[offset + 1U] = static_cast<std::byte>((value >> 8U) & 0xFFU);
}
void patch_u32(std::vector<std::byte>& out, std::size_t offset, std::uint32_t value) {
    for (unsigned shift = 0U; shift < 32U; shift += 8U) {
        out[offset + shift / 8U] = static_cast<std::byte>((value >> shift) & 0xFFU);
    }
}
void append(std::vector<std::byte>& out, std::string_view text) {
    for (char c : text) out.push_back(static_cast<std::byte>(c));
}
void append(std::vector<std::byte>& out, std::span<const std::byte> bytes) {
    out.insert(out.end(), bytes.begin(), bytes.end());
}

std::uint32_t crc32(std::span<const std::byte> bytes) {
    std::uint32_t crc = 0xFFFFFFFFU;
    for (auto value : bytes) {
        crc ^= std::to_integer<std::uint8_t>(value);
        for (unsigned bit = 0U; bit < 8U; ++bit) {
            const auto mask = static_cast<std::uint32_t>(
                0U - static_cast<std::uint32_t>(crc & 1U));
            crc = (crc >> 1U) ^ (0xEDB88320U & mask);
        }
    }
    return ~crc;
}

std::vector<std::byte> ascii(std::string_view text) {
    std::vector<std::byte> out;
    append(out, text);
    return out;
}

std::vector<std::byte> from_hex(std::string_view hex) {
    auto nibble = [](char c) -> std::uint8_t {
        if (c >= '0' && c <= '9') return static_cast<std::uint8_t>(c - '0');
        return static_cast<std::uint8_t>(10 + c - 'a');
    };
    std::vector<std::byte> out;
    for (std::size_t i = 0U; i < hex.size(); i += 2U) {
        out.push_back(static_cast<std::byte>(
            static_cast<std::uint8_t>((nibble(hex[i]) << 4U) | nibble(hex[i + 1U]))));
    }
    return out;
}

std::vector<std::byte> nested_pac() {
    std::vector<std::byte> bytes(0x20U, std::byte{0});
    bytes[0] = std::byte{'P'}; bytes[1] = std::byte{'A'};
    bytes[2] = std::byte{'C'}; bytes[3] = std::byte{0};
    patch_u32(bytes, 4U, 1U);
    patch_u32(bytes, 8U, 0x10U);
    bytes[0x10U] = std::byte{'D'}; bytes[0x11U] = std::byte{'D'};
    bytes[0x12U] = std::byte{'S'}; bytes[0x13U] = std::byte{' '};
    return bytes;
}

ZipFixture make_zip(const std::vector<EntrySpec>& specs) {
    struct Built final {
        std::uint32_t local_offset{};
        std::uint32_t crc{};
        const EntrySpec* spec{};
    };

    ZipFixture fixture;
    std::vector<Built> built;
    built.reserve(specs.size());

    for (const auto& spec : specs) {
        const auto local_offset = static_cast<std::uint32_t>(fixture.bytes.size());
        auto value_crc = crc32(spec.materialized);
        if (spec.wrong_crc) value_crc ^= 0x01020304U;

        u32(fixture.bytes, 0x04034B50U);
        u16(fixture.bytes, 20U); // version needed
        u16(fixture.bytes, 0U);  // flags
        u16(fixture.bytes, spec.method);
        u16(fixture.bytes, 0U); u16(fixture.bytes, 0U);
        u32(fixture.bytes, value_crc);
        u32(fixture.bytes, static_cast<std::uint32_t>(spec.stored.size()));
        u32(fixture.bytes, static_cast<std::uint32_t>(spec.materialized.size()));
        u16(fixture.bytes, static_cast<std::uint16_t>(spec.name.size()));
        u16(fixture.bytes, 0U);
        fixture.local_name_offsets.push_back(fixture.bytes.size());
        append(fixture.bytes, spec.name);
        fixture.local_data_offsets.push_back(fixture.bytes.size());
        append(fixture.bytes, spec.stored);
        built.push_back(Built{local_offset, value_crc, &spec});
    }

    fixture.central_start = fixture.bytes.size();
    for (const auto& item : built) {
        const auto& spec = *item.spec;
        fixture.central_record_offsets.push_back(fixture.bytes.size());
        u32(fixture.bytes, 0x02014B50U);
        u16(fixture.bytes, 0x003FU); // version made by
        u16(fixture.bytes, 20U);    // version needed
        u16(fixture.bytes, 0U);     // flags
        u16(fixture.bytes, spec.method);
        u16(fixture.bytes, 0U); u16(fixture.bytes, 0U);
        u32(fixture.bytes, item.crc);
        u32(fixture.bytes, static_cast<std::uint32_t>(spec.stored.size()));
        u32(fixture.bytes, static_cast<std::uint32_t>(spec.materialized.size()));
        u16(fixture.bytes, static_cast<std::uint16_t>(spec.name.size()));
        u16(fixture.bytes, 0U); u16(fixture.bytes, 0U);
        u16(fixture.bytes, 0U); u16(fixture.bytes, 0U);
        u32(fixture.bytes, 0U);
        u32(fixture.bytes, item.local_offset);
        append(fixture.bytes, spec.name);
    }

    const auto central_size = fixture.bytes.size() - fixture.central_start;
    fixture.eocd_offset = fixture.bytes.size();
    u32(fixture.bytes, 0x06054B50U);
    u16(fixture.bytes, 0U); u16(fixture.bytes, 0U);
    u16(fixture.bytes, static_cast<std::uint16_t>(specs.size()));
    u16(fixture.bytes, static_cast<std::uint16_t>(specs.size()));
    u32(fixture.bytes, static_cast<std::uint32_t>(central_size));
    u32(fixture.bytes, static_cast<std::uint32_t>(fixture.central_start));
    u16(fixture.bytes, 0U);
    return fixture;
}

std::filesystem::path write_fixture(
    const std::vector<std::byte>& bytes,
    std::string_view stem) {
    auto path = std::filesystem::temp_directory_path() /
        (std::string{"dmc-rengine-"} + std::string{stem} + ".nbz");
    std::ofstream stream(path, std::ios::binary | std::ios::trunc);
    stream.write(
        reinterpret_cast<const char*>(bytes.data()),
        static_cast<std::streamsize>(bytes.size()));
    assert(stream.good());
    return path;
}

const dmc::rengine::gdspaces::ResourceRef* find_path(
    const std::vector<dmc::rengine::gdspaces::ResourceRef>& refs,
    std::string_view path) {
    for (const auto& ref : refs) {
        if (ref.id.logical_path == path) return &ref;
    }
    return nullptr;
}

bool has_code(
    const std::vector<dmc::rengine::gdspaces::Diagnostic>& diagnostics,
    std::string_view code) {
    for (const auto& diagnostic : diagnostics) {
        if (diagnostic.code == code) return true;
    }
    return false;
}

} // namespace

int main() {
    using namespace dmc::rengine::gdspaces;

    const auto pac = nested_pac();
    const auto compressed_pac = from_hex("0b70746660646060106080001797600506240000");
    const std::vector<EntrySpec> specs{
        EntrySpec{"plain.txt", 0U, ascii("HELLO"), ascii("HELLO")},
        EntrySpec{"nested.pac", 8U, compressed_pac, pac},
    };

    const auto normal_fixture = make_zip(specs);
    const auto normal_path = write_fixture(normal_fixture.bytes, "nbz-normal");
    NbzZipSource source("nbz-test", normal_path);
    assert(source.valid());
    assert(source.index_receipt().has_value());
    assert(source.index_receipt()->central_offset_matches);
    assert(source.index_receipt()->entry_count_matches);
    assert(source.index_receipt()->walked_entry_count == 2U);

    const auto refs = source.enumerate();
    assert(refs.size() == 2U);
    const auto* plain_ref = find_path(refs, "plain.txt");
    const auto* nested_ref = find_path(refs, "nested.pac");
    assert(plain_ref != nullptr && nested_ref != nullptr);

    const auto plain = source.read(plain_ref->id);
    assert(plain.has_value() && plain->readable());
    assert(plain->bytes == ascii("HELLO"));
    assert(plain->byte_provenance.has_value());
    assert(plain->byte_provenance->kind == ByteOriginKind::direct_source_span);
    assert(plain->byte_provenance->transform == ByteTransform::zip_stored);

    const auto nested = source.read(nested_ref->id);
    assert(nested.has_value() && nested->readable());
    assert(nested->bytes == pac);
    assert(nested->resource.format == "pac");
    assert(nested->resource.container);
    assert(nested->byte_provenance.has_value());
    assert(nested->byte_provenance->kind == ByteOriginKind::transformed_source_span);
    assert(nested->byte_provenance->transform == ByteTransform::zip_deflate);

    const auto registry = dmc::rengine::profiles::dmc3::make_container_parser_registry();
    const auto parsed = registry.parse(
        std::span<const std::byte>{nested->bytes}, nested->resource.id.logical_path);
    assert(parsed.ok());
    const auto expanded = ContainerExpander::expand(*nested, parsed);
    assert(expanded.usable());
    assert(expanded.children.size() == 1U);
    assert(expanded.children[0].payload.resource.format == "dds");
    assert(expanded.children[0].payload.byte_provenance.has_value());
    assert(expanded.children[0].payload.byte_provenance->kind ==
        ByteOriginKind::materialized_parent_span);
    assert(expanded.children[0].payload.byte_provenance->offset == 0x10U);

    // Recovered walk correction #1: declared central offset is receipt data,
    // not seek authority. Corrupt it while keeping central bytes intact.
    auto bad_offset_fixture = make_zip(specs);
    patch_u32(
        bad_offset_fixture.bytes,
        bad_offset_fixture.eocd_offset + 16U,
        static_cast<std::uint32_t>(bad_offset_fixture.central_start + 7U));
    const auto bad_offset_path = write_fixture(bad_offset_fixture.bytes, "nbz-bad-offset");
    NbzZipSource bad_offset("nbz-bad-offset", bad_offset_path);
    assert(bad_offset.valid());
    assert(bad_offset.entries().size() == 2U);
    assert(bad_offset.index_receipt().has_value());
    assert(!bad_offset.index_receipt()->central_offset_matches);
    assert(has_code(
        bad_offset.diagnostics(), "gdspaces.nbz.compat.central-offset-mismatch"));

    // Recovered walk correction #2: declared entry count is not the traversal
    // bound. Both count fields claim one record, central bytes contain two.
    auto bad_count_fixture = make_zip(specs);
    patch_u16(bad_count_fixture.bytes, bad_count_fixture.eocd_offset + 8U, 1U);
    patch_u16(bad_count_fixture.bytes, bad_count_fixture.eocd_offset + 10U, 1U);
    const auto bad_count_path = write_fixture(bad_count_fixture.bytes, "nbz-bad-count");
    NbzZipSource bad_count("nbz-bad-count", bad_count_path);
    assert(bad_count.valid());
    assert(bad_count.entries().size() == 2U);
    assert(bad_count.index_receipt()->declared_entry_count == 1U);
    assert(bad_count.index_receipt()->walked_entry_count == 2U);
    assert(!bad_count.index_receipt()->entry_count_matches);
    assert(has_code(
        bad_count.diagnostics(), "gdspaces.nbz.compat.entry-count-mismatch"));

    // SafeProductValidation: traversal has a product work budget independent
    // of the untrusted EOCD entry count.
    NbzZipSource entry_budget(
        "nbz-entry-budget",
        normal_path,
        NbzZipLimits{
            .max_central_entries = 1U,
            .max_stored_member_bytes = 512ULL * 1024ULL * 1024ULL,
            .max_materialized_member_bytes = 512ULL * 1024ULL * 1024ULL,
        });
    assert(!entry_budget.valid());
    assert(entry_budget.index_receipt().has_value());
    assert(entry_budget.index_receipt()->walked_entry_count == 1U);
    assert(has_code(
        entry_budget.diagnostics(), "gdspaces.nbz.safe.central-entry-budget"));

    // SafeProductValidation: stored bytes are bounded before vector allocation.
    NbzZipSource stored_budget(
        "nbz-stored-budget",
        normal_path,
        NbzZipLimits{
            .max_central_entries = 16U,
            .max_stored_member_bytes = 4U,
            .max_materialized_member_bytes = 64U,
        });
    assert(stored_budget.valid());
    const auto stored_refs = stored_budget.enumerate();
    const auto* stored_plain_ref = find_path(stored_refs, "plain.txt");
    assert(stored_plain_ref != nullptr);
    const auto stored_limited = stored_budget.read(stored_plain_ref->id);
    assert(stored_limited.has_value() && !stored_limited->readable());
    assert(!stored_limited->byte_provenance.has_value());
    assert(has_code(
        stored_limited->diagnostics, "gdspaces.nbz.safe.stored-member-budget"));

    // SafeProductValidation: expected DEFLATE output is bounded before inflate.
    NbzZipSource materialized_budget(
        "nbz-materialized-budget",
        normal_path,
        NbzZipLimits{
            .max_central_entries = 16U,
            .max_stored_member_bytes = 64U,
            .max_materialized_member_bytes = 31U,
        });
    assert(materialized_budget.valid());
    const auto materialized_refs = materialized_budget.enumerate();
    const auto* materialized_nested_ref = find_path(materialized_refs, "nested.pac");
    assert(materialized_nested_ref != nullptr);
    const auto materialized_limited = materialized_budget.read(materialized_nested_ref->id);
    assert(materialized_limited.has_value() && !materialized_limited->readable());
    assert(!materialized_limited->byte_provenance.has_value());
    assert(has_code(
        materialized_limited->diagnostics,
        "gdspaces.nbz.safe.materialized-member-budget"));

    // SafeProductValidation: member payload bytes may not consume any byte of
    // the recovered central-directory domain.
    auto overlap_fixture = make_zip(specs);
    const auto overlap_size =
        overlap_fixture.central_start - overlap_fixture.local_data_offsets[0] + 1U;
    patch_u32(
        overlap_fixture.bytes,
        overlap_fixture.central_record_offsets[0] + 20U,
        static_cast<std::uint32_t>(overlap_size));
    const auto overlap_path = write_fixture(overlap_fixture.bytes, "nbz-overlap");
    NbzZipSource overlap("nbz-overlap", overlap_path);
    assert(!overlap.valid());
    assert(has_code(
        overlap.diagnostics(),
        "gdspaces.nbz.safe.member-overlaps-central-directory"));

    // SafeProductValidation: CRC enforcement is intentionally stricter than
    // the recovered original read path.
    auto wrong_crc_specs = specs;
    wrong_crc_specs[0].wrong_crc = true;
    const auto wrong_crc_fixture = make_zip(wrong_crc_specs);
    const auto wrong_crc_path = write_fixture(wrong_crc_fixture.bytes, "nbz-crc");
    NbzZipSource wrong_crc("nbz-crc", wrong_crc_path);
    assert(wrong_crc.valid());
    const auto crc_ref = wrong_crc.enumerate().front();
    const auto crc_payload = wrong_crc.read(crc_ref.id);
    assert(crc_payload.has_value() && !crc_payload->readable());
    assert(!crc_payload->byte_provenance.has_value());
    assert(has_code(crc_payload->diagnostics, "gdspaces.nbz.safe.crc-mismatch"));

    // SafeProductValidation: a successful zero-byte STORE member still has a
    // valid materialized-byte lineage; emptiness alone is not a failure signal.
    // The same source must drop provenance after its archive becomes stale.
    const std::vector<EntrySpec> stale_specs{
        EntrySpec{"empty.bin", 0U, {}, {}},
    };
    const auto stale_fixture = make_zip(stale_specs);
    const auto stale_path = write_fixture(stale_fixture.bytes, "nbz-stale-size");
    NbzZipSource stale_source("nbz-stale-size", stale_path);
    assert(stale_source.valid());
    const auto stale_refs = stale_source.enumerate();
    assert(stale_refs.size() == 1U);
    const auto empty_payload = stale_source.read(stale_refs.front().id);
    assert(empty_payload.has_value() && empty_payload->readable());
    assert(empty_payload->bytes.empty());
    assert(empty_payload->byte_provenance.has_value());
    assert(empty_payload->byte_provenance->kind == ByteOriginKind::direct_source_span);
    assert(empty_payload->byte_provenance->transform == ByteTransform::zip_stored);
    {
        std::ofstream stream(stale_path, std::ios::binary | std::ios::app);
        const char marker = '\x7f';
        stream.write(&marker, 1);
        assert(stream.good());
    }
    assert(stale_source.valid());
    const auto stale_payload = stale_source.read(stale_refs.front().id);
    assert(stale_payload.has_value() && !stale_payload->readable());
    assert(stale_payload->bytes.empty());
    assert(!stale_payload->byte_provenance.has_value());
    assert(has_code(
        stale_payload->diagnostics,
        "gdspaces.nbz.safe.source-size-changed"));

    // SafeProductValidation: ZIP64 sentinels remain a bounded-product reject.
    auto zip64_fixture = make_zip(specs);
    patch_u32(zip64_fixture.bytes, zip64_fixture.eocd_offset + 16U, 0xFFFFFFFFU);
    const auto zip64_path = write_fixture(zip64_fixture.bytes, "nbz-zip64");
    NbzZipSource zip64("nbz-zip64", zip64_path);
    assert(!zip64.valid());
    assert(has_code(zip64.diagnostics(), "gdspaces.nbz.safe.zip64-unresolved"));

    // SafeProductValidation: local/central filename equality is hardening.
    auto bad_name_fixture = make_zip(specs);
    bad_name_fixture.bytes[bad_name_fixture.local_name_offsets[0]] = std::byte{'X'};
    const auto bad_name_path = write_fixture(bad_name_fixture.bytes, "nbz-name");
    NbzZipSource bad_name("nbz-name", bad_name_path);
    assert(!bad_name.valid());
    assert(has_code(
        bad_name.diagnostics(), "gdspaces.nbz.safe.central-local-name-mismatch"));

    // Unknown non-zero methods are indexed for evidence but not materialized
    // by the safe product reader (original DMC3 method dispatch differs).
    const std::vector<EntrySpec> unknown_specs{
        EntrySpec{"unknown.bin", 99U, ascii("XYZ"), ascii("XYZ")},
    };
    const auto unknown_fixture = make_zip(unknown_specs);
    const auto unknown_path = write_fixture(unknown_fixture.bytes, "nbz-method");
    NbzZipSource unknown("nbz-method", unknown_path);
    assert(unknown.valid());
    const auto unknown_ref = unknown.enumerate().front();
    const auto unknown_payload = unknown.read(unknown_ref.id);
    assert(unknown_payload.has_value() && !unknown_payload->readable());
    assert(!unknown_payload->byte_provenance.has_value());
    assert(has_code(
        unknown_payload->diagnostics,
        "gdspaces.nbz.safe.compression-method-unsupported"));

    // Duplicate logical paths are legal in ZIP. The canonical resource key
    // includes the central index, so the entry index must still resolve each
    // one to its own member rather than collapsing them.
    const std::vector<EntrySpec> duplicate_specs{
        EntrySpec{"same.txt", 0U, ascii("FIRST"), ascii("FIRST")},
        EntrySpec{"same.txt", 0U, ascii("SECOND"), ascii("SECOND")},
    };
    const auto duplicate_fixture = make_zip(duplicate_specs);
    const auto duplicate_path =
        write_fixture(duplicate_fixture.bytes, "nbz-duplicate-name");
    NbzZipSource duplicate_source("nbz-test", duplicate_path);
    assert(duplicate_source.valid());
    const auto duplicate_refs = duplicate_source.enumerate();
    assert(duplicate_refs.size() == 2U);
    assert(duplicate_refs[0].id.canonical() != duplicate_refs[1].id.canonical());

    const auto first = duplicate_source.read(duplicate_refs[0].id);
    const auto second = duplicate_source.read(duplicate_refs[1].id);
    assert(first.has_value() && first->readable());
    assert(second.has_value() && second->readable());
    assert(first->bytes == ascii("FIRST"));
    assert(second->bytes == ascii("SECOND"));

    // An identity that belongs to no member still resolves to nothing.
    auto absent = duplicate_refs[0].id;
    absent.logical_path = "not-a-member.txt";
    assert(!duplicate_source.read(absent).has_value());

    std::filesystem::remove(duplicate_path);
    std::filesystem::remove(normal_path);
    std::filesystem::remove(bad_offset_path);
    std::filesystem::remove(bad_count_path);
    std::filesystem::remove(overlap_path);
    std::filesystem::remove(wrong_crc_path);
    std::filesystem::remove(stale_path);
    std::filesystem::remove(zip64_path);
    std::filesystem::remove(bad_name_path);
    std::filesystem::remove(unknown_path);
    return 0;
}
