#pragma once

#include "dmc_rengine/stageops/domain_workspace.hpp"

#include <vector>

namespace dmc::rengine::stageops {

// Editor-facing query that orders byte-addressable domain objects by exact
// parser-provenance source offset rather than lexicographic object ID. Objects
// without a bounded source span fall back to deterministic ID order.
[[nodiscard]] std::vector<const StageDomainObject*> domain_objects_by_kind_source_order(
    const StageDomainWorkspace& workspace,
    StageDomainKind kind);

} // namespace dmc::rengine::stageops
