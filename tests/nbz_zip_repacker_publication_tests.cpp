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
    namespace gdspaces = dmc::rengine::gdspaces;
    namespace dmc3 = dmc::rengine::profiles::dmc3;

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

    const auto root = std::filesystem::temp_directory_path();
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

    // A destination that already exists is never truncated or replaced.
    const auto sentinel = ascii("DO-NOT-CLOBBER");
    write_file(destination, sentinel);
    const auto existing_result = gdspaces::NbzZipRetailRepacker::write(
        source, *bound.snapshot, {}, destination);
    assert(!existing_result.ok());
    assert(existing_result.status ==
        gdspaces::NbzZipRetailRepackStatus::invalid_destination);
    assert(read_file(destination) == sentinel);
    cleanup(destination);

    // Staging ownership is acquired atomically through directory creation. A
    // pre-existing reservation belongs to someone else and is not removed.
    std::error_code error;
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

    // Successful publication uses a no-replace hard link from the validated
    // same-filesystem staging file, then the staging directory is cleaned.
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
    return 0;
}
