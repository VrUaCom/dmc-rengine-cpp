#pragma once

#include "dmc_rengine/integration/project_graph.hpp"

#include <string>

namespace dmc::rengine::integration {

[[nodiscard]] std::string project_graph_manifest_json(
    const ProjectGraph& graph);

} // namespace dmc::rengine::integration
