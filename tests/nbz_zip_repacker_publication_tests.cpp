#include "dmc_rengine/core/no_replace_publication.hpp"
#include "dmc_rengine/core/sha256.hpp"
#include "dmc_rengine/evidence/artifact.hpp"
#include "dmc_rengine/gdspaces/nbz_zip_artifact_binding.hpp"
#include "dmc_rengine/gdspaces/nbz_zip_repacker.hpp"
#include "dmc_rengine/gdspaces/nbz_zip_source.hpp"
#include "dmc_rengine/profiles/dmc3/nbz_overlay_writer.hpp"
#include "dmc_rengine/profiles/dmc3/volume_bootstrap_policy.hpp"

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

void write_file(
    const std::filesystem::path& path,
    std::span<const std::byte> bytes) {
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
    const auto end = stream.tellg();
    assert(end >= 0);
    std::vector<std::byte> bytes(static_cast<std::size_t>(end));
    stream.seekg(0, std::ios::beg);
    if (!bytes.empty()) {
        stream.read(
            reinterpret_cast<char*>(bytes.data()),
            static_cast<std::streamsize>(bytes.size()));
        assert(stream.gcount() == static_cast<std::streamsize>(bytes.size()));
    }
    return bytes;
}

void cleanup(const std::filesystem::path& path) {
    std::error_code error;
    std::filesystem::remove_all(path, error);
}

} // namespace

int main() {
    namespace core = dmc::rengine::core;
    namespace gdspaces = dmc::rengine::gdspaces;
    namespace dmc3 = dmc::rengine::profiles::dmc3;

    const auto root = std::filesystem::temp_directory_path();
    const auto sentinel = ascii("DO-NOT-CLOBBER");

    const auto shared_destination = root / "dmc-rengine-shared-publication.bin";
    const auto shared_staging = std::filesystem::path{
        shared_destination.string() + ".dmc-rengine-publish.staging"};
    cleanup(shared_destination);
    cleanup(shared_staging);
    const auto shared_bytes = ascii("ATOMIC-PUBLICATION");

    // Successful byte publication is exact and cleans owned staging.
    const auto shared_result = core::publish_bytes_no_replace(
        shared_destination, shared_bytes);
    assert(shared_result.ok());
    assert(read_file(shared_destination) == shared_bytes);
    assert(!std::filesystem::exists(shared_staging));
    cleanup(shared_destination);

    // Failed staged validation must never make a final path visible.
    bool validator_observed_complete_bytes = false;
    const auto rejected_shared = core::publish_bytes_no_replace(
        shared_destination,
        shared_bytes,
        [&](const std::filesystem::path& staged_file) {
            validator_observed_complete_bytes = read_file(staged_file) == shared_bytes;
            return false;
        });
    assert(!rejected_shared.ok());
    assert(rejected_shared.status ==
        core::NoReplacePublicationStatus::staging_validation_failed);
    assert(validator_observed_complete_bytes);
    assert(!std::filesystem::exists(shared_destination));
    assert(!std::filesystem::exists(shared_staging));

    // Existing destination is never truncated or replaced.
    write_file(shared_destination, sentinel);
    const auto existing_shared = core::publish_bytes_no_replace(
        shared_destination, shared_bytes);
    assert(!existing_shared.ok());
    assert(existing_shared.status ==
        core::NoReplacePublicationStatus::destination_exists);
    assert(read_file(shared_destination) == sentinel);
    assert(!std::filesystem::exists(shared_staging));
    cleanup(shared_destination);

    // A pre-existing staging reservation belongs to another publisher.
    std::error_code error;
    assert(std::filesystem::create_directory(shared_staging, error));
    assert(!error);
    const auto shared_owner = shared_staging / "owner.txt";
    write_file(shared_owner, sentinel);
    const auto reserved_shared = core::publish_bytes_no_replace(
        shared_destination, shared_bytes);
    assert(!reserved_shared.ok());
    assert(reserved_shared.status ==
        core::NoReplacePublicationStatus::staging_conflict);
    assert(!std::filesystem::exists(shared_destination));
    assert(read_file(shared_owner) == sentinel);
    cleanup(shared_staging);

    // Model the exact TOCTOU window from exists()->ofstream: validated staging
    // exists first, then another actor creates the destination before commit.
    const auto race_staged = root / "dmc-rengine-race-staged.bin";
    const auto race_destination = root / "dmc-rengine-race-destination.bin";
    cleanup(race_staged);
    cleanup(race_destination);
    write_file(race_staged, shared_bytes);
    write_file(race_destination, sentinel);
    const auto race_result = core::publish_validated_file_no_replace(
        race_staged, race_destination);
    assert(!race_result.ok());
    assert(race_result.status ==
        core::NoReplacePublicationStatus::destination_exists);
    assert(read_file(race_destination) == sentinel);
    assert(read_file(race_staged) == shared_bytes);
    cleanup(race_staged);
    cleanup(race_destination);

    const auto bootstrap = dmc3::VolumeBootstrapPolicy::plan(
        std::vector<std::uint32_t>{0U});
    assert(bootstrap.valid());

    const std::vector<dmc3::NbzOverlayMember> members{
        dmc3::NbzOverlayMember{
            .logical_path = "GData.afs/publication.bin",
            .bytes = ascii("PUBLICATION-SOURCE"),
        },
    };
    const auto generated = dmc3::NbzStoreOverlayWriter::build(
        bootstrap, members);
    assert(generated.ok());

    const auto source_path = root / "dmc-rengine-nbz-publication-source.nbz";
    const auto destination = root / "dmc-rengine-nbz-publication-output.nbz";
    const auto staging = std::filesystem::path{
        destination.string() + ".dmc-rengine-repack.staging"};
    cleanup(source_path);
    cleanup(destination);
    cleanup(staging);
    write_file(source_path, generated.bytes);

    gdspaces::NbzZipSource source("publication-source", source_path);
    assert(source.valid());
    const auto digest = dmc::rengine::core::Sha256::compute(
        std::span<const std::byte>{generated.bytes.data(), generated.bytes.size()});
    const dmc::rengine::evidence::ArtifactIdentity artifact{
        .id = "publication-source",
        .role = "dmc3-retail-nbz",
        .sha256 = digest.hex(),
        .size = static_cast<std::uint64_t>(generated.bytes.size()),
    };
    const auto bound = gdspaces::NbzZipArtifactSerializationBinder::bind(
        source, artifact);
    assert(bound.ok());

    // Repacker retains its existing no-replace contract.
    write_file(destination, sentinel);
    const auto existing_result = gdspaces::NbzZipRetailRepacker::write(
        source, *bound.snapshot, {}, destination);
    assert(!existing_result.ok());
    assert(existing_result.status ==
        gdspaces::NbzZipRetailRepackStatus::invalid_destination);
    assert(read_file(destination) == sentinel);
    cleanup(destination);

    error.clear();
    assert(std::filesystem::create_directory(staging, error));
    assert(!error);
    const auto staging_sentinel = staging / "owner.txt";
    write_file(staging_sentinel, sentinel);
    const auto reserved_result = gdspaces::NbzZipRetailRepacker::write(
        source, *bound.snapshot, {}, destination);
    assert(!reserved_result.ok());
    assert(reserved_result.status ==
        gdspaces::NbzZipRetailRepackStatus::invalid_destination);
    assert(!std::filesystem::exists(destination));
    assert(read_file(staging_sentinel) == sentinel);
    cleanup(staging);

    const auto result = gdspaces::NbzZipRetailRepacker::write(
        source,
        *bound.snapshot,
        {},
        destination,
        gdspaces::NbzZipRetailRepackLimits{.io_chunk_bytes = 7U});
    assert(result.ok());
    assert(std::filesystem::exists(destination));
    assert(!std::filesystem::exists(staging));
    assert(read_file(destination) == generated.bytes);

    gdspaces::NbzZipSource reopened("publication-output", destination);
    assert(reopened.valid());
    assert(reopened.entries().size() == source.entries().size());

    cleanup(source_path);
    cleanup(destination);
    cleanup(staging);
    cleanup(shared_destination);
    cleanup(shared_staging);
    return 0;
}
