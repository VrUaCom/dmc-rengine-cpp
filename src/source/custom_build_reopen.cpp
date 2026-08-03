#include "dmc_rengine/source/custom_build_reopen.hpp"

#include <algorithm>
#include <string>
#include <utility>

namespace dmc::rengine::source {
namespace {

void add_diagnostic(
    CustomBuildReopenContext& context,
    std::string code,
    std::string message) {
    context.diagnostics.push_back(CustomBuildReopenDiagnostic{
        .code = std::move(code),
        .message = std::move(message),
    });
}

} // namespace

const SourceBinaryMapping* CustomBuildReopenContext::mapping_for_rva(
    std::uint64_t rva) const noexcept {
    const auto iterator = std::find_if(
        mappings.begin(), mappings.end(),
        [rva](const SourceBinaryMapping* mapping) {
            return mapping != nullptr && rva >= mapping->output_rva &&
                   rva - mapping->output_rva < mapping->byte_size;
        });
    return iterator == mappings.end() ? nullptr : *iterator;
}

CustomBuildReopenContext reopen_custom_build(
    const integration::ProjectWorkspace& workspace,
    const gdspaces::ResourceId& executable_resource) {
    CustomBuildReopenContext context{
        .executable_resource = executable_resource,
        .build = nullptr,
        .integration_project = nullptr,
        .mappings = {},
        .diagnostics = {},
    };

    const auto* session = workspace.find_session(executable_resource);
    if (session == nullptr || session->executable_context() == nullptr) {
        add_diagnostic(
            context,
            "custom-build-reopen.executable-context-missing",
            "The resource has no analyzed PE executable context.");
        return context;
    }

    const auto& executable = *session->executable_context();
    context.build = workspace.find_custom_build_by_sha256(executable.sha256);
    if (context.build == nullptr) {
        add_diagnostic(
            context,
            "custom-build-reopen.build-record-missing",
            "No Custom Build Record matches the executable SHA-256.");
        return context;
    }

    if (context.build->status == CustomBuildStatus::revoked ||
        context.build->attestation_state == AttestationState::revoked) {
        add_diagnostic(
            context,
            "custom-build-reopen.build-revoked",
            "The Custom Build Record is revoked and cannot be reopened as an active source lineage.");
        return context;
    }

    context.integration_project = workspace.find_source_integration_project(
        context.build->identity.integration_project_id);
    if (context.integration_project == nullptr) {
        add_diagnostic(
            context,
            "custom-build-reopen.integration-project-missing",
            "The Custom Build Record references an unavailable Integration Project.");
        return context;
    }

    const auto expected_artifact_id = "custom-executable:" +
        context.build->identity.custom_build_id;
    const auto* artifact = workspace.artifacts().find(expected_artifact_id);
    if (artifact == nullptr ||
        artifact->sha256 != context.build->identity.executable_sha256 ||
        artifact->size != context.build->executable_size ||
        artifact->role != "custom-recompiled-game-executable") {
        add_diagnostic(
            context,
            "custom-build-reopen.artifact-identity-mismatch",
            "The custom executable Artifact identity is missing or disagrees with the Build Record.");
    }

    if (workspace.graph().find(
            "custom-build:" + context.build->identity.custom_build_id) ==
        nullptr) {
        add_diagnostic(
            context,
            "custom-build-reopen.graph-provenance-missing",
            "The Custom Build Record is absent from Spider Hub provenance.");
    }

    if (session->source_payload().bytes.size() !=
        context.build->executable_size) {
        add_diagnostic(
            context,
            "custom-build-reopen.file-size-mismatch",
            "The executable byte size disagrees with the Build Record.");
    }

    const auto& image = executable.image;
    const auto& expected = context.build->pe_validation;
    if ((image.kind == exe::PeKind::pe32_plus) != expected.pe32_plus ||
        static_cast<std::uint16_t>(image.machine) != expected.machine ||
        image.image_base != expected.image_base ||
        image.entry_point_rva != expected.entry_point_rva ||
        image.subsystem != expected.subsystem ||
        image.section_count != expected.section_count) {
        add_diagnostic(
            context,
            "custom-build-reopen.pe-metadata-mismatch",
            "The analyzed PE metadata disagrees with the Custom Build Record.");
    }

    for (const auto& mapping : context.build->source_binary_mappings) {
        context.mappings.push_back(&mapping);
    }
    std::sort(
        context.mappings.begin(), context.mappings.end(),
        [](const SourceBinaryMapping* left,
           const SourceBinaryMapping* right) {
            return left->output_rva < right->output_rva ||
                (left->output_rva == right->output_rva &&
                 left->id < right->id);
        });
    if (context.mappings.empty()) {
        add_diagnostic(
            context,
            "custom-build-reopen.source-mappings-missing",
            "The Custom Build Record has no source-to-binary mappings.");
    }

    return context;
}

} // namespace dmc::rengine::source
