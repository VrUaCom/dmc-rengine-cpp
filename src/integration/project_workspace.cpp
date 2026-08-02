#include "dmc_rengine/integration/project_workspace.hpp"

#include <utility>

namespace dmc::rengine::integration {
namespace {

[[nodiscard]] ProjectEdgeKind map_relation(
    gdspaces::ResourceRelation relation) noexcept {
    switch (relation) {
    case gdspaces::ResourceRelation::contains:
        return ProjectEdgeKind::contains;
    case gdspaces::ResourceRelation::depends_on:
        return ProjectEdgeKind::depends_on;
    case gdspaces::ResourceRelation::stage_member:
        return ProjectEdgeKind::stage_member;
    case gdspaces::ResourceRelation::evidence_for:
        return ProjectEdgeKind::evidence_for;
    case gdspaces::ResourceRelation::opens_with:
        return ProjectEdgeKind::opens_with;
    }
    return ProjectEdgeKind::related_to;
}

[[nodiscard]] std::string graph_resource_id(
    const gdspaces::ResourceId& resource) {
    return "resource:" + resource.canonical();
}

} // namespace

const ToolRegistry& ProjectWorkspace::tools() const noexcept {
    return tools_;
}

const FormatIntegrationRegistry& ProjectWorkspace::formats() const noexcept {
    return formats_;
}

const evidence::EvidenceRegistry& ProjectWorkspace::evidence() const noexcept {
    return evidence_;
}

const gdspaces::ResourceGraph& ProjectWorkspace::resources() const noexcept {
    return resources_;
}

const ProjectGraph& ProjectWorkspace::graph() const noexcept {
    return graph_;
}

bool ProjectWorkspace::add_evidence(evidence::EvidenceRecord record) {
    return evidence_.add(std::move(record));
}

bool ProjectWorkspace::create_session(
    gdspaces::ResourcePayload payload,
    WorkspaceContext context) {
    if (!payload.resource.valid()) {
        return false;
    }

    const auto key = payload.resource.id.canonical();
    if (sessions_.find(key) != sessions_.end()) {
        return false;
    }

    ResourceWorkspaceSession session(
        std::move(payload), tools_, formats_, context);
    if (!session.valid()) {
        return false;
    }
    if (!resources_.add(session.resource())) {
        return false;
    }

    const auto [iterator, inserted] = sessions_.try_emplace(
        key, std::move(session));
    if (!inserted) {
        return false;
    }
    return sync(iterator->second);
}

const ResourceWorkspaceSession* ProjectWorkspace::find_session(
    const gdspaces::ResourceId& resource) const noexcept {
    return find_session(resource.canonical());
}

const ResourceWorkspaceSession* ProjectWorkspace::find_session(
    std::string_view canonical_id) const noexcept {
    const auto iterator = sessions_.find(canonical_id);
    return iterator == sessions_.end() ? nullptr : &iterator->second;
}

std::vector<const ResourceWorkspaceSession*>
ProjectWorkspace::sessions_for_stage(std::string_view stage_id) const {
    std::vector<const ResourceWorkspaceSession*> result;
    for (const auto& [key, session] : sessions_) {
        static_cast<void>(key);
        const auto* stage = session.stage();
        if (stage != nullptr && stage->identity.stage_id == stage_id) {
            result.push_back(&session);
        }
    }
    return result;
}

std::size_t ProjectWorkspace::session_count() const noexcept {
    return sessions_.size();
}

bool ProjectWorkspace::add_parser_diagnostics(
    const gdspaces::ResourceId& resource,
    std::span<const formats::ParseDiagnostic> diagnostics) {
    auto* session = find_session_mutable(resource);
    if (session == nullptr) {
        return false;
    }
    const auto result = session->add_parser_diagnostics(diagnostics);
    static_cast<void>(sync(*session));
    return result;
}

bool ProjectWorkspace::attach_binary_document(
    const gdspaces::ResourceId& resource,
    binary::Document document) {
    auto* session = find_session_mutable(resource);
    if (session == nullptr ||
        !session->attach_binary_document(std::move(document))) {
        return false;
    }
    return sync(*session);
}

bool ProjectWorkspace::link_evidence_record(
    const gdspaces::ResourceId& resource,
    std::string_view record_id) {
    auto* session = find_session_mutable(resource);
    if (session == nullptr ||
        !session->link_evidence_record(evidence_, record_id)) {
        return false;
    }
    return sync(*session);
}

std::size_t ProjectWorkspace::link_evidence_claim(
    const gdspaces::ResourceId& resource,
    std::string_view claim_id) {
    auto* session = find_session_mutable(resource);
    if (session == nullptr) {
        return 0U;
    }
    const auto linked = session->link_evidence_claim(evidence_, claim_id);
    if (linked != 0U) {
        static_cast<void>(sync(*session));
    }
    return linked;
}

std::size_t ProjectWorkspace::link_format_evidence(
    const gdspaces::ResourceId& resource) {
    auto* session = find_session_mutable(resource);
    if (session == nullptr) {
        return 0U;
    }
    const auto linked = session->link_format_evidence(evidence_);
    if (linked != 0U) {
        static_cast<void>(sync(*session));
    }
    return linked;
}

std::size_t ProjectWorkspace::attach_stage_bundle(
    const gdspaces::StageBundle& bundle) {
    std::size_t attached = 0U;
    for (const auto& member : bundle.all_members()) {
        auto* session = find_session_mutable(member.resource.id);
        if (session != nullptr && session->attach_stage_bundle(bundle)) {
            ++attached;
            static_cast<void>(sync(*session));
        }
    }
    return attached;
}

bool ProjectWorkspace::enable_working_copy(
    const gdspaces::ResourceId& resource) {
    auto* session = find_session_mutable(resource);
    if (session == nullptr || !session->enable_working_copy()) {
        return false;
    }
    return sync(*session);
}

gdspaces::EditResult ProjectWorkspace::apply_edit(
    const gdspaces::ResourceId& resource,
    gdspaces::EditOperation operation,
    gdspaces::ToolTarget producer) {
    auto* session = find_session_mutable(resource);
    if (session == nullptr) {
        return gdspaces::EditResult{
            .applied = false,
            .revision = 0U,
            .error = "Resource workspace session was not found.",
        };
    }

    auto result = session->apply_edit(std::move(operation), producer);
    if (result.applied) {
        static_cast<void>(sync(*session));
    }
    return result;
}

bool ProjectWorkspace::undo_last_edit(
    const gdspaces::ResourceId& resource,
    gdspaces::ToolTarget producer) {
    auto* session = find_session_mutable(resource);
    if (session == nullptr || !session->undo_last_edit(producer)) {
        return false;
    }
    return sync(*session);
}

bool ProjectWorkspace::reset_working_copy(
    const gdspaces::ResourceId& resource,
    gdspaces::ToolTarget producer) {
    auto* session = find_session_mutable(resource);
    if (session == nullptr || !session->reset_working_copy(producer)) {
        return false;
    }
    return sync(*session);
}

bool ProjectWorkspace::request_validation(
    const gdspaces::ResourceId& resource,
    gdspaces::ToolTarget producer,
    std::string subject_id,
    std::string message) {
    auto* session = find_session_mutable(resource);
    if (session == nullptr || !session->request_validation(
            producer, std::move(subject_id), std::move(message))) {
        return false;
    }
    return sync(*session);
}

bool ProjectWorkspace::record_manifest_exported(
    const gdspaces::ResourceId& resource,
    gdspaces::ToolTarget producer,
    std::string manifest_id) {
    auto* session = find_session_mutable(resource);
    if (session == nullptr || !session->record_manifest_exported(
            producer, std::move(manifest_id))) {
        return false;
    }
    return sync(*session);
}

bool ProjectWorkspace::connect_resources(
    const gdspaces::ResourceId& from,
    const gdspaces::ResourceId& to,
    gdspaces::ResourceRelation relation,
    std::string label) {
    if (!resources_.connect(from, to, relation)) {
        return false;
    }
    if (label.empty()) {
        label = std::string(gdspaces::to_string(relation));
    }
    return graph_.connect(ProjectEdge{
        .from = graph_resource_id(from),
        .to = graph_resource_id(to),
        .kind = map_relation(relation),
        .label = std::move(label),
    });
}

bool ProjectWorkspace::refresh(const gdspaces::ResourceId& resource) {
    auto* session = find_session_mutable(resource);
    return session != nullptr && sync(*session);
}

ResourceWorkspaceSession* ProjectWorkspace::find_session_mutable(
    const gdspaces::ResourceId& resource) noexcept {
    const auto iterator = sessions_.find(resource.canonical());
    return iterator == sessions_.end() ? nullptr : &iterator->second;
}

bool ProjectWorkspace::sync(ResourceWorkspaceSession& session) {
    return graph_.ingest_workspace(session, tools_);
}

} // namespace dmc::rengine::integration
