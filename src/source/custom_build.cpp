#include "dmc_rengine/source/custom_build.hpp"

#include <algorithm>
#include <cctype>
#include <limits>
#include <set>
#include <string>

namespace dmc::rengine::source {
namespace {
[[nodiscard]] bool is_lower_hex_sha256(std::string_view value) noexcept {
    return value.size() == 64U && std::all_of(value.begin(), value.end(), [](unsigned char c) {
        return std::isdigit(c) != 0 || (c >= static_cast<unsigned char>('a') && c <= static_cast<unsigned char>('f'));
    });
}
[[nodiscard]] bool unique_non_empty(const std::vector<std::string>& values, bool required) {
    if (required && values.empty()) return false;
    std::set<std::string, std::less<>> unique;
    for (const auto& value : values) if (value.empty() || !unique.insert(value).second) return false;
    return true;
}
[[nodiscard]] bool status_requires_validated_pe(CustomBuildStatus status) noexcept {
    return status != CustomBuildStatus::compiled;
}
[[nodiscard]] bool status_requires_passed_tests(CustomBuildStatus status) noexcept {
    return status != CustomBuildStatus::compiled && status != CustomBuildStatus::revoked && status != CustomBuildStatus::superseded;
}
[[nodiscard]] bool ranges_overlap(std::uint64_t a, std::uint64_t as, std::uint64_t b, std::uint64_t bs) noexcept {
    return a < b + bs && b < a + as;
}
} // namespace

bool IncludedModification::valid() const noexcept {
    return !modification_id.empty() && !version.empty() && is_lower_hex_sha256(package_manifest_sha256);
}
bool ToolchainIdentity::valid() const noexcept {
    return !compiler_name.empty() && !compiler_version.empty() && !linker_name.empty() && !linker_version.empty() &&
           !target_triple.empty() && !cmake_version.empty() && unique_non_empty(compile_flags, true) &&
           unique_non_empty(link_flags, true) && is_lower_hex_sha256(dependency_lock_sha256);
}
bool PeBuildValidation::valid() const noexcept {
    return pe32_plus && machine != 0U && image_base != 0U && entry_point_rva != 0U && subsystem != 0U &&
           section_count != 0U && import_table_valid && exception_directory_valid && load_config_valid &&
           is_lower_hex_sha256(report_sha256);
}
bool SourceBinaryMapping::valid(std::uint64_t output_size, std::uint64_t image_base) const noexcept {
    return !id.empty() && !modification_id.empty() && !modification_version.empty() && !source_unit_id.empty() &&
           !source_path.empty() && source_line_begin != 0U && source_line_end >= source_line_begin &&
           !recovered_symbol_id.empty() && !recovered_symbol_name.empty() && byte_size != 0U &&
           output_file_offset < output_size && byte_size <= output_size - output_file_offset &&
           output_rva <= std::numeric_limits<std::uint64_t>::max() - image_base && output_va == image_base + output_rva &&
           unique_non_empty(evidence_record_ids, true);
}
bool BuildTestResult::valid() const noexcept {
    return !id.empty() && !profile.empty() && !summary.empty() && is_lower_hex_sha256(report_sha256) &&
           unique_non_empty(evidence_record_ids, false);
}
bool CustomBuildIdentity::valid() const noexcept {
    return !custom_build_id.empty() && !semantic_version.empty() && !integration_project_id.empty() && !game_profile.empty() &&
           is_lower_hex_sha256(original_exe_sha256) && !source_schema_version.empty() && !source_revision.empty() &&
           is_lower_hex_sha256(source_tree_sha256) && is_lower_hex_sha256(executable_sha256) &&
           original_exe_sha256 != executable_sha256 && !release_channel.empty() && !maintainer_identity.empty();
}
bool CustomBuildRecord::valid() const noexcept {
    if (!identity.valid() || !toolchain.valid() || executable_size == 0U || included_modifications.empty() ||
        source_binary_mappings.empty() || test_results.empty() || distribution_permission.empty() ||
        !unique_non_empty(resource_package_versions, false) || !unique_non_empty(known_issues, false) ||
        !unique_non_empty(credits, true) || !unique_non_empty(unified_implementation_decision_ids, false)) return false;
    if (status_requires_validated_pe(status) && !pe_validation.valid()) return false;
    if (status_requires_passed_tests(status) && !mandatory_tests_passed()) return false;
    if ((attestation_state == AttestationState::hash_attested || attestation_state == AttestationState::signed ||
         attestation_state == AttestationState::revoked) && attestation_reference.empty()) return false;
    if (status == CustomBuildStatus::revoked && attestation_state != AttestationState::revoked) return false;

    std::set<std::pair<std::string, std::string>> modifications;
    for (const auto& m : included_modifications)
        if (!m.valid() || !modifications.emplace(m.modification_id, m.version).second) return false;

    std::set<std::string, std::less<>> mapping_ids;
    for (std::size_t i = 0U; i < source_binary_mappings.size(); ++i) {
        const auto& m = source_binary_mappings[i];
        if (!m.valid(executable_size, pe_validation.image_base) || !mapping_ids.insert(m.id).second ||
            modifications.find({m.modification_id, m.modification_version}) == modifications.end()) return false;
        for (std::size_t j = i + 1U; j < source_binary_mappings.size(); ++j) {
            const auto& n = source_binary_mappings[j];
            if (ranges_overlap(m.output_file_offset, m.byte_size, n.output_file_offset, n.byte_size) ||
                ranges_overlap(m.output_rva, m.byte_size, n.output_rva, n.byte_size)) return false;
        }
    }
    std::set<std::string, std::less<>> test_ids;
    for (const auto& r : test_results) if (!r.valid() || !test_ids.insert(r.id).second) return false;
    return true;
}
bool CustomBuildRecord::mandatory_tests_passed() const noexcept {
    return std::all_of(test_results.begin(), test_results.end(), [](const BuildTestResult& r) { return !r.mandatory || r.passed; });
}
const SourceBinaryMapping* CustomBuildRecord::mapping_for_rva(std::uint64_t rva) const noexcept {
    const auto it = std::find_if(source_binary_mappings.begin(), source_binary_mappings.end(), [rva](const SourceBinaryMapping& m) {
        return rva >= m.output_rva && rva - m.output_rva < m.byte_size;
    });
    return it == source_binary_mappings.end() ? nullptr : &*it;
}
const IncludedModification* CustomBuildRecord::included_modification(std::string_view modification_id) const noexcept {
    const auto it = std::find_if(included_modifications.begin(), included_modifications.end(), [modification_id](const IncludedModification& m) {
        return m.modification_id == modification_id;
    });
    return it == included_modifications.end() ? nullptr : &*it;
}
bool custom_build_status_matches_project_state(CustomBuildStatus s, IntegrationProjectState p) noexcept {
    switch (s) {
    case CustomBuildStatus::compiled:
        return p == IntegrationProjectState::compiling || p == IntegrationProjectState::build_validated || p == IntegrationProjectState::runtime_testing || p == IntegrationProjectState::release_candidate || p == IntegrationProjectState::approved || p == IntegrationProjectState::released || p == IntegrationProjectState::maintaining;
    case CustomBuildStatus::structurally_validated:
        return p == IntegrationProjectState::build_validated || p == IntegrationProjectState::runtime_testing || p == IntegrationProjectState::release_candidate || p == IntegrationProjectState::approved || p == IntegrationProjectState::released || p == IntegrationProjectState::maintaining;
    case CustomBuildStatus::runtime_tested:
    case CustomBuildStatus::release_candidate:
        return p == IntegrationProjectState::release_candidate || p == IntegrationProjectState::approved || p == IntegrationProjectState::released || p == IntegrationProjectState::maintaining;
    case CustomBuildStatus::approved:
        return p == IntegrationProjectState::approved || p == IntegrationProjectState::released || p == IntegrationProjectState::maintaining;
    case CustomBuildStatus::released:
        return p == IntegrationProjectState::released || p == IntegrationProjectState::maintaining;
    case CustomBuildStatus::revoked:
    case CustomBuildStatus::superseded:
        return p == IntegrationProjectState::released || p == IntegrationProjectState::maintaining || p == IntegrationProjectState::superseded || p == IntegrationProjectState::withdrawn;
    }
    return false;
}
} // namespace dmc::rengine::source
