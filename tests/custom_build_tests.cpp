#include "dmc_rengine/source/custom_build.hpp"

#include <cassert>
#include <string>

namespace {

[[nodiscard]] std::string hash(char value) {
    return std::string(64U, value);
}

[[nodiscard]] dmc::rengine::source::CustomBuildRecord record() {
    using namespace dmc::rengine::source;
    return CustomBuildRecord{
        .identity = CustomBuildIdentity{
            .custom_build_id = "custom-build:dmc3-test-0001",
            .edition_id = "edition:test",
            .semantic_version = "0.1.0",
            .integration_project_id = "integration:test-build",
            .game_profile = "dmc3-hd",
            .original_exe_sha256 =
                "e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082",
            .source_schema_version = "recovered-cpp-v1",
            .source_revision = "source-tree-0001",
            .source_tree_sha256 = hash('a'),
            .executable_sha256 = hash('b'),
            .prior_custom_build_id = {},
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
            .compile_flags = {"/std:c++20", "/O2", "/EHsc"},
            .link_flags = {"/SUBSYSTEM:WINDOWS", "/DYNAMICBASE"},
            .dependency_lock_sha256 = hash('c'),
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
            .report_sha256 = hash('d'),
        },
        .executable_size = 0x4000U,
        .included_modifications = {
            IncludedModification{
                .modification_id = "inventory-core",
                .version = "1.0.0",
                .package_manifest_sha256 = hash('e'),
            },
        },
        .unified_implementation_decision_ids = {
            "decision:inventory-policy",
        },
        .source_binary_mappings = {
            SourceBinaryMapping{
                .id = "mapping:inventory-policy",
                .modification_id = "inventory-core",
                .modification_version = "1.0.0",
                .source_unit_id = "inventory-policy-unit",
                .source_path = "recovered/gameplay/item/inventory_policy.cpp",
                .source_line_begin = 10U,
                .source_line_end = 42U,
                .recovered_symbol_id = "symbol:inventory-capacity",
                .recovered_symbol_name = "InventoryCapacityPolicy",
                .original_rva = 0x2000U,
                .output_file_offset = 0x600U,
                .output_rva = 0x1200U,
                .output_va = 0x140001200ULL,
                .byte_size = 0x40U,
                .confidence = SourceBinaryMappingConfidence::confirmed,
                .evidence_record_ids = {"ev-inventory-policy"},
            },
        },
        .test_results = {
            BuildTestResult{
                .id = "inventory-unit",
                .layer = TestLayer::unit,
                .profile = "inventory-policy",
                .mandatory = true,
                .passed = true,
                .summary = "Inventory policy unit tests passed.",
                .report_sha256 = hash('f'),
                .evidence_record_ids = {"ev-inventory-policy"},
            },
            BuildTestResult{
                .id = "pe-validation",
                .layer = TestLayer::pe_structure,
                .profile = "dmc3-hd-custom-build",
                .mandatory = true,
                .passed = true,
                .summary = "PE structure validation passed.",
                .report_sha256 = hash('1'),
                .evidence_record_ids = {},
            },
        },
        .resource_package_versions = {"dmc3-hd-resources@baseline"},
        .known_issues = {"Runtime smoke test remains pending."},
        .credits = {"VrUaCom", "DMC Rengine maintainers"},
        .distribution_form = DistributionForm::private_test_build,
        .distribution_permission = "Private internal testing only.",
        .attestation_state = AttestationState::hash_attested,
        .attestation_reference = "attestation:custom-build-test",
        .rollback_build_id = "baseline:source-tree-0001",
    };
}

} // namespace

int main() {
    using namespace dmc::rengine::source;

    const auto valid = record();
    assert(valid.valid());
    assert(valid.mandatory_tests_passed());
    assert(valid.mapping_for_rva(0x1200U) != nullptr);
    assert(valid.mapping_for_rva(0x123FU) != nullptr);
    assert(valid.mapping_for_rva(0x1240U) == nullptr);
    assert(valid.included_modification("inventory-core") != nullptr);
    assert(valid.included_modification("missing") == nullptr);

    auto bad_identity = valid;
    bad_identity.identity.executable_sha256 =
        bad_identity.identity.original_exe_sha256;
    assert(!bad_identity.valid());

    auto failed_test = valid;
    failed_test.test_results[0].passed = false;
    assert(!failed_test.mandatory_tests_passed());
    assert(!failed_test.valid());

    auto optional_failure = valid;
    optional_failure.test_results.push_back(BuildTestResult{
        .id = "optional-performance-probe",
        .layer = TestLayer::performance,
        .profile = "exploratory",
        .mandatory = false,
        .passed = false,
        .summary = "Optional performance probe did not meet the target.",
        .report_sha256 = hash('2'),
        .evidence_record_ids = {},
    });
    assert(optional_failure.valid());

    auto bad_va = valid;
    bad_va.source_binary_mappings[0].output_va += 1U;
    assert(!bad_va.valid());

    auto outside_file = valid;
    outside_file.source_binary_mappings[0].output_file_offset = 0x3FF0U;
    outside_file.source_binary_mappings[0].byte_size = 0x20U;
    assert(!outside_file.valid());

    auto overlapping = valid;
    auto second_mapping = overlapping.source_binary_mappings[0];
    second_mapping.id = "mapping:inventory-policy-overlap";
    second_mapping.output_file_offset += 0x10U;
    second_mapping.output_rva += 0x10U;
    second_mapping.output_va += 0x10U;
    overlapping.source_binary_mappings.push_back(second_mapping);
    assert(!overlapping.valid());

    auto unknown_modification = valid;
    unknown_modification.source_binary_mappings[0].modification_id = "missing";
    assert(!unknown_modification.valid());

    auto unsigned_attested = valid;
    unsigned_attested.attestation_reference.clear();
    assert(!unsigned_attested.valid());

    assert(custom_build_status_matches_project_state(
        CustomBuildStatus::compiled,
        IntegrationProjectState::compiling));
    assert(!custom_build_status_matches_project_state(
        CustomBuildStatus::released,
        IntegrationProjectState::build_validated));
    assert(custom_build_status_matches_project_state(
        CustomBuildStatus::released,
        IntegrationProjectState::maintaining));
    return 0;
}
