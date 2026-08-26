#include "dmc_rengine/validation/validation_run.hpp"

#include <cassert>
#include <string>
#include <utility>

namespace {

using dmc::rengine::evidence::ArtifactIdentity;
using dmc::rengine::validation::ChildValidationReceiptBinding;
using dmc::rengine::validation::ValidationDomain;
using dmc::rengine::validation::ValidationEvidenceClass;
using dmc::rengine::validation::ValidationRunSubmission;
using dmc::rengine::validation::ValidationScope;

constexpr const char* kRunId = "v-run-20260826-001";
constexpr const char* kResourceBinding =
    "1111111111111111111111111111111111111111111111111111111111111111";
constexpr const char* kExeSha =
    "2222222222222222222222222222222222222222222222222222222222222222";
constexpr const char* kL1Sha =
    "3333333333333333333333333333333333333333333333333333333333333333";
constexpr const char* kL2Sha =
    "4444444444444444444444444444444444444444444444444444444444444444";
constexpr const char* kL3Sha =
    "5555555555555555555555555555555555555555555555555555555555555555";
constexpr const char* kLvSha =
    "6666666666666666666666666666666666666666666666666666666666666666";
constexpr const char* kRollbackSha =
    "7777777777777777777777777777777777777777777777777777777777777777";

ChildValidationReceiptBinding child(
    std::string id,
    ValidationDomain domain,
    ValidationEvidenceClass evidence_class,
    std::string sha) {
    return ChildValidationReceiptBinding{
        .id = std::move(id),
        .domain = domain,
        .evidence_class = evidence_class,
        .schema = "dmc-rengine.test-receipt.v1",
        .receipt_sha256 = std::move(sha),
        .validation_run_id = kRunId,
        .resource_binding_sha256 = kResourceBinding,
    };
}

ValidationRunSubmission valid_vertical_submission() {
    ValidationRunSubmission submission;
    submission.validation_run_id = kRunId;
    submission.scope = ValidationScope::cross_layer_vertical;
    submission.declared_scope = "representative-dmc3-resource-vertical";
    submission.resource_binding_sha256 = kResourceBinding;
    submission.original_execution_artifact_id = "protected-dmc3-exe";
    submission.artifacts.push_back(ArtifactIdentity{
        .id = "protected-dmc3-exe",
        .role = "original-execution-authority",
        .sha256 = kExeSha,
        .size = 6'567'320U,
    });
    submission.child_receipts.push_back(child(
        "l1-consumption",
        ValidationDomain::l1_materialization,
        ValidationEvidenceClass::v_d_original_process,
        kL1Sha));
    submission.child_receipts.push_back(child(
        "l2-selected-identity",
        ValidationDomain::l2_resolution,
        ValidationEvidenceClass::v_d_original_process,
        kL2Sha));
    submission.child_receipts.push_back(child(
        "l3-consumer-lifecycle",
        ValidationDomain::l3_runtime_lifecycle,
        ValidationEvidenceClass::v_d_original_process,
        kL3Sha));
    submission.child_receipts.push_back(child(
        "lv-original-observation",
        ValidationDomain::lv_live_observation,
        ValidationEvidenceClass::v_a_integrity,
        kLvSha));
    submission.rollback_required = true;
    submission.rollback_receipt_sha256 = kRollbackSha;
    return submission;
}

} // namespace

int main() {
    {
        const auto submission = valid_vertical_submission();
        assert(submission.valid());
        assert(submission.original_equivalence_content_candidate());

        const auto gate =
            dmc::rengine::validation::manual_submission_promotion_gate(submission);
        assert(!gate.eligible());
    }

    {
        auto submission = valid_vertical_submission();
        submission.child_receipts[1].validation_run_id = "another-run";
        assert(!submission.valid());
    }

    {
        auto submission = valid_vertical_submission();
        submission.child_receipts[2].resource_binding_sha256 =
            "8888888888888888888888888888888888888888888888888888888888888888";
        assert(!submission.valid());
    }

    {
        auto submission = valid_vertical_submission();
        submission.child_receipts.pop_back();
        assert(!submission.valid());
    }

    {
        auto submission = valid_vertical_submission();
        submission.child_receipts[0].evidence_class =
            ValidationEvidenceClass::v_c_real_corpus;
        assert(!submission.valid());
    }

    {
        auto submission = valid_vertical_submission();
        submission.child_receipts[3].receipt_sha256 = kL3Sha;
        assert(!submission.valid());
    }

    {
        auto submission = valid_vertical_submission();
        submission.artifacts[0].sha256 =
            "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA";
        assert(!submission.valid());
    }

    {
        auto submission = valid_vertical_submission();
        submission.rollback_required = false;
        assert(!submission.valid());

        submission.rollback_receipt_sha256.clear();
        assert(submission.valid());
    }

    {
        ValidationRunSubmission submission;
        submission.validation_run_id = kRunId;
        submission.scope = ValidationScope::l2;
        submission.declared_scope = "bounded-l2-original-selection";
        submission.resource_binding_sha256 = kResourceBinding;
        submission.original_execution_artifact_id = "protected-dmc3-exe";
        submission.artifacts.push_back(ArtifactIdentity{
            .id = "protected-dmc3-exe",
            .role = "original-execution-authority",
            .sha256 = kExeSha,
            .size = 6'567'320U,
        });
        submission.child_receipts.push_back(child(
            "l2-structural-contract",
            ValidationDomain::l2_resolution,
            ValidationEvidenceClass::v_b_product,
            kL2Sha));

        assert(submission.valid());
        assert(!submission.original_equivalence_content_candidate());
        const auto gate =
            dmc::rengine::validation::manual_submission_promotion_gate(submission);
        assert(!gate.eligible());
    }

    return 0;
}
