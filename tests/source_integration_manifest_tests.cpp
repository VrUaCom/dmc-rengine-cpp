#include "dmc_rengine/core/json.hpp"
#include "dmc_rengine/evidence/packet.hpp"
#include "dmc_rengine/integration/project_workspace.hpp"
#include "dmc_rengine/source/integration_manifest.hpp"
#include "dmc_rengine/source/integration_project.hpp"

#include <cassert>
#include <string>
#include <string_view>

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
        .title = "Manifest source modification",
        .description = "Synthetic source package for manifest validation.",
        .implementation_kind = ModificationImplementationKind::recovered_source,
        .baseline = baseline(),
        .semantic_intent = "Represent a bounded recovered C++ change.",
        .change_units = {
            SourceChangeUnit{
                .id = "unit:" + path,
                .source_path = std::move(path),
                .baseline_sha256 = hash('a'),
                .result_sha256 = hash(result_hash),
                .semantic_intent = "Update one recovered source unit.",
                .affected_symbols = {
                    SourceSymbolRef{
                        .id = "symbol:shared-manifest-function",
                        .kind = SourceSymbolKind::function,
                        .recovered_name = "SharedManifestFunction",
                        .rva = 0x1000U,
                        .evidence_record_ids = {"ev-source-manifest"},
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
                .id = "manifest-unit-test",
                .layer = TestLayer::unit,
                .profile = "source-manifest",
                .expected_result = "Manifest remains deterministic.",
                .mandatory = true,
            },
        },
        .evidence_record_ids = {"ev-source-manifest"},
        .save_compatibility = "No save change.",
        .migration_notes = "No migration required.",
        .rollback_instructions = "Restore source-tree-0001 and rebuild.",
        .known_risks = {"Synthetic test risk."},
        .provenance = ModificationProvenance{
            .authors = {"Test Author"},
            .contribution_ledger_refs = {"ledger:test"},
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
        .id = "packet-source-manifest",
        .title = "Source manifest packet",
        .project = "DMC Rengine",
        .artifacts = {
            ArtifactIdentity{
                .id = "artifact-source-manifest",
                .role = "canonical-research-target",
                .sha256 = baseline().original_exe_sha256,
                .size = 1024U,
            },
        },
        .records = {
            EvidenceRecord{
                .id = "ev-source-manifest",
                .claim_id = "claim-source-manifest",
                .title = "Source manifest evidence",
                .summary = "Synthetic source manifest evidence.",
                .confidence = Confidence::confirmed,
                .locations = {
                    EvidenceLocation{
                        .artifact_id = "artifact-source-manifest",
                        .file_offset = 16U,
                        .size = 4U,
                        .rva = std::nullopt,
                        .va = std::nullopt,
                        .symbol = "SharedManifestFunction",
                        .note = "Synthetic fixture.",
                    },
                },
                .tags = {"source", "manifest"},
                .supersedes = {},
            },
        },
    };
}

[[nodiscard]] const dmc::rengine::core::json::Value* member(
    const dmc::rengine::core::json::Value::Object& object,
    std::string_view name) {
    const auto iterator = object.find(name);
    return iterator == object.end() ? nullptr : &iterator->second;
}

} // namespace

int main() {
    using dmc::rengine::core::json::Parser;
    using dmc::rengine::integration::ProjectWorkspace;
    using namespace dmc::rengine::source;

    ProjectWorkspace workspace;
    assert(workspace.import_evidence_packet(packet()));
    const auto left = package(
        "manifest-left", "recovered/manifest_left.cpp", 'b');
    const auto right = package(
        "manifest-right", "recovered/manifest_right.cpp", 'c');
    assert(workspace.register_source_modification(left));
    assert(workspace.register_source_modification(right));

    IntegrationProject project(
        "integration:manifest-test",
        "Manifest Test Composite Source Build",
        baseline());
    assert(project.configure_governance(
        {"Maintainer"},
        "One maintainer plus mandatory gates.",
        {"windows-release"},
        {"unit", "integration", "runtime-smoke"}));
    assert(project.select_modification(right));
    assert(project.select_modification(left));
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
        "Create one unified recovered implementation.",
        DecisionRecord{
            .id = "decision:manifest-unification",
            .title = "Unify manifest test implementations",
            .rationale = "Both packages affect the same recovered function.",
            .authors = {"Maintainer"},
            .evidence_record_ids = {"ev-source-manifest"},
            .affected_conflict_ids = {conflicts[0].id},
        }));
    assert(project.transition(IntegrationProjectState::integrating));
    assert(project.record_gate(IntegrationGateEvidence{
        .id = "gate:manifest-source-validation",
        .kind = IntegrationGateKind::source_validation,
        .passed = true,
        .summary = "Source validation passed.",
        .evidence_record_ids = {"ev-source-manifest"},
    }));
    assert(project.transition(IntegrationProjectState::source_validated));
    assert(workspace.register_source_integration_project(project));

    const auto json = integration_project_manifest_json(
        workspace, "integration:manifest-test");
    assert(!json.empty());
    assert(json == integration_project_manifest_json(
        workspace, "integration:manifest-test"));
    const auto parsed = Parser::parse(json);
    assert(parsed.ok());
    const auto* root = parsed.value->as_object();
    assert(root != nullptr);

    const auto* terminology_value = member(*root, "terminology");
    assert(terminology_value != nullptr);
    const auto* terminology = terminology_value->as_object();
    assert(terminology != nullptr);
    const auto* primary = member(*terminology, "primary_artifact");
    const auto* plugin_model = member(*terminology, "runtime_plugin_model");
    assert(primary != nullptr && primary->as_string() != nullptr);
    assert(*primary->as_string() == "Composite Source Build");
    assert(plugin_model != nullptr && plugin_model->as_bool() != nullptr);
    assert(!*plugin_model->as_bool());

    const auto* baseline_value = member(*root, "baseline");
    assert(baseline_value != nullptr);
    const auto* baseline_object = baseline_value->as_object();
    assert(baseline_object != nullptr);
    const auto* exe_hash = member(*baseline_object, "original_exe_sha256");
    assert(exe_hash != nullptr && exe_hash->as_string() != nullptr);
    assert(*exe_hash->as_string() == baseline().original_exe_sha256);

    const auto* modifications_value = member(*root, "modifications");
    assert(modifications_value != nullptr);
    const auto* modifications = modifications_value->as_array();
    assert(modifications != nullptr && modifications->size() == 2U);
    const auto* first_modification = (*modifications)[0].as_object();
    const auto* second_modification = (*modifications)[1].as_object();
    assert(first_modification != nullptr);
    assert(second_modification != nullptr);
    assert(*member(*first_modification, "id")->as_string() == "manifest-left");
    assert(*member(*second_modification, "id")->as_string() == "manifest-right");

    const auto* conflict_register_value = member(*root, "conflict_register");
    assert(conflict_register_value != nullptr);
    const auto* conflict_register = conflict_register_value->as_object();
    assert(conflict_register != nullptr);
    const auto* unresolved = member(
        *conflict_register, "unresolved_blocking_count");
    assert(unresolved != nullptr && unresolved->as_u64() != nullptr);
    assert(*unresolved->as_u64() == 0U);
    const auto* records = member(*conflict_register, "records");
    assert(records != nullptr && records->as_array() != nullptr);
    assert(records->as_array()->size() == 1U);

    const auto* decisions_value = member(*root, "decision_records");
    assert(decisions_value != nullptr && decisions_value->as_array() != nullptr);
    assert(decisions_value->as_array()->size() == 1U);
    const auto* gates_value = member(*root, "gates");
    assert(gates_value != nullptr && gates_value->as_array() != nullptr);
    assert(gates_value->as_array()->size() == 3U);

    assert(integration_project_manifest_json(
        workspace, "missing-project").empty());
    return 0;
}
