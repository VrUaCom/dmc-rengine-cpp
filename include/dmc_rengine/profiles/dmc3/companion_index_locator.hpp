#pragma once

#include "dmc_rengine/gdspaces/source.hpp"

#include <optional>
#include <string>
#include <vector>

namespace dmc::rengine::profiles::dmc3 {

enum class CompanionIndexCandidateKind : unsigned char {
    sibling_manifest,
    expanded_directory_manifest,
};

struct CompanionIndexCandidate final {
    CompanionIndexCandidateKind kind{CompanionIndexCandidateKind::sibling_manifest};
    std::string logical_path;
};

struct CompanionIndexDiscoveryResult final {
    std::optional<gdspaces::ResourcePayload> payload;
    std::optional<CompanionIndexCandidateKind> matched_kind;
    std::vector<CompanionIndexCandidate> candidates;
    std::vector<gdspaces::Diagnostic> diagnostics;

    [[nodiscard]] bool ok() const noexcept;
};

// DMC3 extraction-side companion discovery. Candidates are derived only from
// the physical/logical container path. Display names, embedded aliases and
// semantic suffixes are deliberately excluded from this lookup domain.
class CompanionIndexLocator final {
public:
    [[nodiscard]] static std::vector<CompanionIndexCandidate> candidates_for(
        const gdspaces::ResourceId& container);

    [[nodiscard]] static CompanionIndexDiscoveryResult discover(
        const gdspaces::ISource& source,
        const gdspaces::ResourceId& container);
};

} // namespace dmc::rengine::profiles::dmc3
