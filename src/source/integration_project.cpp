#include "dmc_rengine/source/integration_project.hpp"

#include <algorithm>
#include <map>
#include <set>
#include <string>
#include <tuple>
#include <utility>

namespace dmc::rengine::source {
namespace {

[[nodiscard]] bool unique_non_empty(
    const std::vector<std::string>& values,
    bool require_values) {
    if (require_values && values.empty()) {
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

[[nodiscard]] bool constraint_supported(std::string_view constraint) noexcept {
    if (constraint == "*") {
        return true;
    }
    return !constraint.empty() &&
           constraint.find_first_of("<>=^~ ") == std::string_view::npos;
}

[[nodiscard]] std::pair<std::string, std::string> ordered_pair(
    std::string left,
    std::string right) {
    if (right < left) {
        std::swap(left, right);
    }
    return {std::move(left), std::move(right)};
}

[[nodiscard]] ConflictClass symbol_conflict_class(
    SourceSymbolKind kind) noexcept {
    switch (kind) {
    case SourceSymbolKind::function:
        return ConflictClass::same_function_semantic;
    case SourceSymbolKind::class_type:
        return ConflictClass::class_layout;
    case SourceSymbolKind::global:
        return ConflictClass::global_state;
    case SourceSymbolKind::source_file:
        return ConflictClass::textual_source;
    case SourceSymbolKind::initialization_unit:
        return ConflictClass::initialization_order;
    case SourceSymbolKind::resource_binding:
        return ConflictClass::resource_format;
    case SourceSymbolKind::save_schema:
        return ConflictClass::save_format;
    }
    return ConflictClass::textual_source;
}

[[nodiscard]] std::string conflict_id(
    std::string_view project_id,
    std::string left,
    std::string right,
    ConflictOrigin origin,
    ConflictClass conflict_class,
    std::string_view subject) {
    auto ordered = ordered_pair(std::move(left), std::move(right));
    return "conflict:" + std::string(project_id) + ':' + ordered.first + ':' +
           ordered.second + ':' + std::string(to_string(origin)) + ':' +
           std::string(to_string(conflict_class)) + ':' +
           std::string(subject);
}

[[nodiscard]] bool decision_contains_conflict(
    const DecisionRecord& decision,
    std::string_view conflict_id_value) {
    return std::find(
               decision.affected_conflict_ids.begin(),
               decision.affected_conflict_ids.end(),
               conflict_id_value) != decision.affected_conflict_ids.end();
}

} // namespace

bool DependencyIssue::valid() const noexcept {
    return !owner_modification_id.empty() &&
           !dependency_modification_id.empty() &&
           !required_version.empty() && !rationale.empty();
}

bool DependencyReport::passed() const noexcept {
    return blocking_count() == 0U;
}

std::size_t DependencyReport::blocking_count() const noexcept {
    return static_cast<std::size_t>(std::count_if(
        issues.begin(), issues.end(),
        [](const DependencyIssue& issue) {
            return issue.blocking;
        }));
}

bool ConflictRecord::valid() const noexcept {
    if (id.empty() || left_modification_id.empty() ||
        right_modification_id.empty() ||
        left_modification_id == right_modification_id ||
        subject.empty() || rationale.empty()) {
        return false;
    }
    if (status == ConflictResolutionStatus::unresolved) {
        return decision_record_id.empty() && resolution.empty();
    }
    return !decision_record_id.empty() && !resolution.empty();
}

bool ConflictRecord::unresolved_blocking() const noexcept {
    return blocking &&
           (status == ConflictResolutionStatus::unresolved ||
            status == ConflictResolutionStatus::postponed_for_research);
}

bool DecisionRecord::valid() const noexcept {
    return !id.empty() && !title.empty() && !rationale.empty() &&
           unique_non_empty(authors, true) &&
           unique_non_empty(evidence_record_ids, false) &&
           unique_non_empty(affected_conflict_ids, true);
}

bool IntegrationGateEvidence::valid() const noexcept {
    return !id.empty() && !summary.empty() &&
           unique_non_empty(evidence_record_ids, false);
}

IntegrationProject::IntegrationProject(
    std::string id,
    std::string title,
    SourceBaselineIdentity baseline)
    : id_(std::move(id)),
      title_(std::move(title)),
      baseline_(std::move(baseline)) {}

const std::string& IntegrationProject::id() const noexcept {
    return id_;
}

const std::string& IntegrationProject::title() const noexcept {
    return title_;
}

const SourceBaselineIdentity& IntegrationProject::baseline() const noexcept {
    return baseline_;
}

IntegrationProjectState IntegrationProject::state() const noexcept {
    return state_;
}

const std::string& IntegrationProject::failure_reason() const noexcept {
    return failure_reason_;
}

const std::vector<std::string>& IntegrationProject::maintainers() const noexcept {
    return maintainers_;
}

const std::string& IntegrationProject::approval_policy() const noexcept {
    return approval_policy_;
}

const std::vector<std::string>& IntegrationProject::build_profiles() const noexcept {
    return build_profiles_;
}

const std::vector<std::string>& IntegrationProject::test_matrix() const noexcept {
    return test_matrix_;
}

const std::vector<SourceModificationPackage>&
IntegrationProject::selected_modifications() const noexcept {
    return selected_modifications_;
}

const DependencyReport& IntegrationProject::dependency_report() const noexcept {
    return dependency_report_;
}

const std::vector<ConflictRecord>& IntegrationProject::conflicts() const noexcept {
    return conflicts_;
}

const std::vector<DecisionRecord>& IntegrationProject::decisions() const noexcept {
    return decisions_;
}

const std::vector<IntegrationGateEvidence>& IntegrationProject::gates() const noexcept {
    return gates_;
}

bool IntegrationProject::configure_governance(
    std::vector<std::string> maintainers,
    std::string approval_policy,
    std::vector<std::string> build_profiles,
    std::vector<std::string> test_matrix) {
    if (state_ != IntegrationProjectState::draft ||
        !unique_non_empty(maintainers, true) || approval_policy.empty() ||
        !unique_non_empty(build_profiles, true) ||
        !unique_non_empty(test_matrix, true)) {
        return false;
    }
    maintainers_ = std::move(maintainers);
    approval_policy_ = std::move(approval_policy);
    build_profiles_ = std::move(build_profiles);
    test_matrix_ = std::move(test_matrix);
    return true;
}

bool IntegrationProject::select_modification(
    SourceModificationPackage package) {
    if ((state_ != IntegrationProjectState::draft &&
         state_ != IntegrationProjectState::modifications_selected) ||
        !package.valid() || package.baseline != baseline_ ||
        find_modification(package.id) != nullptr) {
        return false;
    }
    selected_modifications_.push_back(std::move(package));
    dependency_report_ = {};
    conflicts_.clear();
    decisions_.clear();
    gates_.clear();
    return true;
}

DependencyReport IntegrationProject::analyze_dependencies() {
    dependency_report_ = {};
    for (const auto& package : selected_modifications_) {
        for (const auto& dependency : package.dependencies) {
            const auto* selected = find_modification(
                dependency.modification_id);
            if (selected == nullptr) {
                dependency_report_.issues.push_back(DependencyIssue{
                    .owner_modification_id = package.id,
                    .dependency_modification_id = dependency.modification_id,
                    .required_version = dependency.version_constraint,
                    .actual_version = {},
                    .kind = DependencyIssueKind::missing_modification,
                    .blocking = dependency.mandatory,
                    .rationale = dependency.rationale,
                });
                continue;
            }
            if (!constraint_supported(dependency.version_constraint)) {
                dependency_report_.issues.push_back(DependencyIssue{
                    .owner_modification_id = package.id,
                    .dependency_modification_id = dependency.modification_id,
                    .required_version = dependency.version_constraint,
                    .actual_version = selected->version,
                    .kind = DependencyIssueKind::unsupported_constraint,
                    .blocking = dependency.mandatory,
                    .rationale = dependency.rationale,
                });
                continue;
            }
            if (!version_constraint_matches(
                    dependency.version_constraint,
                    selected->version)) {
                dependency_report_.issues.push_back(DependencyIssue{
                    .owner_modification_id = package.id,
                    .dependency_modification_id = dependency.modification_id,
                    .required_version = dependency.version_constraint,
                    .actual_version = selected->version,
                    .kind = DependencyIssueKind::version_mismatch,
                    .blocking = dependency.mandatory,
                    .rationale = dependency.rationale,
                });
            }
        }
    }

    const IntegrationGateEvidence gate_value{
        .id = "gate:" + id_ + ":dependency-check",
        .kind = IntegrationGateKind::dependency_check,
        .passed = dependency_report_.passed(),
        .summary = dependency_report_.passed()
            ? "All mandatory source modification dependencies are satisfied."
            : "One or more mandatory source modification dependencies are unresolved.",
        .evidence_record_ids = {},
    };
    const auto iterator = std::find_if(
        gates_.begin(), gates_.end(),
        [](const IntegrationGateEvidence& gate) {
            return gate.kind == IntegrationGateKind::dependency_check;
        });
    if (iterator == gates_.end()) {
        gates_.push_back(gate_value);
    } else {
        *iterator = gate_value;
    }
    return dependency_report_;
}

std::vector<ConflictRecord> IntegrationProject::analyze_conflicts() {
    conflicts_.clear();
    decisions_.clear();
    std::set<std::string, std::less<>> seen;

    const auto add_conflict = [this, &seen](ConflictRecord conflict) {
        if (seen.insert(conflict.id).second) {
            conflicts_.push_back(std::move(conflict));
        }
    };

    for (const auto& package : selected_modifications_) {
        for (const auto& declaration : package.declared_conflicts) {
            const auto* other = find_modification(declaration.modification_id);
            if (other == nullptr) {
                continue;
            }
            auto ordered = ordered_pair(package.id, other->id);
            const auto subject = std::string(to_string(
                declaration.conflict_class));
            add_conflict(ConflictRecord{
                .id = conflict_id(
                    id_, ordered.first, ordered.second,
                    ConflictOrigin::declared,
                    declaration.conflict_class,
                    subject),
                .left_modification_id = ordered.first,
                .right_modification_id = ordered.second,
                .conflict_class = declaration.conflict_class,
                .origin = ConflictOrigin::declared,
                .subject = subject,
                .rationale = declaration.rationale,
                .blocking = declaration.blocking,
                .status = ConflictResolutionStatus::unresolved,
                .decision_record_id = {},
                .resolution = {},
            });
        }
    }

    for (std::size_t left_index = 0U;
         left_index < selected_modifications_.size();
         ++left_index) {
        const auto& left = selected_modifications_[left_index];
        for (std::size_t right_index = left_index + 1U;
             right_index < selected_modifications_.size();
             ++right_index) {
            const auto& right = selected_modifications_[right_index];
            auto ordered = ordered_pair(left.id, right.id);

            std::map<std::string, SourceSymbolKind, std::less<>> left_symbols;
            for (const auto& unit : left.change_units) {
                for (const auto& symbol : unit.affected_symbols) {
                    left_symbols.emplace(symbol.id, symbol.kind);
                }
            }
            for (const auto& unit : right.change_units) {
                for (const auto& symbol : unit.affected_symbols) {
                    const auto iterator = left_symbols.find(symbol.id);
                    if (iterator == left_symbols.end()) {
                        continue;
                    }
                    const auto conflict_class = symbol_conflict_class(
                        iterator->second);
                    add_conflict(ConflictRecord{
                        .id = conflict_id(
                            id_, ordered.first, ordered.second,
                            ConflictOrigin::symbol_overlap,
                            conflict_class,
                            symbol.id),
                        .left_modification_id = ordered.first,
                        .right_modification_id = ordered.second,
                        .conflict_class = conflict_class,
                        .origin = ConflictOrigin::symbol_overlap,
                        .subject = symbol.id,
                        .rationale =
                            "Both modifications affect the same recovered symbol and require semantic review.",
                        .blocking = true,
                        .status = ConflictResolutionStatus::unresolved,
                        .decision_record_id = {},
                        .resolution = {},
                    });
                }
            }

            for (const auto& left_unit : left.change_units) {
                for (const auto& right_unit : right.change_units) {
                    if (left_unit.source_path != right_unit.source_path ||
                        left_unit.result_sha256 == right_unit.result_sha256) {
                        continue;
                    }
                    add_conflict(ConflictRecord{
                        .id = conflict_id(
                            id_, ordered.first, ordered.second,
                            ConflictOrigin::source_path_overlap,
                            ConflictClass::textual_source,
                            left_unit.source_path),
                        .left_modification_id = ordered.first,
                        .right_modification_id = ordered.second,
                        .conflict_class = ConflictClass::textual_source,
                        .origin = ConflictOrigin::source_path_overlap,
                        .subject = left_unit.source_path,
                        .rationale =
                            "Both modifications produce different revisions of the same source unit.",
                        .blocking = true,
                        .status = ConflictResolutionStatus::unresolved,
                        .decision_record_id = {},
                        .resolution = {},
                    });
                }
            }

            for (const auto& key : left.configuration_keys) {
                if (std::find(
                        right.configuration_keys.begin(),
                        right.configuration_keys.end(),
                        key) == right.configuration_keys.end()) {
                    continue;
                }
                add_conflict(ConflictRecord{
                    .id = conflict_id(
                        id_, ordered.first, ordered.second,
                        ConflictOrigin::configuration_overlap,
                        ConflictClass::configuration_key,
                        key),
                    .left_modification_id = ordered.first,
                    .right_modification_id = ordered.second,
                    .conflict_class = ConflictClass::configuration_key,
                    .origin = ConflictOrigin::configuration_overlap,
                    .subject = key,
                    .rationale =
                        "Both modifications declare the same configuration key.",
                    .blocking = true,
                    .status = ConflictResolutionStatus::unresolved,
                    .decision_record_id = {},
                    .resolution = {},
                });
            }
        }
    }

    std::sort(
        conflicts_.begin(), conflicts_.end(),
        [](const ConflictRecord& left, const ConflictRecord& right) {
            return left.id < right.id;
        });

    const IntegrationGateEvidence gate_value{
        .id = "gate:" + id_ + ":conflict-analysis",
        .kind = IntegrationGateKind::conflict_analysis,
        .passed = true,
        .summary = "Dependency-compatible modifications were analyzed for declared, symbol, source-unit, and configuration conflicts.",
        .evidence_record_ids = {},
    };
    const auto iterator = std::find_if(
        gates_.begin(), gates_.end(),
        [](const IntegrationGateEvidence& gate) {
            return gate.kind == IntegrationGateKind::conflict_analysis;
        });
    if (iterator == gates_.end()) {
        gates_.push_back(gate_value);
    } else {
        *iterator = gate_value;
    }
    return conflicts_;
}

bool IntegrationProject::resolve_conflict(
    std::string_view conflict_id_value,
    ConflictResolutionStatus status,
    std::string resolution,
    DecisionRecord decision) {
    if (status == ConflictResolutionStatus::unresolved || resolution.empty() ||
        !decision.valid() ||
        !decision_contains_conflict(decision, conflict_id_value) ||
        std::any_of(
            decisions_.begin(), decisions_.end(),
            [&decision](const DecisionRecord& existing) {
                return existing.id == decision.id;
            })) {
        return false;
    }
    const auto iterator = std::find_if(
        conflicts_.begin(), conflicts_.end(),
        [conflict_id_value](const ConflictRecord& conflict) {
            return conflict.id == conflict_id_value;
        });
    if (iterator == conflicts_.end() ||
        iterator->status != ConflictResolutionStatus::unresolved) {
        return false;
    }
    iterator->status = status;
    iterator->decision_record_id = decision.id;
    iterator->resolution = std::move(resolution);
    decisions_.push_back(std::move(decision));
    return true;
}

bool IntegrationProject::record_gate(IntegrationGateEvidence gate_value) {
    if (!gate_value.valid() ||
        std::any_of(
            gates_.begin(), gates_.end(),
            [&gate_value](const IntegrationGateEvidence& existing) {
                return existing.id == gate_value.id ||
                       existing.kind == gate_value.kind;
            })) {
        return false;
    }
    gates_.push_back(std::move(gate_value));
    return true;
}

bool IntegrationProject::transition(IntegrationProjectState next) {
    if (!transition_allowed(next)) {
        return false;
    }
    state_ = next;
    return true;
}

bool IntegrationProject::fail(
    IntegrationProjectState failure_state,
    std::string reason) {
    if (!is_failure_state(failure_state) || reason.empty() ||
        is_failure_state(state_)) {
        return false;
    }
    state_ = failure_state;
    failure_reason_ = std::move(reason);
    return true;
}

bool IntegrationProject::valid() const noexcept {
    if (id_.empty() || title_.empty() || !baseline_.valid() ||
        !unique_non_empty(maintainers_, true) || approval_policy_.empty() ||
        !unique_non_empty(build_profiles_, true) ||
        !unique_non_empty(test_matrix_, true) ||
        selected_modifications_.empty()) {
        return false;
    }
    std::set<std::string, std::less<>> modification_ids;
    for (const auto& package : selected_modifications_) {
        if (!package.valid() || package.baseline != baseline_ ||
            !modification_ids.insert(package.id).second) {
            return false;
        }
    }
    if (!std::all_of(
            dependency_report_.issues.begin(),
            dependency_report_.issues.end(),
            [](const DependencyIssue& issue) {
                return issue.valid();
            }) ||
        !std::all_of(
            conflicts_.begin(), conflicts_.end(),
            [](const ConflictRecord& conflict) {
                return conflict.valid();
            }) ||
        !std::all_of(
            decisions_.begin(), decisions_.end(),
            [](const DecisionRecord& decision) {
                return decision.valid();
            }) ||
        !std::all_of(
            gates_.begin(), gates_.end(),
            [](const IntegrationGateEvidence& gate_value) {
                return gate_value.valid();
            })) {
        return false;
    }
    return !is_failure_state(state_) || !failure_reason_.empty();
}

std::size_t IntegrationProject::unresolved_blocking_conflict_count() const noexcept {
    return static_cast<std::size_t>(std::count_if(
        conflicts_.begin(), conflicts_.end(),
        [](const ConflictRecord& conflict) {
            return conflict.unresolved_blocking();
        }));
}

const SourceModificationPackage* IntegrationProject::find_modification(
    std::string_view modification_id) const noexcept {
    const auto iterator = std::find_if(
        selected_modifications_.begin(), selected_modifications_.end(),
        [modification_id](const SourceModificationPackage& package) {
            return package.id == modification_id;
        });
    return iterator == selected_modifications_.end() ? nullptr : &*iterator;
}

const IntegrationGateEvidence* IntegrationProject::gate(
    IntegrationGateKind kind) const noexcept {
    const auto iterator = std::find_if(
        gates_.begin(), gates_.end(),
        [kind](const IntegrationGateEvidence& gate_value) {
            return gate_value.kind == kind;
        });
    return iterator == gates_.end() ? nullptr : &*iterator;
}

bool IntegrationProject::transition_allowed(
    IntegrationProjectState next) const noexcept {
    if (is_failure_state(next) || is_failure_state(state_)) {
        return false;
    }
    const auto gate_passed = [this](IntegrationGateKind kind) {
        const auto* gate_value = gate(kind);
        return gate_value != nullptr && gate_value->passed;
    };

    switch (state_) {
    case IntegrationProjectState::draft:
        return next == IntegrationProjectState::modifications_selected &&
               !selected_modifications_.empty() &&
               !maintainers_.empty() && !approval_policy_.empty() &&
               !build_profiles_.empty() && !test_matrix_.empty();
    case IntegrationProjectState::modifications_selected:
        return next == IntegrationProjectState::dependency_checked &&
               gate_passed(IntegrationGateKind::dependency_check) &&
               dependency_report_.passed();
    case IntegrationProjectState::dependency_checked:
        return next == IntegrationProjectState::conflict_analysis &&
               gate_passed(IntegrationGateKind::conflict_analysis);
    case IntegrationProjectState::conflict_analysis:
        return next == IntegrationProjectState::integrating &&
               unresolved_blocking_conflict_count() == 0U;
    case IntegrationProjectState::integrating:
        return next == IntegrationProjectState::source_validated &&
               gate_passed(IntegrationGateKind::source_validation);
    case IntegrationProjectState::source_validated:
        return next == IntegrationProjectState::compiling;
    case IntegrationProjectState::compiling:
        return next == IntegrationProjectState::build_validated &&
               gate_passed(IntegrationGateKind::compilation) &&
               gate_passed(IntegrationGateKind::build_validation);
    case IntegrationProjectState::build_validated:
        return next == IntegrationProjectState::runtime_testing;
    case IntegrationProjectState::runtime_testing:
        return next == IntegrationProjectState::release_candidate &&
               gate_passed(IntegrationGateKind::runtime_testing);
    case IntegrationProjectState::release_candidate:
        return next == IntegrationProjectState::approved &&
               gate_passed(IntegrationGateKind::approval);
    case IntegrationProjectState::approved:
        return next == IntegrationProjectState::released &&
               gate_passed(IntegrationGateKind::release);
    case IntegrationProjectState::released:
        return next == IntegrationProjectState::maintaining;
    case IntegrationProjectState::maintaining:
    case IntegrationProjectState::blocked_by_conflict:
    case IntegrationProjectState::build_failed:
    case IntegrationProjectState::test_failed:
    case IntegrationProjectState::rights_hold:
    case IntegrationProjectState::security_hold:
    case IntegrationProjectState::superseded:
    case IntegrationProjectState::withdrawn:
        return false;
    }
    return false;
}

} // namespace dmc::rengine::source
