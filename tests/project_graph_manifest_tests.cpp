#include "dmc_rengine/core/json.hpp"
#include "dmc_rengine/integration/project_graph.hpp"
#include "dmc_rengine/integration/project_graph_manifest.hpp"

#include <cassert>
#include <string_view>

namespace {

[[nodiscard]] const dmc::rengine::core::json::Value* member(
    const dmc::rengine::core::json::Value::Object& object,
    std::string_view name) {
    const auto iterator = object.find(name);
    return iterator == object.end() ? nullptr : &iterator->second;
}

} // namespace

int main() {
    using dmc::rengine::core::json::Parser;
    using dmc::rengine::integration::ProjectEdge;
    using dmc::rengine::integration::ProjectEdgeKind;
    using dmc::rengine::integration::ProjectGraph;
    using dmc::rengine::integration::ProjectNode;
    using dmc::rengine::integration::ProjectNodeKind;
    using dmc::rengine::integration::project_graph_manifest_json;

    ProjectGraph graph;
    assert(graph.upsert(ProjectNode{
        .id = "resource:synthetic",
        .kind = ProjectNodeKind::resource,
        .label = "Synthetic resource",
        .attributes = {
            {"format", "hits"},
            {"profile", "dmc3-hd"},
        },
    }));
    assert(graph.upsert(ProjectNode{
        .id = "format:hits",
        .kind = ProjectNodeKind::format,
        .label = "hits",
        .attributes = {
            {"maturity", "structural"},
        },
    }));
    assert(graph.upsert(ProjectNode{
        .id = "tool:binary-inspector",
        .kind = ProjectNodeKind::tool,
        .label = "Binary Inspector",
        .attributes = {
            {"lore_name", "The Reliquary"},
        },
    }));
    assert(graph.connect(ProjectEdge{
        .from = "resource:synthetic",
        .to = "format:hits",
        .kind = ProjectEdgeKind::classified_as,
        .label = "hits",
    }));
    assert(graph.connect(ProjectEdge{
        .from = "resource:synthetic",
        .to = "tool:binary-inspector",
        .kind = ProjectEdgeKind::opens_with,
        .label = "companion",
    }));

    const auto json = project_graph_manifest_json(graph);
    assert(!json.empty());
    const auto parsed = Parser::parse(json);
    assert(parsed.ok());
    const auto* root = parsed.value->as_object();
    assert(root != nullptr);

    const auto* schema = member(*root, "schema_version");
    assert(schema != nullptr);
    assert(schema->as_u64() != nullptr);
    assert(*schema->as_u64() == 1U);

    const auto* kind = member(*root, "graph_kind");
    assert(kind != nullptr);
    assert(kind->as_string() != nullptr);
    assert(*kind->as_string() == "dmc-rengine-project-graph");

    const auto* summary_value = member(*root, "summary");
    assert(summary_value != nullptr);
    const auto* summary = summary_value->as_object();
    assert(summary != nullptr);
    const auto* node_count = member(*summary, "node_count");
    assert(node_count != nullptr);
    assert(node_count->as_u64() != nullptr);
    assert(*node_count->as_u64() == 3U);
    const auto* edge_count = member(*summary, "edge_count");
    assert(edge_count != nullptr);
    assert(edge_count->as_u64() != nullptr);
    assert(*edge_count->as_u64() == 2U);

    const auto* nodes_value = member(*root, "nodes");
    assert(nodes_value != nullptr);
    const auto* nodes = nodes_value->as_array();
    assert(nodes != nullptr);
    assert(nodes->size() == 3U);

    const auto* edges_value = member(*root, "edges");
    assert(edges_value != nullptr);
    const auto* edges = edges_value->as_array();
    assert(edges != nullptr);
    assert(edges->size() == 2U);

    const auto second = project_graph_manifest_json(graph);
    assert(second == json);

    ProjectGraph empty;
    const auto empty_json = project_graph_manifest_json(empty);
    assert(Parser::parse(empty_json).ok());
    return 0;
}
