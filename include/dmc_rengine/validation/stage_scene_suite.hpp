#pragma once

#include "dmc_rengine/validation/stage_scene_receipt.hpp"

#include <cstddef>
#include <string>
#include <string_view>
#include <vector>

namespace dmc::rengine::validation {

enum class StageSceneValidationCaseKind {
    bank_a_descriptor,
    bank_b_descriptor,
    shared_resource,
    selector_alias_or_fallback,
    partial_or_unresolved,
};

[[nodiscard]] constexpr std::string_view to_string(
    StageSceneValidationCaseKind kind) noexcept {
    switch (kind) {
    case StageSceneValidationCaseKind::bank_a_descriptor:
        return "bank-a-descriptor";
    case StageSceneValidationCaseKind::bank_b_descriptor:
        return "bank-b-descriptor";
    case StageSceneValidationCaseKind::shared_resource:
        return "shared-resource";
    case StageSceneValidationCaseKind::selector_alias_or_fallback:
        return "selector-alias-or-fallback";
    case StageSceneValidationCaseKind::partial_or_unresolved:
        return "partial-or-unresolved";
    }
    return "partial-or-unresolved";
}

// The case kind is supplied explicitly by the validation harness/evidence plan.
// It is never inferred from `stNNN` naming or catalog row numbering.
struct StageSceneValidationCaseReceipt final {
    StageSceneValidationCaseKind kind{
        StageSceneValidationCaseKind::partial_or_unresolved};
    StageSceneValidationReceipt receipt;
    std::string validation_claim_id;

    [[nodiscard]] bool valid() const noexcept {
        return receipt.valid() && !validation_claim_id.empty();
    }
};

struct StageSceneValidationSuite final {
    std::string suite_id;
    std::vector<StageSceneValidationCaseReceipt> cases;

    [[nodiscard]] bool valid() const noexcept;
    [[nodiscard]] bool covers(StageSceneValidationCaseKind kind) const noexcept;
    [[nodiscard]] bool representative_coverage_complete() const noexcept;
    [[nodiscard]] std::size_t count(StageSceneValidationCaseKind kind) const noexcept;
};

} // namespace dmc::rengine::validation
