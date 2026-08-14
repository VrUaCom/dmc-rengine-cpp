#pragma once

#include "dmc_rengine/formats/hits.hpp"
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
    ParsedByteSource byte_source{ParsedByteSource::immutable_source};
    std::uint64_t byte_revision{};
    std::vector<gdspaces::Diagnostic> diagnostics;

    [[nodiscard]] bool ok() const noexcept;
};

class ResourceAnalyzer final {
public:
    // Canonical analysis of immutable source bytes (revision 0).
    [[nodiscard]] static ResourceAnalysisReport analyze(
        ProjectWorkspace& project,
        const gdspaces::ResourceId& resource);

    // Canonical analysis of the currently enabled WorkingCopy. This uses the
    // same format adapters as source analysis, retains the typed result as
    // `working-copy@revision`, and is the only supported Stage Ops reparse path
    // after edits. No Stage Ops-specific parser implementation is introduced.
    [[nodiscard]] static ResourceAnalysisReport analyze_working_copy(
        ProjectWorkspace& project,
        const gdspaces::ResourceId& resource);
};

} // namespace dmc::rengine::integration
