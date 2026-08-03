#include "dmc_rengine/integration/project_workspace.hpp"

#include <algorithm>
#include <sstream>
#include <string>
#include <utility>

namespace dmc::rengine::integration {
namespace {

[[nodiscard]] std::string modification_key(
    std::string_view modification_id,
    std::string_view version) {
    return std::string(modification_id) + '@' + std::string(version);
}

[[nodiscard]] std::string modification_node_id(
    std::string_view modification_id,
    std::string_view version) {
    return "source-modification:" + modification_key(
        modification_id, version);
}

[[nodiscard]] std::string baseline_node_id(
    const source::SourceBaselineIdentity& baseline) {
    return "source-baseline:" + baseline.original_exe_sha256 + ':' +
           baseline.source_schema_version + ':' + baseline.source_revision;
}

[[nodiscard]] std::string integration_project_node_id(
    std::string_view project_id) {
    return "source-integration-project:" + std::string(project_id);
}

[[nodiscard]] std::string join(
    const std::vector<std::string>& values) {
    std::ostringstream output;
    for (std::size_t index = 0U; index < values.size(); ++index) {
        if (index != 0U) {
            output << ',';
        }
        output << values[index];
    }
    return output.str();
}

[[nodiscard]] std::string bool_text(bool value) {
    return value ? "true" : "false";
}

[[nodiscard]] bool package_revision_matches(
    const source::SourceModificationPackage& left,
    const source::SourceModificationPackage& right) {
    if (left.id != right.id || left.version != right.version ||
        left.baseline != right.baseline ||
        left.implementation_kind != right.implementation_kind ||
        left.change_units.size() != right.change_units.size()) {
        return false;
    }
    for (std::size_t index = 0U; index < left.change_units.size(); ++index) {
        const auto& left_unit = left.change_units[index];
        const auto& right_unit = right.change_units[index];
        if (left_unit.id != right_unit.id ||
            left_unit.source_path != right_unit.source_path ||
            left_unit.baseline_sha256 != right_unit.baseline_sha256 ||
            left_unit.result_sha256 != right_unit.result_sha256) {
            return false;
        }
    }
    return true;
}

[[nodiscard]] bool evidence_loaded(
    const evidence::EvidenceRegistry& registry,
    const std::vector<std::string>& record_ids) {
    return std::all_of(
        record_ids.begin(), record_ids.end(),
        [&registry](const std::string& record_id) {
            return registry.find(record_id) != nullptr;
        });
}

[[nodiscard]] std::string ensure_tool_node(
    ProjectGraph& graph,
    const ToolRegistry& tools,
    gdspaces::ToolTarget target) {
    const auto id = "tool:" + std::string(gdspaces::to_string(target));
    if (graph.find(id) != nullptr) {
        return id;
    }
    const auto* descriptor = tools.find(target);
    static_cast<void>(graph.upsert(ProjectNode{
        .id = id,
        .kind = ProjectNodeKind::tool,
        .label = descriptor == nullptr
            ? std::string(gdspaces::to_string(target))
            : descriptor->product_name,
        .attributes = {
            {"target", std::string(gdspaces::to_string(target))},
        },
    }));
    return id;
}

[[nodiscard]] const source::SourceModificationPackage*
find_selected_modification(
    const source::IntegrationProject& project,
    std::string_view modification_id) {
    const auto iterator = std::find_if(
        project.selected_modifications().begin(),
        project.selected_modifications().end(),
        [modification_id](const source::SourceModificationPackage& package) {
            return package.id == modification_id;
        });
    return iterator == project.selected_modifications().end()
        ? nullptr
        : &*iterator;
}

} // namespace

bool ProjectWorkspace::register_source_modification(
    source::SourceModificationPackage package) {
    if (!package.valid()) {
        return false;
    }
    const auto key = modification_key(package.id, package.version);
    if (source_modifications_.find(key) != source_modifications_.end() ||
        artifacts_.by_sha256(package.baseline.original_exe_sha256).empty() ||
        !evidence_loaded(evidence_, package.evidence_record_ids)) {
        return false;
    }
    for (const auto& unit : package.change_units) {
        for (const auto& symbol : unit.affected_symbols) {
            if (!evidence_loaded(evidence_, symbol.evidence_record_ids)) {
                return false;
            }
            const auto symbol_node_id = "source-symbol:" + symbol.id;
            if (const auto* existing = graph_.find(symbol_node_id);
                existing != nullptr) {
                const auto kind = existing->attributes.find("kind");
                const auto name = existing->attributes.find("recovered_name");
                if (existing->kind != ProjectNodeKind::source_symbol ||
                    kind == existing->attributes.end() ||
                    name == existing->attributes.end() ||
                    kind->second != source::to_string(symbol.kind) ||
                    name->second != symbol.recovered_name) {
                    return false;
                }
            }
        }
    }

    const auto baseline_id = baseline_node_id(package.baseline);
    if (!graph_.upsert(ProjectNode{
            .id = baseline_id,
            .kind = ProjectNodeKind::source_baseline,
            .label = package.baseline.game_profile + " / " +
                package.baseline.source_revision,
            .attributes = {
                {"game_profile", package.baseline.game_profile},
                {"original_exe_sha256", package.baseline.original_exe_sha256},
                {"source_schema_version", package.baseline.source_schema_version},
                {"source_revision", package.baseline.source_revision},
                {"compiler_profile", package.baseline.compiler_profile},
                {"prior_custom_build_id", package.baseline.prior_custom_build_id},
            },
        })) {
        return false;
    }
    for (const auto* artifact : artifacts_.by_sha256(
             package.baseline.original_exe_sha256)) {
        static_cast<void>(graph_.connect(ProjectEdge{
            .from = baseline_id,
            .to = "artifact:" + artifact->id,
            .kind = ProjectEdgeKind::references_artifact,
            .label = "exact original executable fingerprint",
        }));
    }

    const auto modification_id = modification_node_id(
        package.id, package.version);
    if (!graph_.upsert(ProjectNode{
            .id = modification_id,
            .kind = ProjectNodeKind::source_modification,
            .label = package.title,
            .attributes = {
                {"modification_id", package.id},
                {"version", package.version},
                {"implementation_kind", std::string(source::to_string(
                    package.implementation_kind))},
                {"semantic_intent", package.semantic_intent},
                {"save_compatibility", package.save_compatibility},
                {"migration_notes", package.migration_notes},
                {"rollback_instructions", package.rollback_instructions},
                {"authors", join(package.provenance.authors)},
                {"licence", package.provenance.licence},
                {"redistribution_permission",
                    package.provenance.redistribution_permission},
                {"ai_assistance_disclosure",
                    package.provenance.ai_assistance_disclosure},
                {"change_unit_count", std::to_string(
                    package.change_units.size())},
                {"test_count", std::to_string(package.tests.size())},
                {"risk_count", std::to_string(package.known_risks.size())},
            },
        })) {
        return false;
    }
    static_cast<void>(graph_.connect(ProjectEdge{
        .from = modification_id,
        .to = baseline_id,
        .kind = ProjectEdgeKind::based_on,
        .label = "exact reconstructed source baseline",
    }));
    static_cast<void>(graph_.connect(ProjectEdge{
        .from = modification_id,
        .to = ensure_tool_node(
            graph_, tools_, gdspaces::ToolTarget::exe_editor),
        .kind = ProjectEdgeKind::produced_by,
        .label = "EXE Editor source modification package",
    }));
    static_cast<void>(graph_.connect(ProjectEdge{
        .from = modification_id,
        .to = ensure_tool_node(
            graph_, tools_, gdspaces::ToolTarget::build_test_lab),
        .kind = ProjectEdgeKind::delivered_to,
        .label = "mandatory build and test validation",
    }));

    for (const auto& evidence_id : package.evidence_record_ids) {
        static_cast<void>(graph_.connect(ProjectEdge{
            .from = "evidence:" + evidence_id,
            .to = modification_id,
            .kind = ProjectEdgeKind::evidence_for,
            .label = "source modification evidence",
        }));
    }

    for (const auto& unit : package.change_units) {
        const auto unit_node_id = modification_id + ":unit:" + unit.id;
        static_cast<void>(graph_.upsert(ProjectNode{
            .id = unit_node_id,
            .kind = ProjectNodeKind::source_change_unit,
            .label = unit.source_path,
            .attributes = {
                {"unit_id", unit.id},
                {"source_path", unit.source_path},
                {"baseline_sha256", unit.baseline_sha256},
                {"result_sha256", unit.result_sha256},
                {"semantic_intent", unit.semantic_intent},
            },
        }));
        static_cast<void>(graph_.connect(ProjectEdge{
            .from = modification_id,
            .to = unit_node_id,
            .kind = ProjectEdgeKind::changes,
            .label = unit.source_path,
        }));

        for (const auto& symbol : unit.affected_symbols) {
            const auto symbol_node_id = "source-symbol:" + symbol.id;
            static_cast<void>(graph_.upsert(ProjectNode{
                .id = symbol_node_id,
                .kind = ProjectNodeKind::source_symbol,
                .label = symbol.recovered_name,
                .attributes = {
                    {"symbol_id", symbol.id},
                    {"kind", std::string(source::to_string(symbol.kind))},
                    {"recovered_name", symbol.recovered_name},
                    {"rva", symbol.rva.has_value()
                        ? std::to_string(*symbol.rva)
                        : std::string{}},
                },
            }));
            static_cast<void>(graph_.connect(ProjectEdge{
                .from = unit_node_id,
                .to = symbol_node_id,
                .kind = ProjectEdgeKind::affects,
                .label = unit.semantic_intent,
            }));
            for (const auto& evidence_id : symbol.evidence_record_ids) {
                static_cast<void>(graph_.connect(ProjectEdge{
                    .from = "evidence:" + evidence_id,
                    .to = symbol_node_id,
                    .kind = ProjectEdgeKind::evidence_for,
                    .label = "recovered symbol evidence",
                }));
            }
        }
    }

    source_modifications_.emplace(key, std::move(package));
    return true;
}

bool ProjectWorkspace::register_source_integration_project(
    source::IntegrationProject project) {
    if (!project.valid() ||
        source_integration_projects_.find(project.id()) !=
            source_integration_projects_.end() ||
        artifacts_.by_sha256(
            project.baseline().original_exe_sha256).empty()) {
        return false;
    }
    for (const auto& selected : project.selected_modifications()) {
        const auto* registered = find_source_modification(
            selected.id, selected.version);
        if (registered == nullptr ||
            !package_revision_matches(*registered, selected)) {
            return false;
        }
    }

    const auto project_node_id = integration_project_node_id(project.id());
    const auto baseline_id = baseline_node_id(project.baseline());
    if (graph_.find(baseline_id) == nullptr ||
        !graph_.upsert(ProjectNode{
            .id = project_node_id,
            .kind = ProjectNodeKind::integration_project,
            .label = project.title(),
            .attributes = {
                {"project_id", project.id()},
                {"state", std::string(source::to_string(project.state()))},
                {"maintainers", join(project.maintainers())},
                {"approval_policy", project.approval_policy()},
                {"build_profiles", join(project.build_profiles())},
                {"test_matrix", join(project.test_matrix())},
                {"selected_modification_count", std::to_string(
                    project.selected_modifications().size())},
                {"unresolved_blocking_conflicts", std::to_string(
                    project.unresolved_blocking_conflict_count())},
                {"failure_reason", project.failure_reason()},
            },
        })) {
        return false;
    }
    static_cast<void>(graph_.connect(ProjectEdge{
        .from = project_node_id,
        .to = baseline_id,
        .kind = ProjectEdgeKind::based_on,
        .label = "composite source build baseline",
    }));
    static_cast<void>(graph_.connect(ProjectEdge{
        .from = project_node_id,
        .to = ensure_tool_node(
            graph_, tools_, gdspaces::ToolTarget::exe_editor),
        .kind = ProjectEdgeKind::governed_by,
        .label = "source reconstruction and integration environment",
    }));
    static_cast<void>(graph_.connect(ProjectEdge{
        .from = project_node_id,
        .to = ensure_tool_node(
            graph_, tools_, gdspaces::ToolTarget::build_test_lab),
        .kind = ProjectEdgeKind::validated_by,
        .label = "composite build gates",
    }));

    for (const auto& selected : project.selected_modifications()) {
        const auto selected_node = modification_node_id(
            selected.id, selected.version);
        static_cast<void>(graph_.connect(ProjectEdge{
            .from = project_node_id,
            .to = selected_node,
            .kind = ProjectEdgeKind::selects,
            .label = selected.version,
        }));
        for (const auto& dependency : selected.dependencies) {
            const auto* dependency_package = find_selected_modification(
                project, dependency.modification_id);
            if (dependency_package == nullptr) {
                continue;
            }
            static_cast<void>(graph_.connect(ProjectEdge{
                .from = selected_node,
                .to = modification_node_id(
                    dependency_package->id,
                    dependency_package->version),
                .kind = ProjectEdgeKind::depends_on,
                .label = dependency.version_constraint,
            }));
        }
    }

    for (const auto& conflict : project.conflicts()) {
        const auto conflict_node_id = "source-integration-conflict:" +
            conflict.id;
        static_cast<void>(graph_.upsert(ProjectNode{
            .id = conflict_node_id,
            .kind = ProjectNodeKind::integration_conflict,
            .label = conflict.subject,
            .attributes = {
                {"conflict_id", conflict.id},
                {"class", std::string(source::to_string(
                    conflict.conflict_class))},
                {"origin", std::string(source::to_string(conflict.origin))},
                {"blocking", bool_text(conflict.blocking)},
                {"status", std::string(source::to_string(conflict.status))},
                {"rationale", conflict.rationale},
                {"resolution", conflict.resolution},
                {"decision_record_id", conflict.decision_record_id},
            },
        }));
        static_cast<void>(graph_.connect(ProjectEdge{
            .from = project_node_id,
            .to = conflict_node_id,
            .kind = ProjectEdgeKind::contains,
            .label = "integration conflict register",
        }));
        const auto* left = find_selected_modification(
            project, conflict.left_modification_id);
        const auto* right = find_selected_modification(
            project, conflict.right_modification_id);
        if (left != nullptr) {
            static_cast<void>(graph_.connect(ProjectEdge{
                .from = conflict_node_id,
                .to = modification_node_id(left->id, left->version),
                .kind = ProjectEdgeKind::conflicts_with,
                .label = "left modification",
            }));
        }
        if (right != nullptr) {
            static_cast<void>(graph_.connect(ProjectEdge{
                .from = conflict_node_id,
                .to = modification_node_id(right->id, right->version),
                .kind = ProjectEdgeKind::conflicts_with,
                .label = "right modification",
            }));
        }
        if (conflict.unresolved_blocking()) {
            static_cast<void>(graph_.connect(ProjectEdge{
                .from = conflict_node_id,
                .to = project_node_id,
                .kind = ProjectEdgeKind::blocks,
                .label = "unresolved blocking conflict",
            }));
        }
    }

    for (const auto& decision : project.decisions()) {
        const auto decision_node_id = "source-decision:" + decision.id;
        static_cast<void>(graph_.upsert(ProjectNode{
            .id = decision_node_id,
            .kind = ProjectNodeKind::decision_record,
            .label = decision.title,
            .attributes = {
                {"decision_id", decision.id},
                {"rationale", decision.rationale},
                {"authors", join(decision.authors)},
            },
        }));
        static_cast<void>(graph_.connect(ProjectEdge{
            .from = project_node_id,
            .to = decision_node_id,
            .kind = ProjectEdgeKind::contains,
            .label = "integration decision record",
        }));
        for (const auto& conflict_id_value : decision.affected_conflict_ids) {
            static_cast<void>(graph_.connect(ProjectEdge{
                .from = decision_node_id,
                .to = "source-integration-conflict:" + conflict_id_value,
                .kind = ProjectEdgeKind::resolves,
                .label = decision.rationale,
            }));
        }
        for (const auto& evidence_id : decision.evidence_record_ids) {
            if (evidence_.find(evidence_id) != nullptr) {
                static_cast<void>(graph_.connect(ProjectEdge{
                    .from = "evidence:" + evidence_id,
                    .to = decision_node_id,
                    .kind = ProjectEdgeKind::evidence_for,
                    .label = "decision evidence",
                }));
            }
        }
    }

    for (const auto& gate : project.gates()) {
        const auto gate_node_id = "source-integration-gate:" + gate.id;
        static_cast<void>(graph_.upsert(ProjectNode{
            .id = gate_node_id,
            .kind = ProjectNodeKind::integration_gate,
            .label = std::string(source::to_string(gate.kind)),
            .attributes = {
                {"gate_id", gate.id},
                {"kind", std::string(source::to_string(gate.kind))},
                {"passed", bool_text(gate.passed)},
                {"summary", gate.summary},
            },
        }));
        static_cast<void>(graph_.connect(ProjectEdge{
            .from = project_node_id,
            .to = gate_node_id,
            .kind = ProjectEdgeKind::contains,
            .label = "integration gate evidence",
        }));
        static_cast<void>(graph_.connect(ProjectEdge{
            .from = gate_node_id,
            .to = project_node_id,
            .kind = gate.passed
                ? ProjectEdgeKind::satisfies
                : ProjectEdgeKind::blocks,
            .label = gate.summary,
        }));
        for (const auto& evidence_id : gate.evidence_record_ids) {
            if (evidence_.find(evidence_id) != nullptr) {
                static_cast<void>(graph_.connect(ProjectEdge{
                    .from = "evidence:" + evidence_id,
                    .to = gate_node_id,
                    .kind = ProjectEdgeKind::evidence_for,
                    .label = "integration gate evidence",
                }));
            }
        }
    }

    source_integration_projects_.emplace(project.id(), std::move(project));
    return true;
}

const source::SourceModificationPackage*
ProjectWorkspace::find_source_modification(
    std::string_view modification_id,
    std::string_view version) const noexcept {
    const auto iterator = source_modifications_.find(
        modification_key(modification_id, version));
    return iterator == source_modifications_.end()
        ? nullptr
        : &iterator->second;
}

const source::IntegrationProject*
ProjectWorkspace::find_source_integration_project(
    std::string_view project_id) const noexcept {
    const auto iterator = source_integration_projects_.find(project_id);
    return iterator == source_integration_projects_.end()
        ? nullptr
        : &iterator->second;
}

std::size_t ProjectWorkspace::source_modification_count() const noexcept {
    return source_modifications_.size();
}

std::size_t ProjectWorkspace::source_integration_project_count() const noexcept {
    return source_integration_projects_.size();
}

} // namespace dmc::rengine::integration
