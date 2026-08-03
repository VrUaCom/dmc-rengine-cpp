#include "dmc_rengine/integration/project_workspace.hpp"

#include <algorithm>
#include <set>
#include <string>
#include <utility>

namespace dmc::rengine::integration {
namespace {

[[nodiscard]] std::string build_node_id(std::string_view id) {
    return "custom-build:" + std::string(id);
}
[[nodiscard]] std::string modification_node_id(std::string_view id, std::string_view version) {
    return "source-modification:" + std::string(id) + '@' + std::string(version);
}
[[nodiscard]] std::string unit_node_id(std::string_view modification_id, std::string_view version, std::string_view unit_id) {
    return modification_node_id(modification_id, version) + ":unit:" + std::string(unit_id);
}
[[nodiscard]] const source::SourceChangeUnit* find_unit(const source::SourceModificationPackage& package, std::string_view unit_id) {
    const auto it = std::find_if(package.change_units.begin(), package.change_units.end(), [unit_id](const source::SourceChangeUnit& u) { return u.id == unit_id; });
    return it == package.change_units.end() ? nullptr : &*it;
}
[[nodiscard]] const source::SourceSymbolRef* find_symbol(const source::SourceChangeUnit& unit, std::string_view symbol_id) {
    const auto it = std::find_if(unit.affected_symbols.begin(), unit.affected_symbols.end(), [symbol_id](const source::SourceSymbolRef& s) { return s.id == symbol_id; });
    return it == unit.affected_symbols.end() ? nullptr : &*it;
}
[[nodiscard]] bool contains_id(const std::vector<std::string>& values, std::string_view id) {
    return std::find(values.begin(), values.end(), id) != values.end();
}
[[nodiscard]] std::string ensure_tool(ProjectGraph& graph, const ToolRegistry& tools, gdspaces::ToolTarget target) {
    const auto id = "tool:" + std::string(gdspaces::to_string(target));
    if (graph.find(id) == nullptr) {
        const auto* descriptor = tools.find(target);
        static_cast<void>(graph.upsert(ProjectNode{
            .id = id,
            .kind = ProjectNodeKind::tool,
            .label = descriptor == nullptr ? std::string(gdspaces::to_string(target)) : descriptor->product_name,
            .attributes = {{"target", std::string(gdspaces::to_string(target))}},
        }));
    }
    return id;
}

} // namespace

bool ProjectWorkspace::register_custom_build_record(source::CustomBuildRecord record) {
    if (!record.valid() || custom_builds_.find(record.identity.custom_build_id) != custom_builds_.end() ||
        custom_build_ids_by_sha256_.find(record.identity.executable_sha256) != custom_build_ids_by_sha256_.end()) return false;

    const auto* project = find_source_integration_project(record.identity.integration_project_id);
    if (project == nullptr || !source::custom_build_status_matches_project_state(record.status, project->state())) return false;
    const auto& baseline = project->baseline();
    if (record.identity.game_profile != baseline.game_profile ||
        record.identity.original_exe_sha256 != baseline.original_exe_sha256 ||
        record.identity.source_schema_version != baseline.source_schema_version ||
        record.identity.source_revision != baseline.source_revision) return false;
    if (artifacts_.by_sha256(record.identity.original_exe_sha256).empty() ||
        !artifacts_.by_sha256(record.identity.executable_sha256).empty()) return false;

    if (record.included_modifications.size() != project->selected_modifications().size()) return false;
    for (const auto& selected : project->selected_modifications()) {
        const auto* included = record.included_modification(selected.id);
        const auto* registered = find_source_modification(selected.id, selected.version);
        if (included == nullptr || included->version != selected.version || registered == nullptr) return false;
    }

    std::set<std::string, std::less<>> project_decisions;
    for (const auto& decision : project->decisions()) project_decisions.insert(decision.id);
    if (record.unified_implementation_decision_ids.size() != project_decisions.size()) return false;
    for (const auto& id : record.unified_implementation_decision_ids)
        if (project_decisions.find(id) == project_decisions.end()) return false;

    for (const auto& mapping : record.source_binary_mappings) {
        const auto* package = find_source_modification(mapping.modification_id, mapping.modification_version);
        if (package == nullptr) return false;
        const auto* unit = find_unit(*package, mapping.source_unit_id);
        if (unit == nullptr || unit->source_path != mapping.source_path) return false;
        const auto* symbol = find_symbol(*unit, mapping.recovered_symbol_id);
        if (symbol == nullptr || symbol->recovered_name != mapping.recovered_symbol_name) return false;
        for (const auto& evidence_id : mapping.evidence_record_ids) {
            if (evidence_.find(evidence_id) == nullptr ||
                (!contains_id(package->evidence_record_ids, evidence_id) &&
                 !contains_id(symbol->evidence_record_ids, evidence_id))) return false;
        }
    }

    for (const auto& selected : project->selected_modifications()) {
        for (const auto& requirement : selected.tests) {
            if (!requirement.mandatory) continue;
            const auto result = std::find_if(record.test_results.begin(), record.test_results.end(), [&requirement](const source::BuildTestResult& candidate) {
                return candidate.id == requirement.id && candidate.layer == requirement.layer &&
                       candidate.profile == requirement.profile && candidate.mandatory && candidate.passed;
            });
            if (result == record.test_results.end()) return false;
        }
    }
    for (const auto& result : record.test_results)
        for (const auto& evidence_id : result.evidence_record_ids)
            if (evidence_.find(evidence_id) == nullptr) return false;

    if (!record.identity.prior_custom_build_id.empty()) {
        const auto* prior = find_custom_build(record.identity.prior_custom_build_id);
        if (prior == nullptr || prior->identity.game_profile != record.identity.game_profile) return false;
    }

    const auto custom_artifact_id = "custom-executable:" + record.identity.custom_build_id;
    if (!artifacts_.add(evidence::ArtifactIdentity{
            .id = custom_artifact_id,
            .role = "custom-recompiled-game-executable",
            .sha256 = record.identity.executable_sha256,
            .size = record.executable_size,
        })) return false;
    static_cast<void>(graph_.upsert(ProjectNode{
        .id = "artifact:" + custom_artifact_id,
        .kind = ProjectNodeKind::artifact,
        .label = custom_artifact_id,
        .attributes = {{"role", "custom-recompiled-game-executable"}, {"sha256", record.identity.executable_sha256}, {"size", std::to_string(record.executable_size)}},
    }));

    const auto node_id = build_node_id(record.identity.custom_build_id);
    if (!graph_.upsert(ProjectNode{
            .id = node_id,
            .kind = ProjectNodeKind::custom_build,
            .label = record.identity.custom_build_id,
            .attributes = {
                {"semantic_version", record.identity.semantic_version},
                {"edition_id", record.identity.edition_id},
                {"status", std::string(source::to_string(record.status))},
                {"integration_project_id", record.identity.integration_project_id},
                {"source_tree_sha256", record.identity.source_tree_sha256},
                {"executable_sha256", record.identity.executable_sha256},
                {"release_channel", record.identity.release_channel},
                {"distribution_form", std::string(source::to_string(record.distribution_form))},
                {"attestation_state", std::string(source::to_string(record.attestation_state))},
            },
        })) return false;
    static_cast<void>(graph_.connect(ProjectEdge{.from=node_id,.to="source-integration-project:"+record.identity.integration_project_id,.kind=ProjectEdgeKind::built_from,.label="exact composite source build"}));
    static_cast<void>(graph_.connect(ProjectEdge{.from=node_id,.to="artifact:"+custom_artifact_id,.kind=ProjectEdgeKind::produces,.label="custom recompiled executable"}));
    static_cast<void>(graph_.connect(ProjectEdge{.from=ensure_tool(graph_,tools_,gdspaces::ToolTarget::exe_editor),.to=node_id,.kind=ProjectEdgeKind::recognizes,.label="reopen by executable SHA-256"}));
    static_cast<void>(graph_.connect(ProjectEdge{.from=node_id,.to=ensure_tool(graph_,tools_,gdspaces::ToolTarget::build_test_lab),.kind=ProjectEdgeKind::validated_by,.label="build record and test matrix"}));

    for (const auto& included : record.included_modifications)
        static_cast<void>(graph_.connect(ProjectEdge{.from=node_id,.to=modification_node_id(included.modification_id,included.version),.kind=ProjectEdgeKind::includes,.label=included.package_manifest_sha256}));
    for (const auto& decision_id : record.unified_implementation_decision_ids)
        static_cast<void>(graph_.connect(ProjectEdge{.from=node_id,.to="source-decision:"+decision_id,.kind=ProjectEdgeKind::includes,.label="unified implementation decision"}));
    if (!record.identity.prior_custom_build_id.empty())
        static_cast<void>(graph_.connect(ProjectEdge{.from=node_id,.to=build_node_id(record.identity.prior_custom_build_id),.kind=ProjectEdgeKind::lineage_from,.label="prior custom build"}));

    for (const auto& mapping : record.source_binary_mappings) {
        const auto mapping_node = "source-binary-mapping:" + record.identity.custom_build_id + ':' + mapping.id;
        static_cast<void>(graph_.upsert(ProjectNode{
            .id=mapping_node,.kind=ProjectNodeKind::source_binary_mapping,.label=mapping.recovered_symbol_name,
            .attributes={{"mapping_id",mapping.id},{"source_path",mapping.source_path},{"source_line_begin",std::to_string(mapping.source_line_begin)},{"source_line_end",std::to_string(mapping.source_line_end)},{"output_file_offset",std::to_string(mapping.output_file_offset)},{"output_rva",std::to_string(mapping.output_rva)},{"output_va",std::to_string(mapping.output_va)},{"byte_size",std::to_string(mapping.byte_size)},{"confidence",std::string(source::to_string(mapping.confidence))}},
        }));
        static_cast<void>(graph_.connect(ProjectEdge{.from=node_id,.to=mapping_node,.kind=ProjectEdgeKind::contains,.label="source-binary mapping"}));
        static_cast<void>(graph_.connect(ProjectEdge{.from=mapping_node,.to=unit_node_id(mapping.modification_id,mapping.modification_version,mapping.source_unit_id),.kind=ProjectEdgeKind::maps_source_to_binary,.label=mapping.source_path}));
        static_cast<void>(graph_.connect(ProjectEdge{.from=mapping_node,.to="source-symbol:"+mapping.recovered_symbol_id,.kind=ProjectEdgeKind::affects,.label="output RVA mapping"}));
        static_cast<void>(graph_.connect(ProjectEdge{.from=mapping_node,.to="artifact:"+custom_artifact_id,.kind=ProjectEdgeKind::targets,.label="output executable byte range"}));
        for (const auto& evidence_id : mapping.evidence_record_ids)
            static_cast<void>(graph_.connect(ProjectEdge{.from="evidence:"+evidence_id,.to=mapping_node,.kind=ProjectEdgeKind::evidence_for,.label="mapping evidence"}));
    }
    for (const auto& result : record.test_results) {
        const auto test_node = "build-test-result:" + record.identity.custom_build_id + ':' + result.id;
        static_cast<void>(graph_.upsert(ProjectNode{.id=test_node,.kind=ProjectNodeKind::build_test_result,.label=result.id,.attributes={{"layer",std::string(source::to_string(result.layer))},{"profile",result.profile},{"mandatory",result.mandatory?"true":"false"},{"passed",result.passed?"true":"false"},{"summary",result.summary},{"report_sha256",result.report_sha256}}}));
        static_cast<void>(graph_.connect(ProjectEdge{.from=node_id,.to=test_node,.kind=ProjectEdgeKind::tested_by,.label=result.profile}));
        for (const auto& evidence_id : result.evidence_record_ids)
            static_cast<void>(graph_.connect(ProjectEdge{.from="evidence:"+evidence_id,.to=test_node,.kind=ProjectEdgeKind::evidence_for,.label="test result evidence"}));
    }

    custom_build_ids_by_sha256_.emplace(record.identity.executable_sha256, record.identity.custom_build_id);
    custom_builds_.emplace(record.identity.custom_build_id, std::move(record));
    return true;
}

const source::CustomBuildRecord* ProjectWorkspace::find_custom_build(std::string_view id) const noexcept {
    const auto it = custom_builds_.find(id);
    return it == custom_builds_.end() ? nullptr : &it->second;
}
const source::CustomBuildRecord* ProjectWorkspace::find_custom_build_by_sha256(std::string_view sha) const noexcept {
    const auto id = custom_build_ids_by_sha256_.find(sha);
    return id == custom_build_ids_by_sha256_.end() ? nullptr : find_custom_build(id->second);
}
const source::SourceBinaryMapping* ProjectWorkspace::find_custom_build_mapping(std::string_view id, std::uint64_t rva) const noexcept {
    const auto* build = find_custom_build(id);
    return build == nullptr ? nullptr : build->mapping_for_rva(rva);
}
std::size_t ProjectWorkspace::custom_build_count() const noexcept { return custom_builds_.size(); }

} // namespace dmc::rengine::integration
