#pragma once

#include "dmc_rengine/binary/document.hpp"
#include "dmc_rengine/evidence/artifact_registry.hpp"
#include "dmc_rengine/evidence/packet.hpp"
#include "dmc_rengine/evidence/packet_registry.hpp"
#include "dmc_rengine/evidence/record.hpp"
#include "dmc_rengine/evidence/registry.hpp"
#include "dmc_rengine/formats/diagnostic.hpp"
#include "dmc_rengine/gdspaces/resource_graph.hpp"
#include "dmc_rengine/integration/format_registry.hpp"
#include "dmc_rengine/integration/project_graph.hpp"
#include "dmc_rengine/integration/resource_workspace.hpp"
#include "dmc_rengine/integration/tool_registry.hpp"
#include "dmc_rengine/source/custom_build.hpp"
#include "dmc_rengine/source/integration_project.hpp"
#include "dmc_rengine/source/modification.hpp"

#include <cstddef>
#include <cstdint>
#include <map>
#include <span>
#include <string>
#include <string_view>
#include <vector>

namespace dmc::rengine::item {
struct RuntimeChangeRequest;
}

namespace dmc::rengine::validation {
struct ItemRuntimeValidationPlan;
}

namespace dmc::rengine::patch {
struct CompiledItemRuntimePatchPlan;
struct PatchCopyArtifact;
}

namespace dmc::rengine::integration {

class ProjectWorkspace final {
public:
    ProjectWorkspace() = default;

    [[nodiscard]] const ToolRegistry& tools() const noexcept;
    [[nodiscard]] const FormatIntegrationRegistry& formats() const noexcept;
    [[nodiscard]] const evidence::ArtifactRegistry& artifacts() const noexcept;
    [[nodiscard]] const evidence::EvidenceRegistry& evidence() const noexcept;
    [[nodiscard]] const evidence::EvidencePacketRegistry& packets() const noexcept;
    [[nodiscard]] const gdspaces::ResourceGraph& resources() const noexcept;
    [[nodiscard]] const ProjectGraph& graph() const noexcept;

    [[nodiscard]] bool add_evidence(evidence::EvidenceRecord record);
    [[nodiscard]] bool import_evidence_packet(
        const evidence::EvidencePacket& packet);
    [[nodiscard]] bool create_session(
        gdspaces::ResourcePayload payload,
        WorkspaceContext context = {});

    [[nodiscard]] const ResourceWorkspaceSession* find_session(
        const gdspaces::ResourceId& resource) const noexcept;
    [[nodiscard]] const ResourceWorkspaceSession* find_session(
        std::string_view canonical_id) const noexcept;

    // Legacy technical-key query retained for compatibility with older callers.
    // New stage/catalog consumers should query the explicit resource-set id.
    [[nodiscard]] std::vector<const ResourceWorkspaceSession*>
        sessions_for_stage(std::string_view stage_id) const;
    [[nodiscard]] std::vector<const ResourceWorkspaceSession*>
        sessions_for_stage_resource_set(std::string_view resource_set_id) const;

    [[nodiscard]] std::vector<const ResourceWorkspaceSession*>
        executable_sessions_for_artifact(
            std::string_view artifact_id) const;
    [[nodiscard]] std::size_t session_count() const noexcept;

    [[nodiscard]] bool register_source_modification(
        source::SourceModificationPackage package);
    [[nodiscard]] bool register_source_integration_project(
        source::IntegrationProject project);
    [[nodiscard]] const source::SourceModificationPackage*
        find_source_modification(
            std::string_view modification_id,
            std::string_view version) const noexcept;
    [[nodiscard]] const source::IntegrationProject*
        find_source_integration_project(
            std::string_view project_id) const noexcept;
    [[nodiscard]] std::size_t source_modification_count() const noexcept;
    [[nodiscard]] std::size_t source_integration_project_count() const noexcept;

    [[nodiscard]] bool register_custom_build_record(
        source::CustomBuildRecord record);
    [[nodiscard]] const source::CustomBuildRecord* find_custom_build(
        std::string_view custom_build_id) const noexcept;
    [[nodiscard]] const source::CustomBuildRecord* find_custom_build_by_sha256(
        std::string_view executable_sha256) const noexcept;
    [[nodiscard]] const source::SourceBinaryMapping* find_custom_build_mapping(
        std::string_view custom_build_id,
        std::uint64_t rva) const noexcept;
    [[nodiscard]] std::size_t custom_build_count() const noexcept;

    [[nodiscard]] bool register_item_runtime_request(
        const item::RuntimeChangeRequest& request);
    [[nodiscard]] bool register_item_runtime_validation_plan(
        const validation::ItemRuntimeValidationPlan& plan,
        const item::RuntimeChangeRequest& request);
    [[nodiscard]] bool register_compiled_item_runtime_patch_plan(
        const patch::CompiledItemRuntimePatchPlan& compiled);
    [[nodiscard]] bool register_patch_copy_artifact(
        const patch::PatchCopyArtifact& artifact);
    [[nodiscard]] bool record_parser_completed(
        const gdspaces::ResourceId& resource,
        std::string parser_id,
        bool recognized,
        gdspaces::ToolTarget consumer,
        std::uint64_t byte_revision = 0U);
    [[nodiscard]] bool add_parser_diagnostics(
        const gdspaces::ResourceId& resource,
        std::span<const formats::ParseDiagnostic> diagnostics);
    [[nodiscard]] bool attach_parsed_resource(
        const gdspaces::ResourceId& resource,
        ParsedResourceResult parsed);
    [[nodiscard]] bool attach_binary_document(
        const gdspaces::ResourceId& resource,
        binary::Document document);
    [[nodiscard]] bool attach_executable_context(
        const gdspaces::ResourceId& resource,
        ExecutableResourceContext context);
    [[nodiscard]] bool link_evidence_record(
        const gdspaces::ResourceId& resource,
        std::string_view record_id);
    [[nodiscard]] std::size_t link_evidence_claim(
        const gdspaces::ResourceId& resource,
        std::string_view claim_id);
    [[nodiscard]] std::size_t link_format_evidence(
        const gdspaces::ResourceId& resource);
    [[nodiscard]] std::size_t link_artifact_evidence(
        const gdspaces::ResourceId& resource,
        std::string_view sha256);
    [[nodiscard]] std::size_t attach_stage_bundle(
        const gdspaces::StageBundle& bundle);

    [[nodiscard]] bool enable_working_copy(
        const gdspaces::ResourceId& resource);
    [[nodiscard]] gdspaces::EditResult apply_edit(
        const gdspaces::ResourceId& resource,
        gdspaces::EditOperation operation,
        gdspaces::ToolTarget producer);
    [[nodiscard]] bool undo_last_edit(
        const gdspaces::ResourceId& resource,
        gdspaces::ToolTarget producer);
    [[nodiscard]] bool reset_working_copy(
        const gdspaces::ResourceId& resource,
        gdspaces::ToolTarget producer);
    [[nodiscard]] bool request_validation(
        const gdspaces::ResourceId& resource,
        gdspaces::ToolTarget producer,
        std::string subject_id,
        std::string message);
    [[nodiscard]] bool record_manifest_exported(
        const gdspaces::ResourceId& resource,
        gdspaces::ToolTarget producer,
        std::string manifest_id);

    [[nodiscard]] bool connect_resources(
        const gdspaces::ResourceId& from,
        const gdspaces::ResourceId& to,
        gdspaces::ResourceRelation relation,
        std::string label = {});
    [[nodiscard]] bool refresh(const gdspaces::ResourceId& resource);

private:
    [[nodiscard]] ResourceWorkspaceSession* find_session_mutable(
        const gdspaces::ResourceId& resource) noexcept;
    [[nodiscard]] bool sync(ResourceWorkspaceSession& session);
    [[nodiscard]] bool sync_evidence_packet_graph(
        const evidence::EvidencePacket& packet);

    ToolRegistry tools_;
    FormatIntegrationRegistry formats_;
    evidence::ArtifactRegistry artifacts_;
    evidence::EvidenceRegistry evidence_;
    evidence::EvidencePacketRegistry packets_;
    gdspaces::ResourceGraph resources_;
    ProjectGraph graph_;
    std::map<std::string, ResourceWorkspaceSession, std::less<>> sessions_;
    std::map<std::string, source::SourceModificationPackage, std::less<>>
        source_modifications_;
    std::map<std::string, source::IntegrationProject, std::less<>>
        source_integration_projects_;
    std::map<std::string, source::CustomBuildRecord, std::less<>>
        custom_builds_;
    std::map<std::string, std::string, std::less<>>
        custom_build_ids_by_sha256_;
};

} // namespace dmc::rengine::integration
