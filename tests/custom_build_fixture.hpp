#pragma once

#include "dmc_rengine/evidence/packet.hpp"
#include "dmc_rengine/integration/project_workspace.hpp"
#include "dmc_rengine/source/custom_build.hpp"
#include "dmc_rengine/source/integration_project.hpp"
#include "dmc_rengine/source/modification.hpp"

#include <cassert>
#include <string>
#include <utility>

namespace dmc::rengine::tests::custom_build_fixture {

[[nodiscard]] inline std::string hash(char value) {
    return std::string(64U, value);
}

[[nodiscard]] inline source::SourceBaselineIdentity baseline() {
    return source::SourceBaselineIdentity{
        .game_profile = "dmc3-hd",
        .original_exe_sha256 =
            "e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082",
        .source_schema_version = "recovered-cpp-v1",
        .source_revision = "source-tree-0001",
        .compiler_profile = "msvc-x64-dmc3-hd",
        .prior_custom_build_id = {},
    };
}

[[nodiscard]] inline evidence::EvidencePacket packet() {
    using namespace evidence;
    return EvidencePacket{
        .schema_version = 1U,
        .id = "packet-custom-build-fixture",
        .title = "Custom build fixture Evidence Packet",
        .project = "DMC Rengine",
        .artifacts = {
            ArtifactIdentity{
                .id = "artifact-custom-build-original-exe",
                .role = "canonical-research-target",
                .sha256 = baseline().original_exe_sha256,
                .size = 0x4000U,
            },
        },
        .records = {
            EvidenceRecord{
                .id = "ev-custom-build-symbol",
                .claim_id = "claim-custom-build-symbol",
                .title = "Recovered custom build symbol",
                .summary = "Synthetic evidence for custom build source mapping tests.",
                .confidence = Confidence::confirmed,
                .locations = {
                    EvidenceLocation{
                        .artifact_id = "artifact-custom-build-original-exe",
                        .file_offset = 0x200U,
                        .size = 0x40U,
                        .rva = 0x1000U,
                        .va = 0x140001000ULL,
                        .symbol = "InventoryCapacityPolicy",
                        .note = "Synthetic source-build fixture.",
                    },
                },
                .tags = {"source", "custom-build", "synthetic"},
                .supersedes = {},
            },
        },
    };
}

[[nodiscard]] inline source::SourceModificationPackage modification() {
    using namespace source;
    return SourceModificationPackage{
        .id = "inventory-source-core",
        .version = "1.0.0",
        .title = "Inventory Source Core",
        .description = "Recovered source implementation used by custom build tests.",
        .implementation_kind = ModificationImplementationKind::recovered_source,
        .baseline = baseline(),
        .semantic_intent = "Provide one evidence-backed inventory capacity policy.",
        .change_units = {
            SourceChangeUnit{
                .id = "inventory-policy-unit",
                .source_path = "recovered/gameplay/item/inventory_policy.cpp",
                .baseline_sha256 = hash('a'),
                .result_sha256 = hash('b'),
                .semantic_intent = "Replace scattered capacity checks with one recovered policy.",
                .affected_symbols = {
                    SourceSymbolRef{
                        .id = "symbol:inventory-capacity",
                        .kind = SourceSymbolKind::function,
                        .recovered_name = "InventoryCapacityPolicy",
                        .rva = 0x1000U,
                        .evidence_record_ids = {"ev-custom-build-symbol"},
                    },
                },
            },
        },
        .dependencies = {},
        .declared_conflicts = {},
        .configuration_keys = {"inventory.max_count"},
        .build_requirements = {"recovered-cpp-v1", "msvc-x64-dmc3-hd"},
        .tests = {
            ModificationTestRequirement{
                .id = "inventory-source-unit",
                .layer = TestLayer::unit,
                .profile = "inventory-policy",
                .expected_result = "Inventory capacity policy tests pass.",
                .mandatory = true,
            },
        },
        .evidence_record_ids = {"ev-custom-build-symbol"},
        .save_compatibility = "No save schema change.",
        .migration_notes = "No data migration required.",
        .rollback_instructions = "Restore source-tree-0001 and rebuild.",
        .known_risks = {"Runtime behavior still requires copied-build smoke testing."},
        .provenance = ModificationProvenance{
            .authors = {"VrUaCom"},
            .contribution_ledger_refs = {"fixture:custom-build"},
            .reviewers = {"DMC Rengine maintainers"},
            .ai_assistance_disclosure = "AI-assisted fixture; human review required.",
            .licence = "Test-only",
            .redistribution_permission = "Test-only",
        },
    };
}

[[nodiscard]] inline source::IntegrationProject integration_project() {
    using namespace source;
    const auto package = modification();
    IntegrationProject project(
        "integration:custom-build-fixture",
        "Custom Build Fixture Composite Source Build",
        baseline());
    assert(project.configure_governance(
        {"Maintainer"},
        "One maintainer plus mandatory gates.",
        {"windows-release"},
        {"unit", "pe-structure"}));
    assert(project.select_modification(package));
    assert(project.transition(IntegrationProjectState::modifications_selected));
    assert(project.analyze_dependencies().passed());
    assert(project.transition(IntegrationProjectState::dependency_checked));
    assert(project.analyze_conflicts().empty());
    assert(project.transition(IntegrationProjectState::conflict_analysis));
    assert(project.transition(IntegrationProjectState::integrating));
    assert(project.record_gate(IntegrationGateEvidence{
        .id = "gate:custom-build-source-validation",
        .kind = IntegrationGateKind::source_validation,
        .passed = true,
        .summary = "Recovered source validation passed.",
        .evidence_record_ids = {"ev-custom-build-symbol"},
    }));
    assert(project.transition(IntegrationProjectState::source_validated));
    assert(project.transition(IntegrationProjectState::compiling));
    assert(project.record_gate(IntegrationGateEvidence{
        .id = "gate:custom-build-compilation",
        .kind = IntegrationGateKind::compilation,
        .passed = true,
        .summary = "Composite source compilation passed.",
        .evidence_record_ids = {"ev-custom-build-symbol"},
    }));
    assert(project.record_gate(IntegrationGateEvidence{
        .id = "gate:custom-build-structural-validation",
        .kind = IntegrationGateKind::build_validation,
        .passed = true,
        .summary = "Custom executable PE structure validation passed.",
        .evidence_record_ids = {"ev-custom-build-symbol"},
    }));
    assert(project.transition(IntegrationProjectState::build_validated));
    return project;
}

[[nodiscard]] inline source::CustomBuildRecord build_record(
    std::string custom_build_id = "dmc3-custom-build-0001",
    char executable_hash_character = 'c',
    std::string prior_custom_build_id = {}) {
    using namespace source;
    return CustomBuildRecord{
        .identity = CustomBuildIdentity{
            .custom_build_id = std::move(custom_build_id),
            .edition_id = "dmc3-rengine-internal",
            .semantic_version = "0.1.0",
            .integration_project_id = "integration:custom-build-fixture",
            .game_profile = "dmc3-hd",
            .original_exe_sha256 = baseline().original_exe_sha256,
            .source_schema_version = baseline().source_schema_version,
            .source_revision = baseline().source_revision,
            .source_tree_sha256 = hash('d'),
            .executable_sha256 = hash(executable_hash_character),
            .prior_custom_build_id = std::move(prior_custom_build_id),
            .release_channel = "internal-test",
            .maintainer_identity = "DMC Rengine maintainers",
        },
        .status = CustomBuildStatus::structurally_validated,
        .toolchain = ToolchainIdentity{
            .compiler_name = "MSVC",
            .compiler_version = "19.44",
            .linker_name = "link.exe",
            .linker_version = "14.44",
            .target_triple = "x86_64-pc-windows-msvc",
            .cmake_version = "4.0.3",
            .compile_flags = {"/std:c++20", "/O2"},
            .link_flags = {"/SUBSYSTEM:WINDOWS", "/DYNAMICBASE"},
            .dependency_lock_sha256 = hash('e'),
        },
        .pe_validation = PeBuildValidation{
            .pe32_plus = true,
            .machine = 0x8664U,
            .image_base = 0x140000000ULL,
            .entry_point_rva = 0x1000U,
            .subsystem = 2U,
            .section_count = 8U,
            .import_table_valid = true,
            .exception_directory_valid = true,
            .load_config_valid = true,
            .source_mapping_complete = true,
            .report_sha256 = hash('f'),
        },
        .executable_size = 0x4000U,
        .included_modifications = {
            IncludedModification{
                .modification_id = "inventory-source-core",
                .version = "1.0.0",
                .package_manifest_sha256 = hash('1'),
            },
        },
        .unified_implementation_decision_ids = {},
        .source_binary_mappings = {
            SourceBinaryMapping{
                .id = "mapping:inventory-capacity",
                .modification_id = "inventory-source-core",
                .modification_version = "1.0.0",
                .source_unit_id = "inventory-policy-unit",
                .source_path = "recovered/gameplay/item/inventory_policy.cpp",
                .source_line_begin = 10U,
                .source_line_end = 42U,
                .recovered_symbol_id = "symbol:inventory-capacity",
                .recovered_symbol_name = "InventoryCapacityPolicy",
                .original_rva = 0x1000U,
                .output_file_offset = 0x600U,
                .output_rva = 0x1200U,
                .output_va = 0x140001200ULL,
                .byte_size = 0x40U,
                .confidence = SourceBinaryMappingConfidence::confirmed,
                .evidence_record_ids = {"ev-custom-build-symbol"},
            },
        },
        .test_results = {
            BuildTestResult{
                .id = "inventory-source-unit",
                .layer = TestLayer::unit,
                .profile = "inventory-policy",
                .mandatory = true,
                .passed = true,
                .summary = "Inventory source unit test passed.",
                .report_sha256 = hash('2'),
                .evidence_record_ids = {"ev-custom-build-symbol"},
            },
            BuildTestResult{
                .id = "custom-build-pe-structure",
                .layer = TestLayer::pe_structure,
                .profile = "dmc3-hd-custom-build",
                .mandatory = true,
                .passed = true,
                .summary = "Custom executable PE structure test passed.",
                .report_sha256 = hash('3'),
                .evidence_record_ids = {},
            },
        },
        .resource_package_versions = {"dmc3-hd-resources@baseline"},
        .known_issues = {"Runtime smoke test remains pending."},
        .credits = {"VrUaCom", "DMC Rengine maintainers"},
        .distribution_form = DistributionForm::private_test_build,
        .distribution_permission = "Private internal testing only.",
        .attestation_state = AttestationState::hash_attested,
        .attestation_reference = "attestation:custom-build-fixture",
        .rollback_build_id = "baseline:source-tree-0001",
    };
}

inline void prepare_workspace(integration::ProjectWorkspace& workspace) {
    assert(workspace.import_evidence_packet(packet()));
    assert(workspace.register_source_modification(modification()));
    assert(workspace.register_source_integration_project(integration_project()));
}

} // namespace dmc::rengine::tests::custom_build_fixture
