#include "dmc_rengine/integration/workspace_manifest.hpp"

#include <sstream>
#include <string_view>

namespace dmc::rengine::integration {
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
    const std::vector<std::string>& values,
    std::string_view indent) {
    output << '[';
    if (!values.empty()) {
        output << '\n';
        for (std::size_t index = 0; index < values.size(); ++index) {
            output << indent << "  ";
            write_string(output, values[index]);
            if (index + 1U != values.size()) {
                output << ',';
            }
            output << '\n';
        }
        output << indent;
    }
    output << ']';
}

} // namespace

std::string workspace_manifest_json(
    const ResourceWorkspaceSession& workspace) {
    if (!workspace.resource().valid()) {
        return {};
    }

    const auto& resource = workspace.resource();
    const auto* format = workspace.format();
    const auto* binary = workspace.binary_document();
    const auto* stage = workspace.stage();
    const auto* working = workspace.working_copy();

    std::ostringstream output;
    output << "{\n"
           << "  \"schema_version\": 1,\n"
           << "  \"resource\": {\n"
           << "    \"canonical_id\": ";
    write_string(output, resource.id.canonical());
    output << ",\n    \"source_id\": ";
    write_string(output, resource.id.source_id);
    output << ",\n    \"logical_path\": ";
    write_string(output, resource.id.logical_path);
    output << ",\n    \"container_chain\": ";
    write_string(output, resource.id.container_chain);
    output << ",\n    \"offset\": " << resource.id.offset
           << ",\n    \"size\": " << resource.id.size
           << ",\n    \"display_name\": ";
    write_string(output, resource.display_name);
    output << ",\n    \"format\": ";
    write_string(output, resource.format);
    output << ",\n    \"profile\": ";
    write_string(output, resource.profile);
    output << ",\n    \"container\": "
           << (resource.container ? "true" : "false")
           << "\n  },\n"
           << "  \"workspace_status\": ";
    write_string(output, to_string(workspace.status()));
    output << ",\n  \"context\": {\n"
           << "    \"stage\": "
           << (workspace.context().stage_context ? "true" : "false")
           << ",\n    \"menu\": "
           << (workspace.context().menu_context ? "true" : "false")
           << ",\n    \"evidence\": "
           << (workspace.context().evidence_context ? "true" : "false")
           << "\n  },\n"
           << "  \"format_integration\": ";

    if (format == nullptr) {
        output << "null";
    } else {
        output << "{\n    \"parser_id\": ";
        write_string(output, format->parser_id);
        output << ",\n    \"maturity\": ";
        write_string(output, to_string(format->maturity));
        output << ",\n    \"write_policy\": ";
        write_string(output, to_string(format->write_policy));
        output << ",\n    \"binary_adapter\": "
               << (format->binary_adapter ? "true" : "false")
               << ",\n    \"stage_category\": ";
        if (format->stage_category.has_value()) {
            write_string(output, gdspaces::to_string(*format->stage_category));
        } else {
            output << "null";
        }
        output << ",\n    \"evidence_claim_ids\": ";
        write_string_array(output, format->evidence_claim_ids, "    ");
        output << ",\n    \"limitations\": ";
        write_string_array(output, format->limitations, "    ");
        output << "\n  }";
    }

    output << ",\n  \"tool_routes\": [";
    const auto& routes = workspace.routes();
    if (!routes.empty()) {
        output << '\n';
        for (std::size_t index = 0; index < routes.size(); ++index) {
            const auto& route = routes[index];
            output << "    {\n      \"target\": ";
            write_string(output, gdspaces::to_string(route.target));
            output << ",\n      \"role\": ";
            write_string(output, to_string(route.role));
            output << ",\n      \"reason\": ";
            write_string(output, route.reason);
            output << "\n    }";
            if (index + 1U != routes.size()) {
                output << ',';
            }
            output << '\n';
        }
        output << "  ";
    }
    output << "],\n  \"evidence_record_ids\": ";
    write_string_array(output, workspace.evidence_record_ids(), "  ");

    output << ",\n  \"stage_context\": ";
    if (stage == nullptr) {
        output << "null";
    } else {
        output << "{\n    \"stage_id\": ";
        write_string(output, stage->identity.stage_id);
        output << ",\n    \"display_name\": ";
        write_string(output, stage->identity.display_name);
        output << ",\n    \"profile\": ";
        write_string(output, stage->identity.profile);
        output << ",\n    \"exe_evidence_id\": ";
        write_string(output, stage->identity.exe_evidence_id);
        output << ",\n    \"category\": ";
        write_string(output, gdspaces::to_string(stage->category));
        output << ",\n    \"role\": ";
        write_string(output, stage->role);
        output << "\n  }";
    }

    output << ",\n  \"binary_document\": ";
    if (binary == nullptr) {
        output << "null";
    } else {
        output << "{\n    \"byte_size\": " << binary->byte_size()
               << ",\n    \"coverage_bytes\": "
               << binary->coverage_bytes()
               << ",\n    \"region_count\": "
               << binary->regions().size()
               << ",\n    \"field_count\": "
               << binary->fields().size()
               << ",\n    \"ownership_count\": "
               << binary->ownership().size()
               << ",\n    \"annotation_count\": "
               << binary->annotations().size()
               << ",\n    \"unknown_range_count\": "
               << binary->unknown_ranges().size()
               << ",\n    \"region_conflict_count\": "
               << binary->conflicts().size()
               << ",\n    \"ownership_conflict_count\": "
               << binary->ownership_conflicts().size()
               << "\n  }";
    }

    output << ",\n  \"working_copy\": ";
    if (working == nullptr) {
        output << "null";
    } else {
        output << "{\n    \"source_sha256\": ";
        write_string(output, working->source_sha256());
        output << ",\n    \"revision\": " << working->revision()
               << ",\n    \"dirty\": "
               << (working->dirty() ? "true" : "false")
               << ",\n    \"byte_size\": " << working->bytes().size()
               << ",\n    \"operation_count\": "
               << working->history().size()
               << "\n  }";
    }

    output << ",\n  \"events\": [";
    const auto& events = workspace.events().events();
    if (!events.empty()) {
        output << '\n';
        for (std::size_t index = 0; index < events.size(); ++index) {
            const auto& event = events[index];
            output << "    {\n      \"sequence\": " << event.sequence
                   << ",\n      \"type\": ";
            write_string(output, to_string(event.type));
            output << ",\n      \"producer\": ";
            write_string(output, gdspaces::to_string(event.producer));
            output << ",\n      \"consumer\": ";
            if (event.consumer.has_value()) {
                write_string(output, gdspaces::to_string(*event.consumer));
            } else {
                output << "null";
            }
            output << ",\n      \"revision\": " << event.revision
                   << ",\n      \"subject_id\": ";
            write_string(output, event.subject_id);
            output << ",\n      \"message\": ";
            write_string(output, event.message);
            output << "\n    }";
            if (index + 1U != events.size()) {
                output << ',';
            }
            output << '\n';
        }
        output << "  ";
    }

    output << "],\n  \"diagnostics\": [";
    const auto& diagnostics = workspace.diagnostics();
    if (!diagnostics.empty()) {
        output << '\n';
        for (std::size_t index = 0; index < diagnostics.size(); ++index) {
            const auto& diagnostic = diagnostics[index];
            output << "    {\n      \"severity\": ";
            write_string(output, gdspaces::to_string(diagnostic.severity));
            output << ",\n      \"code\": ";
            write_string(output, diagnostic.code);
            output << ",\n      \"message\": ";
            write_string(output, diagnostic.message);
            output << "\n    }";
            if (index + 1U != diagnostics.size()) {
                output << ',';
            }
            output << '\n';
        }
        output << "  ";
    }

    output << "]\n}\n";
    return output.str();
}

} // namespace dmc::rengine::integration
