#include "dmc_rengine/item/runtime_request.hpp"

#include <algorithm>
#include <string>
#include <utility>

namespace dmc::rengine::item {
namespace {

[[nodiscard]] bool patch_grade_confidence(
    evidence::Confidence confidence) noexcept {
    return confidence == evidence::Confidence::confirmed ||
           confidence == evidence::Confidence::high;
}

void add_diagnostic(
    RuntimeChangeRequest& request,
    std::string code,
    std::string message) {
    request.diagnostics.push_back(RuntimeRequestDiagnostic{
        .code = std::move(code),
        .message = std::move(message),
    });
}

[[nodiscard]] RuntimeChangeRequest base_request(
    const ItemWorkspaceView& workspace,
    RuntimeChangeKind kind,
    std::uint32_t requested_value,
    std::string target_artifact_id) {
    RuntimeChangeRequest request{
        .id = std::string(to_string(kind)) + '-' +
            std::to_string(requested_value) + "-r" +
            std::to_string(workspace.working_copy_revision),
        .kind = kind,
        .status = RuntimeRequestStatus::rejected,
        .item_resource = workspace.resource.id,
        .item_revision = workspace.working_copy_revision,
        .producer = gdspaces::ToolTarget::item_editor,
        .consumer = gdspaces::ToolTarget::exe_editor,
        .validators = {
            gdspaces::ToolTarget::evidence_registry,
            gdspaces::ToolTarget::build_test_lab,
        },
        .target_artifact_id = std::move(target_artifact_id),
        .requested_value = requested_value,
        .evidence_record_ids = {},
        .required_guards = {
            "target-artifact-sha256",
            "expected-source-bytes",
            "patch-range-bounds",
            "patch-overlap-conflict-check",
            "atomic-application",
            "rollback-plan",
            "build-and-test-validation",
        },
        .diagnostics = {},
    };

    if (!workspace.valid()) {
        add_diagnostic(
            request,
            "item-request.workspace-invalid",
            "The Item Workspace View is invalid or does not reference an ITM resource.");
    }
    if (request.target_artifact_id.empty()) {
        add_diagnostic(
            request,
            "item-request.target-artifact-missing",
            "A canonical executable artifact identity is required.");
    }
    if (!workspace.has_route("item-editor")) {
        add_diagnostic(
            request,
            "item-request.item-editor-route-missing",
            "The ITM workspace is not routed to Item Editor.");
    }
    if (!workspace.has_route("evidence-registry")) {
        add_diagnostic(
            request,
            "item-request.evidence-route-missing",
            "Runtime-linked item changes require Evidence Registry context.");
    }
    if (!workspace.has_route("build-test-lab")) {
        add_diagnostic(
            request,
            "item-request.validation-route-missing",
            "Runtime-linked item changes require Build & Test Lab validation.");
    }
    return request;
}

[[nodiscard]] bool has_structural_errors(
    const RuntimeChangeRequest& request) noexcept {
    return std::any_of(
        request.diagnostics.begin(), request.diagnostics.end(),
        [](const RuntimeRequestDiagnostic& diagnostic) {
            return diagnostic.code == "item-request.workspace-invalid" ||
                   diagnostic.code == "item-request.target-artifact-missing" ||
                   diagnostic.code == "item-request.item-editor-route-missing" ||
                   diagnostic.code == "item-request.evidence-route-missing" ||
                   diagnostic.code == "item-request.validation-route-missing";
        });
}

} // namespace

RuntimeChangeRequest make_slot_registration_request(
    const ItemWorkspaceView& workspace,
    std::uint32_t slot,
    std::string target_artifact_id) {
    auto request = base_request(
        workspace,
        RuntimeChangeKind::register_item_slot,
        slot,
        std::move(target_artifact_id));
    request.required_guards.push_back("slot-range-50-63");
    request.required_guards.push_back("duplicate-item-id-check");
    request.required_guards.push_back("registry-capacity-check");

    if (slot < 50U || slot > 63U) {
        add_diagnostic(
            request,
            "item-request.slot-outside-confirmed-extension-range",
            "The migrated guarded registration workflow currently covers slots 50 through 63 only.");
        return request;
    }

    const auto* evidence_view = workspace.evidence(
        RuntimeEvidenceKind::registry_slot_guard);
    if (evidence_view == nullptr) {
        add_diagnostic(
            request,
            "item-request.slot-guard-evidence-missing",
            "The item registry slot guard Evidence Record is not loaded.");
        request.status = has_structural_errors(request)
            ? RuntimeRequestStatus::rejected
            : RuntimeRequestStatus::research_required;
        return request;
    }

    request.evidence_record_ids.push_back(evidence_view->record_id);
    if (!patch_grade_confidence(evidence_view->confidence)) {
        add_diagnostic(
            request,
            "item-request.slot-guard-confidence-insufficient",
            "The slot guard evidence confidence is below the patch-planning threshold.");
    }
    if (!evidence_view->location_complete) {
        add_diagnostic(
            request,
            "item-request.slot-guard-location-incomplete",
            "The slot guard must have artifact identity, address, and size before patch planning.");
    }

    if (slot == 50U) {
        request.required_guards.push_back(
            "slot-50-source-bytes-00-01-08-13");
    } else {
        add_diagnostic(
            request,
            "item-request.slot-location-not-confirmed",
            "Only slot 50 currently has a confirmed address and expected source-byte guard. Slots 51 through 63 require dedicated evidence before patch planning.");
    }

    if (has_structural_errors(request)) {
        request.status = RuntimeRequestStatus::rejected;
    } else if (!request.diagnostics.empty()) {
        request.status = RuntimeRequestStatus::research_required;
    } else {
        request.status = RuntimeRequestStatus::ready_for_patch_planning;
    }
    return request;
}

RuntimeChangeRequest make_inventory_limit_request(
    const ItemWorkspaceView& workspace,
    std::uint32_t limit,
    std::string target_artifact_id) {
    auto request = base_request(
        workspace,
        RuntimeChangeKind::set_inventory_limit,
        limit,
        std::move(target_artifact_id));
    request.required_guards.push_back("six-site-consistency-check");
    request.required_guards.push_back("83-f8-immediate-pattern-check");

    if (limit == 0U || limit > 255U) {
        add_diagnostic(
            request,
            "item-request.inventory-limit-out-of-range",
            "The current guarded model supports limits from 1 through 255 and blocks 256 or greater.");
        return request;
    }
    if (limit >= 128U) {
        request.required_guards.push_back("imm8-128-255-encoding-review");
    } else {
        request.required_guards.push_back("imm8-1-127-direct-range");
    }

    const auto* evidence_view = workspace.evidence(
        RuntimeEvidenceKind::inventory_limit_patch_family);
    if (evidence_view == nullptr) {
        add_diagnostic(
            request,
            "item-request.inventory-evidence-missing",
            "The inventory limit patch-family Evidence Record is not loaded.");
        request.status = has_structural_errors(request)
            ? RuntimeRequestStatus::rejected
            : RuntimeRequestStatus::research_required;
        return request;
    }

    request.evidence_record_ids.push_back(evidence_view->record_id);
    if (!patch_grade_confidence(evidence_view->confidence)) {
        add_diagnostic(
            request,
            "item-request.inventory-confidence-insufficient",
            "The inventory limit evidence confidence is below the patch-planning threshold.");
    }
    if (!evidence_view->location_complete) {
        add_diagnostic(
            request,
            "item-request.inventory-locations-incomplete",
            "All six executable locations must be migrated before a patch plan can be generated.");
    }

    if (has_structural_errors(request)) {
        request.status = RuntimeRequestStatus::rejected;
    } else if (!request.diagnostics.empty()) {
        request.status = RuntimeRequestStatus::research_required;
    } else {
        request.status = RuntimeRequestStatus::ready_for_patch_planning;
    }
    return request;
}

} // namespace dmc::rengine::item
