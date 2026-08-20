#pragma once

#include "dmc_rengine/evidence/artifact.hpp"
#include "dmc_rengine/gdspaces/nbz_zip_serialization.hpp"

#include <cstdint>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace dmc::rengine::gdspaces {

// Product-only observation bounds for exact large-archive identity
// verification. The observation is streaming and never materializes the
// complete archive.
struct NbzZipArtifactBindingLimits final {
    std::uint64_t hash_chunk_bytes{1024ULL * 1024ULL};
};

// Serialization snapshot bound to one exact artifact identity by a single
// complete byte observation. Every preserved metadata byte and the SHA-256
// receipt are derived from that same streaming read. This is deliberately not
// a public aggregate: callers cannot self-declare artifact-bound authority.
class ArtifactBoundNbzZipSerializationSnapshot final {
public:
    [[nodiscard]] const evidence::ArtifactIdentity& artifact() const noexcept;
    [[nodiscard]] const NbzZipSerializationSnapshot& serialization() const noexcept;
    [[nodiscard]] std::string_view observed_sha256() const noexcept;
    [[nodiscard]] bool valid() const noexcept;

private:
    friend class NbzZipArtifactSerializationBinder;

    ArtifactBoundNbzZipSerializationSnapshot(
        evidence::ArtifactIdentity artifact,
        NbzZipSerializationSnapshot serialization,
        std::string observed_sha256);

    evidence::ArtifactIdentity artifact_;
    NbzZipSerializationSnapshot serialization_;
    std::string observed_sha256_;
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
