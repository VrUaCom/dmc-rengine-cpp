#include "dmc_rengine/integration/project_workspace.hpp"
#include "dmc_rengine/profiles/dmc3/recovered_source_tree.hpp"

#include <iomanip>
#include <sstream>
#include <utility>

namespace dmc::rengine::integration {
namespace {

[[nodiscard]] std::string hex_u64(std::uint64_t value) {
    std::ostringstream output;
    output << "0x" << std::uppercase << std::hex << value;
    return output.str();
}

[[nodiscard]] std::string evidence_pass_list(
    const std::vector<std::uint32_t>& passes) {
    std::ostringstream output;
    for (std::size_t index = 0; index < passes.size(); ++index) {
        if (index != 0U) {
            output << ',';
        }
        output << passes[index];
    }
    return output.str();
}

[[nodiscard]] std::string recovered_node_id(std::string_view symbol_id) {
    return "source-symbol:" + std::string{symbol_id};
}

[[nodiscard]] bool ingest_dmc3_recovered_tree(
    ProjectGraph& graph,
    std::string_view analysis_node_id) {
    using profiles::dmc3::recovered_source_tree;
    using profiles::dmc3::to_string;

    for (const auto& symbol : recovered_source_tree()) {
        auto attributes = symbol.attributes;
        attributes.insert_or_assign("recovered_kind", std::string{to_string(symbol.kind)});
        attributes.insert_or_assign("status", std::string{to_string(symbol.status)});
        attributes.insert_or_assign("summary", symbol.summary);
        attributes.insert_or_assign(
            "evidence_passes", evidence_pass_list(symbol.evidence_passes));
        if (symbol.va.has_value()) {
            attributes.insert_or_assign("va", hex_u64(*symbol.va));
        }
        if (symbol.size.has_value()) {
            attributes.insert_or_assign("size", hex_u64(*symbol.size));
        }

        const auto node_id = recovered_node_id(symbol.id);
        if (!graph.upsert(ProjectNode{
                .id = node_id,
                .kind = ProjectNodeKind::source_symbol,
                .label = symbol.name,
                .attributes = std::move(attributes),
            })) {
            return false;
        }

        const auto parent_node_id = symbol.parent_id.empty()
            ? std::string{analysis_node_id}
            : recovered_node_id(symbol.parent_id);
        static_cast<void>(graph.connect(ProjectEdge{
            .from = parent_node_id,
            .to = node_id,
            .kind = ProjectEdgeKind::contains,
            .label = symbol.parent_id.empty()
                ? "Recovered Game Source Tree"
                : "Recovered source ownership",
        }));
        static_cast<void>(graph.connect(ProjectEdge{
            .from = node_id,
            .to = "tool:exe-editor",
            .kind = ProjectEdgeKind::produced_by,
            .label = "EXE Editor recovered source",
        }));

        if (!symbol.va.has_value()) {
            continue;
        }
        const auto mapping_id = "source-binary-mapping:" + symbol.id;
        if (!graph.upsert(ProjectNode{
                .id = mapping_id,
                .kind = ProjectNodeKind::source_binary_mapping,
                .label = symbol.name + " binary mapping",
                .attributes = {
                    {"source_symbol_id", symbol.id},
                    {"va", hex_u64(*symbol.va)},
                    {"mapping_status", std::string{to_string(symbol.status)}},
                },
            })) {
            return false;
        }
        static_cast<void>(graph.connect(ProjectEdge{
            .from = node_id,
            .to = mapping_id,
            .kind = ProjectEdgeKind::maps_source_to_binary,
            .label = "Recovered VA mapping",
        }));
        static_cast<void>(graph.connect(ProjectEdge{
            .from = mapping_id,
            .to = std::string{analysis_node_id},
            .kind = ProjectEdgeKind::based_on,
            .label = "Canonical executable analysis",
        }));
    }
    return true;
}

} // namespace

bool ProjectWorkspace::attach_executable_context(
    const gdspaces::ResourceId& resource,
    ExecutableResourceContext context) {
    auto* session = find_session_mutable(resource);
    if (session == nullptr ||
        !session->attach_executable_context(std::move(context)) ||
        !sync(*session)) {
        return false;
    }

    const auto* executable = session->executable_context();
    if (executable == nullptr) {
        return false;
    }

    const auto resource_node_id = "resource:" + resource.canonical();
    const auto analysis_node_id = "executable-analysis:" + resource.canonical();
    if (!graph_.upsert(ProjectNode{
            .id = analysis_node_id,
            .kind = ProjectNodeKind::executable_analysis,
            .label = session->resource().display_name + " PE analysis",
            .attributes = {
                {"sha256", executable->sha256},
                {"pe_kind", std::string(exe::to_string(executable->image.kind))},
                {"machine", std::string(exe::to_string(executable->image.machine))},
                {"image_base", std::to_string(executable->image.image_base)},
                {"entry_point_rva", std::to_string(executable->image.entry_point_rva)},
                {"section_count", std::to_string(executable->image.section_count)},
                {"known_target_id", executable->known_target_id},
                {"known_target_name", executable->known_target_name},
                {"known_target_hash_match",
                 executable->known_target_hash_match ? "true" : "false"},
                {"known_target_metadata_match",
                 executable->known_target_metadata_match ? "true" : "false"},
            },
        })) {
        return false;
    }

    static_cast<void>(graph_.connect(ProjectEdge{
        .from = analysis_node_id,
        .to = resource_node_id,
        .kind = ProjectEdgeKind::derived_from,
        .label = "PE metadata and target identification",
    }));
    static_cast<void>(graph_.connect(ProjectEdge{
        .from = analysis_node_id,
        .to = "tool:exe-editor",
        .kind = ProjectEdgeKind::produced_by,
        .label = "EXE Editor analysis",
    }));

    for (const auto* artifact : artifacts_.by_sha256(executable->sha256)) {
        if (artifact == nullptr) {
            continue;
        }
        static_cast<void>(graph_.connect(ProjectEdge{
            .from = analysis_node_id,
            .to = "artifact:" + artifact->id,
            .kind = ProjectEdgeKind::references_artifact,
            .label = "SHA-256 identity match",
        }));
    }

    if (executable->known_target_hash_match &&
        executable->known_target_id == "dmc3-hdc-phase12-canonical-target" &&
        !ingest_dmc3_recovered_tree(graph_, analysis_node_id)) {
        return false;
    }
    return true;
}

} // namespace dmc::rengine::integration
