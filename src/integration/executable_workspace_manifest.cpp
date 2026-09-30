#include "dmc_rengine/integration/executable_workspace_manifest.hpp"
#include "dmc_rengine/profiles/dmc3/recovered_source_tree.hpp"

#include <algorithm>
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

void write_recovered_source_tree(
    std::ostringstream& output,
    const ExecutableResourceContext& executable) {
    const bool canonical_dmc3 = executable.known_target_hash_match &&
        executable.known_target_id == "dmc3-hdc-phase12-canonical-target";
    output << "  \"recovered_source_tree\": [";
    if (!canonical_dmc3) {
        output << "],\n";
        return;
    }

    const auto& tree = profiles::dmc3::recovered_source_tree();
    if (!tree.empty()) {
        output << '\n';
    }
    for (std::size_t index = 0; index < tree.size(); ++index) {
        const auto& symbol = tree[index];
        output << "    {\n      \"id\": ";
        write_string(output, symbol.id);
        output << ",\n      \"parent_id\": ";
        write_string(output, symbol.parent_id);
        output << ",\n      \"kind\": ";
        write_string(output, profiles::dmc3::to_string(symbol.kind));
        output << ",\n      \"name\": ";
        write_string(output, symbol.name);
        output << ",\n      \"summary\": ";
        write_string(output, symbol.summary);
        output << ",\n      \"status\": ";
        write_string(output, profiles::dmc3::to_string(symbol.status));
        output << ",\n      \"va\": ";
        if (symbol.va.has_value()) {
            output << *symbol.va;
        } else {
            output << "null";
        }
        output << ",\n      \"size\": ";
        if (symbol.size.has_value()) {
            output << *symbol.size;
        } else {
            output << "null";
        }
        output << ",\n      \"evidence_passes\": [";
        for (std::size_t pass_index = 0;
             pass_index < symbol.evidence_passes.size(); ++pass_index) {
            if (pass_index != 0U) {
                output << ',';
            }
            output << symbol.evidence_passes[pass_index];
        }
        output << "],\n      \"attributes\": {";
        if (!symbol.attributes.empty()) {
            output << '\n';
            std::size_t attribute_index = 0U;
            for (const auto& [key, value] : symbol.attributes) {
                output << "        ";
                write_string(output, key);
                output << ": ";
                write_string(output, value);
                if (++attribute_index != symbol.attributes.size()) {
                    output << ',';
                }
                output << '\n';
            }
            output << "      ";
        }
        output << "}\n    }";
        if (index + 1U != tree.size()) {
            output << ',';
        }
        output << '\n';
    }
    if (!tree.empty()) {
        output << "  ";
    }
    output << "],\n";
}

} // namespace

std::string executable_workspace_manifest_json(
    const ProjectWorkspace& project,
    const gdspaces::ResourceId& resource) {
    const auto* session = project.find_session(resource);
    if (session == nullptr || session->executable_context() == nullptr) {
        return {};
    }

    const auto& executable = *session->executable_context();
    auto artifacts = project.artifacts().by_sha256(executable.sha256);
    std::sort(
        artifacts.begin(), artifacts.end(),
        [](const evidence::ArtifactIdentity* left,
           const evidence::ArtifactIdentity* right) {
            return left->id < right->id;
        });

    std::ostringstream output;
    output << "{\n"
           << "  \"schema_version\": 2,\n"
           << "  \"resource\": {\n"
           << "    \"canonical_id\": ";
    write_string(output, session->resource().id.canonical());
    output << ",\n    \"logical_path\": ";
    write_string(output, session->resource().id.logical_path);
    output << ",\n    \"display_name\": ";
    write_string(output, session->resource().display_name);
    output << ",\n    \"format\": ";
    write_string(output, session->resource().format);
    output << ",\n    \"profile\": ";
    write_string(output, session->resource().profile);
    output << "\n  },\n"
           << "  \"pe\": {\n"
           << "    \"sha256\": ";
    write_string(output, executable.sha256);
    output << ",\n    \"kind\": ";
    write_string(output, exe::to_string(executable.image.kind));
    output << ",\n    \"machine\": ";
    write_string(output, exe::to_string(executable.image.machine));
    output << ",\n    \"image_base\": " << executable.image.image_base
           << ",\n    \"entry_point_rva\": "
           << executable.image.entry_point_rva
           << ",\n    \"size_of_image\": "
           << executable.image.size_of_image
           << ",\n    \"size_of_headers\": "
           << executable.image.size_of_headers
           << ",\n    \"subsystem\": " << executable.image.subsystem
           << ",\n    \"section_count\": "
           << executable.image.section_count
           << "\n  },\n"
           << "  \"known_target\": {\n"
           << "    \"id\": ";
    write_string(output, executable.known_target_id);
    output << ",\n    \"name\": ";
    write_string(output, executable.known_target_name);
    output << ",\n    \"hash_match\": "
           << (executable.known_target_hash_match ? "true" : "false")
           << ",\n    \"metadata_match\": "
           << (executable.known_target_metadata_match ? "true" : "false")
           << "\n  },\n"
           << "  \"artifact_matches\": [";

    if (!artifacts.empty()) {
        output << '\n';
        for (std::size_t index = 0; index < artifacts.size(); ++index) {
            const auto& artifact = *artifacts[index];
            output << "    {\n      \"id\": ";
            write_string(output, artifact.id);
            output << ",\n      \"role\": ";
            write_string(output, artifact.role);
            output << ",\n      \"sha256\": ";
            write_string(output, artifact.sha256);
            output << ",\n      \"size\": " << artifact.size
                   << "\n    }";
            if (index + 1U != artifacts.size()) {
                output << ',';
            }
            output << '\n';
        }
        output << "  ";
    }

    output << "],\n  \"evidence_record_ids\": [";
    const auto& evidence_ids = session->evidence_record_ids();
    if (!evidence_ids.empty()) {
        output << '\n';
        for (std::size_t index = 0; index < evidence_ids.size(); ++index) {
            output << "    ";
            write_string(output, evidence_ids[index]);
            if (index + 1U != evidence_ids.size()) {
                output << ',';
            }
            output << '\n';
        }
        output << "  ";
    }

    output << "],\n  \"sections\": [";
    const auto& sections = executable.image.sections;
    if (!sections.empty()) {
        output << '\n';
        for (std::size_t index = 0; index < sections.size(); ++index) {
            const auto& section = sections[index];
            output << "    {\n      \"name\": ";
            write_string(output, section.name);
            output << ",\n      \"virtual_address\": "
                   << section.virtual_address
                   << ",\n      \"virtual_size\": "
                   << section.virtual_size
                   << ",\n      \"raw_offset\": "
                   << section.raw_offset
                   << ",\n      \"raw_size\": "
                   << section.raw_size
                   << ",\n      \"characteristics\": "
                   << section.characteristics
                   << "\n    }";
            if (index + 1U != sections.size()) {
                output << ',';
            }
            output << '\n';
        }
        output << "  ";
    }

    output << "],\n";
    write_recovered_source_tree(output, executable);
    output << "  \"workspace\": {\n"
           << "    \"status\": ";
    write_string(output, to_string(session->status()));
    output << ",\n    \"event_count\": " << session->events().size()
           << ",\n    \"diagnostic_count\": "
           << session->diagnostics().size()
           << ",\n    \"validation_request_count\": "
           << session->events().by_type(
                  WorkspaceEventType::validation_requested).size()
           << "\n  }\n}\n";
    return output.str();
}

} // namespace dmc::rengine::integration
