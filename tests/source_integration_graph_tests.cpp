#include "dmc_rengine/evidence/packet.hpp"
#include "dmc_rengine/integration/project_workspace.hpp"
#include "dmc_rengine/source/integration_project.hpp"

#include <algorithm>
#include <cassert>
#include <string>

namespace {

[[nodiscard]] std::string hash(char value) {
    return std::string(64U, value);
}

[[nodiscard]] dmc::rengine::source::SourceBaselineIdentity baseline() {
    return dmc::rengine::source::SourceBaselineIdentity{
        .game_profile = "dmc3-hd",
        .original_exe_sha256 =
            "e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082",
        .source_schema_version = "recovered-cpp-v1",
        .source_revision = "source-tree-0001",
        .compiler_profile = "msvc-x64-dmc3-hd",
        .prior_custom_build_id = {},
    };
}

[[nodiscard]] dmc::rengine::source::SourceModificationPackage package(
    std::string id,
    std::string path,
    char result_hash) {
    using namespace dmc::rengine::source;
    return SourceModificationPackage{
        .id = std::move(id),
        .version = "1.0.0",
        .title = "Source integration graph test package",
        .description = "Synthetic package used to validate ProjectWorkspace source integration.",
        .implementation_kind = ModificationImplementationKind::recovered_source,
        .baseline = baseline(),
        .semantic_intent = "Validate source modification provenance and conflict decisions.",
        .change_units = {
            SourceChangeUnit{
                .id = "unit:" + path,
                .source_path = std::move(path),
                .baseline_sha256 = hash('a'),
                .result_sha256 = hash(result_hash),
                .semantic_intent = "Change the recovered inventory policy.",
                .affected_symbols = {
                    SourceSymbolRef{
                        .id = "symbol:inventory-capacity",
                        .kind = SourceSymbolKind::function,
                        .recovered_name = "InventoryCapacityPolicy",
                        .rva = 0x1000U,
                        .evidence_record_ids = {"ev-source-integration-test"},
                    },
                },
            },
        },
        .dependencies = {},
        .declared_conflicts = {},
        .configuration_keys = {},
        .build_requirements = {"recovered-cpp-v1"},
        .tests = {
            ModificationTestRequirement{
                .id = "source-integration-unit",
                .layer = TestLayer::unit,
                .profile = "source-integration",
                .expected_result = "Source package graph remains deterministic.",
                .mandatory = true,
            },
        },
        .evidence_record_ids = {"ev-source-integration-test"},
        .save_compatibility = "No save schema change.",
        .migration_notes = "Synthetic fixture.",
        .rollback_instructions = "Rebuild the exact baseline source revision.",
        .known_risks = {"Synthetic fixture risk."},
        .provenance = ModificationProvenance{
            .authors = {"Test Author"},
            .contribution_ledger_refs = {"test-ledger"},
            .reviewers = {"Test Reviewer"},
            .ai_assistance_disclosure = "Synthetic fixture.",
            .licence = "Test-only",
            .redistribution_permission = "Test-only",
        },
    };
}

[[nodiscard]] dmc::rengine::evidence::EvidencePacket packet() {
    using namespace dmc::rengine::evidence;
    return EvidencePacket{
        .schema_version = 1U,
        .id = "packet-source-integration-test",
        .title = "Source integration test packet",
        .project = "DMC Rengine",
        .artifacts = {
            ArtifactIdentity{
                .id = "artifact-dmc3-canonical-source-test",
                .role = "canonical-research-target",
                .sha256 = baseline().original_exe_sha256,
                .size = 1024U,
            },
        },
        .records = {
            EvidenceRecord{
                .id = "ev-source-integration-test",
                .claim_id = "claim-source-integration-test",
                .title = "Synthetic recovered source symbol",
                .summary = "Evidence for a synthetic recovered inventory policy symbol.",
                .confidence = Confidence::confirmed,
                .locations = {
                    EvidenceLocation{
                        .artifact_id = "artifact-dmc3-canonical-source-test",
                        .file_offset = 16U,
                        .size = 4U,
                        .rva = std::nullopt,
                        .va = std::nullopt,
                        .symbol = "InventoryCapacityPolicy",
                        .note = "Synthetic graph fixture.",
                    },
                },
                .tags = {"source", "integration", "synthetic"},
                .supersedes = {},
            },
        },
    };
}

[[nodiscard]] dmc::rengine::source::IntegrationProject integration_project(
    const dmc::rengine::source::SourceModificationPackage& left,
    const dmc::rengine::source::SourceModificationPackage& right) {
    using namespace dmc::rengine::source;
    IntegrationProject project(
        "integration:source-graph-test",
        "Source Graph Test Integration",
        baseline());
    assert(project.configure_governance(
        {"Maintainer"},
        "One maintainer plus mandatory gates.",
        {"windows-release"},
        {"unit", "integration"}));
    assert(project.select_modification(left));
    assert(project.select_modification(right));
    assert(project.transition(
        IntegrationProjectState::modifications_selected));
    assert(project.analyze_dependencies().passed());
    assert(project.transition(IntegrationProjectState::dependency_checked));
    const auto conflicts = project.analyze_conflicts();
    assert(conflicts.size() == 1U);
    assert(project.transition(IntegrationProjectState::conflict_analysis));
    assert(project.resolve_conflict(
        conflicts[0].id,
        ConflictResolutionStatus::resolved,
        "Use one unified recovered source implementation.",
        DecisionRecord{
            .id = "decision:source-graph-test",
            .title = "Unify overlapping recovered symbol",
            .rationale = "Two separate implementations must not own the same recovered function.",
            .authors = {"Maintainer"},
            .evidence_record_ids = {"ev-source-integration-test"},
            .affected_conflict_ids = {conflicts[0].id},
        }));
    assert(project.transition(IntegrationProjectState::integrating));
    assert(project.record_gate(IntegrationGateEvidence{
        .id = "gate:source-graph-validation",
        .kind = IntegrationGateKind::source_validation,
        .passed = true,
        .summary = "Synthetic source validation passed.",
        .evidence_record_ids = {"ev-source-integration-test"},
    }));
    assert(project.transition(IntegrationProjectState::source_validated));
    return project;
}

} // namespace

int main() {
    using dmc::rengine::integration::ProjectEdgeKind;
    using dmc::rengine::integration::ProjectNodeKind;
    using dmc::rengine::integration::ProjectWorkspace;

    ProjectWorkspace workspace;
    assert(workspace.import_evidence_packet(packet()));

    const auto left = package(
        "inventory-source-a",
        "recovered/gameplay/item/inventory_a.cpp",
        'b');
    const auto right = package(
        "inventory-source-b",
        "recovered/gameplay/item/inventory_b.cpp",
        'c');
    assert(workspace.register_source_modification(left));
    assert(workspace.register_source_modification(right));
    assert(!workspace.register_source_modification(left));
    assert(workspace.source_modification_count() == 2U);
    assert(workspace.find_source_modification(
        "inventory-source-a", "1.0.0") != nullptr);

    const auto project = integration_project(left, right);
    assert(workspace.register_source_integration_project(project));
    assert(!workspace.register_source_integration_project(project));
    assert(workspace.source_integration_project_count() == 1U);
    assert(workspace.find_source_integration_project(
        "integration:source-graph-test") != nullptr);

    assert(workspace.graph().nodes(ProjectNodeKind::source_baseline).size() == 1U);
    assert(workspace.graph().nodes(ProjectNodeKind::source_modification).size() == 2U);
    assert(workspace.graph().nodes(ProjectNodeKind::source_change_unit).size() == 2U);
    assert(workspace.graph().nodes(ProjectNodeKind::source_symbol).size() == 1U);
    assert(workspace.graph().nodes(ProjectNodeKind::integration_project).size() == 1U);
    assert(workspace.graph().nodes(ProjectNodeKind::integration_conflict).size() == 1U);
    assert(workspace.graph().nodes(ProjectNodeKind::decision_record).size() == 1U);
    assert(workspace.graph().nodes(ProjectNodeKind::integration_gate).size() == 3U);

    bool selected_left = false;
    bool selected_right = false;
    bool decision_resolves = false;
    bool evidence_symbol = false;
    for (const auto& edge : workspace.graph().all_edges()) {
        if (edge.from ==
                "source-integration-project:integration:source-graph-test" &&
            edge.to == "source-modification:inventory-source-a@1.0.0" &&
            edge.kind == ProjectEdgeKind::selects) {
            selected_left = true;
        }
        if (edge.from ==
                "source-integration-project:integration:source-graph-test" &&
            edge.to == "source-modification:inventory-source-b@1.0.0" &&
            edge.kind == ProjectEdgeKind::selects) {
            selected_right = true;
        }
        if (edge.from == "source-decision:decision:source-graph-test" &&
            edge.kind == ProjectEdgeKind::resolves) {
            decision_resolves = true;
        }
        if (edge.from == "evidence:ev-source-integration-test" &&
            edge.to == "source-symbol:symbol:inventory-capacity" &&
            edge.kind == ProjectEdgeKind::evidence_for) {
            evidence_symbol = true;
        }
    }
    assert(selected_left);
    assert(selected_right);
    assert(decision_resolves);
    assert(evidence_symbol);

    auto missing_evidence = package(
        "missing-evidence-source",
        "recovered/missing_evidence.cpp",
        'd');
    missing_evidence.evidence_record_ids = {"ev-not-loaded"};
    assert(missing_evidence.valid());
    assert(!workspace.register_source_modification(missing_evidence));

    const auto unregistered = package(
        "unregistered-source",
        "recovered/unregistered.cpp",
        'e');
    auto incomplete_project = integration_project(left, unregistered);
    assert(!workspace.register_source_integration_project(
        incomplete_project));
    return 0;
}
