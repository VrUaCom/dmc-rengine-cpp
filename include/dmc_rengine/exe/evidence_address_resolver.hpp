#pragma once

#include "dmc_rengine/evidence/artifact.hpp"
#include "dmc_rengine/evidence/location.hpp"
#include "dmc_rengine/integration/executable_context.hpp"

#include <cstdint>
#include <optional>
#include <string>
#include <vector>

namespace dmc::rengine::exe {

struct EvidenceAddressDiagnostic final {
    std::string code;
    std::string message;

    [[nodiscard]] bool valid() const noexcept {
        return !code.empty() && !message.empty();
    }
};

struct ResolvedEvidenceLocation final {
    std::string artifact_id;
    std::uint64_t file_offset{};
    std::uint64_t rva{};
    std::uint64_t va{};
    std::uint64_t size{};
    std::string symbol;
    std::string note;

    [[nodiscard]] bool valid(std::uint64_t file_size) const noexcept {
        return !artifact_id.empty() && size != 0U &&
               file_offset <= file_size && size <= file_size - file_offset;
    }
};

struct EvidenceAddressResolution final {
    std::optional<ResolvedEvidenceLocation> location;
    std::vector<EvidenceAddressDiagnostic> diagnostics;

    [[nodiscard]] bool ok() const noexcept {
        return location.has_value() && diagnostics.empty();
    }
};

[[nodiscard]] EvidenceAddressResolution resolve_evidence_location(
    const evidence::EvidenceLocation& location,
    const evidence::ArtifactIdentity& artifact,
    const integration::ExecutableResourceContext& executable,
    std::uint64_t file_size);

} // namespace dmc::rengine::exe
