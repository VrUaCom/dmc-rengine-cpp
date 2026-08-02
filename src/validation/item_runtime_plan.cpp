#include "dmc_rengine/validation/item_runtime_plan.hpp"

#include "dmc_rengine/integration/executable_evidence.hpp"

#include <algorithm>
#include <string>
#include <utility>

namespace dmc::rengine::validation {
namespace {

void add_requirement(
    ItemRuntimeValidationPlan& plan,
    RequirementKind kind,
    RequirementStatus status,
    std::string detail,
    std::vector<std::string> evidence_ids = {}) {
    plan.requirements.push_back(ValidationRequirement{
        .id = std::string(to_string(kind)),
        .kind = kind,
        .status = status,
        .mandatory = true,
        .detail = std::move(detail),
        .evidence_record_ids = std::move(evidence_ids),
    });
    if (status == RequirementStatus::blocked) {
        plan.blockers.push_back(std::string(to_string(kind)));
    }
}

[[nodiscard]] bool has_guard(
    const item::RuntimeChangeRequest& request,
    std::string_view guard) noexcept {
    return std::find(
        request.required_guards.begin(),
        request.required_guards.end(),
        guard) != request.required_guards.end();
}

} // namespace

bool ItemRuntimeValidationPlan::ready_for_execution() const noexcept {
    if (!valid() || !blockers.empty()) {
        return false;
    }
    return std::none_of(
        requirements.begin(), requirements.end(),
        [](const ValidationRequirement& requirement) {
            return requirement.mandatory &&
                   requirement.status == RequirementStatus::blocked;
        });
}

ItemRuntimeValidationPlan build_item_runtime_plan(
    const integration::ProjectWorkspace& project,
    const item::RuntimeChangeRequest& request) {
    ItemRuntimeValidationPlan plan{
        .id = "validation-plan:" + request.id,
        .runtime_request_id = request.id,
        .item_resource = request.item_resource,
        .producer = gdspaces::ToolTarget::build_test_lab,
        .requirements = {},
        .blockers = {},
    };

    if (!request.valid() ||
        request.status == item::RuntimeRequestStatus::rejected) {
        add_requirement(
            plan,
            RequirementKind::request_registered,
            RequirementStatus::blocked,
            "The runtime request is invalid or rejected and cannot enter validation.");
        return plan;
    }
    if (request.status == item::RuntimeRequestStatus::research_required) {
        plan.blockers.push_back("runtime-request-research-required");
    }

    const auto request_node = "runtime-request:" + request.id;
    add_requirement(
        plan,
        RequirementKind::request_registered,
        project.graph().find(request_node) != nullptr
            ? RequirementStatus::ready
            : RequirementStatus::blocked,
        project.graph().find(request_node) != nullptr
            ? "The runtime request is registered in Spider Hub with provenance edges."
            : "The runtime request has not been registered in ProjectWorkspace.");

    const auto* item_session = project.find_session(request.item_resource);
    const auto revision_ready = item_session != nullptr &&
        item_session->working_copy() != nullptr &&
        item_session->working_copy()->revision() == request.item_revision;
    add_requirement(
        plan,
        RequirementKind::item_revision,
        revision_ready ? RequirementStatus::ready : RequirementStatus::blocked,
        revision_ready
            ? "The request matches the active ITM WorkingCopy revision."
            : "The ITM WorkingCopy is missing or has moved to a different revision.");

    const auto* artifact = project.artifacts().find(request.target_artifact_id);
    add_requirement(
        plan,
        RequirementKind::artifact_identity,
        artifact != nullptr ? RequirementStatus::ready : RequirementStatus::blocked,
        artifact != nullptr
            ? "The target executable artifact ID and SHA-256 are loaded."
            : "The target executable artifact is absent from ArtifactRegistry.");

    const auto executable_sessions =
        project.executable_sessions_for_artifact(request.target_artifact_id);
    const auto executable_ready = executable_sessions.size() == 1U;
    add_requirement(
        plan,
        RequirementKind::executable_session,
        executable_ready ? RequirementStatus::ready : RequirementStatus::blocked,
        executable_ready
            ? "Exactly one analyzed executable session matches the target artifact SHA-256."
            : executable_sessions.empty()
                ? "No analyzed executable session matches the target artifact SHA-256."
                : "More than one executable session matches the artifact; the target is ambiguous.");

    bool records_ready = !request.evidence_record_ids.empty();
    for (const auto& record_id : request.evidence_record_ids) {
        if (project.evidence().find(record_id) == nullptr) {
            records_ready = false;
        }
    }
    add_requirement(
        plan,
        RequirementKind::evidence_records,
        records_ready ? RequirementStatus::ready : RequirementStatus::blocked,
        records_ready
            ? "Every request Evidence Record is loaded."
            : "One or more request Evidence Records are missing.",
        request.evidence_record_ids);

    bool addresses_ready = executable_ready && records_ready;
    if (addresses_ready) {
        for (const auto& record_id : request.evidence_record_ids) {
            const auto resolution = integration::resolve_executable_evidence(
                project,
                executable_sessions.front()->resource().id,
                record_id);
            if (!resolution.ok()) {
                addresses_ready = false;
                break;
            }
        }
    }
    add_requirement(
        plan,
        RequirementKind::evidence_address_resolution,
        addresses_ready ? RequirementStatus::ready : RequirementStatus::blocked,
        addresses_ready
            ? "All request Evidence Records resolve to bounded file-offset/RVA/VA ranges in the target executable."
            : "At least one request Evidence Record cannot be resolved against the target executable.",
        request.evidence_record_ids);

    const auto source_guards =
        has_guard(request, "target-artifact-sha256") &&
        has_guard(request, "expected-source-bytes");
    add_requirement(
        plan,
        RequirementKind::source_byte_guards,
        source_guards ? RequirementStatus::ready : RequirementStatus::blocked,
        source_guards
            ? "Artifact hash and expected source-byte guards are required."
            : "Artifact hash or expected source-byte guard is missing.");

    add_requirement(
        plan,
        RequirementKind::range_bounds,
        has_guard(request, "patch-range-bounds")
            ? RequirementStatus::ready
            : RequirementStatus::blocked,
        "Patch ranges must remain inside the resolved executable file.");
    add_requirement(
        plan,
        RequirementKind::overlap_conflicts,
        has_guard(request, "patch-overlap-conflict-check")
            ? RequirementStatus::ready
            : RequirementStatus::blocked,
        "Patch sites must pass overlap and conflict analysis.");
    add_requirement(
        plan,
        RequirementKind::rollback,
        has_guard(request, "rollback-plan")
            ? RequirementStatus::ready
            : RequirementStatus::blocked,
        "A rollback plan is mandatory before patch execution.");

    add_requirement(
        plan,
        RequirementKind::unit_tests,
        RequirementStatus::pending_execution,
        "Run unit tests for request serialization, address resolution, and guard validation.");
    add_requirement(
        plan,
        RequirementKind::integration_tests,
        RequirementStatus::pending_execution,
        "Run Item Editor, EXE Editor, Evidence, Patch Engine, and manifest integration tests.");
    add_requirement(
        plan,
        RequirementKind::runtime_smoke_test,
        RequirementStatus::pending_execution,
        "Run a controlled game/runtime smoke test against a copied executable artifact.");
    add_requirement(
        plan,
        RequirementKind::manifest_export,
        RequirementStatus::pending_execution,
        "Export a deterministic validation and patch-plan manifest.");
    return plan;
}

} // namespace dmc::rengine::validation
