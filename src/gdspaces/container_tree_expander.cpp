#include "dmc_rengine/gdspaces/container_tree_expander.hpp"

#include "dmc_rengine/gdspaces/byte_provenance.hpp"

#include <algorithm>
#include <functional>
#include <limits>
#include <map>
#include <set>
#include <sstream>
#include <string>
#include <utility>

namespace dmc::rengine::gdspaces {
namespace {

void add_diagnostic(
    ContainerTreeExpansion& result,
    DiagnosticSeverity severity,
    std::string code,
    std::string message,
    const ResourceId& resource) {
    result.diagnostics.push_back(Diagnostic{
        .severity = severity,
        .code = std::move(code),
        .message = std::move(message),
        .resource = resource,
    });
}

void append_string_field(std::ostringstream& output, std::string_view value) {
    output << value.size() << ':' << value << '|';
}

[[nodiscard]] std::string parse_cache_key(
    const ResourcePayload& payload,
    std::string_view parser_id) {
    std::ostringstream output;
    append_string_field(output, parser_id);

    if (!payload.byte_provenance.has_value()) {
        output << "identity|";
        append_string_field(output, payload.resource.id.canonical());
        return output.str();
    }

    const auto& provenance = *payload.byte_provenance;
    if (!provenance.valid()) {
        output << "invalid-lineage|";
        append_string_field(output, payload.resource.id.canonical());
        return output.str();
    }

    output << "provenance|" << to_string(provenance.kind) << '|';
    append_string_field(output, provenance.authority_id);
    output << provenance.offset << '|'
           << provenance.stored_size << '|'
           << provenance.materialized_size << '|'
           << to_string(provenance.transform) << '|';
    if (provenance.crc32.has_value()) {
        output << *provenance.crc32;
    } else {
        output << '-';
    }
    return output.str();
}

[[nodiscard]] std::size_t count_new_nodes(
    const ResourceGraph& graph,
    const ContainerExpansion& expansion) {
    std::size_t count = 0U;
    for (const auto& child : expansion.children) {
        if (graph.find(child.payload.resource.id) == nullptr) {
            ++count;
        }
    }
    return count;
}

} // namespace

bool ContainerTreeExpansionLimits::valid() const noexcept {
    return max_expanded_containers > 0U &&
        max_graph_nodes > 0U &&
        max_parsed_container_bytes > 0U;
}

bool ContainerTreeExpansion::complete() const noexcept {
    if (!fully_expanded) {
        return false;
    }
    return std::none_of(
        diagnostics.begin(), diagnostics.end(),
        [](const Diagnostic& diagnostic) {
            return diagnostic.severity == DiagnosticSeverity::error;
        });
}

ContainerTreeExpansion ContainerTreeExpander::expand(
    const ResourcePayload& root,
    const formats::ContainerParserRegistry& registry,
    ContainerTreeExpansionLimits limits) {
    ContainerTreeExpansion result;

    if (!limits.valid()) {
        result.fully_expanded = false;
        add_diagnostic(
            result,
            DiagnosticSeverity::error,
            "gdspaces.container-tree.invalid-limits",
            "Recursive container expansion limits are invalid.",
            root.resource.id);
        return result;
    }

    if (!root.resource.valid() || !root.readable()) {
        result.fully_expanded = false;
        add_diagnostic(
            result,
            DiagnosticSeverity::error,
            "gdspaces.container-tree.invalid-root",
            "The recursive container root is not a valid readable resource payload.",
            root.resource.id);
        return result;
    }

    static_cast<void>(result.graph.add(root.resource));
    std::map<std::string, formats::ContainerParseResult, std::less<>> parse_cache;
    std::set<std::string, std::less<>> active_byte_domains;

    std::function<void(const ResourcePayload&, std::size_t, bool)> visit;
    visit = [&](const ResourcePayload& payload, std::size_t depth, bool is_root) {
        const auto* parser = registry.select(
            std::span<const std::byte>{payload.bytes},
            payload.resource.id.logical_path);
        if (parser == nullptr) {
            result.fully_expanded = false;
            add_diagnostic(
                result,
                is_root ? DiagnosticSeverity::error : DiagnosticSeverity::warning,
                "gdspaces.container-tree.no-parser",
                "No registered container parser recognizes this resource; the node is preserved without nested expansion.",
                payload.resource.id);
            return;
        }

        if (result.expanded_container_count >= limits.max_expanded_containers) {
            result.fully_expanded = false;
            add_diagnostic(
                result,
                DiagnosticSeverity::error,
                "gdspaces.container-tree.container-budget",
                "The maximum number of recursively expanded container identities was reached.",
                payload.resource.id);
            return;
        }

        const auto cache_key = parse_cache_key(payload, parser->id());
        if (!active_byte_domains.insert(cache_key).second) {
            result.fully_expanded = false;
            add_diagnostic(
                result,
                DiagnosticSeverity::warning,
                "gdspaces.container-tree.byte-domain-cycle",
                "The same parser/byte domain is already active in this ancestry; recursive expansion stopped to prevent a cycle.",
                payload.resource.id);
            return;
        }

        const formats::ContainerParseResult* parsed = nullptr;
        const auto cached = parse_cache.find(cache_key);
        if (cached != parse_cache.end()) {
            ++result.parse_cache_hits;
            parsed = &cached->second;
        } else {
            const auto byte_count = static_cast<std::uint64_t>(payload.bytes.size());
            if (result.parsed_container_bytes >
                    std::numeric_limits<std::uint64_t>::max() - byte_count ||
                result.parsed_container_bytes + byte_count >
                    limits.max_parsed_container_bytes) {
                result.fully_expanded = false;
                add_diagnostic(
                    result,
                    DiagnosticSeverity::error,
                    "gdspaces.container-tree.byte-budget",
                    "The recursive parsed-container byte budget would be exceeded.",
                    payload.resource.id);
                active_byte_domains.erase(cache_key);
                return;
            }

            auto parse_result = registry.parse(
                std::span<const std::byte>{payload.bytes},
                payload.resource.id.logical_path);
            auto [iterator, inserted] = parse_cache.emplace(
                cache_key, std::move(parse_result));
            static_cast<void>(inserted);
            parsed = &iterator->second;
            ++result.parser_invocation_count;
            result.parsed_container_bytes += byte_count;
        }

        auto expansion = ContainerExpander::expand(payload, *parsed);
        const auto new_nodes = count_new_nodes(result.graph, expansion);
        if (result.graph.resource_count() > limits.max_graph_nodes ||
            new_nodes > limits.max_graph_nodes - result.graph.resource_count()) {
            result.fully_expanded = false;
            add_diagnostic(
                result,
                DiagnosticSeverity::error,
                "gdspaces.container-tree.node-budget",
                "Expanding this container identity would exceed the resource-graph node budget.",
                payload.resource.id);
            active_byte_domains.erase(cache_key);
            return;
        }

        ++result.expanded_container_count;
        if (!expansion.usable()) {
            result.fully_expanded = false;
        }
        result.diagnostics.insert(
            result.diagnostics.end(),
            expansion.diagnostics.begin(),
            expansion.diagnostics.end());
        ContainerExpander::connect_graph(expansion, result.graph);

        std::vector<ResourcePayload> nested_payloads;
        nested_payloads.reserve(expansion.children.size());
        for (const auto& child : expansion.children) {
            if (!child.entry.populated ||
                !child.payload.readable() ||
                !child.payload.resource.container) {
                continue;
            }

            if (depth >= limits.max_nested_depth) {
                result.fully_expanded = false;
                add_diagnostic(
                    result,
                    DiagnosticSeverity::warning,
                    "gdspaces.container-tree.depth-budget",
                    "A nested container was preserved but not expanded because the configured depth limit was reached.",
                    child.payload.resource.id);
                continue;
            }
            nested_payloads.push_back(child.payload);
        }

        result.expansions.push_back(std::move(expansion));
        for (const auto& nested : nested_payloads) {
            visit(nested, depth + 1U, false);
        }
        active_byte_domains.erase(cache_key);
    };

    visit(root, 0U, true);
    return result;
}

} // namespace dmc::rengine::gdspaces
