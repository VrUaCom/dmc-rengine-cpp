#include "dmc_rengine/validation/stage_scene_receipt_json.hpp"

#include <iomanip>
#include <sstream>
#include <string>
#include <string_view>

namespace dmc::rengine::validation {
namespace {

[[nodiscard]] std::string json_escape(std::string_view value) {
    std::ostringstream stream;
    stream << '"';
    for (const unsigned char character : value) {
        switch (character) {
        case '"': stream << "\\\""; break;
        case '\\': stream << "\\\\"; break;
        case '\b': stream << "\\b"; break;
        case '\f': stream << "\\f"; break;
        case '\n': stream << "\\n"; break;
        case '\r': stream << "\\r"; break;
        case '\t': stream << "\\t"; break;
        default:
            if (character < 0x20U) {
                stream << "\\u"
                       << std::hex << std::setw(4) << std::setfill('0')
                       << static_cast<unsigned int>(character)
                       << std::dec << std::setfill(' ');
            } else {
                stream << static_cast<char>(character);
            }
            break;
        }
    }
    stream << '"';
    return stream.str();
}

[[nodiscard]] std::string bool_json(bool value) {
    return value ? "true" : "false";
}

} // namespace

std::string stage_scene_validation_receipt_json(
    const StageSceneValidationReceipt& receipt) {
    if (!receipt.valid()) {
        return {};
    }

    std::ostringstream json;
    json << "{\n"
         << "  \"schema_version\": 1,\n"
         << "  \"receipt_id\": " << json_escape(receipt.receipt_id) << ",\n"
         << "  \"identity\": {\n"
         << "    \"profile\": " << json_escape(receipt.profile) << ",\n"
         << "    \"catalog_entry_id\": "
         << json_escape(receipt.catalog_entry_id) << ",\n"
         << "    \"source_table_id\": "
         << json_escape(receipt.source_table_id) << ",\n"
         << "    \"global_catalog_row\": "
         << receipt.global_catalog_row << ",\n"
         << "    \"source_row_index\": "
         << receipt.source_row_index << ",\n"
         << "    \"numeric_stage_id\": ";
    if (receipt.numeric_stage_id.has_value()) {
        json << *receipt.numeric_stage_id;
    } else {
        json << "null";
    }
    json << ",\n"
         << "    \"semantic_stage_id\": "
         << json_escape(receipt.semantic_stage_id) << ",\n"
         << "    \"executable_evidence_id\": "
         << json_escape(receipt.executable_evidence_id) << "\n"
         << "  },\n"
         << "  \"stage_revision\": " << receipt.stage_revision << ",\n"
         << "  \"assembly_status\": "
         << json_escape(stageops::to_string(receipt.assembly_status)) << ",\n"
         << "  \"game_readiness\": "
         << json_escape(stageops::to_string(receipt.game_readiness)) << ",\n"
         << "  \"derived_state_stale\": "
         << bool_json(receipt.derived_state_stale) << ",\n"
         << "  \"scene_current_for_active_bytes\": "
         << bool_json(receipt.scene_current_for_active_bytes) << ",\n"
         << "  \"counts\": {\n"
         << "    \"requirements\": " << receipt.requirement_count << ",\n"
         << "    \"unresolved_requirements\": "
         << receipt.unresolved_requirement_count << ",\n"
         << "    \"resources\": " << receipt.resource_count << ",\n"
         << "    \"materialized_resources\": "
         << receipt.materialized_resource_count << ",\n"
         << "    \"dirty_resources\": "
         << receipt.dirty_resource_count << ",\n"
         << "    \"missing_project_sessions\": "
         << receipt.missing_project_session_count << ",\n"
         << "    \"domain_objects\": "
         << receipt.domain_object_count << ",\n"
         << "    \"stale_typed_results\": "
         << receipt.stale_typed_result_count << "\n"
         << "  },\n"
         << "  \"domain_counts\": {";

    bool first = true;
    for (const auto& [kind, count] : receipt.domain_counts) {
        if (!first) {
            json << ',';
        }
        json << "\n    " << json_escape(kind) << ": " << count;
        first = false;
    }
    if (!receipt.domain_counts.empty()) {
        json << '\n';
    }
    json << "  }\n"
         << "}\n";
    return json.str();
}

} // namespace dmc::rengine::validation
