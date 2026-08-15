#pragma once

#include "dmc_rengine/gdspaces/working_copy.hpp"
#include "dmc_rengine/stageops/operations_session.hpp"

#include <cstdint>
#include <string>

namespace dmc::rengine::stageops {

// Consumer-facing mutation request. ModViz and future Stage Ops panels submit
// these commands instead of mutating ProjectWorkspace WorkingCopies directly.
struct StageEditCommand final {
    std::string id;
    std::uint64_t expected_stage_revision{};
    std::string resource_id;
    gdspaces::EditOperation operation;
    std::string producer;

    [[nodiscard]] bool valid() const noexcept {
        return !id.empty() &&
            !resource_id.empty() &&
            !operation.id.empty() &&
            !producer.empty();
    }
};

struct StageEditCommandResult final {
    bool applied{false};
    std::uint64_t stage_revision{};
    std::uint64_t resource_revision{};
    bool derived_state_stale{false};
    std::string error;
};

class StageEditCommandExecutor final {
public:
    [[nodiscard]] static StageEditCommandResult execute(
        StageOperationsSession& session,
        StageEditCommand command);
};

} // namespace dmc::rengine::stageops
