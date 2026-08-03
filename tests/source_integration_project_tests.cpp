#include "dmc_rengine/source/integration_project.hpp"

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

[[nodiscard]] dmc::rengine::source::SourceModificationPackage make_package(
    std::string id,
    std::string version,
    std::string path,
    char result_hash,
    std::string symbol_id,
    std::string configuration_key) {
    using namespace dmc::rengine::source;
    return SourceModificationPackage{
        .id = std::move(id),
        .version = std::move(version),
        .title = "Test source modification",
        .description = "Synthetic source modification for integration project tests.",
        .implementation_kind = ModificationImplementationKind::recovered_source,
        .baseline = baseline(),
        .semantic_intent = "Test deterministic integration and conflict governance.",
        .change_units = {
            SourceChangeUnit{
                .id = "unit:" + path,
                .source_path = std::move(path),
                .baseline_sha256 = hash('a'),
                .result_sha256 = hash(result_hash),
                .semantic_intent = "Change one recovered source unit.",
                .affected_symbols = {
                    SourceSymbolRef{
                        .id = std::move(symbol_id),
                        .kind = SourceSymbolKind::function,
                        .recovered_name = "RecoveredFunction",
                        .rva = 0x1000U,
                        .evidence_record_ids = {"ev-source-test"},
                    },
                },
            },
        },
        .dependencies = {},
        .declared_conflicts = {},
        .configuration_keys = {std::move(configuration_key)},
        .build_requirements = {"recovered-cpp-v1"},
        .tests = {
            ModificationTestRequirement{
                .id = "test:unit",
                .layer = TestLayer::unit,
                .profile = "source-unit",
                .expected_result = "Recovered unit behavior remains bounded.",
                .mandatory = true,
            },
        },
        .evidence_record_ids = {"ev-source-test"},
        .save_compatibility = "No save schema changes.",
        .migration_notes = "No data migration required.",
        .rollback_instructions = "Remove the modification and rebuild the baseline.",
        .known_risks = {"Synthetic test risk."},
        .provenance = ModificationProvenance{
            .authors = {"Test Author"},
            .contribution_ledger_refs = {"test-ledger"},
            .reviewers = {"Test Reviewer"},
            .ai_assistance_disclosure = "Synthetic test fixture.",
            .licence = "Test-only",
            .redistribution_permission = "Test-only",
        },
    };
}

[[nodiscard]] dmc::rengine::source::IntegrationGateEvidence gate(
    dmc::rengine::source::IntegrationGateKind kind,
    std::string id) {
    return dmc::rengine::source::IntegrationGateEvidence{
        .id = std::move(id),
        .kind = kind,
        .passed = true,
        .summary = "Synthetic passing integration gate.",
        .evidence_record_ids = {"ev-source-test"},
    };
}

} // namespace

int main() {
    using namespace dmc::rengine::source;

    auto core = make_package(
        "inventory-core", "1.0.0",
        "recovered/gameplay/item/inventory_core.cpp", 'b',
        "symbol:inventory-capacity", "inventory.max_count");
    auto extension = make_package(
        "inventory-extension", "1.0.0",
        "recovered/gameplay/item/inventory_extension.cpp", 'c',
        "symbol:inventory-capacity", "inventory.extension_mode");
    extension.dependencies.push_back(ModificationDependency{
        .modification_id = "inventory-core",
        .version_constraint = "1.0.0",
        .mandatory = true,
        .rationale = "The extension requires the recovered inventory core policy.",
    });
    assert(core.valid());
    assert(extension.valid());

    IntegrationProject project(
        "integration:dmc3-ultimate-test",
        "DMC3 Ultimate Test Build",
        baseline());
    assert(project.configure_governance(
        {"Maintainer A", "Maintainer B"},
        "Two maintainer approvals plus passing mandatory gates.",
        {"windows-release", "ubuntu-analysis"},
        {"unit", "integration", "runtime-smoke"}));
    assert(project.select_modification(core));
    assert(project.select_modification(extension));
    assert(project.transition(
        IntegrationProjectState::modifications_selected));

    const auto dependency_report = project.analyze_dependencies();
    assert(dependency_report.passed());
    assert(dependency_report.issues.empty());
    assert(project.transition(IntegrationProjectState::dependency_checked));

    const auto conflicts = project.analyze_conflicts();
    assert(conflicts.size() == 1U);
    assert(conflicts[0].origin == ConflictOrigin::symbol_overlap);
    assert(conflicts[0].conflict_class ==
        ConflictClass::same_function_semantic);
    assert(project.transition(IntegrationProjectState::conflict_analysis));
    assert(project.unresolved_blocking_conflict_count() == 1U);
    assert(!project.transition(IntegrationProjectState::integrating));

    const auto conflict_id = conflicts[0].id;
    assert(project.resolve_conflict(
        conflict_id,
        ConflictResolutionStatus::resolved,
        "Create one unified InventoryCapacityPolicy implementation and preserve both feature test sets.",
        DecisionRecord{
            .id = "decision:unified-inventory-policy",
            .title = "Unify inventory capacity implementations",
            .rationale = "Both modifications affect the same recovered function and must not be stacked independently.",
            .authors = {"Maintainer A", "Maintainer B"},
            .evidence_record_ids = {"ev-source-test"},
            .affected_conflict_ids = {conflict_id},
        }));
    assert(project.unresolved_blocking_conflict_count() == 0U);
    assert(project.transition(IntegrationProjectState::integrating));

    assert(project.record_gate(gate(
        IntegrationGateKind::source_validation,
        "gate:source-validation")));
    assert(project.transition(IntegrationProjectState::source_validated));
    assert(project.transition(IntegrationProjectState::compiling));
    assert(project.record_gate(gate(
        IntegrationGateKind::compilation,
        "gate:compilation")));
    assert(project.record_gate(gate(
        IntegrationGateKind::build_validation,
        "gate:build-validation")));
    assert(project.transition(IntegrationProjectState::build_validated));
    assert(project.transition(IntegrationProjectState::runtime_testing));
    assert(project.record_gate(gate(
        IntegrationGateKind::runtime_testing,
        "gate:runtime-testing")));
    assert(project.transition(IntegrationProjectState::release_candidate));
    assert(project.record_gate(gate(
        IntegrationGateKind::approval,
        "gate:approval")));
    assert(project.transition(IntegrationProjectState::approved));
    assert(project.record_gate(gate(
        IntegrationGateKind::release,
        "gate:release")));
    assert(project.transition(IntegrationProjectState::released));
    assert(project.transition(IntegrationProjectState::maintaining));
    assert(project.valid());
    assert(project.state() == IntegrationProjectState::maintaining);

    auto missing_dependency = make_package(
        "dependent-only", "1.0.0",
        "recovered/dependent.cpp", 'd',
        "symbol:dependent", "dependent.enabled");
    missing_dependency.dependencies.push_back(ModificationDependency{
        .modification_id = "missing-core",
        .version_constraint = "1.0.0",
        .mandatory = true,
        .rationale = "Required core is absent.",
    });
    IntegrationProject blocked(
        "integration:blocked", "Blocked Integration", baseline());
    assert(blocked.configure_governance(
        {"Maintainer"}, "Single test approval.",
        {"windows-release"}, {"unit"}));
    assert(blocked.select_modification(missing_dependency));
    assert(blocked.transition(
        IntegrationProjectState::modifications_selected));
    const auto blocked_dependencies = blocked.analyze_dependencies();
    assert(!blocked_dependencies.passed());
    assert(blocked_dependencies.blocking_count() == 1U);
    assert(!blocked.transition(IntegrationProjectState::dependency_checked));
    assert(blocked.fail(
        IntegrationProjectState::build_failed,
        "Synthetic failure path."));
    assert(blocked.valid());
    assert(blocked.state() == IntegrationProjectState::build_failed);
    return 0;
}
