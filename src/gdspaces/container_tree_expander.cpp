#include "dmc_rengine/gdspaces/container_tree_expander.hpp"

#include "dmc_rengine/core/sha256.hpp"

#include <algorithm>
#include <functional>
#include <limits>
#include <map>
#include <optional>
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

void append_text_field(std::ostringstream& output, std::string_view value) {
    output << value.size() << ':';
    output.write(value.data(), static_cast<std::streamsize>(value.size()));
    output << '|';
}

[[nodiscard]] std::optional<std::string> byte_identity_cache_key(
    const ResourcePayload& payload,
    const formats::IContainerParser& parser) {
    if (!parser.supports_byte_identity_reuse() ||
        !payload.byte_provenance.has_value() ||
        !payload.byte_provenance->valid()) {
        return std::nullopt;
    }

    const auto& provenance = *payload.byte_provenance;
    if (provenance.materialized_size !=
        static_cast<std::uint64_t>(payload.bytes.size())) {
        return std::nullopt;
    }

    // Provenance identifies the claimed byte domain. A content hash binds the
    // cache key to the bytes actually supplied to parse(), so malformed/manual
    // payload construction cannot reuse a parse merely by copying provenance.
    const auto content_hash = core::Sha256::hex(
        std::span<const std::byte>{payload.bytes});

    std::ostringstream output;
    append_text_field(output, parser.id());
    output << static_cast<unsigned>(provenance.kind) << '|';
    append_text_field(output, provenance.authority_id);
    output << provenance.offset << '|'
           << provenance.stored_size << '|'
           << provenance.materialized_size << '|'
           << static_cast<unsigned>(provenance.transform) << '|';
    if (provenance.crc32.has_value()) {
        output << *provenance.crc32;
    } else {
        output << '-';
    }
    output << '|';
    append_text_field(output, content_hash);
    return output.str();
}

[[nodiscard]] bool consume_parse_budget(
    ContainerTreeExpansion& result,
    const ContainerTreeExpansionLimits& limits,
    const ResourcePayload& payload) {
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
            "The recursive parsed-container byte budget would be exceeded by another parser execution.",
            payload.resource.id);
        return false;
    }

    result.parsed_container_bytes += byte_count;
    ++result.parser_execution_count;
    return true;
}

[[nodiscard]] std::size_t count_new_graph_nodes(
    const ContainerExpansion& expansion,
    const ResourceGraph& graph) {
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

    // Cache entries are parser results only. ResourceId/graph identities are
    // never deduplicated by this map.
    std::map<std::string, formats::ContainerParseResult, std::less<>> parse_cache;

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
                "No registered container parser recognizes this resource; its identity remains in the graph without nested expansion.",
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

        const formats::ContainerParseResult* parsed = nullptr;
        std::optional<formats::ContainerParseResult> uncached_parse;
        const auto cache_key = byte_identity_cache_key(payload, *parser);

        if (cache_key.has_value()) {
            const auto cached = parse_cache.find(*cache_key);
            if (cached != parse_cache.end()) {
                parsed = &cached->second;
                ++result.reused_parse_count;
            } else {
                if (!consume_parse_budget(result, limits, payload)) {
                    return;
                }
                auto parse_result = parser->parse(
                    std::span<const std::byte>{payload.bytes},
                    payload.resource.id.logical_path);
                const auto [inserted, ignored] = parse_cache.emplace(
                    *cache_key, std::move(parse_result));
                static_cast<void>(ignored);
                parsed = &inserted->second;
            }
        } else {
            if (!consume_parse_budget(result, limits, payload)) {
                return;
            }
            uncached_parse = parser->parse(
                std::span<const std::byte>{payload.bytes},
                payload.resource.id.logical_path);
            parsed = &*uncached_parse;
        }

        auto expansion = ContainerExpander::expand(payload, *parsed);
        const auto new_nodes = count_new_graph_nodes(expansion, result.graph);
        if (result.graph.resource_count() > limits.max_graph_nodes ||
            new_nodes > limits.max_graph_nodes - result.graph.resource_count()) {
            result.fully_expanded = false;
            add_diagnostic(
                result,
                DiagnosticSeverity::error,
                "gdspaces.container-tree.node-budget",
                "Expanding this container identity would exceed the resource-graph node budget.",
                payload.resource.id);
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
                    "A nested container identity was preserved but not expanded because the configured depth limit was reached.",
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
