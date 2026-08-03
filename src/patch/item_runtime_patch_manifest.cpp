#include "dmc_rengine/patch/item_runtime_patch_manifest.hpp"

#include <sstream>
#include <string_view>

namespace dmc::rengine::patch {
namespace {

[[nodiscard]] std::string escape_json(std::string_view value) {
    std::ostringstream output;
    constexpr char hex[] = "0123456789abcdef";
    for (const unsigned char character : value) {
        switch (character) {
        case '"': output << "\\\""; break;
        case '\\': output << "\\\\"; break;
        case '\b': output << "\\b"; break;
        case '\f': output << "\\f"; break;
        case '\n': output << "\\n"; break;
        case '\r': output << "\\r"; break;
        case '\t': output << "\\t"; break;
        default:
            if (character < 0x20U) {
                output << "\\u00"
                       << hex[(character >> 4U) & 0x0FU]
                       << hex[character & 0x0FU];
            } else {
                output << static_cast<char>(character);
            }
            break;
        }
    }
    return output.str();
}

void write_string(std::ostringstream& output, std::string_view value) {
    output << '"' << escape_json(value) << '"';
}

} // namespace

std::string item_runtime_patch_plan_manifest_json(
    const integration::ProjectWorkspace& project,
    const CompiledItemRuntimePatchPlan& compiled) {
    if (!compiled.valid()) {
        return {};
    }

    const auto& metadata = compiled.metadata;
    const auto* item_session = project.find_session(metadata.item_resource);
    const auto* executable_session = project.find_session(
        metadata.executable_resource);
    const auto* artifact = project.artifacts().find(
        metadata.target_artifact_id);
    const auto* record = project.evidence().find(
        metadata.evidence_record_id);
    if (item_session == nullptr || executable_session == nullptr ||
        artifact == nullptr || record == nullptr ||
        executable_session->executable_context() == nullptr) {
        return {};
    }

    const auto& byte_patch = compiled.guarded_plan.patches().front();
    std::ostringstream output;
    output << "{\n"
           << "  \"schema_version\": 1,\n"
           << "  \"patch_plan\": {\n"
           << "    \"id\": ";
    write_string(output, metadata.id);
    output << ",\n    \"runtime_request_id\": ";
    write_string(output, metadata.runtime_request_id);
    output << ",\n    \"validation_plan_id\": ";
    write_string(output, metadata.validation_plan_id);
    output << ",\n    \"graph_registered\": "
           << (project.graph().find(metadata.id) != nullptr ? "true" : "false")
           << ",\n    \"applied\": false,\n"
           << "    \"source_modified\": false,\n"
           << "    \"patch_count\": "
           << compiled.guarded_plan.patches().size()
           << "\n  },\n"
           << "  \"item_resource\": {\n"
           << "    \"canonical_id\": ";
    write_string(output, metadata.item_resource.canonical());
    output << ",\n    \"logical_path\": ";
    write_string(output, item_session->resource().id.logical_path);
    output << ",\n    \"patch_plan_event_count\": "
           << item_session->events().by_type(
                  integration::WorkspaceEventType::patch_plan_compiled).size()
           << "\n  },\n"
           << "  \"executable_resource\": {\n"
           << "    \"canonical_id\": ";
    write_string(output, metadata.executable_resource.canonical());
    output << ",\n    \"logical_path\": ";
    write_string(output, executable_session->resource().id.logical_path);
    output << ",\n    \"source_sha256\": ";
    write_string(output, metadata.source_sha256);
    output << ",\n    \"source_size\": "
           << executable_session->source_payload().bytes.size()
           << "\n  },\n"
           << "  \"target_artifact\": {\n"
           << "    \"id\": ";
    write_string(output, artifact->id);
    output << ",\n    \"role\": ";
    write_string(output, artifact->role);
    output << ",\n    \"sha256\": ";
    write_string(output, artifact->sha256);
    output << ",\n    \"size\": " << artifact->size
           << "\n  },\n"
           << "  \"evidence\": {\n"
           << "    \"record_id\": ";
    write_string(output, record->id);
    output << ",\n    \"claim_id\": ";
    write_string(output, record->claim_id);
    output << ",\n    \"confidence\": ";
    write_string(output, evidence::to_string(record->confidence));
    output << ",\n    \"symbol\": ";
    write_string(output, record->locations.empty()
        ? std::string_view{}
        : std::string_view{record->locations.front().symbol});
    output << "\n  },\n"
           << "  \"resolved_location\": {\n"
           << "    \"file_offset\": " << metadata.file_offset
           << ",\n    \"rva\": " << metadata.rva
           << ",\n    \"va\": " << metadata.va
           << ",\n    \"size\": " << metadata.size
           << "\n  },\n"
           << "  \"byte_patch\": {\n"
           << "    \"offset\": " << byte_patch.offset
           << ",\n    \"expected_hex\": ";
    write_string(output, metadata.expected_hex);
    output << ",\n    \"replacement_hex\": ";
    write_string(output, metadata.replacement_hex);
    output << ",\n    \"description\": ";
    write_string(output, metadata.description);
    output << "\n  },\n"
           << "  \"safety\": {\n"
           << "    \"immutable_source_verified\": true,\n"
           << "    \"expected_bytes_verified\": true,\n"
           << "    \"fixed_size_replacement\": true,\n"
           << "    \"guarded_plan_only\": true,\n"
           << "    \"original_file_write_performed\": false\n"
           << "  }\n}\n";
    return output.str();
}

} // namespace dmc::rengine::patch
