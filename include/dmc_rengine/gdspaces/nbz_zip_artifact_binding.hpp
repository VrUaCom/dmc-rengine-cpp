#pragma once

#include "dmc_rengine/evidence/artifact.hpp"
#include "dmc_rengine/gdspaces/nbz_zip_serialization.hpp"

#include <cstdint>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace dmc::rengine::gdspaces {

// Product-only hashing bounds for exact large-archive identity verification.
// The hash pass is streaming and never materializes the complete archive.
struct NbzZipArtifactBindingLimits final {
    std::uint64_t hash_chunk_bytes{1024ULL * 1024ULL};
};

// Serialization snapshot bound to one exact artifact identity by two complete
// streaming hash passes around the metadata scan. This is deliberately not a
// public aggregate: callers cannot self-declare artifact-bound authority by
// copying SHA text into fields. Only NbzZipArtifactSerializationBinder may
// construct an instance.
class ArtifactBoundNbzZipSerializationSnapshot final {
public:
    [[nodiscard]] const evidence::ArtifactIdentity& artifact() const noexcept;
    [[nodiscard]] const NbzZipSerializationSnapshot& serialization() const noexcept;
    [[nodiscard]] std::string_view pre_scan_sha256() const noexcept;
    [[nodiscard]] std::string_view post_scan_sha256() const noexcept;
    [[nodiscard]] bool valid() const noexcept;

private:
    friend class NbzZipArtifactSerializationBinder;

    ArtifactBoundNbzZipSerializationSnapshot(
        evidence::ArtifactIdentity artifact,
        NbzZipSerializationSnapshot serialization,
        std::string pre_scan_sha256,
        std::string post_scan_sha256);

    evidence::ArtifactIdentity artifact_;
    NbzZipSerializationSnapshot serialization_;
    std::string pre_scan_sha256_;
    std::string post_scan_sha256_;
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
