#pragma once

#include "dmc_rengine/gdspaces/open_router.hpp"
#include "dmc_rengine/gdspaces/resource_id.hpp"
#include "dmc_rengine/integration/project_workspace.hpp"
#include "dmc_rengine/item/runtime_request.hpp"

#include <string>
#include <string_view>
#include <vector>

namespace dmc::rengine::validation {

enum class RequirementKind {
    request_registered,
    item_revision,
    artifact_identity,
    executable_session,
    evidence_records,
    evidence_address_resolution,
    source_byte_guards,
    range_bounds,
    overlap_conflicts,
    rollback,
    unit_tests,
    integration_tests,
    runtime_smoke_test,
    manifest_export,
};

[[nodiscard]] constexpr std::string_view to_string(
    RequirementKind kind) noexcept {
    switch (kind) {
    case RequirementKind::request_registered: return "request-registered";
    case RequirementKind::item_revision: return "item-revision";
    case RequirementKind::artifact_identity: return "artifact-identity";
    case RequirementKind::executable_session: return "executable-session";
    case RequirementKind::evidence_records: return "evidence-records";
    case RequirementKind::evidence_address_resolution: return "evidence-address-resolution";
    case RequirementKind::source_byte_guards: return "source-byte-guards";
    case RequirementKind::range_bounds: return "range-bounds";
    case RequirementKind::overlap_conflicts: return "overlap-conflicts";
    case RequirementKind::rollback: return "rollback";
    case RequirementKind::unit_tests: return "unit-tests";
    case RequirementKind::integration_tests: return "integration-tests";
    case RequirementKind::runtime_smoke_test: return "runtime-smoke-test";
    case RequirementKind::manifest_export: return "manifest-export";
    }
    return "request-registered";
}

enum class RequirementStatus {
    ready,
    blocked,
    pending_execution,
};

[[nodiscard]] constexpr std::string_view to_string(
    RequirementStatus status) noexcept {
    switch (status) {
    case RequirementStatus::ready: return "ready";
    case RequirementStatus::blocked: return "blocked";
    case RequirementStatus::pending_execution: return "pending-execution";
    }
    return "blocked";
}

struct ValidationRequirement final {
    std::string id;
    RequirementKind kind{RequirementKind::request_registered};
    RequirementStatus status{RequirementStatus::blocked};
    bool mandatory{true};
    std::string detail;
    std::vector<std::string> evidence_record_ids;

    [[nodiscard]] bool valid() const noexcept {
        return !id.empty() && !detail.empty();
    }
};

struct ItemRuntimeValidationPlan final {
    std::string id;
    std::string runtime_request_id;
    gdspaces::ResourceId item_resource;
    gdspaces::ToolTarget producer{gdspaces::ToolTarget::build_test_lab};
    std::vector<ValidationRequirement> requirements;
    std::vector<std::string> blockers;

    [[nodiscard]] bool valid() const noexcept {
        return !id.empty() && !runtime_request_id.empty() &&
               item_resource.valid() && !requirements.empty();
    }
    [[nodiscard]] bool ready_for_execution() const noexcept;
};

[[nodiscard]] ItemRuntimeValidationPlan build_item_runtime_plan(
    const integration::ProjectWorkspace& project,
    const item::RuntimeChangeRequest& request);

} // namespace dmc::rengine::validation
