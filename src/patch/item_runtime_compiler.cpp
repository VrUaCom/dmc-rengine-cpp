#include "dmc_rengine/patch/item_runtime_compiler.hpp"

#include "dmc_rengine/integration/executable_evidence.hpp"

#include <algorithm>
#include <cstddef>
#include <span>
#include <string>
#include <utility>

namespace dmc::rengine::patch {
namespace {

void add_diagnostic(
    ItemRuntimePatchCompilationResult& result,
    std::string code,
    std::string message) {
    result.diagnostics.push_back(PatchCompilationDiagnostic{
        .code = std::move(code),
        .message = std::move(message),
    });
}

[[nodiscard]] std::string bytes_hex(std::span<const std::byte> bytes) {
    constexpr char digits[] = "0123456789abcdef";
    std::string result;
    result.reserve(bytes.size() * 2U);
    for (const auto value : bytes) {
        const auto byte = std::to_integer<unsigned int>(value);
        result.push_back(digits[(byte >> 4U) & 0x0FU]);
        result.push_back(digits[byte & 0x0FU]);
    }
    return result;
}

[[nodiscard]] bool request_declares_evidence(
    const item::RuntimeChangeRequest& request,
    std::string_view evidence_record_id) noexcept {
    return std::find(
        request.evidence_record_ids.begin(),
        request.evidence_record_ids.end(),
        evidence_record_id) != request.evidence_record_ids.end();
}

} // namespace

ItemRuntimePatchCompilationResult ItemRuntimePatchCompiler::compile(
    const integration::ProjectWorkspace& project,
    const item::RuntimeChangeRequest& request,
    const validation::ItemRuntimeValidationPlan& validation_plan,
    ItemRuntimePatchInput input) {
    ItemRuntimePatchCompilationResult result;

    if (!input.valid()) {
        add_diagnostic(
            result,
            "patch-compiler.input-invalid",
            "Patch input requires one Evidence Record, equal non-zero byte ranges, and a description.");
        return result;
    }
    if (!request.valid() ||
        request.status != item::RuntimeRequestStatus::ready_for_patch_planning) {
        add_diagnostic(
            result,
            "patch-compiler.request-not-ready",
            "Only a valid ready-for-patch-planning runtime request can be compiled.");
        return result;
    }
    if (!validation_plan.valid() ||
        !validation_plan.ready_for_execution() ||
        validation_plan.runtime_request_id != request.id ||
        validation_plan.item_resource != request.item_resource ||
        validation_plan.producer != gdspaces::ToolTarget::build_test_lab) {
        add_diagnostic(
            result,
            "patch-compiler.validation-plan-not-ready",
            "The Trial Chamber plan is invalid, blocked, or belongs to another runtime request.");
        return result;
    }
    if (project.graph().find("runtime-request:" + request.id) == nullptr) {
        add_diagnostic(
            result,
            "patch-compiler.request-not-registered",
            "The runtime request is absent from Spider Hub provenance.");
        return result;
    }
    if (project.graph().find(validation_plan.id) == nullptr) {
        add_diagnostic(
            result,
            "patch-compiler.validation-plan-not-registered",
            "The Trial Chamber plan is absent from Spider Hub provenance.");
        return result;
    }
    if (!request_declares_evidence(request, input.evidence_record_id)) {
        add_diagnostic(
            result,
            "patch-compiler.evidence-not-declared",
            "The selected Evidence Record is not declared by the runtime request.");
        return result;
    }

    const auto* artifact = project.artifacts().find(request.target_artifact_id);
    if (artifact == nullptr) {
        add_diagnostic(
            result,
            "patch-compiler.artifact-missing",
            "The target executable artifact is absent from ArtifactRegistry.");
        return result;
    }
    const auto executable_sessions =
        project.executable_sessions_for_artifact(request.target_artifact_id);
    if (executable_sessions.size() != 1U) {
        add_diagnostic(
            result,
            "patch-compiler.executable-session-ambiguous",
            executable_sessions.empty()
                ? "No analyzed executable session matches the target artifact."
                : "More than one analyzed executable session matches the target artifact.");
        return result;
    }

    const auto* executable_session = executable_sessions.front();
    const auto* executable_context = executable_session->executable_context();
    if (executable_context == nullptr ||
        executable_context->sha256 != artifact->sha256) {
        add_diagnostic(
            result,
            "patch-compiler.executable-context-mismatch",
            "The analyzed executable context does not match the target artifact SHA-256.");
        return result;
    }

    const auto resolution = integration::resolve_executable_evidence(
        project,
        executable_session->resource().id,
        input.evidence_record_id);
    if (!resolution.ok()) {
        for (const auto& diagnostic : resolution.diagnostics) {
            add_diagnostic(result, diagnostic.code, diagnostic.message);
        }
        if (resolution.diagnostics.empty()) {
            add_diagnostic(
                result,
                "patch-compiler.evidence-unresolved",
                "The Evidence Record could not be resolved to executable file bytes.");
        }
        return result;
    }
    if (resolution.locations.size() != 1U) {
        add_diagnostic(
            result,
            "patch-compiler.evidence-location-ambiguous",
            "Patch compilation requires exactly one resolved EvidenceLocation.");
        return result;
    }

    const auto& location = resolution.locations.front();
    if (location.size != input.expected.size()) {
        add_diagnostic(
            result,
            "patch-compiler.range-size-mismatch",
            "Expected and replacement byte counts must match the resolved EvidenceLocation size.");
        return result;
    }
    if (input.expected == input.replacement) {
        add_diagnostic(
            result,
            "patch-compiler.no-op-replacement",
            "Replacement bytes must differ from expected source bytes.");
        return result;
    }

    const auto source = std::span<const std::byte>{
        executable_session->source_payload().bytes};
    if (location.file_offset > source.size() ||
        location.size > source.size() - location.file_offset) {
        add_diagnostic(
            result,
            "patch-compiler.range-out-of-source",
            "The resolved patch range extends beyond the immutable executable payload.");
        return result;
    }

    const auto offset = static_cast<std::size_t>(location.file_offset);
    const auto source_range = source.subspan(offset, input.expected.size());
    if (!std::equal(
            input.expected.begin(), input.expected.end(),
            source_range.begin(), source_range.end())) {
        add_diagnostic(
            result,
            "patch-compiler.expected-bytes-mismatch",
            "The immutable executable payload does not match the explicit expected-byte guard.");
        return result;
    }

    const auto plan_id = "patch-plan:" + request.id + ':' +
        input.evidence_record_id + ":r" +
        std::to_string(request.item_revision);
    GuardedPatchPlan guarded_plan(plan_id, artifact->sha256);
    if (!guarded_plan.add(BytePatch{
            .offset = location.file_offset,
            .expected = input.expected,
            .replacement = input.replacement,
            .description = input.description,
        })) {
        add_diagnostic(
            result,
            "patch-compiler.guarded-plan-rejected",
            "GuardedPatchPlan rejected the compiled byte patch.");
        return result;
    }

    CompiledPatchMetadata metadata{
        .id = plan_id,
        .runtime_request_id = request.id,
        .validation_plan_id = validation_plan.id,
        .item_resource = request.item_resource,
        .executable_resource = executable_session->resource().id,
        .target_artifact_id = request.target_artifact_id,
        .evidence_record_id = input.evidence_record_id,
        .file_offset = location.file_offset,
        .rva = location.rva,
        .va = location.va,
        .size = location.size,
        .source_sha256 = artifact->sha256,
        .expected_hex = bytes_hex(input.expected),
        .replacement_hex = bytes_hex(input.replacement),
        .description = input.description,
    };
    if (!metadata.valid()) {
        add_diagnostic(
            result,
            "patch-compiler.metadata-invalid",
            "Compiled patch metadata failed internal validity checks.");
        return result;
    }

    result.compiled = CompiledItemRuntimePatchPlan{
        .guarded_plan = std::move(guarded_plan),
        .metadata = std::move(metadata),
    };
    return result;
}

} // namespace dmc::rengine::patch
