#pragma once

#include "dmc_rengine/exe/code_graph.hpp"
#include "dmc_rengine/exe/executable_report.hpp"
#include "dmc_rengine/exe/function_map.hpp"

#include <cstddef>
#include <string>

namespace dmc::rengine::exe {

struct FunctionMapReportOptions final {
    /// Emit every inventoried function rather than only those with an
    /// attribution. The full table is large and regenerable, so a committed
    /// record normally carries the attributed subset plus the aggregates.
    bool include_unattributed{false};
    /// Cap on emitted function entries; zero means no cap.
    std::size_t function_limit{0U};
    bool include_referenced_strings{true};
};

/// Deterministic JSON report for the function-level map.
[[nodiscard]] std::string to_json(const ExecutableArtifactIdentity& artifact,
                                  const CodeGraph& graph, const FunctionMap& map,
                                  const FunctionMapReportOptions& options = {});

} // namespace dmc::rengine::exe
