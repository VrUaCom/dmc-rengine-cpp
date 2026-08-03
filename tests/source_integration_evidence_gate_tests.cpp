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

[[nodiscard]] dmc::rengine::source::SourceModificationPackage package(
    std::string id,
    char result_hash) {
    using namespace dmc::rengine::source;
    return SourceModificationPackage{
        .id = std::move(id),
        .version = "1.0.0",
        .title = "Evidence gate package",
        .description = "Synthetic package for integration evidence gate tests.",
        .implementation_kind = ModificationImplementationKind::recovered_source,
        .baseline = baseline(),
        .semantic_intent = "Validate that integration records cannot cite unknown evidence.",
        .change_units = {
            SourceChangeUnit{
                .id = "evidence-gate-unit",
                .source_path = "recovered/evidence_gate.cpp",
                .baseline_sha256 = hash('a'),
                .result_sha256 = hash(result_hash),
                .semantic_intent = "Change one evidence-backed recovered function.",
                .affected_symbols = {
                    SourceSymbolRef{
                        .id = "symbol:evidence-gate",
                        .kind = SourceSymbolKind::function,
                        .recovered_name = "EvidenceGateFunction",
                        .rva = 0x1000U,
                        .evidence_record_ids = {"ev-known-source"},
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
                .id = "evidence-gate-test",
                .layer = TestLayer::unit,
                .profile = "source-evidence-gate",
                .expected_result = "Unknown integration evidence references are rejected.",
                .mandatory = true,
            },
        },
        .evidence_record_ids = {"ev-known-source"},
        .save_compatibility = "No save change.",
        .migration_notes = "No migration required.",
        .rollback_instructions = "Restore the exact baseline source revision.",
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

} // namespace

int main() {
    using namespace dmc::rengine::source;

    const auto left = package("evidence-left", 'b');
    const auto right = package("evidence-right", 'c');
    IntegrationProject project(
        "integration:evidence-gate",
        "Evidence Gate Integration",
        baseline());
    assert(project.configure_governance(
        {"Maintainer"},
        "One maintainer plus passing mandatory gates.",
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

    const auto dangling_decision = DecisionRecord{
        .id = "decision:dangling-evidence",
        .title = "Invalid dangling evidence decision",
        .rationale = "This record intentionally cites unknown evidence.",
        .authors = {"Maintainer"},
        .evidence_record_ids = {"ev-not-selected"},
        .affected_conflict_ids = {conflicts[0].id},
    };
    assert(dangling_decision.valid());
    assert(!project.resolve_conflict(
        conflicts[0].id,
        ConflictResolutionStatus::resolved,
        "Invalid resolution must be rejected.",
        dangling_decision));
    assert(project.unresolved_blocking_conflict_count() == 1U);

    assert(project.resolve_conflict(
        conflicts[0].id,
        ConflictResolutionStatus::resolved,
        "Use one evidence-backed unified implementation.",
        DecisionRecord{
            .id = "decision:known-evidence",
            .title = "Evidence-backed resolution",
            .rationale = "The selected modifications already declare the cited Evidence Record.",
            .authors = {"Maintainer"},
            .evidence_record_ids = {"ev-known-source"},
            .affected_conflict_ids = {conflicts[0].id},
        }));
    assert(project.transition(IntegrationProjectState::integrating));

    const auto dangling_gate = IntegrationGateEvidence{
        .id = "gate:dangling-source-validation",
        .kind = IntegrationGateKind::source_validation,
        .passed = true,
        .summary = "Invalid gate intentionally cites unknown evidence.",
        .evidence_record_ids = {"ev-not-selected"},
    };
    assert(dangling_gate.valid());
    assert(!project.record_gate(dangling_gate));
    assert(!project.transition(IntegrationProjectState::source_validated));

    assert(project.record_gate(IntegrationGateEvidence{
        .id = "gate:known-source-validation",
        .kind = IntegrationGateKind::source_validation,
        .passed = true,
        .summary = "Source validation is backed by selected modification evidence.",
        .evidence_record_ids = {"ev-known-source"},
    }));
    assert(project.transition(IntegrationProjectState::source_validated));
    assert(project.valid());
    return 0;
}
