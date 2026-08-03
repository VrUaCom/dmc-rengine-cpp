#include "dmc_rengine/source/custom_build.hpp"

#include <algorithm>
#include <cctype>
#include <limits>
#include <set>
#include <string>

namespace dmc::rengine::source {
namespace {

[[nodiscard]] bool is_lower_hex_sha256(std::string_view value) noexcept {
    return value.size() == 64U &&
           std::all_of(
               value.begin(), value.end(),
               [](unsigned char character) {
                   return std::isdigit(character) != 0 ||
                          (character >= static_cast<unsigned char>('a') &&
                           character <= static_cast<unsigned char>('f'));
               });
}

[[nodiscard]] bool unique_non_empty(
    const std::vector<std::string>& values,
    bool required) {
    if (required && values.empty()) {
        return false;
    }
    std::set<std::string, std::less<>> unique;
    for (const auto& value : values) {
        if (value.empty() || !unique.insert(value).second) {
            return false;
        }
    }
    return true;
}

[[nodiscard]] bool status_requires_passed_tests(
    CustomBuildStatus status) noexcept {
    return status != CustomBuildStatus::compiled;
}

[[nodiscard]] bool status_requires_runtime_smoke(
    CustomBuildStatus status) noexcept {
    return status == CustomBuildStatus::runtime_tested ||
           status == CustomBuildStatus::release_candidate ||
           status == CustomBuildStatus::approved ||
           status == CustomBuildStatus::released;
}

[[nodiscard]] bool has_passed_mandatory_layer(
    const std::vector<BuildTestResult>& results,
    TestLayer layer) {
    return std::any_of(
        results.begin(), results.end(),
        [layer](const BuildTestResult& result) {
            return result.layer == layer && result.mandatory && result.passed;
        });
}

[[nodiscard]] bool ranges_overlap(
    std::uint64_t left_offset,
    std::uint64_t left_size,
    std::uint64_t right_offset,
    std::uint64_t right_size) noexcept {
    if (left_offset <= right_offset) {
        return right_offset - left_offset < left_size;
    }
    return left_offset - right_offset < right_size;
}

} // namespace

bool IncludedModification::valid() const noexcept {
    return !modification_id.empty() && !version.empty() &&
           is_lower_hex_sha256(package_manifest_sha256);
}

bool ToolchainIdentity::valid() const noexcept {
    return !compiler_name.empty() && !compiler_version.empty() &&
           !linker_name.empty() && !linker_version.empty() &&
           !target_triple.empty() && !cmake_version.empty() &&
           unique_non_empty(compile_flags, true) &&
           unique_non_empty(link_flags, true) &&
           is_lower_hex_sha256(dependency_lock_sha256);
}

bool PeBuildValidation::valid() const noexcept {
    return pe32_plus && machine != 0U && image_base != 0U &&
           entry_point_rva != 0U && subsystem != 0U &&
           section_count != 0U && import_table_valid &&
           exception_directory_valid && load_config_valid &&
           source_mapping_complete && is_lower_hex_sha256(report_sha256);
}

bool SourceBinaryMapping::valid(
    std::uint64_t output_size,
    std::uint64_t image_base) const noexcept {
    if (id.empty() || modification_id.empty() ||
        modification_version.empty() || source_unit_id.empty() ||
        source_path.empty() || source_line_begin == 0U ||
        source_line_end < source_line_begin ||
        recovered_symbol_id.empty() || recovered_symbol_name.empty() ||
        byte_size == 0U || output_file_offset >= output_size ||
        byte_size > output_size - output_file_offset ||
        output_rva > std::numeric_limits<std::uint64_t>::max() - image_base ||
        output_va != image_base + output_rva ||
        output_rva > std::numeric_limits<std::uint64_t>::max() - byte_size ||
        !unique_non_empty(evidence_record_ids, true)) {
        return false;
    }
    return true;
}

bool BuildTestResult::valid() const noexcept {
    return !id.empty() && !profile.empty() && !summary.empty() &&
           is_lower_hex_sha256(report_sha256) &&
           unique_non_empty(evidence_record_ids, false);
}

bool CustomBuildIdentity::valid() const noexcept {
    return !custom_build_id.empty() && !semantic_version.empty() &&
           !integration_project_id.empty() && !game_profile.empty() &&
           is_lower_hex_sha256(original_exe_sha256) &&
           !source_schema_version.empty() && !source_revision.empty() &&
           is_lower_hex_sha256(source_tree_sha256) &&
           is_lower_hex_sha256(executable_sha256) &&
           original_exe_sha256 != executable_sha256 &&
           !release_channel.empty() && !maintainer_identity.empty();
}

bool CustomBuildRecord::valid() const noexcept {
    if (!identity.valid() || !toolchain.valid() || !pe_validation.valid() ||
        executable_size == 0U || included_modifications.empty() ||
        source_binary_mappings.empty() || test_results.empty() ||
        distribution_permission.empty() ||
        !unique_non_empty(resource_package_versions, false) ||
        !unique_non_empty(known_issues, false) ||
        !unique_non_empty(credits, true) ||
        !unique_non_empty(unified_implementation_decision_ids, false)) {
        return false;
    }
    if (status_requires_passed_tests(status) && !mandatory_tests_passed()) {
        return false;
    }
    if (status != CustomBuildStatus::compiled &&
        !has_passed_mandatory_layer(test_results, TestLayer::pe_structure)) {
        return false;
    }
    if (status_requires_runtime_smoke(status) &&
        !has_passed_mandatory_layer(test_results, TestLayer::runtime_smoke)) {
        return false;
    }
    if ((attestation_state == AttestationState::hash_attested ||
         attestation_state == AttestationState::signed_build ||
         attestation_state == AttestationState::revoked) &&
        attestation_reference.empty()) {
        return false;
    }
    if (status == CustomBuildStatus::revoked &&
        attestation_state != AttestationState::revoked) {
        return false;
    }
    if (status == CustomBuildStatus::released &&
        (attestation_state == AttestationState::none ||
         attestation_state == AttestationState::unsigned_internal ||
         attestation_state == AttestationState::revoked)) {
        return false;
    }

    std::set<std::pair<std::string, std::string>> modifications;
    for (const auto& modification : included_modifications) {
        if (!modification.valid() ||
            !modifications.emplace(
                modification.modification_id,
                modification.version).second) {
            return false;
        }
    }

    std::set<std::string, std::less<>> mapping_ids;
    for (std::size_t index = 0U;
         index < source_binary_mappings.size();
         ++index) {
        const auto& mapping = source_binary_mappings[index];
        if (!mapping.valid(executable_size, pe_validation.image_base) ||
            !mapping_ids.insert(mapping.id).second ||
            modifications.find({
                mapping.modification_id,
                mapping.modification_version}) == modifications.end()) {
            return false;
        }
        for (std::size_t other_index = index + 1U;
             other_index < source_binary_mappings.size();
             ++other_index) {
            const auto& other = source_binary_mappings[other_index];
            if (ranges_overlap(
                    mapping.output_file_offset,
                    mapping.byte_size,
                    other.output_file_offset,
                    other.byte_size) ||
                ranges_overlap(
                    mapping.output_rva,
                    mapping.byte_size,
                    other.output_rva,
                    other.byte_size)) {
                return false;
            }
        }
    }

    std::set<std::string, std::less<>> test_ids;
    for (const auto& result : test_results) {
        if (!result.valid() || !test_ids.insert(result.id).second) {
            return false;
        }
    }
    return true;
}

bool CustomBuildRecord::mandatory_tests_passed() const noexcept {
    return std::all_of(
        test_results.begin(), test_results.end(),
        [](const BuildTestResult& result) {
            return !result.mandatory || result.passed;
        });
}

const SourceBinaryMapping* CustomBuildRecord::mapping_for_rva(
    std::uint64_t rva) const noexcept {
    const auto iterator = std::find_if(
        source_binary_mappings.begin(), source_binary_mappings.end(),
        [rva](const SourceBinaryMapping& mapping) {
            return rva >= mapping.output_rva &&
                   rva - mapping.output_rva < mapping.byte_size;
        });
    return iterator == source_binary_mappings.end() ? nullptr : &*iterator;
}

const IncludedModification* CustomBuildRecord::included_modification(
    std::string_view modification_id) const noexcept {
    const auto iterator = std::find_if(
        included_modifications.begin(), included_modifications.end(),
        [modification_id](const IncludedModification& modification) {
            return modification.modification_id == modification_id;
        });
    return iterator == included_modifications.end() ? nullptr : &*iterator;
}

bool custom_build_status_matches_project_state(
    CustomBuildStatus build_status,
    IntegrationProjectState project_state) noexcept {
    switch (build_status) {
    case CustomBuildStatus::compiled:
        return project_state == IntegrationProjectState::compiling ||
               project_state == IntegrationProjectState::build_validated ||
               project_state == IntegrationProjectState::runtime_testing ||
               project_state == IntegrationProjectState::release_candidate ||
               project_state == IntegrationProjectState::approved ||
               project_state == IntegrationProjectState::released ||
               project_state == IntegrationProjectState::maintaining;
    case CustomBuildStatus::structurally_validated:
        return project_state == IntegrationProjectState::build_validated ||
               project_state == IntegrationProjectState::runtime_testing ||
               project_state == IntegrationProjectState::release_candidate ||
               project_state == IntegrationProjectState::approved ||
               project_state == IntegrationProjectState::released ||
               project_state == IntegrationProjectState::maintaining;
    case CustomBuildStatus::runtime_tested:
    case CustomBuildStatus::release_candidate:
        return project_state == IntegrationProjectState::release_candidate ||
               project_state == IntegrationProjectState::approved ||
               project_state == IntegrationProjectState::released ||
               project_state == IntegrationProjectState::maintaining;
    case CustomBuildStatus::approved:
        return project_state == IntegrationProjectState::approved ||
               project_state == IntegrationProjectState::released ||
               project_state == IntegrationProjectState::maintaining;
    case CustomBuildStatus::released:
        return project_state == IntegrationProjectState::released ||
               project_state == IntegrationProjectState::maintaining;
    case CustomBuildStatus::revoked:
    case CustomBuildStatus::superseded:
        return project_state == IntegrationProjectState::released ||
               project_state == IntegrationProjectState::maintaining ||
               project_state == IntegrationProjectState::superseded ||
               project_state == IntegrationProjectState::withdrawn;
    }
    return false;
}

} // namespace dmc::rengine::source
