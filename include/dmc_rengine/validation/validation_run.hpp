#pragma once

#include "dmc_rengine/evidence/artifact.hpp"

#include <algorithm>
#include <cctype>
#include <cstdint>
#include <set>
#include <string>
#include <string_view>
#include <type_traits>
#include <vector>

namespace dmc::rengine::validation {

enum class ValidationDomain : std::uint8_t {
    l1_materialization,
    l2_resolution,
    l3_runtime_lifecycle,
    lv_live_observation,
};

enum class ValidationEvidenceClass : std::uint8_t {
    v_a_integrity,
    v_b_product,
    v_c_real_corpus,
    v_d_original_process,
    v_e_breadth,
};

enum class ValidationScope : std::uint8_t {
    l1,
    l2,
    l3,
    cross_layer_vertical,
};

[[nodiscard]] inline bool is_canonical_sha256(std::string_view value) noexcept {
    if (value.size() != 64U) {
        return false;
    }
    return std::all_of(
        value.begin(), value.end(),
        [](unsigned char character) {
            return (character >= static_cast<unsigned char>('0') &&
                    character <= static_cast<unsigned char>('9')) ||
                   (character >= static_cast<unsigned char>('a') &&
                    character <= static_cast<unsigned char>('f'));
        });
}

[[nodiscard]] inline bool is_bounded_identifier(std::string_view value) noexcept {
    if (value.empty() || value.size() > 128U) {
        return false;
    }
    return std::all_of(
        value.begin(), value.end(),
        [](unsigned char character) {
            return std::isalnum(character) != 0 ||
                   character == static_cast<unsigned char>('-') ||
                   character == static_cast<unsigned char>('_') ||
                   character == static_cast<unsigned char>('.') ||
                   character == static_cast<unsigned char>(':');
        });
}

[[nodiscard]] inline bool is_bounded_text(std::string_view value,
                                          std::size_t limit) noexcept {
    return !value.empty() && value.size() <= limit;
}

struct ChildValidationReceiptBinding final {
    std::string id;
    ValidationDomain domain{ValidationDomain::l1_materialization};
    ValidationEvidenceClass evidence_class{ValidationEvidenceClass::v_a_integrity};
    std::string schema;
    std::string receipt_sha256;
    std::string validation_run_id;
    std::string resource_binding_sha256;

    [[nodiscard]] bool valid() const noexcept {
        return is_bounded_text(id, 128U) &&
               is_bounded_text(schema, 128U) &&
               is_canonical_sha256(receipt_sha256) &&
               is_bounded_identifier(validation_run_id) &&
               is_canonical_sha256(resource_binding_sha256);
    }

    friend bool operator==(const ChildValidationReceiptBinding&,
                           const ChildValidationReceiptBinding&) = default;
};

struct ValidationRunSubmission final {
    std::uint32_t schema_version{1};
    std::string validation_run_id;
    ValidationScope scope{ValidationScope::cross_layer_vertical};
    std::string declared_scope;
    std::string resource_binding_sha256;
    std::string original_execution_artifact_id;
    std::vector<evidence::ArtifactIdentity> artifacts;
    std::vector<ChildValidationReceiptBinding> child_receipts;
    bool rollback_required{false};
    std::string rollback_receipt_sha256;

    [[nodiscard]] bool valid() const {
        if (schema_version != 1U ||
            !is_bounded_identifier(validation_run_id) ||
            !is_bounded_text(declared_scope, 256U) ||
            !is_canonical_sha256(resource_binding_sha256) ||
            !is_bounded_text(original_execution_artifact_id, 128U)) {
            return false;
        }

        if (rollback_required && !is_canonical_sha256(rollback_receipt_sha256)) {
            return false;
        }
        if (!rollback_required && !rollback_receipt_sha256.empty()) {
            return false;
        }

        std::set<std::string> artifact_ids;
        bool original_execution_artifact_found = false;
        for (const auto& artifact : artifacts) {
            if (!artifact.valid() || !is_canonical_sha256(artifact.sha256) ||
                !artifact_ids.insert(artifact.id).second) {
                return false;
            }
            if (artifact.id == original_execution_artifact_id) {
                if (artifact.size == 0U) {
                    return false;
                }
                original_execution_artifact_found = true;
            }
        }
        if (!original_execution_artifact_found) {
            return false;
        }

        std::set<std::string> child_ids;
        std::set<std::string> child_hashes;
        bool has_l1 = false;
        bool has_l2 = false;
        bool has_l3 = false;
        bool has_lv = false;
        bool l1_vd = false;
        bool l2_vd = false;
        bool l3_vd = false;

        for (const auto& child : child_receipts) {
            if (!child.valid() ||
                child.validation_run_id != validation_run_id ||
                child.resource_binding_sha256 != resource_binding_sha256 ||
                !child_ids.insert(child.id).second ||
                !child_hashes.insert(child.receipt_sha256).second) {
                return false;
            }

            switch (child.domain) {
            case ValidationDomain::l1_materialization:
                has_l1 = true;
                l1_vd = l1_vd ||
                        child.evidence_class == ValidationEvidenceClass::v_d_original_process;
                break;
            case ValidationDomain::l2_resolution:
                has_l2 = true;
                l2_vd = l2_vd ||
                        child.evidence_class == ValidationEvidenceClass::v_d_original_process;
                break;
            case ValidationDomain::l3_runtime_lifecycle:
                has_l3 = true;
                l3_vd = l3_vd ||
                        child.evidence_class == ValidationEvidenceClass::v_d_original_process;
                break;
            case ValidationDomain::lv_live_observation:
                has_lv = true;
                break;
            }
        }

        switch (scope) {
        case ValidationScope::l1:
            return has_l1;
        case ValidationScope::l2:
            return has_l2;
        case ValidationScope::l3:
            return has_l3;
        case ValidationScope::cross_layer_vertical:
            return has_l1 && has_l2 && has_l3 && has_lv &&
                   l1_vd && l2_vd && l3_vd;
        }

        return false;
    }

    [[nodiscard]] bool original_equivalence_content_candidate() const {
        if (!valid()) {
            return false;
        }

        if (scope == ValidationScope::cross_layer_vertical) {
            return true;
        }

        return std::any_of(
            child_receipts.begin(), child_receipts.end(),
            [](const ChildValidationReceiptBinding& child) {
                return child.evidence_class ==
                       ValidationEvidenceClass::v_d_original_process;
            });
    }
};

class TrustedValidationBinder;

class ValidationPromotionGate final {
public:
    ValidationPromotionGate() = default;

    [[nodiscard]] bool eligible() const noexcept {
        return eligible_;
    }

private:
    explicit ValidationPromotionGate(bool eligible) noexcept
        : eligible_(eligible) {}

    bool eligible_{false};

    friend class TrustedValidationBinder;
};

// Defined here, rather than merely forward-declared, so another translation
// unit cannot define the friend type and manufacture access to the private
// promotion constructor. A future trusted-publisher slice must deliberately
// evolve this authority type in this header.
class TrustedValidationBinder final {
public:
    TrustedValidationBinder(const TrustedValidationBinder&) = delete;
    TrustedValidationBinder& operator=(const TrustedValidationBinder&) = delete;
    TrustedValidationBinder(TrustedValidationBinder&&) = delete;
    TrustedValidationBinder& operator=(TrustedValidationBinder&&) = delete;

private:
    TrustedValidationBinder() = default;
};

static_assert(!std::is_constructible_v<ValidationPromotionGate, bool>);
static_assert(!std::is_default_constructible_v<TrustedValidationBinder>);
static_assert(!std::is_copy_constructible_v<TrustedValidationBinder>);
static_assert(!std::is_move_constructible_v<TrustedValidationBinder>);

[[nodiscard]] inline ValidationPromotionGate manual_submission_promotion_gate(
    const ValidationRunSubmission&) noexcept {
    // A structurally perfect self-authored/manual submission can never create
    // trusted promotion authority. The authority type above currently exposes
    // no trusted binding operation at all.
    return {};
}

} // namespace dmc::rengine::validation
