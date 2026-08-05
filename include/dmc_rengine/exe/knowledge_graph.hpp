#pragma once

#include "dmc_rengine/exe/model.hpp"

#include <cstdint>
#include <optional>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

namespace dmc::rengine::exe {

enum class GraphNodeKind : std::uint8_t {
    runtime_range,
    function_fragment,
    logical_function,
    recovered_source_unit,
    class_model,
    vtable,
    global_initializer,
    memory_arena,
    subsystem,
    resource_binding,
};

enum class GraphEdgeKind : std::uint8_t {
    contains,
    calls,
    reads,
    writes,
    installs,
    inherits,
    owns,
    binds,
    maps_to,
    initialized_by,
    belongs_to,
    supersedes,
};

struct GraphNode {
    std::string id;
    GraphNodeKind kind{GraphNodeKind::logical_function};
    std::string label;
    EntityStatus status{EntityStatus::candidate};
};

struct GraphEdge {
    std::string id;
    std::string source_id;
    std::string target_id;
    GraphEdgeKind kind{GraphEdgeKind::contains};
    EntityStatus status{EntityStatus::candidate};
};

class ExeKnowledgeGraph {
public:
    [[nodiscard]] bool add_node(GraphNode node);
    [[nodiscard]] bool add_edge(GraphEdge edge);

    [[nodiscard]] const GraphNode* find_node(std::string_view id) const noexcept;
    [[nodiscard]] const GraphEdge* find_edge(std::string_view id) const noexcept;

    [[nodiscard]] std::vector<const GraphEdge*> outgoing(
        std::string_view source_id,
        std::optional<GraphEdgeKind> kind = std::nullopt) const;

    [[nodiscard]] bool valid() const noexcept;
    [[nodiscard]] std::size_t node_count() const noexcept { return nodes_.size(); }
    [[nodiscard]] std::size_t edge_count() const noexcept { return edges_.size(); }

private:
    std::unordered_map<std::string, GraphNode> nodes_;
    std::unordered_map<std::string, GraphEdge> edges_;
};

} // namespace dmc::rengine::exe
