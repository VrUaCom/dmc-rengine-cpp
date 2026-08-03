#pragma once

#include "dmc_rengine/source/integration_project.hpp"
#include "dmc_rengine/source/modification.hpp"

#include <cstdint>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace dmc::rengine::source {

enum class CustomBuildStatus {
    compiled,
    structurally_validated,
    runtime_tested,
    release_candidate,
    approved,
    released,
    revoked,
    superseded,
};

[[nodiscard]] constexpr std::string_view to_string(
    CustomBuildStatus status) noexcept {
    switch (status) {
    case CustomBuildStatus::compiled: return "compiled";
    case CustomBuildStatus::structurally_validated:
        return "structurally-validated";
    case CustomBuildStatus::runtime_tested: return "runtime-tested";
    case CustomBuildStatus::release_candidate: return "release-candidate";
    case CustomBuildStatus::approved: return "approved";
    case CustomBuildStatus::released: return "released";
    case CustomBuildStatus::revoked: return "revoked";
    case CustomBuildStatus::superseded: return "superseded";
    }
    return "compiled";
}

enum class DistributionForm {
    private_test_build,
    full_custom_executable,
    binary_delta,
    source_modification_package,
    local_reconstruction_installer,
};

[[nodiscard]] constexpr std::string_view to_string(
    DistributionForm form) noexcept {
    switch (form) {
    case DistributionForm::private_test_build:
        return "private-test-build";
    case DistributionForm::full_custom_executable:
        return "full-custom-executable";
    case DistributionForm::binary_delta: return "binary-delta";
    case DistributionForm::source_modification_package:
        return "source-modification-package";
    case DistributionForm::local_reconstruction_installer:
        return "local-reconstruction-installer";
    }
    return "private-test-build";
}

enum class AttestationState {
    none,
    unsigned_internal,
    hash_attested,
    signed_build,
    revoked,
};

[[nodiscard]] constexpr std::string_view to_string(
    AttestationState state) noexcept {
    switch (state) {
    case AttestationState::none: return "none";
    case AttestationState::unsigned_internal: return "unsigned-internal";
    case AttestationState::hash_attested: return "hash-attested";
    case AttestationState::signed_build: return "signed";
    case AttestationState::revoked: return "revoked";
    }
    return "none";
}

enum class SourceBinaryMappingConfidence {
    confirmed,
    high,
    candidate,
    research_required,
};

[[nodiscard]] constexpr std::string_view to_string(
    SourceBinaryMappingConfidence confidence) noexcept {
    switch (confidence) {
    case SourceBinaryMappingConfidence::confirmed: return "confirmed";
    case SourceBinaryMappingConfidence::high: return "high";
    case SourceBinaryMappingConfidence::candidate: return "candidate";
    case SourceBinaryMappingConfidence::research_required:
        return "research-required";
    }
    return "research-required";
}

struct IncludedModification final {
    std::string modification_id;
    std::string version;
    std::string package_manifest_sha256;
    [[nodiscard]] bool valid() const noexcept;
    friend bool operator==(
        const IncludedModification&,
        const IncludedModification&) = default;
};

struct ToolchainIdentity final {
    std::string compiler_name;
    std::string compiler_version;
    std::string linker_name;
    std::string linker_version;
    std::string target_triple;
    std::string cmake_version;
    std::vector<std::string> compile_flags;
    std::vector<std::string> link_flags;
    std::string dependency_lock_sha256;
    [[nodiscard]] bool valid() const noexcept;
};

struct PeBuildValidation final {
    bool pe32_plus{false};
    std::uint16_t machine{};
    std::uint64_t image_base{};
    std::uint32_t entry_point_rva{};
    std::uint16_t subsystem{};
    std::uint16_t section_count{};
    bool import_table_valid{false};
    bool exception_directory_valid{false};
    bool load_config_valid{false};
    bool source_mapping_complete{false};
    std::string report_sha256;
    [[nodiscard]] bool valid() const noexcept;
};

struct SourceBinaryMapping final {
    std::string id;
    std::string modification_id;
    std::string modification_version;
    std::string source_unit_id;
    std::string source_path;
    std::uint32_t source_line_begin{};
    std::uint32_t source_line_end{};
    std::string recovered_symbol_id;
    std::string recovered_symbol_name;
    std::optional<std::uint64_t> original_rva;
    std::uint64_t output_file_offset{};
    std::uint64_t output_rva{};
    std::uint64_t output_va{};
    std::uint64_t byte_size{};
    SourceBinaryMappingConfidence confidence{
        SourceBinaryMappingConfidence::research_required};
    std::vector<std::string> evidence_record_ids;
    [[nodiscard]] bool valid(
        std::uint64_t output_size,
        std::uint64_t image_base) const noexcept;
};

struct BuildTestResult final {
    std::string id;
    TestLayer layer{TestLayer::unit};
    std::string profile;
    bool mandatory{true};
    bool passed{false};
    std::string summary;
    std::string report_sha256;
    std::vector<std::string> evidence_record_ids;
    [[nodiscard]] bool valid() const noexcept;
};

struct CustomBuildIdentity final {
    std::string custom_build_id;
    std::string edition_id;
    std::string semantic_version;
    std::string integration_project_id;
    std::string game_profile;
    std::string original_exe_sha256;
    std::string source_schema_version;
    std::string source_revision;
    std::string source_tree_sha256;
    std::string executable_sha256;
    std::string prior_custom_build_id;
    std::string release_channel;
    std::string maintainer_identity;
    [[nodiscard]] bool valid() const noexcept;
};

struct CustomBuildRecord final {
    CustomBuildIdentity identity;
    CustomBuildStatus status{CustomBuildStatus::compiled};
    ToolchainIdentity toolchain;
    PeBuildValidation pe_validation;
    std::uint64_t executable_size{};
    std::vector<IncludedModification> included_modifications;
    std::vector<std::string> unified_implementation_decision_ids;
    std::vector<SourceBinaryMapping> source_binary_mappings;
    std::vector<BuildTestResult> test_results;
    std::vector<std::string> resource_package_versions;
    std::vector<std::string> known_issues;
    std::vector<std::string> credits;
    DistributionForm distribution_form{
        DistributionForm::private_test_build};
    std::string distribution_permission;
    AttestationState attestation_state{AttestationState::none};
    std::string attestation_reference;
    std::string rollback_build_id;

    [[nodiscard]] bool valid() const noexcept;
    [[nodiscard]] bool mandatory_tests_passed() const noexcept;
    [[nodiscard]] const SourceBinaryMapping* mapping_for_rva(
        std::uint64_t rva) const noexcept;
    [[nodiscard]] const IncludedModification* included_modification(
        std::string_view modification_id) const noexcept;
};

[[nodiscard]] bool custom_build_status_matches_project_state(
    CustomBuildStatus build_status,
    IntegrationProjectState project_state) noexcept;

} // namespace dmc::rengine::source
