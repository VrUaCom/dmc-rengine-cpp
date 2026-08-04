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

} // namespace

int main(int argc, char** argv) {
    using dmc::rengine::core::json::Parser;

    assert(argc == 2);
    const auto parsed = Parser::parse(read_text(argv[1]));
    assert(parsed.ok());
    const auto* root = parsed.value->as_object();
    assert(root != nullptr);

    const auto* schema_version = member(*root, "schema_version");
    assert(schema_version != nullptr && schema_version->as_u64() != nullptr);
    assert(*schema_version->as_u64() == 1U);

    const auto* target_value = member(*root, "target");
    assert(target_value != nullptr && target_value->as_object() != nullptr);
    const auto* target = target_value->as_object();
    const auto* target_hash = member(
        *target, "canonical_executable_sha256");
    const auto* target_size = member(
        *target, "canonical_executable_size");
    assert(target_hash != nullptr && target_hash->as_string() != nullptr);
    assert(*target_hash->as_string() ==
        "e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082");
    assert(target_size != nullptr && target_size->as_u64() != nullptr);
    assert(*target_size->as_u64() == 3735552U);

    const auto* authorities_value = member(*root, "authorities");
    assert(authorities_value != nullptr &&
           authorities_value->as_array() != nullptr);
    const auto& authorities = *authorities_value->as_array();
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

    assert(string_array_contains(
        member(*process_control, "owns"),
        "pass-state"));
    assert(string_array_contains(
        member(*process_control, "must_not_own"),
        "standalone-technical-facts-without-evidence"));

    const auto* latest_pass = member(*knowledge_base, "latest_verified_pass");
    assert(latest_pass != nullptr && latest_pass->as_u64() != nullptr);
    assert(*latest_pass->as_u64() == 32U);
    assert(string_array_contains(
        member(*knowledge_base, "owns"),
        "open-gaps"));

    const auto* source_snapshot = member(*recovered_source, "latest_snapshot");
    const auto* source_hash = member(
        *recovered_source, "latest_snapshot_sha256");
    const auto* source_count = member(
        *recovered_source, "latest_snapshot_file_count");
    assert(source_snapshot != nullptr &&
           source_snapshot->as_string() != nullptr);
    assert(*source_snapshot->as_string() ==
        "Recovered_Source_Skeleton_v1_7_Pass32.zip");
    assert(source_hash != nullptr && source_hash->as_string() != nullptr);
    assert(*source_hash->as_string() ==
        "978fd6c8c8c1e1de1cf2a50bf9328fce6be345b9f3c70ab30ffb92c195ac22d6");
    assert(source_count != nullptr && source_count->as_u64() != nullptr);
    assert(*source_count->as_u64() == 217U);

    const auto* legacy_status = member(*legacy_registry, "status");
    assert(legacy_status != nullptr && legacy_status->as_string() != nullptr);
    assert(*legacy_status->as_string() == "archived-read-only");
    assert(string_array_contains(
        member(*legacy_registry, "must_not_be_used_as"),
        "write-target-for-new-findings"));

    assert(string_array_contains(
        member(*github, "owns"),
        "machine-readable-evidence"));
    assert(string_array_contains(
        member(*github, "owns"),
        "ci-receipts"));

    const auto* synchronization_order = member(
        *root, "synchronization_order");
    assert(string_array_contains(
        synchronization_order,
        "github-source-evidence-and-tests"));
    assert(string_array_contains(
        synchronization_order,
        "drive-sync-and-readback"));

    const auto* artifacts_value = member(*root, "verified_artifacts");
    assert(artifacts_value != nullptr && artifacts_value->as_array() != nullptr);
    const auto& artifacts = *artifacts_value->as_array();
    assert(artifacts.size() == 2U);
    assert(find_object_by_id(artifacts, "pass32-complete-zip") != nullptr);
    assert(find_object_by_id(
        artifacts,
        "recovered-source-v1-7-pass32") != nullptr);

    const auto* excluded_value = member(*root, "excluded_builds");
    assert(excluded_value != nullptr && excluded_value->as_array() != nullptr);
    const auto& excluded = *excluded_value->as_array();
    const auto* vanilla = find_object_by_id(
        excluded,
        "drive-vanilla-dmc3-2026-01-23");
    assert(vanilla != nullptr);
    const auto* vanilla_hash = member(*vanilla, "sha256");
    assert(vanilla_hash != nullptr && vanilla_hash->as_string() != nullptr);
    assert(*vanilla_hash->as_string() ==
        "81c7e61983564113b5105e931d9f185accc14e44ae147d27f720c2d50935c7d6");

    const auto* blockers_value = member(*root, "current_blockers");
    assert(blockers_value != nullptr && blockers_value->as_array() != nullptr);
    const auto& blockers = *blockers_value->as_array();
    assert(blockers.size() == 4U);
    assert(find_object_by_id(
        blockers,
        "blocker-source-tree-partially-promoted") != nullptr);
    assert(find_object_by_id(
        blockers,
        "blocker-drive-artifact-hashes-missing") != nullptr);
    assert(find_object_by_id(
        blockers,
        "blocker-pass33-save-payload-semantics-open") != nullptr);
    assert(find_object_by_id(
        blockers,
        "blocker-pass32-function-byte-provenance-incomplete") != nullptr);
    assert(find_object_by_id(
        blockers,
        "blocker-pass31-git-receipt-unresolved") == nullptr);
    assert(find_object_by_id(
        blockers,
        "blocker-process-control-row-schema-drift") == nullptr);

    const auto* corrections_value = member(*root, "resolved_corrections");
    assert(corrections_value != nullptr &&
           corrections_value->as_array() != nullptr);
    const auto& corrections = *corrections_value->as_array();
    assert(corrections.size() == 4U);

    const auto* receipt = find_object_by_id(
        corrections,
        "correction-pass31-git-receipt");
    const auto* process_schema = find_object_by_id(
        corrections,
        "correction-process-control-schema-drift");
    const auto* knowledge_schema = find_object_by_id(
        corrections,
        "correction-knowledge-base-schema-drift");
    const auto* source_gap = find_object_by_id(
        corrections,
        "correction-pass31-product-source-gap");
    assert(receipt != nullptr);
    assert(process_schema != nullptr);
    assert(knowledge_schema != nullptr);
    assert(source_gap != nullptr);

    const auto* previous_receipt = member(*receipt, "previous_receipt");
    const auto* replacement_receipt = member(*receipt, "replacement_receipt");
    assert(previous_receipt != nullptr &&
           previous_receipt->as_string() != nullptr);
    assert(*previous_receipt->as_string() ==
        "784b06b7fef5e526fc98721629fe3c509fe41a228");
    assert(replacement_receipt != nullptr &&
           replacement_receipt->as_string() != nullptr);
    assert(*replacement_receipt->as_string() ==
        "76ed3b7a02ee83a6285834495ac0e4c9f84845e3");
    assert(string_array_contains(
        member(*receipt, "repository_paths"),
        "evidence/save/dmc3-pc-save-pass31.evidence.json"));
    assert(string_array_contains(
        member(*receipt, "repository_paths"),
        "src/save/pc_save_file.cpp"));

    const auto* pending_value = member(*root, "pending_promotions");
    assert(pending_value != nullptr && pending_value->as_array() != nullptr);
    const auto& pending = *pending_value->as_array();
    const auto* pass32 = find_object_by_id(
        pending,
        "promotion-pass32-record-envelope");
    assert(pass32 != nullptr);
    assert(string_array_contains(
        member(*pass32, "required_outputs"),
        "evidence/save/dmc3-pc-save-pass32.evidence.json"));
    assert(string_array_contains(
        member(*pass32, "required_outputs"),
        "drive-readback-receipt"));
    return 0;
}
