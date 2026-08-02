#pragma once

#include "dmc_rengine/gdspaces/diagnostic.hpp"
#include "dmc_rengine/gdspaces/resource_id.hpp"
#include "dmc_rengine/integration/project_workspace.hpp"

#include <string>
#include <vector>

namespace dmc::rengine::integration {

struct ResourceAnalysisReport final {
    gdspaces::ResourceId resource;
    std::string format;
    std::string parser_id;
    bool parser_available{false};
    bool recognized{false};
    bool binary_document_attached{false};
    std::vector<gdspaces::Diagnostic> diagnostics;

    [[nodiscard]] bool ok() const noexcept;
};

class ResourceAnalyzer final {
public:
    [[nodiscard]] static ResourceAnalysisReport analyze(
        ProjectWorkspace& project,
        const gdspaces::ResourceId& resource);
};

} // namespace dmc::rengine::integration
