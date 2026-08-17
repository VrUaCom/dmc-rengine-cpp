#include "dmc_rengine/gdspaces/container_tree_expander.hpp"

#include "dmc_rengine/core/sha256.hpp"
#include "dmc_rengine/gdspaces/byte_provenance.hpp"

#include <algorithm>
#include <functional>
#include <limits>
#include <map>
#include <optional>
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

[[nodiscard]] std::optional<std::string> reusable_parse_cache_key(
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

    // Lineage identifies the claimed byte domain. Bind reuse additionally to
    // the exact materialized bytes actually supplied to parse(), so copied or
    // stale provenance metadata cannot borrow another payload's parse result.
    const auto content_hash = core::Sha256::compute(
        std::span<const std::byte>{payload.bytes}).hex();

    std::ostringstream output;
    append_string_field(output, parser.id());
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
    output << '|';
    append_string_field(output, content_hash);
    return output.str();
}

[[nodiscard]] std::string active_domain_key(
    const ResourcePayload& payload,
    const formats::IContainerParser& parser,
    const std::optional<std::string>& cache_key) {
    if (cache_key.has_value()) {
        return std::string{"byte|"} + *cache_key;
    }

    std::ostringstream output;
    output << "identity|";
    append_string_field(output, parser.id());
    append_string_field(output, payload.resource.id.canonical());
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

[[nodiscard]] bool consume_parser_budget(
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
            "The recursive parsed-container byte budget would be exceeded.",
            payload.resource.id);
        return false;
    }

    ++result.parser_invocation_count;
    result.parsed_container_bytes += byte_count;
    return true;
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

    // This cache stores parser results only. It never deduplicates ResourceId
    // or graph identity, and it is available only to parsers that explicitly
    // declare byte-only semantics.
    std::map<std::string, formats::ContainerParseResult, std::less<>> parse_cache;
    std::set<std::string, std::less<>> active_domains;

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

        const auto cache_key = reusable_parse_cache_key(payload, *parser);
        const auto ancestry_key = active_domain_key(payload, *parser, cache_key);
        if (!active_domains.insert(ancestry_key).second) {
            result.fully_expanded = false;
            add_diagnostic(
                result,
                DiagnosticSeverity::warning,
                "gdspaces.container-tree.byte-domain-cycle",
                "The same parser/byte-or-identity domain is already active in this ancestry; recursive expansion stopped to prevent a cycle.",
                payload.resource.id);
            return;
        }

        const auto release_active = [&]() {
            active_domains.erase(ancestry_key);
        };

        const formats::ContainerParseResult* parsed = nullptr;
        std::optional<formats::ContainerParseResult> uncached_parse;

        if (cache_key.has_value()) {
            const auto cached = parse_cache.find(*cache_key);
            if (cached != parse_cache.end()) {
                ++result.parse_cache_hits;
                parsed = &cached->second;
            } else {
                if (!consume_parser_budget(result, limits, payload)) {
                    release_active();
                    return;
                }
                auto parse_result = parser->parse(
                    std::span<const std::byte>{payload.bytes},
                    payload.resource.id.logical_path);
                auto [iterator, inserted] = parse_cache.emplace(
                    *cache_key, std::move(parse_result));
                static_cast<void>(inserted);
                parsed = &iterator->second;
            }
        } else {
            if (!consume_parser_budget(result, limits, payload)) {
                release_active();
                return;
            }
            uncached_parse = parser->parse(
                std::span<const std::byte>{payload.bytes},
                payload.resource.id.logical_path);
            parsed = &*uncached_parse;
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
            release_active();
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
        release_active();
    };

    visit(root, 0U, true);
    return result;
}

} // namespace dmc::rengine::gdspaces
