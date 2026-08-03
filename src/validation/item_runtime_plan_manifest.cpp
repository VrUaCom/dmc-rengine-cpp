#include "dmc_rengine/validation/item_runtime_plan_manifest.hpp"

#include <algorithm>
#include <sstream>
#include <string>
#include <string_view>
#include <vector>

namespace dmc::rengine::validation {
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

void write_string_array(
    std::ostringstream& output,
    std::vector<std::string> values,
    std::string_view indent) {
    std::sort(values.begin(), values.end());
    values.erase(std::unique(values.begin(), values.end()), values.end());
    output << '[';
    if (!values.empty()) {
        output << '\n';
        for (std::size_t index = 0; index < values.size(); ++index) {
            output << indent;
            write_string(output, values[index]);
            if (index + 1U != values.size()) {
                output << ',';
            }
            output << '\n';
        }
        output << std::string(indent.size() >= 2U ? indent.size() - 2U : 0U, ' ');
    }
    output << ']';
}

} // namespace

std::string item_runtime_plan_manifest_json(
    const integration::ProjectWorkspace& project,
    const ItemRuntimeValidationPlan& plan,
    const item::RuntimeChangeRequest& request) {
    if (!plan.valid() || !request.valid() ||
        plan.runtime_request_id != request.id ||
        plan.item_resource != request.item_resource) {
        return {};
    }

    const auto* item_session = project.find_session(plan.item_resource);
    const auto* artifact = project.artifacts().find(request.target_artifact_id);
    if (item_session == nullptr || artifact == nullptr) {
        return {};
    }

    auto executable_sessions =
        project.executable_sessions_for_artifact(request.target_artifact_id);
    std::sort(
        executable_sessions.begin(), executable_sessions.end(),
        [](const integration::ResourceWorkspaceSession* left,
           const integration::ResourceWorkspaceSession* right) {
            return left->resource().id.canonical() <
                   right->resource().id.canonical();
        });

    std::vector<const ValidationRequirement*> requirements;
    requirements.reserve(plan.requirements.size());
    for (const auto& requirement : plan.requirements) {
        requirements.push_back(&requirement);
    }
    std::sort(
        requirements.begin(), requirements.end(),
        [](const ValidationRequirement* left,
           const ValidationRequirement* right) {
            return left->id < right->id;
        });

    std::vector<std::string> validator_names;
    validator_names.reserve(request.validators.size());
    for (const auto validator : request.validators) {
        validator_names.emplace_back(gdspaces::to_string(validator));
    }

    std::ostringstream output;
    output << "{\n"
           << "  \"schema_version\": 1,\n"
           << "  \"plan\": {\n"
           << "    \"id\": ";
    write_string(output, plan.id);
    output << ",\n    \"runtime_request_id\": ";
    write_string(output, plan.runtime_request_id);
    output << ",\n    \"producer\": ";
    write_string(output, gdspaces::to_string(plan.producer));
    output << ",\n    \"ready_for_execution\": "
           << (plan.ready_for_execution() ? "true" : "false")
           << ",\n    \"graph_registered\": "
           << (project.graph().find(plan.id) != nullptr ? "true" : "false")
           << ",\n    \"requirement_count\": "
           << plan.requirements.size()
           << ",\n    \"blocker_count\": "
           << plan.blockers.size()
           << "\n  },\n"
           << "  \"item_resource\": {\n"
           << "    \"canonical_id\": ";
    write_string(output, item_session->resource().id.canonical());
    output << ",\n    \"logical_path\": ";
    write_string(output, item_session->resource().id.logical_path);
    output << ",\n    \"working_copy_revision\": "
           << request.item_revision
           << ",\n    \"event_count\": "
           << item_session->events().size()
           << ",\n    \"validation_plan_event_count\": "
           << item_session->events().by_type(
                  integration::WorkspaceEventType::validation_plan_created).size()
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
           << "  \"executable_resources\": [";
    if (!executable_sessions.empty()) {
        output << '\n';
        for (std::size_t index = 0; index < executable_sessions.size(); ++index) {
            const auto* session = executable_sessions[index];
            const auto* executable = session->executable_context();
            output << "    {\n      \"canonical_id\": ";
            write_string(output, session->resource().id.canonical());
            output << ",\n      \"logical_path\": ";
            write_string(output, session->resource().id.logical_path);
            output << ",\n      \"sha256\": ";
            write_string(output, executable == nullptr ? std::string_view{} :
                std::string_view{executable->sha256});
            output << ",\n      \"known_target_id\": ";
            write_string(output, executable == nullptr ? std::string_view{} :
                std::string_view{executable->known_target_id});
            output << "\n    }";
            if (index + 1U != executable_sessions.size()) {
                output << ',';
            }
            output << '\n';
        }
        output << "  ";
    }
    output << "],\n  \"blockers\": ";
    write_string_array(output, plan.blockers, "    ");
    output << ",\n  \"requirements\": [";

    if (!requirements.empty()) {
        output << '\n';
        for (std::size_t index = 0; index < requirements.size(); ++index) {
            const auto& requirement = *requirements[index];
            output << "    {\n      \"id\": ";
            write_string(output, requirement.id);
            output << ",\n      \"kind\": ";
            write_string(output, to_string(requirement.kind));
            output << ",\n      \"status\": ";
            write_string(output, to_string(requirement.status));
            output << ",\n      \"mandatory\": "
                   << (requirement.mandatory ? "true" : "false")
                   << ",\n      \"detail\": ";
            write_string(output, requirement.detail);
            output << ",\n      \"evidence_record_ids\": ";
            write_string_array(
                output, requirement.evidence_record_ids, "        ");
            output << "\n    }";
            if (index + 1U != requirements.size()) {
                output << ',';
            }
            output << '\n';
        }
        output << "  ";
    }

    output << "],\n  \"runtime_request\": {\n"
           << "    \"id\": ";
    write_string(output, request.id);
    output << ",\n    \"kind\": ";
    write_string(output, item::to_string(request.kind));
    output << ",\n    \"status\": ";
    write_string(output, item::to_string(request.status));
    output << ",\n    \"producer\": ";
    write_string(output, gdspaces::to_string(request.producer));
    output << ",\n    \"consumer\": ";
    write_string(output, gdspaces::to_string(request.consumer));
    output << ",\n    \"requested_value\": "
           << request.requested_value
           << ",\n    \"evidence_record_ids\": ";
    write_string_array(output, request.evidence_record_ids, "      ");
    output << ",\n    \"required_guards\": ";
    write_string_array(output, request.required_guards, "      ");
    output << ",\n    \"validators\": ";
    write_string_array(output, validator_names, "      ");
    output << "\n  }\n}\n";
    return output.str();
}

} // namespace dmc::rengine::validation
