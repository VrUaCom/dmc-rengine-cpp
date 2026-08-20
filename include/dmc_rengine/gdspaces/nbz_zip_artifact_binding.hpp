#pragma once

#include "dmc_rengine/evidence/artifact.hpp"
#include "dmc_rengine/gdspaces/nbz_zip_serialization.hpp"

#include <cstdint>
#include <optional>
#include <string>
#include <vector>

namespace dmc::rengine::gdspaces {

// Product-only hashing bounds for exact large-archive identity verification.
// The hash pass is streaming and never materializes the complete archive.
struct NbzZipArtifactBindingLimits final {
    std::uint64_t hash_chunk_bytes{1024ULL * 1024ULL};
};

// Serialization snapshot bound to one exact artifact identity by two complete
// streaming hash passes around the metadata scan. This proves the preserved
// source spans came from the expected archive during the scan window. A future
// copier/repacker must still revalidate the artifact at its own I/O boundary.
struct ArtifactBoundNbzZipSerializationSnapshot final {
    evidence::ArtifactIdentity artifact;
    NbzZipSerializationSnapshot serialization;
    std::string pre_scan_sha256;
    std::string post_scan_sha256;

    [[nodiscard]] bool valid() const noexcept;
};

struct NbzZipArtifactBindingResult final {
    std::optional<ArtifactBoundNbzZipSerializationSnapshot> snapshot;
    std::vector<Diagnostic> diagnostics;

    [[nodiscard]] bool ok() const noexcept;
};

class NbzZipArtifactSerializationBinder final {
public:
    [[nodiscard]] static NbzZipArtifactBindingResult bind(
        const NbzZipSource& source,
        const evidence::ArtifactIdentity& expected_artifact,
        NbzZipSerializationLimits serialization_limits = {},
        NbzZipArtifactBindingLimits binding_limits = {});
};

} // namespace dmc::rengine::gdspaces
