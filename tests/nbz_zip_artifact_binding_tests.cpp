#include "dmc_rengine/core/sha256.hpp"
#include "dmc_rengine/evidence/artifact.hpp"
#include "dmc_rengine/gdspaces/nbz_zip_artifact_binding.hpp"
#include "dmc_rengine/gdspaces/nbz_zip_repacker.hpp"
#include "dmc_rengine/profiles/dmc3/nbz_overlay_writer.hpp"
#include "dmc_rengine/profiles/dmc3/volume_bootstrap_policy.hpp"

#include <algorithm>
#include <cassert>
#include <cctype>
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>
#include <vector>

namespace {

[[nodiscard]] std::vector<std::byte> ascii(std::string_view text) {
    std::vector<std::byte> bytes;
    bytes.reserve(text.size());
    for (const char value : text) {
        bytes.push_back(static_cast<std::byte>(value));
    }
    return bytes;
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
            static_cast<std::uint8_t>(
                (nibble(hex[index]) << 4U) | nibble(hex[index + 1U]))));
    }
    return bytes;
}

void put_u16_le(
    std::span<std::byte> bytes,
    std::size_t offset,
    std::uint16_t value) {
    bytes[offset] = static_cast<std::byte>(value & 0xFFU);
    bytes[offset + 1U] = static_cast<std::byte>((value >> 8U) & 0xFFU);
}

void put_u32_le(
    std::span<std::byte> bytes,
    std::size_t offset,
    std::uint32_t value) {
    for (unsigned shift = 0U; shift < 32U; shift += 8U) {
        bytes[offset + shift / 8U] =
            static_cast<std::byte>((value >> shift) & 0xFFU);
    }
}

[[nodiscard]] std::filesystem::path write_temp_nbz(
    std::span<const std::byte> bytes,
    std::string_view stem) {
    const auto path = std::filesystem::temp_directory_path() /
        (std::string{"dmc-rengine-"} + std::string{stem} + ".nbz");
    std::ofstream stream(path, std::ios::binary | std::ios::trunc);
    stream.write(
        reinterpret_cast<const char*>(bytes.data()),
        static_cast<std::streamsize>(bytes.size()));
    assert(stream.good());
    return path;
}

void remove_if_present(const std::filesystem::path& path) {
    std::error_code error;
    std::filesystem::remove(path, error);
    std::filesystem::remove(
        std::filesystem::path{path.string() + ".dmc-rengine-repack.tmp"},
        error);
}

[[nodiscard]] std::string uppercase_hex(std::string value) {
    std::transform(
        value.begin(), value.end(), value.begin(),
        [](unsigned char character) {
            return static_cast<char>(std::toupper(character));
        });
    return value;
}

[[nodiscard]] dmc::rengine::evidence::ArtifactIdentity artifact_for(
    std::span<const std::byte> bytes,
    std::string sha_override = {}) {
    auto sha = dmc::rengine::core::Sha256::compute(bytes).hex();
    if (!sha_override.empty()) {
        sha = std::move(sha_override);
    }
    return dmc::rengine::evidence::ArtifactIdentity{
        .id = "test-dmc3-volume",
        .role = "dmc3-retail-nbz",
        .sha256 = std::move(sha),
        .size = static_cast<std::uint64_t>(bytes.size()),
    };
}

[[nodiscard]] bool has_error(
    const dmc::rengine::gdspaces::NbzZipArtifactBindingResult& result,
    std::string_view code) {
    return std::any_of(
        result.diagnostics.begin(), result.diagnostics.end(),
        [code](const auto& diagnostic) {
            return diagnostic.severity ==
                    dmc::rengine::gdspaces::DiagnosticSeverity::error &&
                diagnostic.code == code;
        });
}

[[nodiscard]] std::optional<dmc::rengine::gdspaces::ResourcePayload>
read_path(
    const dmc::rengine::gdspaces::NbzZipSource& source,
    std::string_view logical_path) {
    const auto refs = source.enumerate();
    const auto found = std::find_if(
        refs.begin(), refs.end(),
        [logical_path](const auto& resource) {
            return resource.id.logical_path == logical_path;
        });
    if (found == refs.end()) {
        return std::nullopt;
    }
    return source.read(found->id);
}

} // namespace

int main() {
    namespace gdspaces = dmc::rengine::gdspaces;
    namespace dmc3 = dmc::rengine::profiles::dmc3;

    static_assert(!std::is_aggregate_v<
        gdspaces::ArtifactBoundNbzZipSerializationSnapshot>);

    const auto bootstrap = dmc3::VolumeBootstrapPolicy::plan(
        std::vector<std::uint32_t>{0U, 1U});
    assert(bootstrap.valid());

    const std::vector<dmc3::NbzOverlayMember> members{
        dmc3::NbzOverlayMember{
            .logical_path = "GData.afs/test.bin",
            .bytes = ascii("artifact-bound-member"),
        },
        dmc3::NbzOverlayMember{
            .logical_path = "SAVEDATA/second.bin",
            .bytes = ascii("second-member"),
        },
    };
    const auto overlay = dmc3::NbzStoreOverlayWriter::build(
        bootstrap, members);
    assert(overlay.ok());

    const auto path = write_temp_nbz(overlay.bytes, "artifact-binding");
    gdspaces::NbzZipSource source("artifact-bound-source", path);
    assert(source.valid());

    const auto expected = artifact_for(overlay.bytes);
    const auto bound = gdspaces::NbzZipArtifactSerializationBinder::bind(
        source,
        expected,
        {},
        gdspaces::NbzZipArtifactBindingLimits{.hash_chunk_bytes = 7U});
    assert(bound.ok());
    assert(bound.snapshot.has_value());
    assert(bound.snapshot->artifact() == expected);
    assert(bound.snapshot->serialization().valid());
    assert(bound.snapshot->serialization().source_id == source.id());
    assert(bound.snapshot->serialization().archive_size == overlay.bytes.size());
    assert(bound.snapshot->serialization().entries.size() == members.size());
    assert(bound.snapshot->observed_sha256() == expected.sha256);

    // ArtifactIdentity accepts hexadecimal case variants; binding compares the
    // digest semantically rather than requiring caller-specific text casing.
    auto uppercase = expected;
    uppercase.sha256 = uppercase_hex(uppercase.sha256);
    const auto uppercase_bound =
        gdspaces::NbzZipArtifactSerializationBinder::bind(
            source,
            uppercase,
            {},
            gdspaces::NbzZipArtifactBindingLimits{.hash_chunk_bytes = 13U});
    assert(uppercase_bound.ok());
    assert(uppercase_bound.snapshot->artifact().sha256 == uppercase.sha256);

    auto wrong_sha = expected;
    wrong_sha.sha256[0] = wrong_sha.sha256[0] == '0' ? '1' : '0';
    const auto wrong_hash_result =
        gdspaces::NbzZipArtifactSerializationBinder::bind(source, wrong_sha);
    assert(!wrong_hash_result.ok());
    assert(has_error(
        wrong_hash_result,
        "gdspaces.nbz.artifact-binding.hash-mismatch"));

    auto wrong_size = expected;
    ++wrong_size.size;
    const auto wrong_size_result =
        gdspaces::NbzZipArtifactSerializationBinder::bind(source, wrong_size);
    assert(!wrong_size_result.ok());
    assert(has_error(
        wrong_size_result,
        "gdspaces.nbz.artifact-binding.identity-size"));

    const auto zero_chunk_result =
        gdspaces::NbzZipArtifactSerializationBinder::bind(
            source,
            expected,
            {},
            gdspaces::NbzZipArtifactBindingLimits{.hash_chunk_bytes = 0U});
    assert(!zero_chunk_result.ok());
    assert(has_error(
        zero_chunk_result,
        "gdspaces.nbz.artifact-binding.hash-chunk"));

    // The canonical scanner can now operate on an explicit captured-byte view.
    // Mutating the backing path while that scan runs must not change the bytes
    // being validated, which is the key seam used by artifact binding.
    auto tampered_bytes = overlay.bytes;
    assert(!tampered_bytes.empty());
    tampered_bytes[0] ^= std::byte{0x01};

    bool mutated_during_scan = false;
    gdspaces::NbzZipSerializationReadExact captured_reader =
        [&](std::uint64_t offset, std::span<std::byte> output) {
            if (!mutated_during_scan) {
                const auto mutated_path =
                    write_temp_nbz(tampered_bytes, "artifact-binding");
                assert(mutated_path == path);
                mutated_during_scan = true;
            }
            if (offset > overlay.bytes.size() ||
                output.size() > overlay.bytes.size() -
                    static_cast<std::size_t>(offset)) {
                return false;
            }
            std::copy_n(
                overlay.bytes.begin() +
                    static_cast<std::ptrdiff_t>(offset),
                static_cast<std::ptrdiff_t>(output.size()),
                output.begin());
            return true;
        };
    const auto captured_scan =
        gdspaces::NbzZipSerializationScanner::scan_with_reader(
            source, captured_reader);
    assert(mutated_during_scan);
    assert(captured_scan.ok());

    // A source index can become stale if the archive is replaced after source
    // construction. Binding observes the actual path once, so a same-size
    // replacement cannot inherit trusted metadata from the earlier index.
    const auto stale_source_result =
        gdspaces::NbzZipArtifactSerializationBinder::bind(source, expected);
    assert(!stale_source_result.ok());
    assert(has_error(
        stale_source_result,
        "gdspaces.nbz.artifact-binding.hash-mismatch"));

    // Stronger stale-index regression: replace only accepted EOCD receipt
    // metadata, keep the archive size and entry framing intact, and provide the
    // correct SHA-256 for the replacement artifact. A fresh NbzZipSource still
    // accepts this ZIP with its compatibility warning, but the old source index
    // must not receive authority over the replacement's terminal EOCD.
    assert(source.index_receipt().has_value());
    auto eocd_replacement_bytes = overlay.bytes;
    const auto eocd_offset = static_cast<std::size_t>(
        source.index_receipt()->eocd_offset);
    assert(eocd_offset + 20U <= eocd_replacement_bytes.size());
    eocd_replacement_bytes[eocd_offset + 16U] ^= std::byte{0x01};
    const auto eocd_replacement_path =
        write_temp_nbz(eocd_replacement_bytes, "artifact-binding");
    assert(eocd_replacement_path == path);

    gdspaces::NbzZipSource replacement_source(
        "artifact-bound-replacement-source", path);
    assert(replacement_source.valid());
    assert(replacement_source.index_receipt().has_value());
    assert(
        replacement_source.index_receipt()->declared_central_offset !=
        source.index_receipt()->declared_central_offset);

    const auto replacement_identity = artifact_for(eocd_replacement_bytes);
    const auto stale_eocd_result =
        gdspaces::NbzZipArtifactSerializationBinder::bind(
            source, replacement_identity);
    assert(!stale_eocd_result.ok());
    assert(has_error(
        stale_eocd_result,
        "gdspaces.nbz.serialization.eocd-receipt-mismatch"));

    // Restore the exact artifact and prove the same already-indexed source can
    // be rebound once its underlying bytes again match the expected identity.
    const auto restored_path =
        write_temp_nbz(overlay.bytes, "artifact-binding");
    assert(restored_path == path);
    const auto rebound =
        gdspaces::NbzZipArtifactSerializationBinder::bind(source, expected);
    assert(rebound.ok());
    assert(rebound.snapshot->observed_sha256() == expected.sha256);

    // Writer-side identity invariant: invoking the retail repacker with no
    // replacements must preserve the complete archive byte identity.
    const auto identity_path = std::filesystem::temp_directory_path() /
        "dmc-rengine-retail-identity.nbz";
    remove_if_present(identity_path);
    const auto identity_repack = gdspaces::NbzZipRetailRepacker::write(
        source,
        *rebound.snapshot,
        {},
        identity_path,
        gdspaces::NbzZipRetailRepackLimits{.io_chunk_bytes = 11U});
    assert(identity_repack.ok());
    assert(identity_repack.receipt->source_sha256 == expected.sha256);
    assert(identity_repack.receipt->output_sha256 == expected.sha256);
    assert(identity_repack.receipt->source_size == overlay.bytes.size());
    assert(identity_repack.receipt->output_size == overlay.bytes.size());

    // First size-changing retail authoring receipt. The first STORE member grows
    // while the second physical local region stays opaque/byte-preserved; local
    // offsets, central metadata and EOCD are rebuilt and canonical reopen must
    // materialize the exact requested bytes.
    const auto changed_bytes = ascii(
        "artifact-bound-member-expanded-for-retail-repack");
    const std::vector<gdspaces::NbzZipMemberReplacement> replacements{
        gdspaces::NbzZipMemberReplacement{
            .central_index = 0U,
            .materialized_bytes = changed_bytes,
        },
    };
    const auto changed_path = std::filesystem::temp_directory_path() /
        "dmc-rengine-retail-changed.nbz";
    remove_if_present(changed_path);
    const auto changed_repack = gdspaces::NbzZipRetailRepacker::write(
        source,
        *rebound.snapshot,
        replacements,
        changed_path,
        gdspaces::NbzZipRetailRepackLimits{.io_chunk_bytes = 13U});
    assert(changed_repack.ok());
    assert(changed_repack.receipt->output_size > overlay.bytes.size());
    assert(changed_repack.receipt->entries.size() == members.size());
    assert(changed_repack.receipt->entries[0].changed);
    assert(!changed_repack.receipt->entries[1].changed);
    assert(
        changed_repack.receipt->entries[1].local_header_offset >
        source.entries()[1].local_header_offset);

    gdspaces::NbzZipSource changed_source(
        "retail-changed-reopen", changed_path);
    assert(changed_source.valid());
    const auto changed_payload = read_path(changed_source, "GData.afs/test.bin");
    assert(changed_payload.has_value());
    assert(changed_payload->bytes == changed_bytes);
    const auto untouched_payload = read_path(
        changed_source, "SAVEDATA/second.bin");
    assert(untouched_payload.has_value());
    assert(untouched_payload->bytes == members[1].bytes);

    // Method-8 + bit-3 data-descriptor authoring regression. This is a fixed
    // synthetic classic-ZIP fixture with one raw-DEFLATE member, zero local
    // CRC/size fields, and a signed 16-byte descriptor. The repacker must keep
    // method 8, generate deterministic stored-block DEFLATE, rewrite the
    // descriptor and central sizes, then reopen/materialize the exact edit.
    const auto method8_fixture = from_hex(
        "504b0304140008000800831822500000000000000000000000000b000000"
        "6d6574686f64382e62696e"
        "4b494dcb492c494dd1cd2fca4ccfcc4bcc0100"
        "504b0708c0b3be781300000011000000"
        "504b0102140314000800080083182250c0b3be7813000000110000000b000000"
        "00000000000020000000000000006d6574686f64382e62696e"
        "504b05060000000001000100390000004c0000000000");
    const auto method8_source_path = write_temp_nbz(
        method8_fixture, "retail-method8-source");
    gdspaces::NbzZipSource method8_source(
        "retail-method8-source", method8_source_path);
    assert(method8_source.valid());
    assert(method8_source.entries().size() == 1U);
    assert(method8_source.entries()[0].compression_method == 8U);
    assert((method8_source.entries()[0].flags & 0x0008U) != 0U);

    const auto method8_identity = artifact_for(method8_fixture);
    const auto method8_bound = gdspaces::NbzZipArtifactSerializationBinder::bind(
        method8_source,
        method8_identity,
        {},
        gdspaces::NbzZipArtifactBindingLimits{.hash_chunk_bytes = 9U});
    assert(method8_bound.ok());

    const auto method8_changed_bytes = ascii(
        "deflated-replacement-with-size-change-and-data-descriptor");
    const std::vector<gdspaces::NbzZipMemberReplacement> method8_replacements{
        gdspaces::NbzZipMemberReplacement{
            .central_index = 0U,
            .materialized_bytes = method8_changed_bytes,
        },
    };
    const auto method8_output_path = std::filesystem::temp_directory_path() /
        "dmc-rengine-retail-method8-output.nbz";
    remove_if_present(method8_output_path);
    const auto method8_repack = gdspaces::NbzZipRetailRepacker::write(
        method8_source,
        *method8_bound.snapshot,
        method8_replacements,
        method8_output_path,
        gdspaces::NbzZipRetailRepackLimits{.io_chunk_bytes = 7U});
    assert(method8_repack.ok());
    assert(method8_repack.receipt->entries.size() == 1U);
    assert(method8_repack.receipt->entries[0].changed);
    assert(method8_repack.receipt->entries[0].compression_method == 8U);
    assert(
        method8_repack.receipt->entries[0].compressed_size ==
        method8_changed_bytes.size() + 5U);

    gdspaces::NbzZipSource method8_reopened(
        "retail-method8-reopen", method8_output_path);
    assert(method8_reopened.valid());
    assert(method8_reopened.entries()[0].compression_method == 8U);
    assert((method8_reopened.entries()[0].flags & 0x0008U) != 0U);
    const auto method8_payload = read_path(method8_reopened, "method8.bin");
    assert(method8_payload.has_value());
    assert(method8_payload->bytes == method8_changed_bytes);

    // Physical alias arbitration. Duplicate central identities can point at one
    // local record, but a changed physical record must be acknowledged by every
    // alias and all aliases must request byte-identical materialized payloads.
    const std::vector<dmc3::NbzOverlayMember> alias_member{
        dmc3::NbzOverlayMember{
            .logical_path = "alias.bin",
            .bytes = ascii("alias-original"),
        },
    };
    const auto alias_base = dmc3::NbzStoreOverlayWriter::build(
        bootstrap, alias_member);
    assert(alias_base.ok());
    assert(alias_base.receipt.has_value());
    assert(alias_base.receipt->members.size() == 1U);

    const auto alias_central_offset = static_cast<std::size_t>(
        alias_base.receipt->central_offset);
    const auto alias_central_size = static_cast<std::size_t>(
        alias_base.receipt->central_size);
    assert(alias_central_size > 0U);
    assert(
        alias_central_offset + alias_central_size + 22U ==
        alias_base.bytes.size());

    std::vector<std::byte> alias_fixture;
    alias_fixture.reserve(alias_base.bytes.size() + alias_central_size);
    alias_fixture.insert(
        alias_fixture.end(),
        alias_base.bytes.begin(),
        alias_base.bytes.begin() +
            static_cast<std::ptrdiff_t>(alias_central_offset));
    const auto central_begin = alias_base.bytes.begin() +
        static_cast<std::ptrdiff_t>(alias_central_offset);
    const auto central_end = central_begin +
        static_cast<std::ptrdiff_t>(alias_central_size);
    alias_fixture.insert(alias_fixture.end(), central_begin, central_end);
    alias_fixture.insert(alias_fixture.end(), central_begin, central_end);
    const auto alias_eocd_offset = alias_fixture.size();
    alias_fixture.insert(
        alias_fixture.end(),
        central_end,
        alias_base.bytes.end());
    assert(alias_fixture.size() == alias_base.bytes.size() + alias_central_size);
    put_u16_le(alias_fixture, alias_eocd_offset + 8U, 2U);
    put_u16_le(alias_fixture, alias_eocd_offset + 10U, 2U);
    put_u32_le(
        alias_fixture,
        alias_eocd_offset + 12U,
        static_cast<std::uint32_t>(alias_central_size * 2U));
    put_u32_le(
        alias_fixture,
        alias_eocd_offset + 16U,
        static_cast<std::uint32_t>(alias_central_offset));

    const auto alias_source_path = write_temp_nbz(
        alias_fixture, "retail-alias-source");
    gdspaces::NbzZipSource alias_source(
        "retail-alias-source", alias_source_path);
    assert(alias_source.valid());
    assert(alias_source.entries().size() == 2U);
    assert(
        alias_source.entries()[0].local_header_offset ==
        alias_source.entries()[1].local_header_offset);

    const auto alias_identity = artifact_for(alias_fixture);
    const auto alias_bound = gdspaces::NbzZipArtifactSerializationBinder::bind(
        alias_source,
        alias_identity,
        {},
        gdspaces::NbzZipArtifactBindingLimits{.hash_chunk_bytes = 5U});
    assert(alias_bound.ok());

    const auto alias_changed = ascii("alias-expanded-replacement");
    const auto alias_other = ascii("alias-conflicting-replacement");
    const auto alias_incomplete_path = std::filesystem::temp_directory_path() /
        "dmc-rengine-retail-alias-incomplete.nbz";
    remove_if_present(alias_incomplete_path);
    const std::vector<gdspaces::NbzZipMemberReplacement> alias_incomplete{
        gdspaces::NbzZipMemberReplacement{
            .central_index = 0U,
            .materialized_bytes = alias_changed,
        },
    };
    const auto alias_incomplete_result = gdspaces::NbzZipRetailRepacker::write(
        alias_source,
        *alias_bound.snapshot,
        alias_incomplete,
        alias_incomplete_path,
        gdspaces::NbzZipRetailRepackLimits{.io_chunk_bytes = 5U});
    assert(!alias_incomplete_result.ok());
    assert(
        alias_incomplete_result.status ==
        gdspaces::NbzZipRetailRepackStatus::alias_replacement_incomplete);
    assert(!std::filesystem::exists(alias_incomplete_path));

    const auto alias_conflict_path = std::filesystem::temp_directory_path() /
        "dmc-rengine-retail-alias-conflict.nbz";
    remove_if_present(alias_conflict_path);
    const std::vector<gdspaces::NbzZipMemberReplacement> alias_conflict{
        gdspaces::NbzZipMemberReplacement{
            .central_index = 0U,
            .materialized_bytes = alias_changed,
        },
        gdspaces::NbzZipMemberReplacement{
            .central_index = 1U,
            .materialized_bytes = alias_other,
        },
    };
    const auto alias_conflict_result = gdspaces::NbzZipRetailRepacker::write(
        alias_source,
        *alias_bound.snapshot,
        alias_conflict,
        alias_conflict_path,
        gdspaces::NbzZipRetailRepackLimits{.io_chunk_bytes = 5U});
    assert(!alias_conflict_result.ok());
    assert(
        alias_conflict_result.status ==
        gdspaces::NbzZipRetailRepackStatus::alias_replacement_conflict);
    assert(!std::filesystem::exists(alias_conflict_path));

    const auto alias_output_path = std::filesystem::temp_directory_path() /
        "dmc-rengine-retail-alias-output.nbz";
    remove_if_present(alias_output_path);
    const std::vector<gdspaces::NbzZipMemberReplacement> alias_replacements{
        gdspaces::NbzZipMemberReplacement{
            .central_index = 0U,
            .materialized_bytes = alias_changed,
        },
        gdspaces::NbzZipMemberReplacement{
            .central_index = 1U,
            .materialized_bytes = alias_changed,
        },
    };
    const auto alias_repack = gdspaces::NbzZipRetailRepacker::write(
        alias_source,
        *alias_bound.snapshot,
        alias_replacements,
        alias_output_path,
        gdspaces::NbzZipRetailRepackLimits{.io_chunk_bytes = 5U});
    assert(alias_repack.ok());
    assert(alias_repack.receipt->entries.size() == 2U);
    assert(alias_repack.receipt->entries[0].changed);
    assert(alias_repack.receipt->entries[1].changed);
    assert(
        alias_repack.receipt->entries[0].local_header_offset ==
        alias_repack.receipt->entries[1].local_header_offset);

    gdspaces::NbzZipSource alias_reopened(
        "retail-alias-reopen", alias_output_path);
    assert(alias_reopened.valid());
    assert(alias_reopened.entries().size() == 2U);
    assert(
        alias_reopened.entries()[0].local_header_offset ==
        alias_reopened.entries()[1].local_header_offset);
    const auto alias_payload = read_path(alias_reopened, "alias.bin");
    assert(alias_payload.has_value());
    assert(alias_payload->bytes == alias_changed);

    std::error_code remove_error;
    std::filesystem::remove(path, remove_error);
    std::filesystem::remove(identity_path, remove_error);
    std::filesystem::remove(changed_path, remove_error);
    std::filesystem::remove(method8_source_path, remove_error);
    std::filesystem::remove(method8_output_path, remove_error);
    std::filesystem::remove(alias_source_path, remove_error);
    std::filesystem::remove(alias_incomplete_path, remove_error);
    std::filesystem::remove(alias_conflict_path, remove_error);
    std::filesystem::remove(alias_output_path, remove_error);
    return 0;
}
