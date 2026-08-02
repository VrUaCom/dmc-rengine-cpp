#include "dmc_rengine/integration/project_graph_manifest.hpp"

#include <map>
#include <sstream>
#include <string_view>

namespace dmc::rengine::integration {
namespace {

[[nodiscard]] std::string escape_json(std::string_view value) {
    std::ostringstream output;
    constexpr char hex[] = "0123456789abcdef";

    for (const unsigned char character : value) {
        switch (character) {
        case '"': output << "\\\""; break;
        case '\\': output << "\\\\"; break;
        case '\b': output << "\\b"; break;
        case '\f': output << "\\f"; break;
        case '\n': output << "\\n"; break;
        case '\r': output << "\\r"; break;
        case '\t': output << "\\t"; break;
        default:
            if (character < 0x20U) {
                output << "\\u00"
                       << hex[(character >> 4U) & 0x0FU]
                       << hex[character & 0x0FU];
            } else {
                output << static_cast<char>(character);
            }
            break;
        }
    }
    return output.str();
}

void write_string(std::ostringstream& output, std::string_view value) {
    output << '"' << escape_json(value) << '"';
}

} // namespace

std::string project_graph_manifest_json(const ProjectGraph& graph) {
    std::map<std::string, std::size_t, std::less<>> node_kind_counts;
    std::map<std::string, std::size_t, std::less<>> edge_kind_counts;

    for (const auto& [id, node] : graph.all_nodes()) {
        static_cast<void>(id);
        ++node_kind_counts[std::string(to_string(node.kind))];
    }
    for (const auto& edge : graph.all_edges()) {
        ++edge_kind_counts[std::string(to_string(edge.kind))];
    }

    std::ostringstream output;
    output << "{\n"
           << "  \"schema_version\": 1,\n"
           << "  \"graph_kind\": \"dmc-rengine-project-graph\",\n"
           << "  \"summary\": {\n"
           << "    \"node_count\": " << graph.node_count() << ",\n"
           << "    \"edge_count\": " << graph.edge_count() << "\n"
           << "  },\n"
           << "  \"node_kind_counts\": {";

    if (!node_kind_counts.empty()) {
        output << '\n';
        std::size_t index = 0U;
        for (const auto& [kind, count] : node_kind_counts) {
            output << "    ";
            write_string(output, kind);
            output << ": " << count;
            if (++index != node_kind_counts.size()) {
                output << ',';
            }
            output << '\n';
        }
        output << "  ";
    }

    output << "},\n  \"edge_kind_counts\": {";
    if (!edge_kind_counts.empty()) {
        output << '\n';
        std::size_t index = 0U;
        for (const auto& [kind, count] : edge_kind_counts) {
            output << "    ";
            write_string(output, kind);
            output << ": " << count;
            if (++index != edge_kind_counts.size()) {
                output << ',';
            }
            output << '\n';
        }
        output << "  ";
    }

    output << "},\n  \"nodes\": [";
    if (!graph.all_nodes().empty()) {
        output << '\n';
        std::size_t index = 0U;
        for (const auto& [id, node] : graph.all_nodes()) {
            output << "    {\n      \"id\": ";
            write_string(output, id);
            output << ",\n      \"kind\": ";
            write_string(output, to_string(node.kind));
            output << ",\n      \"label\": ";
            write_string(output, node.label);
            output << ",\n      \"attributes\": {";
            if (!node.attributes.empty()) {
                output << '\n';
                std::size_t attribute_index = 0U;
                for (const auto& [name, value] : node.attributes) {
                    output << "        ";
                    write_string(output, name);
                    output << ": ";
                    write_string(output, value);
                    if (++attribute_index != node.attributes.size()) {
                        output << ',';
                    }
                    output << '\n';
                }
                output << "      ";
            }
            output << "}\n    }";
            if (++index != graph.all_nodes().size()) {
                output << ',';
            }
            output << '\n';
        }
        output << "  ";
    }

    output << "],\n  \"edges\": [";
    const auto& edges = graph.all_edges();
    if (!edges.empty()) {
        output << '\n';
        for (std::size_t index = 0U; index < edges.size(); ++index) {
            const auto& edge = edges[index];
            output << "    {\n      \"from\": ";
            write_string(output, edge.from);
            output << ",\n      \"to\": ";
            write_string(output, edge.to);
            output << ",\n      \"kind\": ";
            write_string(output, to_string(edge.kind));
            output << ",\n      \"label\": ";
            write_string(output, edge.label);
            output << "\n    }";
            if (index + 1U != edges.size()) {
                output << ',';
            }
            output << '\n';
        }
        output << "  ";
    }

    output << "]\n}\n";
    return output.str();
}

} // namespace dmc::rengine::integration
