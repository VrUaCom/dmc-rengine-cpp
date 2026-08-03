#pragma once

#include "dmc_rengine/source/modification.hpp"

#include <cstddef>
#include <string>
#include <string_view>
#include <vector>

namespace dmc::rengine::source {

enum class IntegrationProjectState {
    draft,
    modifications_selected,
    dependency_checked,
    conflict_analysis,
    integrating,
    source_validated,
    compiling,
    build_validated,
    runtime_testing,
    release_candidate,
    approved,
    released,
    maintaining,
    blocked_by_conflict,
    build_failed,
    test_failed,
    rights_hold,
    security_hold,
    superseded,
    withdrawn,
};

[[nodiscard]] constexpr std::string_view to_string(
    IntegrationProjectState state) noexcept {
    switch (state) {
    case IntegrationProjectState::draft: return "DRAFT";
    case IntegrationProjectState::modifications_selected:
        return "MODIFICATIONS_SELECTED";
    case IntegrationProjectState::dependency_checked:
        return "DEPENDENCY_CHECKED";
    case IntegrationProjectState::conflict_analysis:
        return "CONFLICT_ANALYSIS";
    case IntegrationProjectState::integrating: return "INTEGRATING";
    case IntegrationProjectState::source_validated:
        return "SOURCE_VALIDATED";
    case IntegrationProjectState::compiling: return "COMPILING";
    case IntegrationProjectState::build_validated:
        return "BUILD_VALIDATED";
    case IntegrationProjectState::runtime_testing:
        return "RUNTIME_TESTING";
    case IntegrationProjectState::release_candidate:
        return "RELEASE_CANDIDATE";
    case IntegrationProjectState::approved: return "APPROVED";
    case IntegrationProjectState::released: return "RELEASED";
    case IntegrationProjectState::maintaining: return "MAINTAINING";
    case IntegrationProjectState::blocked_by_conflict:
        return "BLOCKED_BY_CONFLICT";
    case IntegrationProjectState::build_failed: return "BUILD_FAILED";
    case IntegrationProjectState::test_failed: return "TEST_FAILED";
    case IntegrationProjectState::rights_hold: return "RIGHTS_HOLD";
    case IntegrationProjectState::security_hold: return "SECURITY_HOLD";
    case IntegrationProjectState::superseded: return "SUPERSEDED";
    case IntegrationProjectState::withdrawn: return "WITHDRAWN";
    }
    return "DRAFT";
}

[[nodiscard]] constexpr bool is_failure_state(
    IntegrationProjectState state) noexcept {
    switch (state) {
    case IntegrationProjectState::blocked_by_conflict:
    case IntegrationProjectState::build_failed:
    case IntegrationProjectState::test_failed:
    case IntegrationProjectState::rights_hold:
    case IntegrationProjectState::security_hold:
    case IntegrationProjectState::superseded:
    case IntegrationProjectState::withdrawn:
        return true;
    case IntegrationProjectState::draft:
    case IntegrationProjectState::modifications_selected:
    case IntegrationProjectState::dependency_checked:
    case IntegrationProjectState::conflict_analysis:
    case IntegrationProjectState::integrating:
    case IntegrationProjectState::source_validated:
    case IntegrationProjectState::compiling:
    case IntegrationProjectState::build_validated:
    case IntegrationProjectState::runtime_testing:
    case IntegrationProjectState::release_candidate:
    case IntegrationProjectState::approved:
    case IntegrationProjectState::released:
    case IntegrationProjectState::maintaining:
        return false;
    }
    return true;
}

enum class DependencyIssueKind {
    missing_modification,
    version_mismatch,
    unsupported_constraint,
};

[[nodiscard]] constexpr std::string_view to_string(
    DependencyIssueKind kind) noexcept {
    switch (kind) {
    case DependencyIssueKind::missing_modification:
        return "missing-modification";
    case DependencyIssueKind::version_mismatch:
        return "version-mismatch";
    case DependencyIssueKind::unsupported_constraint:
        return "unsupported-constraint";
    }
    return "missing-modification";
}

struct DependencyIssue final {
    std::string owner_modification_id;
    std::string dependency_modification_id;
    std::string required_version;
    std::string actual_version;
    DependencyIssueKind kind{DependencyIssueKind::missing_modification};
    bool blocking{true};
    std::string rationale;

    [[nodiscard]] bool valid() const noexcept;
};

struct DependencyReport final {
    std::vector<DependencyIssue> issues;

    [[nodiscard]] bool passed() const noexcept;
    [[nodiscard]] std::size_t blocking_count() const noexcept;
};

enum class ConflictOrigin {
    declared,
    symbol_overlap,
    source_path_overlap,
    configuration_overlap,
};

[[nodiscard]] constexpr std::string_view to_string(
    ConflictOrigin origin) noexcept {
    switch (origin) {
    case ConflictOrigin::declared: return "declared";
    case ConflictOrigin::symbol_overlap: return "symbol-overlap";
    case ConflictOrigin::source_path_overlap: return "source-path-overlap";
    case ConflictOrigin::configuration_overlap:
        return "configuration-overlap";
    }
    return "declared";
}

enum class ConflictResolutionStatus {
    unresolved,
    resolved,
    rejected_modification,
    configuration_dependent,
    postponed_for_research,
};

[[nodiscard]] constexpr std::string_view to_string(
    ConflictResolutionStatus status) noexcept {
    switch (status) {
    case ConflictResolutionStatus::unresolved: return "unresolved";
    case ConflictResolutionStatus::resolved: return "resolved";
    case ConflictResolutionStatus::rejected_modification:
        return "rejected-modification";
    case ConflictResolutionStatus::configuration_dependent:
        return "configuration-dependent";
    case ConflictResolutionStatus::postponed_for_research:
        return "postponed-for-research";
    }
    return "unresolved";
}

struct ConflictRecord final {
    std::string id;
    std::string left_modification_id;
    std::string right_modification_id;
    ConflictClass conflict_class{ConflictClass::textual_source};
    ConflictOrigin origin{ConflictOrigin::declared};
    std::string subject;
    std::string rationale;
    bool blocking{true};
    ConflictResolutionStatus status{
        ConflictResolutionStatus::unresolved};
    std::string decision_record_id;
    std::string resolution;

    [[nodiscard]] bool valid() const noexcept;
    [[nodiscard]] bool unresolved_blocking() const noexcept;
};

struct DecisionRecord final {
    std::string id;
    std::string title;
    std::string rationale;
    std::vector<std::string> authors;
    std::vector<std::string> evidence_record_ids;
    std::vector<std::string> affected_conflict_ids;

    [[nodiscard]] bool valid() const noexcept;
};

enum class IntegrationGateKind {
    dependency_check,
    conflict_analysis,
    source_validation,
    compilation,
    build_validation,
    runtime_testing,
    approval,
    release,
};

[[nodiscard]] constexpr std::string_view to_string(
    IntegrationGateKind gate) noexcept {
    switch (gate) {
    case IntegrationGateKind::dependency_check:
        return "dependency-check";
    case IntegrationGateKind::conflict_analysis:
        return "conflict-analysis";
    case IntegrationGateKind::source_validation:
        return "source-validation";
    case IntegrationGateKind::compilation: return "compilation";
    case IntegrationGateKind::build_validation:
        return "build-validation";
    case IntegrationGateKind::runtime_testing:
        return "runtime-testing";
    case IntegrationGateKind::approval: return "approval";
    case IntegrationGateKind::release: return "release";
    }
    return "dependency-check";
}

struct IntegrationGateEvidence final {
    std::string id;
    IntegrationGateKind kind{IntegrationGateKind::dependency_check};
    bool passed{false};
    std::string summary;
    std::vector<std::string> evidence_record_ids;

    [[nodiscard]] bool valid() const noexcept;
};

class IntegrationProject final {
public:
    IntegrationProject(
        std::string id,
        std::string title,
        SourceBaselineIdentity baseline);

    [[nodiscard]] const std::string& id() const noexcept;
    [[nodiscard]] const std::string& title() const noexcept;
    [[nodiscard]] const SourceBaselineIdentity& baseline() const noexcept;
    [[nodiscard]] IntegrationProjectState state() const noexcept;
    [[nodiscard]] const std::string& failure_reason() const noexcept;
    [[nodiscard]] const std::vector<std::string>& maintainers() const noexcept;
    [[nodiscard]] const std::string& approval_policy() const noexcept;
    [[nodiscard]] const std::vector<std::string>& build_profiles() const noexcept;
    [[nodiscard]] const std::vector<std::string>& test_matrix() const noexcept;
    [[nodiscard]] const std::vector<SourceModificationPackage>&
        selected_modifications() const noexcept;
    [[nodiscard]] const DependencyReport& dependency_report() const noexcept;
    [[nodiscard]] const std::vector<ConflictRecord>& conflicts() const noexcept;
    [[nodiscard]] const std::vector<DecisionRecord>& decisions() const noexcept;
    [[nodiscard]] const std::vector<IntegrationGateEvidence>& gates() const noexcept;

    [[nodiscard]] bool configure_governance(
        std::vector<std::string> maintainers,
        std::string approval_policy,
        std::vector<std::string> build_profiles,
        std::vector<std::string> test_matrix);
    [[nodiscard]] bool select_modification(SourceModificationPackage package);
    [[nodiscard]] DependencyReport analyze_dependencies();
    [[nodiscard]] std::vector<ConflictRecord> analyze_conflicts();
    [[nodiscard]] bool resolve_conflict(
        std::string_view conflict_id,
        ConflictResolutionStatus status,
        std::string resolution,
        DecisionRecord decision);
    [[nodiscard]] bool record_gate(IntegrationGateEvidence gate);
    [[nodiscard]] bool transition(IntegrationProjectState next);
    [[nodiscard]] bool fail(
        IntegrationProjectState failure_state,
        std::string reason);

    [[nodiscard]] bool valid() const noexcept;
    [[nodiscard]] std::size_t unresolved_blocking_conflict_count() const noexcept;
    [[nodiscard]] const SourceModificationPackage* find_modification(
        std::string_view modification_id) const noexcept;
    [[nodiscard]] const IntegrationGateEvidence* gate(
        IntegrationGateKind kind) const noexcept;

private:
    [[nodiscard]] bool transition_allowed(
        IntegrationProjectState next) const noexcept;

    std::string id_;
    std::string title_;
    SourceBaselineIdentity baseline_;
    IntegrationProjectState state_{IntegrationProjectState::draft};
    std::string failure_reason_;
    std::vector<std::string> maintainers_;
    std::string approval_policy_;
    std::vector<std::string> build_profiles_;
    std::vector<std::string> test_matrix_;
    std::vector<SourceModificationPackage> selected_modifications_;
    DependencyReport dependency_report_;
    std::vector<ConflictRecord> conflicts_;
    std::vector<DecisionRecord> decisions_;
    std::vector<IntegrationGateEvidence> gates_;
};

} // namespace dmc::rengine::source
