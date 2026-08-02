#pragma once

#include "dmc_rengine/exe/evidence_address_resolver.hpp"
#include "dmc_rengine/integration/project_workspace.hpp"

#include <string>
#include <vector>

namespace dmc::rengine::integration {

struct ExecutableEvidenceResolutionReport final {
    gdspaces::ResourceId executable_resource;
    std::string evidence_record_id;
    std::vector<exe::ResolvedEvidenceLocation> locations;
    std::vector<exe::EvidenceAddressDiagnostic> diagnostics;

    [[nodiscard]] bool ok() const noexcept {
        return !locations.empty() && diagnostics.empty();
    }
};

[[nodiscard]] ExecutableEvidenceResolutionReport resolve_executable_evidence(
    const ProjectWorkspace& project,
    const gdspaces::ResourceId& executable_resource,
    std::string_view evidence_record_id);

} // namespace dmc::rengine::integration
