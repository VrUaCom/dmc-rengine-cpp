#include "dmc_rengine/item/workspace_manifest.hpp"

#include "dmc_rengine/item/workspace_view.hpp"

#include <algorithm>
#include <sstream>
#include <string_view>
#include <vector>

namespace dmc::rengine::item {
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

std::string workspace_manifest_json(
    const integration::ProjectWorkspace& project,
    const gdspaces::ResourceId& item_resource,
    std::span<const RuntimeChangeRequest> requests) {
    const auto view = build_workspace_view(project, item_resource);
    if (!view.valid()) {
        return {};
    }

    std::vector<const RuntimeEvidenceView*> evidence;
    evidence.reserve(view.runtime_evidence.size());
    for (const auto& item : view.runtime_evidence) {
        evidence.push_back(&item);
    }
    std::sort(
        evidence.begin(), evidence.end(),
        [](const RuntimeEvidenceView* left, const RuntimeEvidenceView* right) {
            return left->record_id < right->record_id;
        });

    std::vector<const RuntimeChangeRequest*> sorted_requests;
    sorted_requests.reserve(requests.size());
    for (const auto& request : requests) {
        if (request.item_resource == item_resource) {
            sorted_requests.push_back(&request);
        }
    }
    std::sort(
        sorted_requests.begin(), sorted_requests.end(),
        [](const RuntimeChangeRequest* left, const RuntimeChangeRequest* right) {
            return left->id < right->id;
        });

    std::ostringstream output;
    output << "{\n"
           << "  \"schema_version\": 1,\n"
           << "  \"resource\": {\n"
           << "    \"canonical_id\": ";
    write_string(output, view.resource.id.canonical());
    output << ",\n    \"logical_path\": ";
    write_string(output, view.resource.id.logical_path);
    output << ",\n    \"display_name\": ";
    write_string(output, view.resource.display_name);
    output << ",\n    \"format\": ";
    write_string(output, view.resource.format);
    output << ",\n    \"profile\": ";
    write_string(output, view.resource.profile);
    output << "\n  },\n"
           << "  \"workspace\": {\n"
           << "    \"status\": ";
    write_string(output, integration::to_string(view.workspace_status));
    output << ",\n    \"format_maturity\": ";
    write_string(output, integration::to_string(view.format_maturity));
    output << ",\n    \"write_policy\": ";
    write_string(output, integration::to_string(view.write_policy));
    output << ",\n    \"working_copy_revision\": "
           << view.working_copy_revision
           << ",\n    \"dirty\": " << (view.dirty ? "true" : "false")
           << ",\n    \"binary_document\": "
           << (view.binary_document ? "true" : "false")
           << ",\n    \"binary_coverage_bytes\": "
           << view.binary_coverage_bytes
           << ",\n    \"diagnostic_count\": "
           << view.diagnostic_count
           << "\n  },\n"
           << "  \"routes\": [";

    if (!view.routes.empty()) {
        output << '\n';
        for (std::size_t index = 0; index < view.routes.size(); ++index) {
            output << "    ";
            write_string(output, view.routes[index]);
            if (index + 1U != view.routes.size()) {
                output << ',';
            }
            output << '\n';
        }
        output << "  ";
    }

    output << "],\n  \"runtime_evidence\": [";
    if (!evidence.empty()) {
        output << '\n';
        for (std::size_t index = 0; index < evidence.size(); ++index) {
            const auto& item = *evidence[index];
            output << "    {\n      \"kind\": ";
            write_string(output, to_string(item.kind));
            output << ",\n      \"record_id\": ";
            write_string(output, item.record_id);
            output << ",\n      \"claim_id\": ";
            write_string(output, item.claim_id);
            output << ",\n      \"confidence\": ";
            write_string(output, evidence::to_string(item.confidence));
            output << ",\n      \"summary\": ";
            write_string(output, item.summary);
            output << ",\n      \"location_complete\": "
                   << (item.location_complete ? "true" : "false")
                   << "\n    }";
            if (index + 1U != evidence.size()) {
                output << ',';
            }
            output << '\n';
        }
        output << "  ";
    }

    output << "],\n  \"runtime_requests\": [";
    if (!sorted_requests.empty()) {
        output << '\n';
        for (std::size_t index = 0; index < sorted_requests.size(); ++index) {
            const auto& request = *sorted_requests[index];
            output << "    {\n      \"id\": ";
            write_string(output, request.id);
            output << ",\n      \"kind\": ";
            write_string(output, to_string(request.kind));
            output << ",\n      \"status\": ";
            write_string(output, to_string(request.status));
            output << ",\n      \"producer\": ";
            write_string(output, gdspaces::to_string(request.producer));
            output << ",\n      \"consumer\": ";
            write_string(output, gdspaces::to_string(request.consumer));
            output << ",\n      \"target_artifact_id\": ";
            write_string(output, request.target_artifact_id);
            output << ",\n      \"requested_value\": "
                   << request.requested_value
                   << ",\n      \"item_revision\": "
                   << request.item_revision
                   << ",\n      \"evidence_record_ids\": [";
            for (std::size_t evidence_index = 0;
                 evidence_index < request.evidence_record_ids.size();
                 ++evidence_index) {
                if (evidence_index != 0U) {
                    output << ", ";
                }
                write_string(output, request.evidence_record_ids[evidence_index]);
            }
            output << "],\n      \"required_guards\": [";
            for (std::size_t guard_index = 0;
                 guard_index < request.required_guards.size();
                 ++guard_index) {
                if (guard_index != 0U) {
                    output << ", ";
                }
                write_string(output, request.required_guards[guard_index]);
            }
            output << "],\n      \"validators\": [";
            for (std::size_t validator_index = 0;
                 validator_index < request.validators.size();
                 ++validator_index) {
                if (validator_index != 0U) {
                    output << ", ";
                }
                write_string(
                    output,
                    gdspaces::to_string(request.validators[validator_index]));
            }
            output << "],\n      \"diagnostics\": [";
            for (std::size_t diagnostic_index = 0;
                 diagnostic_index < request.diagnostics.size();
                 ++diagnostic_index) {
                if (diagnostic_index != 0U) {
                    output << ", ";
                }
                write_string(output, request.diagnostics[diagnostic_index].code);
            }
            output << "]\n    }";
            if (index + 1U != sorted_requests.size()) {
                output << ',';
            }
            output << '\n';
        }
        output << "  ";
    }

    output << "]\n}\n";
    return output.str();
}

} // namespace dmc::rengine::item
