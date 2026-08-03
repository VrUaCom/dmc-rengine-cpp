#include "dmc_rengine/source/custom_build_manifest.hpp"

#include <algorithm>
#include <sstream>
#include <string_view>
#include <vector>

namespace dmc::rengine::source {
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
    const std::vector<std::string>& values) {
    output << '[';
    for (std::size_t index = 0U; index < values.size(); ++index) {
        if (index != 0U) {
            output << ", ";
        }
        write_string(output, values[index]);
    }
    output << ']';
}

} // namespace

std::string custom_build_manifest_json(
    const integration::ProjectWorkspace& workspace,
    std::string_view custom_build_id) {
    const auto* build = workspace.find_custom_build(custom_build_id);
    if (build == nullptr || !build->valid()) {
        return {};
    }
    const auto* project = workspace.find_source_integration_project(
        build->identity.integration_project_id);
    if (project == nullptr) {
        return {};
    }

    std::vector<const IncludedModification*> modifications;
    for (const auto& modification : build->included_modifications) {
        modifications.push_back(&modification);
    }
    std::sort(
        modifications.begin(), modifications.end(),
        [](const IncludedModification* left,
           const IncludedModification* right) {
            return left->modification_id < right->modification_id ||
                (left->modification_id == right->modification_id &&
                 left->version < right->version);
        });

    std::vector<const SourceBinaryMapping*> mappings;
    for (const auto& mapping : build->source_binary_mappings) {
        mappings.push_back(&mapping);
    }
    std::sort(
        mappings.begin(), mappings.end(),
        [](const SourceBinaryMapping* left,
           const SourceBinaryMapping* right) {
            return left->output_rva < right->output_rva ||
                (left->output_rva == right->output_rva &&
                 left->id < right->id);
        });

    std::vector<const BuildTestResult*> tests;
    for (const auto& test : build->test_results) {
        tests.push_back(&test);
    }
    std::sort(
        tests.begin(), tests.end(),
        [](const BuildTestResult* left, const BuildTestResult* right) {
            return left->id < right->id;
        });

    const auto build_node_id = "custom-build:" +
        build->identity.custom_build_id;
    const auto custom_artifact_id = "custom-executable:" +
        build->identity.custom_build_id;
    const auto artifact_registered = workspace.artifacts().find(
        custom_artifact_id) != nullptr;
    const auto hash_reopens = workspace.find_custom_build_by_sha256(
        build->identity.executable_sha256) == build;

    std::ostringstream output;
    output << "{\n"
           << "  \"schema_version\": 1,\n"
           << "  \"terminology\": {\n"
           << "    \"source_artifact\": \"Composite Source Build\",\n"
           << "    \"binary_artifact\": \"Custom Recompiled Game EXE\",\n"
           << "    \"runtime_plugin_model\": false\n"
           << "  },\n"
           << "  \"identity\": {\n"
           << "    \"custom_build_id\": ";
    write_string(output, build->identity.custom_build_id);
    output << ",\n    \"edition_id\": ";
    write_string(output, build->identity.edition_id);
    output << ",\n    \"semantic_version\": ";
    write_string(output, build->identity.semantic_version);
    output << ",\n    \"integration_project_id\": ";
    write_string(output, build->identity.integration_project_id);
    output << ",\n    \"game_profile\": ";
    write_string(output, build->identity.game_profile);
    output << ",\n    \"original_exe_sha256\": ";
    write_string(output, build->identity.original_exe_sha256);
    output << ",\n    \"source_schema_version\": ";
    write_string(output, build->identity.source_schema_version);
    output << ",\n    \"source_revision\": ";
    write_string(output, build->identity.source_revision);
    output << ",\n    \"source_tree_sha256\": ";
    write_string(output, build->identity.source_tree_sha256);
    output << ",\n    \"executable_sha256\": ";
    write_string(output, build->identity.executable_sha256);
    output << ",\n    \"prior_custom_build_id\": ";
    write_string(output, build->identity.prior_custom_build_id);
    output << ",\n    \"release_channel\": ";
    write_string(output, build->identity.release_channel);
    output << ",\n    \"maintainer_identity\": ";
    write_string(output, build->identity.maintainer_identity);
    output << "\n  },\n"
           << "  \"integration_project\": {\n"
           << "    \"title\": ";
    write_string(output, project->title());
    output << ",\n    \"state\": ";
    write_string(output, to_string(project->state()));
    output << ",\n    \"graph_registered\": "
           << (workspace.graph().find(
                   "source-integration-project:" + project->id()) != nullptr
                   ? "true"
                   : "false")
           << "\n  },\n"
           << "  \"build\": {\n"
           << "    \"status\": ";
    write_string(output, to_string(build->status));
    output << ",\n    \"executable_size\": " << build->executable_size
           << ",\n    \"graph_registered\": "
           << (workspace.graph().find(build_node_id) != nullptr
                   ? "true"
                   : "false")
           << ",\n    \"artifact_registered\": "
           << (artifact_registered ? "true" : "false")
           << ",\n    \"sha256_reopen_lookup\": "
           << (hash_reopens ? "true" : "false")
           << "\n  },\n"
           << "  \"toolchain\": {\n"
           << "    \"compiler_name\": ";
    write_string(output, build->toolchain.compiler_name);
    output << ",\n    \"compiler_version\": ";
    write_string(output, build->toolchain.compiler_version);
    output << ",\n    \"linker_name\": ";
    write_string(output, build->toolchain.linker_name);
    output << ",\n    \"linker_version\": ";
    write_string(output, build->toolchain.linker_version);
    output << ",\n    \"target_triple\": ";
    write_string(output, build->toolchain.target_triple);
    output << ",\n    \"cmake_version\": ";
    write_string(output, build->toolchain.cmake_version);
    output << ",\n    \"compile_flags\": ";
    write_string_array(output, build->toolchain.compile_flags);
    output << ",\n    \"link_flags\": ";
    write_string_array(output, build->toolchain.link_flags);
    output << ",\n    \"dependency_lock_sha256\": ";
    write_string(output, build->toolchain.dependency_lock_sha256);
    output << "\n  },\n"
           << "  \"pe_validation\": {\n"
           << "    \"pe32_plus\": "
           << (build->pe_validation.pe32_plus ? "true" : "false")
           << ",\n    \"machine\": " << build->pe_validation.machine
           << ",\n    \"image_base\": "
           << build->pe_validation.image_base
           << ",\n    \"entry_point_rva\": "
           << build->pe_validation.entry_point_rva
           << ",\n    \"subsystem\": " << build->pe_validation.subsystem
           << ",\n    \"section_count\": "
           << build->pe_validation.section_count
           << ",\n    \"import_table_valid\": "
           << (build->pe_validation.import_table_valid ? "true" : "false")
           << ",\n    \"exception_directory_valid\": "
           << (build->pe_validation.exception_directory_valid
                   ? "true"
                   : "false")
           << ",\n    \"load_config_valid\": "
           << (build->pe_validation.load_config_valid ? "true" : "false")
           << ",\n    \"source_mapping_complete\": "
           << (build->pe_validation.source_mapping_complete
                   ? "true"
                   : "false")
           << ",\n    \"report_sha256\": ";
    write_string(output, build->pe_validation.report_sha256);
    output << "\n  },\n"
           << "  \"included_modifications\": [";
    if (!modifications.empty()) {
        output << '\n';
        for (std::size_t index = 0U; index < modifications.size(); ++index) {
            const auto& modification = *modifications[index];
            output << "    {\n      \"modification_id\": ";
            write_string(output, modification.modification_id);
            output << ",\n      \"version\": ";
            write_string(output, modification.version);
            output << ",\n      \"package_manifest_sha256\": ";
            write_string(output, modification.package_manifest_sha256);
            output << "\n    }";
            if (index + 1U != modifications.size()) {
                output << ',';
            }
            output << '\n';
        }
        output << "  ";
    }
    output << "],\n  \"unified_implementation_decision_ids\": ";
    write_string_array(
        output,
        build->unified_implementation_decision_ids);
    output << ",\n  \"source_binary_mappings\": [";
    if (!mappings.empty()) {
        output << '\n';
        for (std::size_t index = 0U; index < mappings.size(); ++index) {
            const auto& mapping = *mappings[index];
            output << "    {\n      \"id\": ";
            write_string(output, mapping.id);
            output << ",\n      \"modification_id\": ";
            write_string(output, mapping.modification_id);
            output << ",\n      \"modification_version\": ";
            write_string(output, mapping.modification_version);
            output << ",\n      \"source_unit_id\": ";
            write_string(output, mapping.source_unit_id);
            output << ",\n      \"source_path\": ";
            write_string(output, mapping.source_path);
            output << ",\n      \"source_line_begin\": "
                   << mapping.source_line_begin
                   << ",\n      \"source_line_end\": "
                   << mapping.source_line_end
                   << ",\n      \"recovered_symbol_id\": ";
            write_string(output, mapping.recovered_symbol_id);
            output << ",\n      \"recovered_symbol_name\": ";
            write_string(output, mapping.recovered_symbol_name);
            output << ",\n      \"original_rva\": ";
            if (mapping.original_rva.has_value()) {
                output << *mapping.original_rva;
            } else {
                output << "null";
            }
            output << ",\n      \"output_file_offset\": "
                   << mapping.output_file_offset
                   << ",\n      \"output_rva\": "
                   << mapping.output_rva
                   << ",\n      \"output_va\": "
                   << mapping.output_va
                   << ",\n      \"byte_size\": "
                   << mapping.byte_size
                   << ",\n      \"confidence\": ";
            write_string(output, to_string(mapping.confidence));
            output << ",\n      \"evidence_record_ids\": ";
            write_string_array(output, mapping.evidence_record_ids);
            output << "\n    }";
            if (index + 1U != mappings.size()) {
                output << ',';
            }
            output << '\n';
        }
        output << "  ";
    }
    output << "],\n  \"test_results\": [";
    if (!tests.empty()) {
        output << '\n';
        for (std::size_t index = 0U; index < tests.size(); ++index) {
            const auto& test = *tests[index];
            output << "    {\n      \"id\": ";
            write_string(output, test.id);
            output << ",\n      \"layer\": ";
            write_string(output, to_string(test.layer));
            output << ",\n      \"profile\": ";
            write_string(output, test.profile);
            output << ",\n      \"mandatory\": "
                   << (test.mandatory ? "true" : "false")
                   << ",\n      \"passed\": "
                   << (test.passed ? "true" : "false")
                   << ",\n      \"summary\": ";
            write_string(output, test.summary);
            output << ",\n      \"report_sha256\": ";
            write_string(output, test.report_sha256);
            output << ",\n      \"evidence_record_ids\": ";
            write_string_array(output, test.evidence_record_ids);
            output << "\n    }";
            if (index + 1U != tests.size()) {
                output << ',';
            }
            output << '\n';
        }
        output << "  ";
    }
    output << "],\n"
           << "  \"resource_package_versions\": ";
    write_string_array(output, build->resource_package_versions);
    output << ",\n  \"known_issues\": ";
    write_string_array(output, build->known_issues);
    output << ",\n  \"credits\": ";
    write_string_array(output, build->credits);
    output << ",\n  \"distribution\": {\n"
           << "    \"form\": ";
    write_string(output, to_string(build->distribution_form));
    output << ",\n    \"permission\": ";
    write_string(output, build->distribution_permission);
    output << ",\n    \"attestation_state\": ";
    write_string(output, to_string(build->attestation_state));
    output << ",\n    \"attestation_reference\": ";
    write_string(output, build->attestation_reference);
    output << ",\n    \"rollback_build_id\": ";
    write_string(output, build->rollback_build_id);
    output << "\n  }\n}\n";
    return output.str();
}

} // namespace dmc::rengine::source
