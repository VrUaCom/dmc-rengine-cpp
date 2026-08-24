#pragma once

#include "dmc_rengine/evidence/artifact.hpp"
#include "dmc_rengine/gdspaces/nbz_zip_artifact_binding.hpp"
#include "dmc_rengine/gdspaces/nbz_zip_source.hpp"

#include <cstddef>
#include <cstdint>
#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <vector>

namespace dmc::rengine::gdspaces {

struct NbzZipArtifactMemberLimits final {
    std::uint64_t hash_chunk_bytes{1024ULL * 1024ULL};
    std::uint64_t max_stored_member_bytes{512ULL * 1024ULL * 1024ULL};
    std::uint64_t max_materialized_member_bytes{512ULL * 1024ULL * 1024ULL};
};

// One selected member materialized from a complete archive observation whose
// SHA-256 matches an already artifact-bound serialization snapshot. The member
// bytes are therefore bound to the same exact archive identity as the index
// metadata even though filesystem access uses a separate read pass.
class ArtifactBoundNbzZipMemberObservation final {
public:
    [[nodiscard]] const evidence::ArtifactIdentity& artifact() const noexcept;
    [[nodiscard]] const NbzZipEntry& entry() const noexcept;
    [[nodiscard]] std::span<const std::byte> materialized_bytes() const noexcept;
    [[nodiscard]] const ByteProvenance& byte_provenance() const noexcept;
    [[nodiscard]] std::string_view observed_sha256() const noexcept;
    [[nodiscard]] bool valid() const noexcept;

private:
    friend class NbzZipArtifactMemberObserver;

    ArtifactBoundNbzZipMemberObservation(
        evidence::ArtifactIdentity artifact,
        NbzZipEntry entry,
        std::vector<std::byte> materialized_bytes,
        ByteProvenance byte_provenance,
        std::string observed_sha256);

    evidence::ArtifactIdentity artifact_;
    NbzZipEntry entry_;
    std::vector<std::byte> materialized_bytes_;
    ByteProvenance byte_provenance_;
    std::string observed_sha256_;
};

struct NbzZipArtifactMemberObservationResult final {
    std::optional<ArtifactBoundNbzZipMemberObservation> observation;
    std::vector<Diagnostic> diagnostics;

    [[nodiscard]] bool ok() const noexcept;
};

class NbzZipArtifactMemberObserver final {
public:
    [[nodiscard]] static NbzZipArtifactMemberObservationResult observe(
        const NbzZipSource& source,
        const ArtifactBoundNbzZipSerializationSnapshot& bound_snapshot,
        std::uint32_t central_index,
        NbzZipArtifactMemberLimits limits = {});
};

} // namespace dmc::rengine::gdspaces
