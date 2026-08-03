#include "dmc_rengine/source/modification.hpp"

#include <algorithm>
#include <cctype>
#include <set>
#include <string_view>

namespace dmc::rengine::source {
namespace {

[[nodiscard]] bool is_lower_hex_sha256(std::string_view value) noexcept {
    if (value.size() != 64U) {
        return false;
    }
    return std::all_of(value.begin(), value.end(), [](unsigned char character) {
        return std::isdigit(character) != 0 ||
               (character >= static_cast<unsigned char>('a') &&
                character <= static_cast<unsigned char>('f'));
    });
}

[[nodiscard]] bool unique_non_empty(
    const std::vector<std::string>& values,
    bool require_non_empty) {
    if (require_non_empty && values.empty()) {
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

template <typename T, typename IdAccessor>
[[nodiscard]] bool unique_valid_objects(
    const std::vector<T>& values,
    bool require_non_empty,
    IdAccessor id_accessor) {
    if (require_non_empty && values.empty()) {
        return false;
    }
    std::set<std::string, std::less<>> unique;
    for (const auto& value : values) {
        const auto& id = id_accessor(value);
        if (!value.valid() || id.empty() || !unique.insert(id).second) {
            return false;
        }
    }
    return true;
}

} // namespace

bool SourceBaselineIdentity::valid() const noexcept {
    return !game_profile.empty() &&
           is_lower_hex_sha256(original_exe_sha256) &&
           !source_schema_version.empty() &&
           !source_revision.empty() &&
           !compiler_profile.empty();
}

bool SourceSymbolRef::valid() const noexcept {
    return !id.empty() && !recovered_name.empty() &&
           unique_non_empty(evidence_record_ids, false);
}

bool SourceChangeUnit::valid() const noexcept {
    return !id.empty() && !source_path.empty() &&
           is_lower_hex_sha256(baseline_sha256) &&
           is_lower_hex_sha256(result_sha256) &&
           baseline_sha256 != result_sha256 &&
           !semantic_intent.empty() &&
           unique_valid_objects(
               affected_symbols,
               true,
               [](const SourceSymbolRef& symbol) -> const std::string& {
                   return symbol.id;
               });
}

bool ModificationDependency::valid() const noexcept {
    return !modification_id.empty() &&
           !version_constraint.empty() &&
           !rationale.empty();
}

bool ModificationConflictDeclaration::valid() const noexcept {
    return !modification_id.empty() && !rationale.empty();
}

bool ModificationTestRequirement::valid() const noexcept {
    return !id.empty() && !profile.empty() && !expected_result.empty();
}

bool ModificationProvenance::valid() const noexcept {
    return unique_non_empty(authors, true) &&
           unique_non_empty(contribution_ledger_refs, false) &&
           unique_non_empty(reviewers, false) &&
           !licence.empty() &&
           !redistribution_permission.empty();
}

bool SourceModificationPackage::valid() const noexcept {
    if (id.empty() || version.empty() || title.empty() || description.empty() ||
        !baseline.valid() || semantic_intent.empty() ||
        save_compatibility.empty() || migration_notes.empty() ||
        rollback_instructions.empty() || !provenance.valid()) {
        return false;
    }
    if (!unique_valid_objects(
            change_units,
            true,
            [](const SourceChangeUnit& unit) -> const std::string& {
                return unit.id;
            }) ||
        !unique_valid_objects(
            dependencies,
            false,
            [](const ModificationDependency& dependency)
                -> const std::string& {
                return dependency.modification_id;
            }) ||
        !unique_valid_objects(
            declared_conflicts,
            false,
            [](const ModificationConflictDeclaration& conflict)
                -> const std::string& {
                return conflict.modification_id;
            }) ||
        !unique_valid_objects(
            tests,
            true,
            [](const ModificationTestRequirement& test)
                -> const std::string& {
                return test.id;
            })) {
        return false;
    }
    if (!unique_non_empty(configuration_keys, false) ||
        !unique_non_empty(build_requirements, true) ||
        !unique_non_empty(evidence_record_ids, true) ||
        !unique_non_empty(known_risks, true)) {
        return false;
    }
    const auto self_dependency = std::any_of(
        dependencies.begin(), dependencies.end(),
        [this](const ModificationDependency& dependency) {
            return dependency.modification_id == id;
        });
    const auto self_conflict = std::any_of(
        declared_conflicts.begin(), declared_conflicts.end(),
        [this](const ModificationConflictDeclaration& conflict) {
            return conflict.modification_id == id;
        });
    return !self_dependency && !self_conflict;
}

bool version_constraint_matches(
    std::string_view constraint,
    std::string_view version) noexcept {
    return constraint == "*" || constraint == version;
}

} // namespace dmc::rengine::source
