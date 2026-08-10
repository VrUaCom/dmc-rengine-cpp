#include "dmc_rengine/gdspaces/container_tree_expander.hpp"

#include <algorithm>
#include <functional>
#include <limits>
#include <set>
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

[[nodiscard]] std::string physical_range_key(const ResourceId& id) {
    return id.source_id + "|" + std::to_string(id.offset) + "|" +
        std::to_string(id.size);
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
    std::set<std::string, std::less<>> parsed_physical_ranges;

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

        const auto physical_key = physical_range_key(payload.resource.id);
        if (!parsed_physical_ranges.insert(physical_key).second) {
            result.fully_expanded = false;
            add_diagnostic(
                result,
                DiagnosticSeverity::warning,
                "gdspaces.container-tree.repeated-physical-range",
                "A container physical byte range was reached through another identity and was not reparsed recursively.",
                payload.resource.id);
            return;
        }

        if (result.expanded_container_count >= limits.max_expanded_containers) {
            result.fully_expanded = false;
            add_diagnostic(
                result,
                DiagnosticSeverity::error,
                "gdspaces.container-tree.container-budget",
                "The maximum number of recursively expanded containers was reached.",
                payload.resource.id);
            return;
        }

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
            return;
        }

        const auto parsed = registry.parse(
            std::span<const std::byte>{payload.bytes},
            payload.resource.id.logical_path);
        auto expansion = ContainerExpander::expand(payload, parsed);

        if (result.graph.resource_count() > limits.max_graph_nodes ||
            expansion.children.size() >
                limits.max_graph_nodes - result.graph.resource_count()) {
            result.fully_expanded = false;
            add_diagnostic(
                result,
                DiagnosticSeverity::error,
                "gdspaces.container-tree.node-budget",
                "Expanding this container would exceed the resource-graph node budget.",
                payload.resource.id);
            return;
        }

        ++result.expanded_container_count;
        result.parsed_container_bytes += byte_count;
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
    };

    visit(root, 0U, true);
    return result;
}

} // namespace dmc::rengine::gdspaces
