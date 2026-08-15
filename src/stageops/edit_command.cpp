#include "dmc_rengine/stageops/edit_command.hpp"

#include <utility>

namespace dmc::rengine::stageops {

StageEditCommandResult StageEditCommandExecutor::execute(
    StageOperationsSession& session,
    StageEditCommand command) {
    if (!session.valid()) {
        return StageEditCommandResult{
            .applied = false,
            .stage_revision = session.stage_revision(),
            .resource_revision = 0U,
            .derived_state_stale = session.derived_state_stale(),
            .error = "Stage Ops operations session is invalid.",
        };
    }
    if (!command.valid()) {
        return StageEditCommandResult{
            .applied = false,
            .stage_revision = session.stage_revision(),
            .resource_revision = 0U,
            .derived_state_stale = session.derived_state_stale(),
            .error = "Stage edit command is invalid.",
        };
    }
    if (command.expected_stage_revision != session.stage_revision()) {
        return StageEditCommandResult{
            .applied = false,
            .stage_revision = session.stage_revision(),
            .resource_revision = 0U,
            .derived_state_stale = session.derived_state_stale(),
            .error = "Stage edit command was produced from a stale scene revision.",
        };
    }

    // WorkingCopy creation is idempotent and remains Stage Ops-coordinated. A
    // consumer never needs direct ProjectWorkspace mutation access.
    if (!session.enable_editing(command.resource_id)) {
        return StageEditCommandResult{
            .applied = false,
            .stage_revision = session.stage_revision(),
            .resource_revision = 0U,
            .derived_state_stale = session.derived_state_stale(),
            .error = "Stage resource cannot enter editable WorkingCopy state.",
        };
    }

    auto result = session.apply_edit(
        command.resource_id,
        std::move(command.operation));
    return StageEditCommandResult{
        .applied = result.applied,
        .stage_revision = result.stage_revision,
        .resource_revision = result.resource_revision,
        .derived_state_stale = session.derived_state_stale(),
        .error = std::move(result.error),
    };
}

} // namespace dmc::rengine::stageops
