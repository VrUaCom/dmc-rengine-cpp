#include "dmc_rengine/core/sha256.hpp"
#include "dmc_rengine/evidence/artifact.hpp"
#include "dmc_rengine/gdspaces/nbz_zip_artifact_binding.hpp"
#include "dmc_rengine/gdspaces/nbz_zip_artifact_member.hpp"
#include "dmc_rengine/gdspaces/nbz_zip_source.hpp"
#include "dmc_rengine/profiles/dmc3/nbz_overlay_writer.hpp"
#include "dmc_rengine/profiles/dmc3/volume_bootstrap_policy.hpp"

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

void write_file(
    const std::filesystem::path& path,
    std::span<const std::byte> bytes) {
    std::ofstream stream(path, std::ios::binary | std::ios::trunc);
    stream.write(
        reinterpret_cast<const char*>(bytes.data()),
        static_cast<std::streamsize>(bytes.size()));
    assert(stream.good());
}

[[nodiscard]] bool has_error(
    const dmc::rengine::gdspaces::NbzZipArtifactMemberObservationResult& result,
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
    namespace core = dmc::rengine::core;
    namespace evidence = dmc::rengine::evidence;
    namespace gdspaces = dmc::rengine::gdspaces;
    namespace dmc3 = dmc::rengine::profiles::dmc3;

    const auto bootstrap = dmc3::VolumeBootstrapPolicy::plan(
        std::vector<std::uint32_t>{0U});
    assert(bootstrap.valid());

    const auto first_bytes = ascii("artifact-stable-member");
    const auto second_bytes = ascii("untouched-second-member");
    const std::vector<dmc3::NbzOverlayMember> members{
        dmc3::NbzOverlayMember{
            .logical_path = "GData.afs/member.bin",
            .bytes = first_bytes,
        },
        dmc3::NbzOverlayMember{
            .logical_path = "SAVEDATA/second.bin",
            .bytes = second_bytes,
        },
    };
    const auto overlay = dmc3::NbzStoreOverlayWriter::build(bootstrap, members);
    assert(overlay.ok());

    const auto path = std::filesystem::temp_directory_path() /
        "dmc-rengine-artifact-member.nbz";
    std::error_code error;
    std::filesystem::remove(path, error);
    write_file(path, overlay.bytes);

    gdspaces::NbzZipSource source("artifact-member-source", path);
    assert(source.valid());
    assert(source.entries().size() == 2U);

    const auto digest = core::Sha256::compute(
        std::span<const std::byte>{overlay.bytes});
    const evidence::ArtifactIdentity artifact{
        .id = "artifact-member-test",
        .role = "dmc3-retail-nbz",
        .sha256 = digest.hex(),
        .size = static_cast<std::uint64_t>(overlay.bytes.size()),
    };
    const auto bound = gdspaces::NbzZipArtifactSerializationBinder::bind(
        source,
        artifact,
        {},
        gdspaces::NbzZipArtifactBindingLimits{.hash_chunk_bytes = 7U});
    assert(bound.ok());

    const auto observed = gdspaces::NbzZipArtifactMemberObserver::observe(
        source,
        *bound.snapshot,
        source.entries()[0].central_index,
        gdspaces::NbzZipArtifactMemberLimits{.hash_chunk_bytes = 5U});
    assert(observed.ok());
    assert(observed.observation->artifact() == artifact);
    assert(observed.observation->entry().logical_path == "GData.afs/member.bin");
    assert(observed.observation->materialized_bytes().size() == first_bytes.size());
    assert(std::equal(
        observed.observation->materialized_bytes().begin(),
        observed.observation->materialized_bytes().end(),
        first_bytes.begin()));
    assert(observed.observation->byte_provenance().transform ==
        gdspaces::ByteTransform::zip_stored);
    assert(observed.observation->observed_sha256() == artifact.sha256);

    // The bound metadata cannot authorize member bytes from a later archive
    // replacement. Same-size mutation must fail on exact artifact identity.
    auto tampered = overlay.bytes;
    const auto data_offset = static_cast<std::size_t>(source.entries()[0].data_offset);
    assert(data_offset < tampered.size());
    tampered[data_offset] ^= std::byte{0x01};
    write_file(path, tampered);
    const auto stale = gdspaces::NbzZipArtifactMemberObserver::observe(
        source,
        *bound.snapshot,
        source.entries()[0].central_index,
        gdspaces::NbzZipArtifactMemberLimits{.hash_chunk_bytes = 11U});
    assert(!stale.ok());
    assert(has_error(stale, "gdspaces.nbz.artifact-member.hash-mismatch"));

    // Restore the exact artifact and verify bounded failure modes.
    write_file(path, overlay.bytes);
    const auto missing = gdspaces::NbzZipArtifactMemberObserver::observe(
        source, *bound.snapshot, 0xFFFFFFFFU);
    assert(!missing.ok());
    assert(has_error(missing, "gdspaces.nbz.artifact-member.invalid-entry"));

    const auto zero_chunk = gdspaces::NbzZipArtifactMemberObserver::observe(
        source,
        *bound.snapshot,
        source.entries()[0].central_index,
        gdspaces::NbzZipArtifactMemberLimits{.hash_chunk_bytes = 0U});
    assert(!zero_chunk.ok());
    assert(has_error(zero_chunk, "gdspaces.nbz.artifact-member.budget"));

    // Fixed classic-ZIP method-8 fixture with bit-3 data descriptor. This
    // validates the exact retail acquisition path used for raw-DEFLATE members,
    // not just STORE overlays.
    const auto method8_fixture = from_hex(
        "504b0304140008000800831822500000000000000000000000000b000000"
        "6d6574686f64382e62696e"
        "4b494dcb492c494dd1cd2fca4ccfcc4bcc0100"
        "504b0708c0b3be781300000011000000"
        "504b0102140314000800080083182250c0b3be7813000000110000000b000000"
        "00000000000020000000000000006d6574686f64382e62696e"
        "504b05060000000001000100390000004c0000000000");
    const auto method8_path = std::filesystem::temp_directory_path() /
        "dmc-rengine-artifact-member-method8.nbz";
    std::filesystem::remove(method8_path, error);
    write_file(method8_path, method8_fixture);

    gdspaces::NbzZipSource method8_source(
        "artifact-member-method8-source", method8_path);
    assert(method8_source.valid());
    assert(method8_source.entries().size() == 1U);
    assert(method8_source.entries()[0].compression_method == 8U);
    assert((method8_source.entries()[0].flags & 0x0008U) != 0U);

    const auto method8_digest = core::Sha256::compute(
        std::span<const std::byte>{method8_fixture});
    const evidence::ArtifactIdentity method8_artifact{
        .id = "artifact-member-method8-test",
        .role = "dmc3-retail-nbz",
        .sha256 = method8_digest.hex(),
        .size = static_cast<std::uint64_t>(method8_fixture.size()),
    };
    const auto method8_bound = gdspaces::NbzZipArtifactSerializationBinder::bind(
        method8_source,
        method8_artifact,
        {},
        gdspaces::NbzZipArtifactBindingLimits{.hash_chunk_bytes = 9U});
    assert(method8_bound.ok());

    const auto method8_observed = gdspaces::NbzZipArtifactMemberObserver::observe(
        method8_source,
        *method8_bound.snapshot,
        method8_source.entries()[0].central_index,
        gdspaces::NbzZipArtifactMemberLimits{.hash_chunk_bytes = 7U});
    assert(method8_observed.ok());
    const auto expected_method8 = ascii("deflated-original");
    assert(method8_observed.observation->materialized_bytes().size() ==
        expected_method8.size());
    assert(std::equal(
        method8_observed.observation->materialized_bytes().begin(),
        method8_observed.observation->materialized_bytes().end(),
        expected_method8.begin()));
    assert(method8_observed.observation->byte_provenance().transform ==
        gdspaces::ByteTransform::zip_deflate);
    assert(method8_observed.observation->observed_sha256() ==
        method8_artifact.sha256);

    std::filesystem::remove(path, error);
    std::filesystem::remove(method8_path, error);
    return 0;
}
