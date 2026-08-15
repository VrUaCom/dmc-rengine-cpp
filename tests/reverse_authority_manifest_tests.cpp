#include "dmc_rengine/core/json.hpp"

#include <algorithm>
#include <cassert>
#include <fstream>
#include <sstream>
#include <string>
#include <string_view>

namespace {

[[nodiscard]] std::string read_text(const char* path) {
    std::ifstream stream(path, std::ios::binary);
    assert(stream.good());
    std::ostringstream output;
    output << stream.rdbuf();
    return output.str();
}

[[nodiscard]] const dmc::rengine::core::json::Value* member(
    const dmc::rengine::core::json::Value::Object& object,
    std::string_view name) {
    const auto iterator = object.find(name);
    return iterator == object.end() ? nullptr : &iterator->second;
}

[[nodiscard]] const dmc::rengine::core::json::Value::Object* find_object_by_id(
    const dmc::rengine::core::json::Value::Array& values,
    std::string_view id) {
    for (const auto& value : values) {
        const auto* object = value.as_object();
        if (object == nullptr) {
            continue;
        }
        const auto* id_value = member(*object, "id");
        if (id_value != nullptr && id_value->as_string() != nullptr &&
            *id_value->as_string() == id) {
            return object;
        }
    }
    return nullptr;
}

[[nodiscard]] bool string_array_contains(
    const dmc::rengine::core::json::Value* value,
    std::string_view expected) {
    if (value == nullptr || value->as_array() == nullptr) {
        return false;
    }
    return std::any_of(
        value->as_array()->begin(), value->as_array()->end(),
        [expected](const auto& candidate) {
            return candidate.as_string() != nullptr &&
                   *candidate.as_string() == expected;
        });
}

[[nodiscard]] const dmc::rengine::core::json::Value::Array& array_member(
    const dmc::rengine::core::json::Value::Object& object,
    std::string_view name) {
    const auto* value = member(object, name);
    assert(value != nullptr && value->as_array() != nullptr);
    return *value->as_array();
}

[[nodiscard]] std::string_view string_member(
    const dmc::rengine::core::json::Value::Object& object,
    std::string_view name) {
    const auto* value = member(object, name);
    assert(value != nullptr && value->as_string() != nullptr);
    return *value->as_string();
}

[[nodiscard]] std::uint64_t u64_member(
    const dmc::rengine::core::json::Value::Object& object,
    std::string_view name) {
    const auto* value = member(object, name);
    assert(value != nullptr && value->as_u64() != nullptr);
    return *value->as_u64();
}

} // namespace

int main(int argc, char** argv) {
    using dmc::rengine::core::json::Parser;

    assert(argc == 2);
    const auto parsed = Parser::parse(read_text(argv[1]));
    assert(parsed.ok());
    const auto* root = parsed.value->as_object();
    assert(root != nullptr);

    assert(u64_member(*root, "schema_version") == 1U);

    const auto* target_value = member(*root, "target");
    assert(target_value != nullptr && target_value->as_object() != nullptr);
    const auto& target = *target_value->as_object();
    assert(string_member(target, "canonical_executable_sha256") ==
        "e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082");
    assert(u64_member(target, "canonical_executable_size") == 6356432U);

    const auto& authorities = array_member(*root, "authorities");
    assert(authorities.size() == 6U);
    const auto* process_control = find_object_by_id(
        authorities, "drive-process-control");
    const auto* knowledge_base = find_object_by_id(
        authorities, "drive-knowledge-base");
    const auto* legacy_registry = find_object_by_id(
        authorities, "drive-reverse-registry-legacy");
    const auto* recovered_source = find_object_by_id(
        authorities, "drive-recovered-source");
    const auto* github = find_object_by_id(
        authorities, "github-active-cpp");
    assert(process_control != nullptr);
    assert(knowledge_base != nullptr);
    assert(legacy_registry != nullptr);
    assert(recovered_source != nullptr);
    assert(github != nullptr);

    assert(string_array_contains(member(*process_control, "owns"), "pass-state"));
    assert(string_array_contains(
        member(*process_control, "must_not_own"),
        "standalone-technical-facts-without-evidence"));
    assert(u64_member(*knowledge_base, "latest_verified_pass") == 32U);
    assert(string_array_contains(member(*knowledge_base, "owns"), "open-gaps"));
    assert(string_member(*legacy_registry, "status") == "archived-read-only");
    assert(string_array_contains(
        member(*legacy_registry, "must_not_be_used_as"),
        "write-target-for-new-findings"));

    assert(string_member(*recovered_source, "latest_snapshot") ==
        "Recovered_Source_Skeleton_v1_7_Pass32.zip");
    assert(string_member(*recovered_source, "latest_snapshot_sha256") ==
        "978fd6c8c8c1e1de1cf2a50bf9328fce6be345b9f3c70ab30ffb92c195ac22d6");
    assert(u64_member(*recovered_source, "latest_snapshot_file_count") == 217U);
    assert(string_array_contains(
        member(*github, "owns"), "machine-readable-evidence"));
    assert(string_array_contains(member(*github, "owns"), "ci-receipts"));

    assert(string_array_contains(
        member(*root, "synchronization_order"),
        "github-source-evidence-and-tests"));
    assert(string_array_contains(
        member(*root, "synchronization_order"),
        "drive-sync-and-readback"));

    const auto& artifacts = array_member(*root, "verified_artifacts");
    assert(artifacts.size() == 2U);
    assert(find_object_by_id(artifacts, "pass32-complete-zip") != nullptr);
    assert(find_object_by_id(
        artifacts, "recovered-source-v1-7-pass32") != nullptr);

    const auto& receipts = array_member(*root, "implementation_receipts");
    assert(receipts.size() == 1U);
    const auto* pass32_receipt = find_object_by_id(
        receipts, "pass32-pc-save-product-promotion-pr42");
    assert(pass32_receipt != nullptr);
    assert(string_member(*pass32_receipt, "drive_file_id") ==
        "1jtbyj7OmYA6rnwlzQ5iHgy6ZBKhTHH9OwqDH_e05PfY");
    assert(string_member(*pass32_receipt, "implementation_content_head_sha") ==
        "b91b1408bbc26e097107c14fed78dfa343ce948b");
    assert(u64_member(*pass32_receipt, "implementation_ci_run_id") ==
        30949044678ULL);
    assert(string_member(*pass32_receipt, "receipt_sync_head_sha") ==
        "192b2254885dfce304ea6922ab83d3391208e9f6");
    assert(u64_member(*pass32_receipt, "receipt_sync_ci_run_id") ==
        30949736308ULL);
    assert(string_member(*pass32_receipt, "ci_conclusion") == "success");

    const auto& excluded = array_member(*root, "excluded_builds");
    const auto* vanilla = find_object_by_id(
        excluded, "drive-vanilla-dmc3-2026-01-23");
    assert(vanilla != nullptr);
    assert(string_member(*vanilla, "sha256") ==
        "81c7e61983564113b5105e931d9f185accc14e44ae147d27f720c2d50935c7d6");

    const auto& blockers = array_member(*root, "current_blockers");
    assert(blockers.size() == 4U);
    assert(find_object_by_id(
        blockers, "blocker-source-tree-partially-promoted") != nullptr);
    assert(find_object_by_id(
        blockers, "blocker-drive-artifact-hashes-missing") != nullptr);
    assert(find_object_by_id(
        blockers, "blocker-pass33-save-payload-semantics-open") != nullptr);
    assert(find_object_by_id(
        blockers, "blocker-pass32-function-byte-provenance-incomplete") != nullptr);

    const auto& corrections = array_member(*root, "resolved_corrections");
    assert(corrections.size() == 5U);

    const auto* artifact_size_correction = find_object_by_id(
        corrections, "correction-canonical-executable-size");
    assert(artifact_size_correction != nullptr);
    assert(string_member(*artifact_size_correction, "status") == "resolved");
    assert(u64_member(*artifact_size_correction, "previous_size") == 3735552U);
    assert(u64_member(*artifact_size_correction, "replacement_size") == 6356432U);
    assert(string_member(*artifact_size_correction, "sha256") ==
        string_member(target, "canonical_executable_sha256"));
    assert(u64_member(*artifact_size_correction, "replacement_size") ==
        u64_member(target, "canonical_executable_size"));

    const auto* receipt = find_object_by_id(
        corrections, "correction-pass31-git-receipt");
    assert(receipt != nullptr);
    assert(string_member(*receipt, "previous_receipt") ==
        "784b06b7fef5e526fc98721629fe3c509fe41a228");
    assert(string_member(*receipt, "replacement_receipt") ==
        "76ed3b7a02ee83a6285834495ac0e4c9f84845e3");
    assert(string_array_contains(
        member(*receipt, "repository_paths"),
        "evidence/save/dmc3-pc-save-pass31.evidence.json"));
    assert(string_array_contains(
        member(*receipt, "repository_paths"),
        "src/save/pc_save_file.cpp"));

    assert(find_object_by_id(
        corrections, "correction-process-control-schema-drift") != nullptr);
    assert(find_object_by_id(
        corrections, "correction-knowledge-base-schema-drift") != nullptr);
    assert(find_object_by_id(
        corrections, "correction-pass31-product-source-gap") != nullptr);

    const auto& pending = array_member(*root, "pending_promotions");
    const auto* pass32 = find_object_by_id(
        pending, "promotion-pass32-record-envelope");
    assert(pass32 != nullptr);
    assert(string_member(*pass32, "status") ==
        "technical-gates-green-human-review-pending");
    assert(string_array_contains(member(*pass32, "remaining_gates"), "human-review"));
    assert(string_array_contains(member(*pass32, "remaining_gates"), "merge"));
    assert(string_array_contains(
        member(*pass32, "required_outputs"),
        "evidence/save/dmc3-pc-save-pass32.evidence.json"));
    assert(string_array_contains(
        member(*pass32, "required_outputs"),
        "drive-readback-receipt"));

    return 0;
}
