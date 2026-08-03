#include "dmc_rengine/patch/copy_execution_manifest.hpp"

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

std::string copy_execution_manifest_json(
    const integration::ProjectWorkspace& project,
    const PatchCopyArtifact& artifact) {
    if (!artifact.valid()) {
        return {};
    }

    const auto* session = project.find_session(artifact.source_resource);
    const auto* target = project.artifacts().find(artifact.target_artifact_id);
    const auto* execution_node = project.graph().find(artifact.id);
    const auto* rollback_node = project.graph().find(
        artifact.rollback_plan_id);
    if (session == nullptr || target == nullptr ||
        session->executable_context() == nullptr) {
        return {};
    }

    const auto& rollback_patch = artifact.rollback_plan.patches().front();
    std::ostringstream output;
    output << "{\n"
           << "  \"schema_version\": 1,\n"
           << "  \"execution\": {\n"
           << "    \"id\": ";
    write_string(output, artifact.id);
    output << ",\n    \"patch_plan_id\": ";
    write_string(output, artifact.patch_plan_id);
    output << ",\n    \"mode\": \"copy-buffer-only\",\n"
           << "    \"graph_registered\": "
           << (execution_node != nullptr ? "true" : "false")
           << ",\n    \"original_file_write_performed\": false,\n"
           << "    \"immutable_source_modified\": false\n"
           << "  },\n"
           << "  \"source\": {\n"
           << "    \"resource_id\": ";
    write_string(output, artifact.source_resource.canonical());
    output << ",\n    \"logical_path\": ";
    write_string(output, session->resource().id.logical_path);
    output << ",\n    \"artifact_id\": ";
    write_string(output, target->id);
    output << ",\n    \"sha256\": ";
    write_string(output, artifact.source_sha256);
    output << ",\n    \"size\": "
           << session->source_payload().bytes.size()
           << "\n  },\n"
           << "  \"copied_output\": {\n"
           << "    \"sha256\": ";
    write_string(output, artifact.output_sha256);
    output << ",\n    \"size\": " << artifact.output_bytes.size()
           << ",\n    \"bytes_retained_in_memory\": true,\n"
           << "    \"filesystem_path\": null\n"
           << "  },\n"
           << "  \"rollback\": {\n"
           << "    \"id\": ";
    write_string(output, artifact.rollback_plan_id);
    output << ",\n    \"graph_registered\": "
           << (rollback_node != nullptr ? "true" : "false")
           << ",\n    \"target_sha256\": ";
    write_string(output, artifact.rollback_plan.target_sha256());
    output << ",\n    \"restores_sha256\": ";
    write_string(output, artifact.source_sha256);
    output << ",\n    \"verified_exact_restore\": true,\n"
           << "    \"patch_count\": "
           << artifact.rollback_plan.patches().size()
           << ",\n    \"offset\": " << rollback_patch.offset
           << ",\n    \"byte_count\": "
           << rollback_patch.expected.size()
           << "\n  },\n"
           << "  \"workspace_events\": {\n"
           << "    \"patch_copy_executed\": "
           << session->events().by_type(
                  integration::WorkspaceEventType::patch_copy_executed).size()
           << "\n  }\n}\n";
    return output.str();
}

} // namespace dmc::rengine::patch
