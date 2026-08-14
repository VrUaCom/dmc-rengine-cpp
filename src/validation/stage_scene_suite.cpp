#include "dmc_rengine/validation/stage_scene_suite.hpp"

#include <algorithm>
#include <array>
#include <set>
#include <string>

namespace dmc::rengine::validation {

bool StageSceneValidationSuite::valid() const noexcept {
    if (suite_id.empty() || cases.empty()) {
        return false;
    }

    std::set<std::string, std::less<>> receipt_ids;
    for (const auto& item : cases) {
        if (!item.valid() ||
            !receipt_ids.insert(item.receipt.receipt_id).second) {
            return false;
        }
    }
    return true;
}

bool StageSceneValidationSuite::covers(
    StageSceneValidationCaseKind kind) const noexcept {
    return std::any_of(
        cases.begin(), cases.end(),
        [kind](const StageSceneValidationCaseReceipt& item) {
            return item.kind == kind;
        });
}

std::size_t StageSceneValidationSuite::count(
    StageSceneValidationCaseKind kind) const noexcept {
    return static_cast<std::size_t>(std::count_if(
        cases.begin(), cases.end(),
        [kind](const StageSceneValidationCaseReceipt& item) {
            return item.kind == kind;
        }));
}

bool StageSceneValidationSuite::representative_coverage_complete() const noexcept {
    if (!valid()) {
        return false;
    }

    constexpr std::array required{
        StageSceneValidationCaseKind::bank_a_descriptor,
        StageSceneValidationCaseKind::bank_b_descriptor,
        StageSceneValidationCaseKind::shared_resource,
        StageSceneValidationCaseKind::selector_alias_or_fallback,
        StageSceneValidationCaseKind::partial_or_unresolved,
    };
    return std::all_of(
        required.begin(), required.end(),
        [this](StageSceneValidationCaseKind kind) {
            return covers(kind);
        });
}

} // namespace dmc::rengine::validation
