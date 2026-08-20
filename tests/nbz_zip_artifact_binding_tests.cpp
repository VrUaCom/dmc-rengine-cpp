#include "dmc_rengine/core/sha256.hpp"
#include "dmc_rengine/evidence/artifact.hpp"
#include "dmc_rengine/gdspaces/nbz_zip_artifact_binding.hpp"
#include "dmc_rengine/profiles/dmc3/nbz_overlay_writer.hpp"
#include "dmc_rengine/profiles/dmc3/volume_bootstrap_policy.hpp"

#include <algorithm>
#include <cassert>
#include <cctype>
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <fstream>
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

    // Restore the exact artifact and prove the same already-indexed source can
    // be rebound once its underlying bytes again match the expected identity.
    const auto restored_path =
        write_temp_nbz(overlay.bytes, "artifact-binding");
    assert(restored_path == path);
    const auto rebound =
        gdspaces::NbzZipArtifactSerializationBinder::bind(source, expected);
    assert(rebound.ok());
    assert(rebound.snapshot->observed_sha256() == expected.sha256);

    std::error_code remove_error;
    std::filesystem::remove(path, remove_error);
    return 0;
}
