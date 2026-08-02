#include "dmc_rengine/integration/stage_workspace_manifest.hpp"

#include <algorithm>
#include <map>
#include <sstream>
#include <string>
#include <string_view>
#include <vector>

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

[[nodiscard]] bool has_route(
    const ResourceWorkspaceSession& session,
    gdspaces::ToolTarget target) {
    return std::any_of(
        session.routes().begin(), session.routes().end(),
        [target](const ToolRoute& route) {
            return route.target == target;
        });
}

[[nodiscard]] bool requires_modviz_route(
    gdspaces::StageResourceCategory category) noexcept {
    switch (category) {
    case gdspaces::StageResourceCategory::models:
    case gdspaces::StageResourceCategory::textures:
    case gdspaces::StageResourceCategory::animations:
    case gdspaces::StageResourceCategory::cameras:
    case gdspaces::StageResourceCategory::lighting:
    case gdspaces::StageResourceCategory::positions:
    case gdspaces::StageResourceCategory::effects:
    case gdspaces::StageResourceCategory::collision:
        return true;
    case gdspaces::StageResourceCategory::scripts:
    case gdspaces::StageResourceCategory::events:
    case gdspaces::StageResourceCategory::sounds:
    case gdspaces::StageResourceCategory::unknown:
        return false;
    }
    return false;
}

} // namespace

std::string stage_workspace_manifest_json(
    const ProjectWorkspace& project,
    std::string_view stage_id) {
    auto sessions = project.sessions_for_stage(stage_id);
    if (sessions.empty()) {
        return {};
    }

    std::sort(
        sessions.begin(), sessions.end(),
        [](const ResourceWorkspaceSession* left,
           const ResourceWorkspaceSession* right) {
            return left->resource().id.canonical() <
                   right->resource().id.canonical();
        });

    const auto* first_stage = sessions.front()->stage();
    if (first_stage == nullptr) {
        return {};
    }

    std::map<std::string, std::size_t, std::less<>> category_counts;
    std::size_t dirty_count = 0U;
    std::size_t read_only_count = 0U;
    std::size_t binary_document_count = 0U;
    std::size_t evidence_link_count = 0U;
    std::size_t event_count = 0U;
    std::size_t validation_request_count = 0U;
    std::size_t modviz_relevant_count = 0U;
    bool stage_ops_consistent = true;
    bool modviz_consistent = true;

    for (const auto* session : sessions) {
        const auto* stage = session->stage();
        if (stage == nullptr ||
            stage->identity.stage_id != first_stage->identity.stage_id ||
            stage->identity.profile != first_stage->identity.profile) {
            return {};
        }

        ++category_counts[std::string(gdspaces::to_string(stage->category))];
        if (session->status() == WorkspaceStatus::editable_dirty) {
            ++dirty_count;
        }
        if (session->status() == WorkspaceStatus::read_only) {
            ++read_only_count;
        }
        if (session->binary_document() != nullptr) {
            ++binary_document_count;
        }
        evidence_link_count += session->evidence_record_ids().size();
        event_count += session->events().size();
        validation_request_count += session->events().by_type(
            WorkspaceEventType::validation_requested).size();
        stage_ops_consistent = stage_ops_consistent &&
            has_route(*session, gdspaces::ToolTarget::stage_ops);

        if (requires_modviz_route(stage->category)) {
            ++modviz_relevant_count;
            modviz_consistent = modviz_consistent &&
                has_route(*session, gdspaces::ToolTarget::modviz_scene);
        }
    }

    const auto stage_node_id = "stage:" + first_stage->identity.profile + ':' +
        first_stage->identity.stage_id;
    const auto stage_graph_edges = project.graph().outgoing(stage_node_id).size();

    std::ostringstream output;
    output << "{\n"
           << "  \"schema_version\": 1,\n"
           << "  \"stage\": {\n"
           << "    \"profile\": ";
    write_string(output, first_stage->identity.profile);
    output << ",\n    \"stage_id\": ";
    write_string(output, first_stage->identity.stage_id);
    output << ",\n    \"display_name\": ";
    write_string(output, first_stage->identity.display_name);
    output << ",\n    \"exe_evidence_id\": ";
    write_string(output, first_stage->identity.exe_evidence_id);
    output << "\n  },\n"
           << "  \"summary\": {\n"
           << "    \"resource_count\": " << sessions.size() << ",\n"
           << "    \"dirty_resource_count\": " << dirty_count << ",\n"
           << "    \"read_only_resource_count\": " << read_only_count << ",\n"
           << "    \"binary_document_count\": "
           << binary_document_count << ",\n"
           << "    \"evidence_link_count\": "
           << evidence_link_count << ",\n"
           << "    \"event_count\": " << event_count << ",\n"
           << "    \"validation_request_count\": "
           << validation_request_count << ",\n"
           << "    \"stage_graph_edge_count\": "
           << stage_graph_edges << ",\n"
           << "    \"modviz_relevant_resource_count\": "
           << modviz_relevant_count << ",\n"
           << "    \"stage_ops_route_consistent\": "
           << (stage_ops_consistent ? "true" : "false") << ",\n"
           << "    \"modviz_scene_route_consistent\": "
           << (modviz_consistent ? "true" : "false") << "\n"
           << "  },\n"
           << "  \"category_counts\": {";

    if (!category_counts.empty()) {
        output << '\n';
        std::size_t index = 0U;
        for (const auto& [category, count] : category_counts) {
            output << "    ";
            write_string(output, category);
            output << ": " << count;
            if (++index != category_counts.size()) {
                output << ',';
            }
            output << '\n';
        }
        output << "  ";
    }
    output << "},\n  \"consumers\": [\n"
           << "    \"stage-ops\",\n"
           << "    \"modviz-scene\",\n"
           << "    \"binary-inspector\",\n"
           << "    \"evidence-registry\",\n"
           << "    \"spider-hub\",\n"
           << "    \"build-test-lab\"\n"
           << "  ],\n"
           << "  \"resources\": [\n";

    for (std::size_t index = 0; index < sessions.size(); ++index) {
        const auto& session = *sessions[index];
        const auto* stage = session.stage();
        const auto* format = session.format();
        const auto* binary = session.binary_document();
        const auto* working = session.working_copy();

        output << "    {\n"
               << "      \"canonical_id\": ";
        write_string(output, session.resource().id.canonical());
        output << ",\n      \"logical_path\": ";
        write_string(output, session.resource().id.logical_path);
        output << ",\n      \"display_name\": ";
        write_string(output, session.resource().display_name);
        output << ",\n      \"format\": ";
        write_string(output, session.resource().format);
        output << ",\n      \"category\": ";
        write_string(output, gdspaces::to_string(stage->category));
        output << ",\n      \"role\": ";
        write_string(output, stage->role);
        output << ",\n      \"workspace_status\": ";
        write_string(output, to_string(session.status()));
        output << ",\n      \"format_maturity\": ";
        write_string(
            output,
            format == nullptr ? std::string_view{"unregistered"}
                              : to_string(format->maturity));
        output << ",\n      \"write_policy\": ";
        write_string(
            output,
            format == nullptr ? std::string_view{"read-only"}
                              : to_string(format->write_policy));
        output << ",\n      \"modviz_relevant\": "
               << (requires_modviz_route(stage->category) ? "true" : "false")
               << ",\n      \"binary_document\": "
               << (binary != nullptr ? "true" : "false")
               << ",\n      \"binary_coverage_bytes\": "
               << (binary == nullptr ? 0U : binary->coverage_bytes())
               << ",\n      \"working_copy_revision\": "
               << (working == nullptr ? 0U : working->revision())
               << ",\n      \"working_copy_dirty\": "
               << (working != nullptr && working->dirty() ? "true" : "false")
               << ",\n      \"evidence_record_ids\": ";
        write_string_array(output, session.evidence_record_ids(), "      ");
        output << ",\n      \"event_count\": " << session.events().size()
               << ",\n      \"diagnostic_count\": "
               << session.diagnostics().size()
               << "\n    }";
        if (index + 1U != sessions.size()) {
            output << ',';
        }
        output << '\n';
    }

    output << "  ]\n}\n";
    return output.str();
}

} // namespace dmc::rengine::integration
