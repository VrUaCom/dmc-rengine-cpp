#include "dmc_rengine/core/sha256.hpp"
#include "dmc_rengine/evidence/artifact.hpp"
#include "dmc_rengine/gdspaces/nbz_zip_artifact_binding.hpp"
#include "dmc_rengine/gdspaces/nbz_zip_source.hpp"
#include "dmc_rengine/profiles/dmc3/nbz_retail_repacker.hpp"

#include <algorithm>
#include <array>
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

constexpr std::uint32_t local_signature = 0x04034B50U;
constexpr std::uint32_t central_signature = 0x02014B50U;
constexpr std::uint32_t eocd_signature = 0x06054B50U;
constexpr std::uint32_t descriptor_signature = 0x08074B50U;

void append_u16(std::vector<std::byte>& out, std::uint16_t value) {
    out.push_back(static_cast<std::byte>(value & 0xFFU));
    out.push_back(static_cast<std::byte>((value >> 8U) & 0xFFU));
}

void append_u32(std::vector<std::byte>& out, std::uint32_t value) {
    for (unsigned shift = 0U; shift < 32U; shift += 8U) {
        out.push_back(static_cast<std::byte>((value >> shift) & 0xFFU));
    }
}

void append_text(std::vector<std::byte>& out, std::string_view text) {
    for (const auto value : text) {
        out.push_back(static_cast<std::byte>(value));
    }
}

void append_bytes(
    std::vector<std::byte>& out,
    std::span<const std::byte> bytes) {
    out.insert(out.end(), bytes.begin(), bytes.end());
}

[[nodiscard]] std::vector<std::byte> ascii(std::string_view text) {
    std::vector<std::byte> out;
    out.reserve(text.size());
    append_text(out, text);
    return out;
}

[[nodiscard]] std::uint32_t crc32_of(std::span<const std::byte> bytes) noexcept {
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
    std::string path;
    std::uint16_t flags{};
    std::uint16_t method{};
    std::uint16_t dos_time{};
    std::uint16_t dos_date{};
    std::uint32_t crc{};
    std::uint32_t compressed_size{};
    std::uint32_t uncompressed_size{};
    std::uint32_t local_offset{};
    std::vector<std::byte> local_extra;
    std::vector<std::byte> central_extra;
    std::string comment;
};

[[nodiscard]] std::vector<std::byte> make_retail_fixture() {
    const std::string path0 = "GData.afs/test.pac";
    const std::string path1 = "SAVEDATA/unchanged.bin";
    const std::array<std::byte, 7> deflated_hello{
        std::byte{0xF3}, std::byte{0x70}, std::byte{0xF5}, std::byte{0xF1},
        std::byte{0xF1}, std::byte{0x07}, std::byte{0x00},
    };
    const auto unchanged = ascii("UNCHANGED");

    FixtureEntry first{
        .path = path0,
        .flags = 0U,
        .method = 8U,
        .dos_time = 0x1234U,
        .dos_date = 0x5678U,
        .crc = 0xC1446436U,
        .compressed_size = static_cast<std::uint32_t>(deflated_hello.size()),
        .uncompressed_size = 5U,
        .local_offset = 0U,
        .local_extra = {std::byte{0xAA}, std::byte{0xBB}, std::byte{0xCC}},
        .central_extra = {
            std::byte{0x01}, std::byte{0x02},
            std::byte{0x03}, std::byte{0x04}},
        .comment = "C0",
    };
    FixtureEntry second{
        .path = path1,
        .flags = 0x0008U,
        .method = 0U,
        .dos_time = 0x2345U,
        .dos_date = 0x6789U,
        .crc = 0xE301DCD6U,
        .compressed_size = static_cast<std::uint32_t>(unchanged.size()),
        .uncompressed_size = static_cast<std::uint32_t>(unchanged.size()),
        .local_offset = 0U,
        .local_extra = {std::byte{0x10}, std::byte{0x20}},
        .central_extra = {std::byte{0x05}, std::byte{0x06}},
        .comment = "C1!",
    };

    std::vector<std::byte> out;
    append_text(out, "SFX!");

    auto append_local = [&](FixtureEntry& entry, std::span<const std::byte> stored) {
        entry.local_offset = static_cast<std::uint32_t>(out.size());
        append_u32(out, local_signature);
        append_u16(out, 20U);
        append_u16(out, entry.flags);
        append_u16(out, entry.method);
        append_u16(out, entry.dos_time);
        append_u16(out, entry.dos_date);
        append_u32(out, entry.crc);
        append_u32(out, entry.compressed_size);
        append_u32(out, entry.uncompressed_size);
        append_u16(out, static_cast<std::uint16_t>(entry.path.size()));
        append_u16(out, static_cast<std::uint16_t>(entry.local_extra.size()));
        append_text(out, entry.path);
        append_bytes(out, entry.local_extra);
        append_bytes(out, stored);
    };

    append_local(first, deflated_hello);
    out.push_back(std::byte{0xDE});
    out.push_back(std::byte{0xAD});

    append_local(second, unchanged);
    append_u32(out, descriptor_signature);
    append_u32(out, second.crc);
    append_u32(out, second.compressed_size);
    append_u32(out, second.uncompressed_size);
    out.push_back(std::byte{0xFE});

    const auto central_offset = static_cast<std::uint32_t>(out.size());
    auto append_central = [&](const FixtureEntry& entry) {
        append_u32(out, central_signature);
        append_u16(out, 0x031EU);
        append_u16(out, 20U);
        append_u16(out, entry.flags);
        append_u16(out, entry.method);
        append_u16(out, entry.dos_time);
        append_u16(out, entry.dos_date);
        append_u32(out, entry.crc);
        append_u32(out, entry.compressed_size);
        append_u32(out, entry.uncompressed_size);
        append_u16(out, static_cast<std::uint16_t>(entry.path.size()));
        append_u16(out, static_cast<std::uint16_t>(entry.central_extra.size()));
        append_u16(out, static_cast<std::uint16_t>(entry.comment.size()));
        append_u16(out, 0U);
        append_u16(out, 0U);
        append_u32(out, 0x20U);
        append_u32(out, entry.local_offset);
        append_text(out, entry.path);
        append_bytes(out, entry.central_extra);
        append_text(out, entry.comment);
    };
    append_central(first);
    append_central(second);
    const auto central_size =
        static_cast<std::uint32_t>(out.size()) - central_offset;

    constexpr std::string_view archive_comment = "ARCHIVE-COMMENT";
    append_u32(out, eocd_signature);
    append_u16(out, 0U);
    append_u16(out, 0U);
    append_u16(out, 2U);
    append_u16(out, 2U);
    append_u32(out, central_size);
    append_u32(out, central_offset);
    append_u16(out, static_cast<std::uint16_t>(archive_comment.size()));
    append_text(out, archive_comment);
    return out;
}

[[nodiscard]] std::filesystem::path temp_path(std::string_view stem) {
    return std::filesystem::temp_directory_path() /
        (std::string{"dmc-rengine-"} + std::string{stem} + ".nbz");
}

void write_file(
    const std::filesystem::path& path,
    std::span<const std::byte> bytes) {
    std::error_code error;
    std::filesystem::remove(path, error);
    std::ofstream stream(path, std::ios::binary | std::ios::trunc);
    stream.write(
        reinterpret_cast<const char*>(bytes.data()),
        static_cast<std::streamsize>(bytes.size()));
    assert(stream.good());
}

[[nodiscard]] std::vector<std::byte> read_file(
    const std::filesystem::path& path) {
    std::ifstream stream(path, std::ios::binary | std::ios::ate);
    assert(stream.good());
    const auto size = stream.tellg();
    assert(size >= 0);
    std::vector<std::byte> bytes(static_cast<std::size_t>(size));
    stream.seekg(0, std::ios::beg);
    if (!bytes.empty()) {
        stream.read(
            reinterpret_cast<char*>(bytes.data()),
            static_cast<std::streamsize>(bytes.size()));
        assert(stream.gcount() == static_cast<std::streamsize>(bytes.size()));
    }
    return bytes;
}

[[nodiscard]] dmc::rengine::evidence::ArtifactIdentity artifact_for(
    std::string id,
    std::span<const std::byte> bytes) {
    const auto digest = dmc::rengine::core::Sha256::compute(bytes);
    return dmc::rengine::evidence::ArtifactIdentity{
        .id = std::move(id),
        .role = "test-nbz-retail",
        .sha256 = digest.hex(),
        .size = static_cast<std::uint64_t>(bytes.size()),
    };
}

[[nodiscard]] const dmc::rengine::gdspaces::ResourceRef* find_ref(
    const std::vector<dmc::rengine::gdspaces::ResourceRef>& refs,
    std::string_view path) {
    const auto found = std::find_if(
        refs.begin(), refs.end(),
        [path](const auto& ref) { return ref.id.logical_path == path; });
    return found == refs.end() ? nullptr : &*found;
}

[[nodiscard]] std::vector<std::byte> read_range(
    const std::filesystem::path& path,
    std::uint64_t offset,
    std::uint64_t size) {
    std::ifstream stream(path, std::ios::binary);
    assert(stream.good());
    stream.seekg(static_cast<std::streamoff>(offset), std::ios::beg);
    assert(stream.good());
    std::vector<std::byte> bytes(static_cast<std::size_t>(size));
    if (!bytes.empty()) {
        stream.read(
            reinterpret_cast<char*>(bytes.data()),
            static_cast<std::streamsize>(bytes.size()));
        assert(stream.gcount() == static_cast<std::streamsize>(bytes.size()));
    }
    return bytes;
}

void zero_range(
    std::vector<std::byte>& bytes,
    std::size_t offset,
    std::size_t size) {
    assert(offset + size <= bytes.size());
    std::fill(
        bytes.begin() + static_cast<std::ptrdiff_t>(offset),
        bytes.begin() + static_cast<std::ptrdiff_t>(offset + size),
        std::byte{0});
}

} // namespace

int main() {
    namespace evidence = dmc::rengine::evidence;
    namespace gdspaces = dmc::rengine::gdspaces;
    namespace dmc3 = dmc::rengine::profiles::dmc3;

    const auto source_bytes = make_retail_fixture();
    const auto source_path = temp_path("retail-source");
    const auto identity_path = temp_path("retail-identity");
    const auto changed_path = temp_path("retail-changed");
    const auto failure_path = temp_path("retail-failure");
    write_file(source_path, source_bytes);
    std::error_code remove_error;
    std::filesystem::remove(identity_path, remove_error);
    std::filesystem::remove(changed_path, remove_error);
    std::filesystem::remove(failure_path, remove_error);

    gdspaces::NbzZipSource source("retail-source", source_path);
    assert(source.valid());
    assert(source.entries().size() == 2U);
    assert(source.entries()[0].compression_method == 8U);
    assert(source.entries()[1].flags == 0x0008U);

    const auto source_artifact = artifact_for(
        "retail-source.nbz",
        std::span<const std::byte>{source_bytes});
    const auto bound = gdspaces::NbzZipArtifactSerializationBinder::bind(
        source,
        source_artifact,
        gdspaces::NbzZipSerializationLimits{.max_metadata_bytes = 1U << 20U},
        gdspaces::NbzZipArtifactBindingLimits{.hash_chunk_bytes = 7U});
    assert(bound.ok());
    assert(bound.snapshot->serialization().entries[1].uses_data_descriptor);

    // Tier 1: no-edit repack must be byte-identical, not merely parse-equivalent.
    const auto identity = dmc3::NbzRetailRepacker::write(
        source,
        *bound.snapshot,
        identity_path,
        {} ,
        dmc3::NbzRetailRepackLimits{
            .io_chunk_bytes = 5U,
            .max_replacement_bytes = 1U << 20U,
            .max_metadata_bytes = 1U << 20U,
        });
    assert(identity.ok());
    assert(identity.receipt->changed_entry_count == 0U);
    assert(identity.receipt->byte_identical);
    assert(identity.receipt->source_artifact.sha256 ==
        identity.receipt->output_artifact.sha256);
    assert(read_file(identity_path) == source_bytes);

    // Tier 2: size-changing replacement converts the changed member to STORE
    // while preserving all unrelated retail metadata and opaque local tails.
    const auto replacement_bytes = ascii("HELLO-CHANGED-AND-LONGER");
    const std::vector<dmc3::NbzRetailReplacement> replacements{
        dmc3::NbzRetailReplacement{
            .central_index = 0U,
            .expected_logical_path = "GData.afs/test.pac",
            .bytes = replacement_bytes,
        },
    };
    const auto changed = dmc3::NbzRetailRepacker::write(
        source,
        *bound.snapshot,
        changed_path,
        replacements,
        dmc3::NbzRetailRepackLimits{
            .io_chunk_bytes = 6U,
            .max_replacement_bytes = 1U << 20U,
            .max_metadata_bytes = 1U << 20U,
        });
    assert(changed.ok());
    assert(changed.receipt->changed_entry_count == 1U);
    assert(!changed.receipt->byte_identical);
    assert(changed.receipt->entries[0].changed);
    assert(changed.receipt->entries[0].original_method == 8U);
    assert(changed.receipt->entries[0].output_method == 0U);
    assert(!changed.receipt->entries[1].changed);
    assert(changed.receipt->entries[1].output_method == 0U);

    gdspaces::NbzZipSource changed_source("retail-changed", changed_path);
    assert(changed_source.valid());
    const auto changed_refs = changed_source.enumerate();
    const auto* changed_ref = find_ref(changed_refs, "GData.afs/test.pac");
    const auto* unchanged_ref = find_ref(changed_refs, "SAVEDATA/unchanged.bin");
    assert(changed_ref != nullptr);
    assert(unchanged_ref != nullptr);
    const auto changed_payload = changed_source.read(changed_ref->id);
    const auto unchanged_payload = changed_source.read(unchanged_ref->id);
    assert(changed_payload.has_value());
    assert(unchanged_payload.has_value());
    assert(changed_payload->bytes == replacement_bytes);
    assert(unchanged_payload->bytes == ascii("UNCHANGED"));

    const auto rebound = gdspaces::NbzZipArtifactSerializationBinder::bind(
        changed_source,
        changed.receipt->output_artifact,
        gdspaces::NbzZipSerializationLimits{.max_metadata_bytes = 1U << 20U},
        gdspaces::NbzZipArtifactBindingLimits{.hash_chunk_bytes = 9U});
    assert(rebound.ok());

    const auto& before = bound.snapshot->serialization();
    const auto& after = rebound.snapshot->serialization();
    assert(before.entries.size() == after.entries.size());
    assert(before.prefix_size == after.prefix_size);
    assert(read_range(source_path, 0U, before.prefix_size) ==
        read_range(changed_path, 0U, after.prefix_size));

    // Changed local metadata: filename/extra/time/version/flags remain; only
    // method/CRC/sizes are intentionally rewritten.
    auto before_local0 = before.entries[0].local_prefix_bytes;
    auto after_local0 = after.entries[0].local_prefix_bytes;
    assert(before_local0.size() == after_local0.size());
    zero_range(before_local0, 8U, 2U);
    zero_range(after_local0, 8U, 2U);
    zero_range(before_local0, 14U, 12U);
    zero_range(after_local0, 14U, 12U);
    assert(before_local0 == after_local0);

    auto before_central0 = before.entries[0].central_record_bytes;
    auto after_central0 = after.entries[0].central_record_bytes;
    assert(before_central0.size() == after_central0.size());
    zero_range(before_central0, 10U, 2U);
    zero_range(after_central0, 10U, 2U);
    zero_range(before_central0, 16U, 12U);
    zero_range(after_central0, 16U, 12U);
    zero_range(before_central0, 42U, 4U);
    zero_range(after_central0, 42U, 4U);
    assert(before_central0 == after_central0);

    // Entire unchanged second local region, including bit-3 descriptor and
    // opaque tail/gap byte, is copied byte-for-byte at its shifted location.
    const auto before_region1 = read_range(
        source_path,
        before.entries[1].local_record_offset,
        before.entries[1].local_region_size);
    const auto after_region1 = read_range(
        changed_path,
        after.entries[1].local_record_offset,
        after.entries[1].local_region_size);
    assert(before_region1 == after_region1);

    auto before_central1 = before.entries[1].central_record_bytes;
    auto after_central1 = after.entries[1].central_record_bytes;
    zero_range(before_central1, 42U, 4U);
    zero_range(after_central1, 42U, 4U);
    assert(before_central1 == after_central1);

    auto before_eocd = before.eocd_bytes;
    auto after_eocd = after.eocd_bytes;
    assert(before_eocd.size() == after_eocd.size());
    zero_range(before_eocd, 16U, 4U);
    zero_range(after_eocd, 16U, 4U);
    assert(before_eocd == after_eocd);

    // A changed bit-3 entry remains fail-closed in this tier.
    const std::vector<dmc3::NbzRetailReplacement> descriptor_replacement{
        dmc3::NbzRetailReplacement{
            .central_index = 1U,
            .expected_logical_path = "SAVEDATA/unchanged.bin",
            .bytes = ascii("NEW"),
        },
    };
    const auto descriptor_blocked = dmc3::NbzRetailRepacker::write(
        source,
        *bound.snapshot,
        failure_path,
        descriptor_replacement);
    assert(!descriptor_blocked.ok());
    assert(
        descriptor_blocked.status ==
        dmc3::NbzRetailRepackStatus::data_descriptor_replacement_unsupported);
    assert(!std::filesystem::exists(failure_path));

    // Exact physical identity/path guards remain fail-closed.
    const std::vector<dmc3::NbzRetailReplacement> wrong_path{
        dmc3::NbzRetailReplacement{
            .central_index = 0U,
            .expected_logical_path = "wrong.pac",
            .bytes = replacement_bytes,
        },
    };
    const auto wrong_path_result = dmc3::NbzRetailRepacker::write(
        source,
        *bound.snapshot,
        failure_path,
        wrong_path);
    assert(!wrong_path_result.ok());
    assert(
        wrong_path_result.status ==
        dmc3::NbzRetailRepackStatus::replacement_path_mismatch);

    const std::vector<dmc3::NbzRetailReplacement> duplicate{
        replacements.front(),
        replacements.front(),
    };
    const auto duplicate_result = dmc3::NbzRetailRepacker::write(
        source,
        *bound.snapshot,
        failure_path,
        duplicate);
    assert(!duplicate_result.ok());
    assert(
        duplicate_result.status ==
        dmc3::NbzRetailRepackStatus::duplicate_replacement);

    // Source artifact is independently revalidated during the writer I/O
    // boundary; stale artifact-bound metadata is not indefinite permission.
    auto tampered_source = source_bytes;
    tampered_source[5U] ^= std::byte{0x01};
    write_file(source_path, tampered_source);
    const auto stale_source = dmc3::NbzRetailRepacker::write(
        source,
        *bound.snapshot,
        failure_path,
        {});
    assert(!stale_source.ok());
    assert(
        stale_source.status ==
            dmc3::NbzRetailRepackStatus::source_artifact_mismatch ||
        stale_source.status == dmc3::NbzRetailRepackStatus::source_read_failure);
    assert(!std::filesystem::exists(failure_path));

    write_file(source_path, source_bytes);

    std::filesystem::remove(source_path, remove_error);
    std::filesystem::remove(identity_path, remove_error);
    std::filesystem::remove(changed_path, remove_error);
    std::filesystem::remove(failure_path, remove_error);
    return 0;
}
