#include "dmc_rengine/exe/project.hpp"

#include <algorithm>
#include <sstream>
#include <utility>

namespace dmc::rengine::exe {
namespace {

[[nodiscard]] std::string escape_json(std::string_view text) {
    std::string output;
    output.reserve(text.size());
    for (const char character : text) {
        switch (character) {
        case '\\': output += "\\\\"; break;
        case '"': output += "\\\""; break;
        case '\n': output += "\\n"; break;
        case '\r': output += "\\r"; break;
        case '\t': output += "\\t"; break;
        default: output += character; break;
        }
    }
    return output;
}

[[nodiscard]] GraphNode node(
    std::string id,
    GraphNodeKind kind,
    std::string label,
    EntityStatus status) {
    return GraphNode{
        .id = std::move(id),
        .kind = kind,
        .label = std::move(label),
        .status = status,
    };
}

} // namespace

bool ExeKnowledgeGraph::add_node(GraphNode value) {
    if (value.id.empty()) {
        return false;
    }
    return nodes_.emplace(value.id, std::move(value)).second;
}

bool ExeKnowledgeGraph::add_edge(GraphEdge value) {
    if (value.id.empty() || value.source_id.empty() || value.target_id.empty()) {
        return false;
    }
    if (!nodes_.contains(value.source_id) || !nodes_.contains(value.target_id)) {
        return false;
    }
    return edges_.emplace(value.id, std::move(value)).second;
}

const GraphNode* ExeKnowledgeGraph::find_node(std::string_view id) const noexcept {
    const auto iterator = nodes_.find(std::string{id});
    return iterator == nodes_.end() ? nullptr : &iterator->second;
}

const GraphEdge* ExeKnowledgeGraph::find_edge(std::string_view id) const noexcept {
    const auto iterator = edges_.find(std::string{id});
    return iterator == edges_.end() ? nullptr : &iterator->second;
}

std::vector<const GraphEdge*> ExeKnowledgeGraph::outgoing(
    std::string_view source_id,
    std::optional<GraphEdgeKind> kind) const {
    std::vector<const GraphEdge*> result;
    for (const auto& [id, edge] : edges_) {
        (void)id;
        if (edge.source_id == source_id && (!kind.has_value() || edge.kind == *kind)) {
            result.push_back(&edge);
        }
    }
    std::ranges::sort(result, {}, [](const GraphEdge* edge) { return edge->id; });
    return result;
}

bool ExeKnowledgeGraph::valid() const noexcept {
    return std::ranges::all_of(edges_, [this](const auto& item) {
        const auto& edge = item.second;
        return nodes_.contains(edge.source_id) && nodes_.contains(edge.target_id);
    });
}

ExecutableProject::ExecutableProject(ExecutableProjectManifest manifest)
    : manifest_(std::move(manifest)) {}

template <typename T>
bool ExecutableProject::insert_unique(
    std::vector<T>& values,
    T value,
    std::string_view id) {
    if (id.empty()) {
        return false;
    }
    const auto duplicate = std::ranges::find_if(values, [id](const T& current) {
        return current.id == id;
    });
    if (duplicate != values.end()) {
        return false;
    }
    values.push_back(std::move(value));
    return true;
}

bool ExecutableProject::add_runtime_range(RuntimeRange value) {
    const auto id = value.id;
    if (!insert_unique(runtime_ranges_, std::move(value), id)) return false;
    return graph_.add_node(node(id, GraphNodeKind::runtime_range, id, EntityStatus::confirmed));
}

bool ExecutableProject::add_fragment(FunctionFragment value) {
    const auto id = value.id;
    if (!insert_unique(fragments_, std::move(value), id)) return false;
    return graph_.add_node(node(id, GraphNodeKind::function_fragment, id, EntityStatus::candidate));
}

bool ExecutableProject::add_logical_function(LogicalFunction value) {
    const auto id = value.id;
    const auto label = value.recovered_name.empty() ? id : value.recovered_name;
    const auto status = value.status;
    if (!insert_unique(logical_functions_, std::move(value), id)) return false;
    return graph_.add_node(node(id, GraphNodeKind::logical_function, label, status));
}

bool ExecutableProject::add_source_unit(RecoveredSourceUnit value) {
    const auto id = value.id;
    const auto label = value.path;
    const auto status = value.status;
    if (!insert_unique(source_units_, std::move(value), id)) return false;
    return graph_.add_node(node(id, GraphNodeKind::recovered_source_unit, label, status));
}

bool ExecutableProject::add_source_mapping(SourceBinaryMapping value) {
    if (value.source_unit_id.empty() || value.logical_function_id.empty()) return false;
    source_mappings_.push_back(std::move(value));
    return true;
}

bool ExecutableProject::add_class(ClassModel value) {
    const auto id = value.id;
    const auto label = value.name;
    if (!insert_unique(classes_, std::move(value), id)) return false;
    return graph_.add_node(node(id, GraphNodeKind::class_model, label, EntityStatus::candidate));
}

bool ExecutableProject::add_vtable(VtableModel value) {
    const auto id = value.id;
    if (!insert_unique(vtables_, std::move(value), id)) return false;
    return graph_.add_node(node(id, GraphNodeKind::vtable, id, EntityStatus::candidate));
}

bool ExecutableProject::add_global_initializer(GlobalInitializerModel value) {
    const auto id = value.id;
    const auto label = value.semantic_role;
    const auto status = value.status;
    if (!insert_unique(global_initializers_, std::move(value), id)) return false;
    return graph_.add_node(node(id, GraphNodeKind::global_initializer, label, status));
}

bool ExecutableProject::add_memory_arena(MemoryArenaModel value) {
    const auto id = value.id;
    const auto label = value.name;
    const auto status = value.status;
    if (!insert_unique(memory_arenas_, std::move(value), id)) return false;
    return graph_.add_node(node(id, GraphNodeKind::memory_arena, label, status));
}

bool ExecutableProject::add_subsystem(SubsystemModel value) {
    const auto id = value.id;
    const auto label = value.name;
    const auto status = value.status;
    if (!insert_unique(subsystems_, std::move(value), id)) return false;
    return graph_.add_node(node(id, GraphNodeKind::subsystem, label, status));
}

bool ExecutableProject::add_resource_binding(ResourceBindingModel value) {
    const auto id = value.id;
    const auto label = value.logical_path;
    const auto status = value.status;
    if (!insert_unique(resource_bindings_, std::move(value), id)) return false;
    return graph_.add_node(node(id, GraphNodeKind::resource_binding, label, status));
}

bool ExecutableProject::valid() const noexcept {
    if (manifest_.schema_version != 1U || manifest_.phase != "0.01a" ||
        manifest_.language_standard != "C++20" || manifest_.project_id.empty() ||
        manifest_.target.sha256.empty() || manifest_.first_reverse_pass > manifest_.last_reverse_pass) {
        return false;
    }
    if (manifest_.first_reverse_pass != 0U || manifest_.last_reverse_pass < 32U) {
        return false;
    }
    return graph_.valid();
}

std::string ExecutableProject::manifest_json() const {
    std::ostringstream output;
    output << "{\n"
           << "  \"schema_version\": " << manifest_.schema_version << ",\n"
           << "  \"project_id\": \"" << escape_json(manifest_.project_id) << "\",\n"
           << "  \"phase\": \"" << escape_json(manifest_.phase) << "\",\n"
           << "  \"language_standard\": \"" << escape_json(manifest_.language_standard) << "\",\n"
           << "  \"target_sha256\": \"" << escape_json(manifest_.target.sha256) << "\",\n"
           << "  \"first_reverse_pass\": " << manifest_.first_reverse_pass << ",\n"
           << "  \"last_reverse_pass\": " << manifest_.last_reverse_pass << ",\n"
           << "  \"authority_count\": " << manifest_.authorities.size() << ",\n"
           << "  \"graph_node_count\": " << graph_.node_count() << ",\n"
           << "  \"graph_edge_count\": " << graph_.edge_count() << "\n"
           << "}\n";
    return output.str();
}

} // namespace dmc::rengine::exe
