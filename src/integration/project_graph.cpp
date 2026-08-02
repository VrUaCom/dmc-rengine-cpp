#include "dmc_rengine/integration/project_graph.hpp"

#include <algorithm>
#include <sstream>
#include <utility>

namespace dmc::rengine::integration {
namespace {

[[nodiscard]] std::string resource_node_id(
    const gdspaces::ResourceId& resource) {
    return "resource:" + resource.canonical();
}

[[nodiscard]] std::string tool_node_id(gdspaces::ToolTarget tool) {
    return "tool:" + std::string(gdspaces::to_string(tool));
}

[[nodiscard]] std::string stage_node_id(
    const StageResourceContext& stage) {
    return "stage:" + stage.identity.profile + ':' + stage.identity.stage_id;
}

[[nodiscard]] std::string join_capabilities(
    const std::vector<ToolCapability>& capabilities) {
    std::ostringstream output;
    for (std::size_t index = 0; index < capabilities.size(); ++index) {
        if (index != 0U) {
            output << ',';
        }
        output << to_string(capabilities[index]);
    }
    return output.str();
}

[[nodiscard]] std::string bool_text(bool value) {
    return value ? "true" : "false";
}

} // namespace

bool ProjectGraph::upsert(ProjectNode node) {
    if (!node.valid()) {
        return false;
    }

    const auto iterator = nodes_.find(node.id);
    if (iterator != nodes_.end() && iterator->second.kind != node.kind) {
        return false;
    }
    nodes_[node.id] = std::move(node);
    return true;
}

bool ProjectGraph::connect(ProjectEdge edge) {
    if (!edge.valid() || find(edge.from) == nullptr || find(edge.to) == nullptr) {
        return false;
    }
    if (std::find(edges_.begin(), edges_.end(), edge) != edges_.end()) {
        return false;
    }
    edges_.push_back(std::move(edge));
    std::sort(
        edges_.begin(), edges_.end(),
        [](const ProjectEdge& left, const ProjectEdge& right) {
            if (left.from != right.from) {
                return left.from < right.from;
            }
            if (left.to != right.to) {
                return left.to < right.to;
            }
            if (left.kind != right.kind) {
                return static_cast<int>(left.kind) <
                       static_cast<int>(right.kind);
            }
            return left.label < right.label;
        });
    return true;
}

const ProjectNode* ProjectGraph::find(std::string_view id) const noexcept {
    const auto iterator = nodes_.find(id);
    return iterator == nodes_.end() ? nullptr : &iterator->second;
}

std::vector<const ProjectNode*> ProjectGraph::nodes(
    ProjectNodeKind kind) const {
    std::vector<const ProjectNode*> result;
    for (const auto& [id, node] : nodes_) {
        static_cast<void>(id);
        if (node.kind == kind) {
            result.push_back(&node);
        }
    }
    return result;
}

std::vector<const ProjectEdge*> ProjectGraph::outgoing(
    std::string_view node_id) const {
    std::vector<const ProjectEdge*> result;
    for (const auto& edge : edges_) {
        if (edge.from == node_id) {
            result.push_back(&edge);
        }
    }
    return result;
}

std::vector<const ProjectEdge*> ProjectGraph::incoming(
    std::string_view node_id) const {
    std::vector<const ProjectEdge*> result;
    for (const auto& edge : edges_) {
        if (edge.to == node_id) {
            result.push_back(&edge);
        }
    }
    return result;
}

bool ProjectGraph::ingest_workspace(
    const ResourceWorkspaceSession& workspace,
    const ToolRegistry& tools) {
    if (!workspace.resource().valid()) {
        return false;
    }

    const auto resource_id = resource_node_id(workspace.resource().id);
    if (!upsert(ProjectNode{
            .id = resource_id,
            .kind = ProjectNodeKind::resource,
            .label = workspace.resource().display_name,
            .attributes = {
                {"canonical_id", workspace.resource().id.canonical()},
                {"source_id", workspace.resource().id.source_id},
                {"logical_path", workspace.resource().id.logical_path},
                {"container_chain", workspace.resource().id.container_chain},
                {"format", workspace.resource().format},
                {"profile", workspace.resource().profile},
                {"container", bool_text(workspace.resource().container)},
                {"workspace_status", std::string(to_string(workspace.status()))},
            },
        })) {
        return false;
    }

    const auto ensure_tool = [this, &tools](gdspaces::ToolTarget target) {
        const auto id = tool_node_id(target);
        const auto* descriptor = tools.find(target);
        ProjectNode node{
            .id = id,
            .kind = ProjectNodeKind::tool,
            .label = descriptor == nullptr
                ? std::string(gdspaces::to_string(target))
                : descriptor->product_name,
            .attributes = {
                {"target", std::string(gdspaces::to_string(target))},
            },
        };
        if (descriptor != nullptr) {
            node.attributes["lore_name"] = descriptor->lore_name;
            node.attributes["capabilities"] =
                join_capabilities(descriptor->capabilities);
        }
        static_cast<void>(upsert(std::move(node)));
        return id;
    };

    if (const auto* format = workspace.format(); format != nullptr) {
        const auto format_id = "format:" + format->format;
        static_cast<void>(upsert(ProjectNode{
            .id = format_id,
            .kind = ProjectNodeKind::format,
            .label = format->format,
            .attributes = {
                {"maturity", std::string(to_string(format->maturity))},
                {"write_policy", std::string(to_string(format->write_policy))},
                {"binary_adapter", bool_text(format->binary_adapter)},
            },
        }));
        static_cast<void>(connect(ProjectEdge{
            .from = resource_id,
            .to = format_id,
            .kind = ProjectEdgeKind::classified_as,
            .label = format->format,
        }));

        if (!format->parser_id.empty()) {
            const auto parser_id = "parser:" + format->parser_id;
            static_cast<void>(upsert(ProjectNode{
                .id = parser_id,
                .kind = ProjectNodeKind::parser,
                .label = format->parser_id,
                .attributes = {
                    {"format", format->format},
                    {"maturity", std::string(to_string(format->maturity))},
                },
            }));
            static_cast<void>(connect(ProjectEdge{
                .from = resource_id,
                .to = parser_id,
                .kind = ProjectEdgeKind::parsed_by,
                .label = format->parser_id,
            }));
        }
    }

    for (const auto& route : workspace.routes()) {
        const auto tool_id = ensure_tool(route.target);
        static_cast<void>(connect(ProjectEdge{
            .from = resource_id,
            .to = tool_id,
            .kind = ProjectEdgeKind::opens_with,
            .label = std::string(to_string(route.role)) + ": " + route.reason,
        }));
    }

    for (const auto& evidence_id : workspace.evidence_record_ids()) {
        const auto node_id = "evidence:" + evidence_id;
        static_cast<void>(upsert(ProjectNode{
            .id = node_id,
            .kind = ProjectNodeKind::evidence_record,
            .label = evidence_id,
            .attributes = {},
        }));
        static_cast<void>(connect(ProjectEdge{
            .from = node_id,
            .to = resource_id,
            .kind = ProjectEdgeKind::evidence_for,
            .label = "workspace evidence link",
        }));
    }

    if (const auto* stage = workspace.stage(); stage != nullptr) {
        const auto node_id = stage_node_id(*stage);
        static_cast<void>(upsert(ProjectNode{
            .id = node_id,
            .kind = ProjectNodeKind::stage,
            .label = stage->identity.display_name.empty()
                ? stage->identity.stage_id
                : stage->identity.display_name,
            .attributes = {
                {"profile", stage->identity.profile},
                {"stage_id", stage->identity.stage_id},
                {"exe_evidence_id", stage->identity.exe_evidence_id},
                {"category", std::string(gdspaces::to_string(stage->category))},
            },
        }));
        static_cast<void>(connect(ProjectEdge{
            .from = node_id,
            .to = resource_id,
            .kind = ProjectEdgeKind::stage_member,
            .label = stage->role,
        }));
    }

    if (const auto* binary = workspace.binary_document(); binary != nullptr) {
        const auto node_id = "binary-document:" +
            workspace.resource().id.canonical();
        static_cast<void>(upsert(ProjectNode{
            .id = node_id,
            .kind = ProjectNodeKind::binary_document,
            .label = workspace.resource().display_name + " structure",
            .attributes = {
                {"byte_size", std::to_string(binary->byte_size())},
                {"coverage_bytes", std::to_string(binary->coverage_bytes())},
                {"regions", std::to_string(binary->regions().size())},
                {"fields", std::to_string(binary->fields().size())},
                {"unknown_ranges", std::to_string(binary->unknown_ranges().size())},
            },
        }));
        static_cast<void>(connect(ProjectEdge{
            .from = node_id,
            .to = resource_id,
            .kind = ProjectEdgeKind::derived_from,
            .label = "structural interpretation",
        }));
        static_cast<void>(connect(ProjectEdge{
            .from = node_id,
            .to = ensure_tool(gdspaces::ToolTarget::binary_inspector),
            .kind = ProjectEdgeKind::produced_by,
            .label = "Binary Inspector document",
        }));
    }

    if (const auto* working = workspace.working_copy(); working != nullptr) {
        const auto node_id = "working-copy:" +
            workspace.resource().id.canonical();
        static_cast<void>(upsert(ProjectNode{
            .id = node_id,
            .kind = ProjectNodeKind::working_copy,
            .label = workspace.resource().display_name + " working copy",
            .attributes = {
                {"source_sha256", working->source_sha256()},
                {"revision", std::to_string(working->revision())},
                {"dirty", bool_text(working->dirty())},
                {"byte_size", std::to_string(working->bytes().size())},
                {"operation_count", std::to_string(working->history().size())},
            },
        }));
        static_cast<void>(connect(ProjectEdge{
            .from = node_id,
            .to = resource_id,
            .kind = ProjectEdgeKind::derived_from,
            .label = "immutable source revision",
        }));
    }

    for (const auto& event : workspace.events().events()) {
        const auto event_id = "event:" +
            workspace.resource().id.canonical() + ':' +
            std::to_string(event.sequence);
        static_cast<void>(upsert(ProjectNode{
            .id = event_id,
            .kind = ProjectNodeKind::event,
            .label = std::string(to_string(event.type)) + " #" +
                std::to_string(event.sequence),
            .attributes = {
                {"event_type", std::string(to_string(event.type))},
                {"revision", std::to_string(event.revision)},
                {"subject_id", event.subject_id},
                {"message", event.message},
            },
        }));
        static_cast<void>(connect(ProjectEdge{
            .from = resource_id,
            .to = event_id,
            .kind = ProjectEdgeKind::records_event,
            .label = std::string(to_string(event.type)),
        }));
        static_cast<void>(connect(ProjectEdge{
            .from = event_id,
            .to = ensure_tool(event.producer),
            .kind = ProjectEdgeKind::produced_by,
            .label = "event producer",
        }));
        if (event.consumer.has_value()) {
            static_cast<void>(connect(ProjectEdge{
                .from = event_id,
                .to = ensure_tool(*event.consumer),
                .kind = ProjectEdgeKind::delivered_to,
                .label = "event consumer",
            }));
        }

        if (event.type == WorkspaceEventType::validation_requested) {
            static_cast<void>(connect(ProjectEdge{
                .from = ensure_tool(gdspaces::ToolTarget::build_test_lab),
                .to = resource_id,
                .kind = ProjectEdgeKind::validates,
                .label = event.subject_id,
            }));
        }

        if (event.type == WorkspaceEventType::manifest_exported &&
            !event.subject_id.empty()) {
            const auto manifest_id = "manifest:" + event.subject_id;
            static_cast<void>(upsert(ProjectNode{
                .id = manifest_id,
                .kind = ProjectNodeKind::manifest,
                .label = event.subject_id,
                .attributes = {
                    {"revision", std::to_string(event.revision)},
                },
            }));
            static_cast<void>(connect(ProjectEdge{
                .from = manifest_id,
                .to = resource_id,
                .kind = ProjectEdgeKind::derived_from,
                .label = "workspace metadata snapshot",
            }));
            static_cast<void>(connect(ProjectEdge{
                .from = manifest_id,
                .to = ensure_tool(event.producer),
                .kind = ProjectEdgeKind::produced_by,
                .label = "manifest producer",
            }));
        }
    }

    return true;
}

const std::map<std::string, ProjectNode, std::less<>>&
ProjectGraph::all_nodes() const noexcept {
    return nodes_;
}

const std::vector<ProjectEdge>& ProjectGraph::all_edges() const noexcept {
    return edges_;
}

std::size_t ProjectGraph::node_count() const noexcept {
    return nodes_.size();
}

std::size_t ProjectGraph::edge_count() const noexcept {
    return edges_.size();
}

} // namespace dmc::rengine::integration
