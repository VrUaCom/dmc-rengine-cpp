#include "dmc_rengine/source/integration_manifest.hpp"

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
    const std::vector<std::string>& values,
    std::size_t indent) {
    output << '[';
    if (!values.empty()) {
        output << '\n';
        const std::string prefix(indent, ' ');
        for (std::size_t index = 0U; index < values.size(); ++index) {
            output << prefix;
            write_string(output, values[index]);
            if (index + 1U != values.size()) {
                output << ',';
            }
            output << '\n';
        }
        output << std::string(indent - 2U, ' ');
    }
    output << ']';
}

[[nodiscard]] std::string modification_node_id(
    const SourceModificationPackage& package) {
    return "source-modification:" + package.id + '@' + package.version;
}

} // namespace

std::string integration_project_manifest_json(
    const integration::ProjectWorkspace& workspace,
    std::string_view project_id) {
    const auto* project = workspace.find_source_integration_project(project_id);
    if (project == nullptr || !project->valid()) {
        return {};
    }

    std::vector<const SourceModificationPackage*> modifications;
    for (const auto& package : project->selected_modifications()) {
        modifications.push_back(&package);
    }
    std::sort(
        modifications.begin(), modifications.end(),
        [](const SourceModificationPackage* left,
           const SourceModificationPackage* right) {
            return std::tie(left->id, left->version) <
                   std::tie(right->id, right->version);
        });

    std::vector<const ConflictRecord*> conflicts;
    for (const auto& conflict : project->conflicts()) {
        conflicts.push_back(&conflict);
    }
    std::sort(
        conflicts.begin(), conflicts.end(),
        [](const ConflictRecord* left, const ConflictRecord* right) {
            return left->id < right->id;
        });

    std::vector<const DecisionRecord*> decisions;
    for (const auto& decision : project->decisions()) {
        decisions.push_back(&decision);
    }
    std::sort(
        decisions.begin(), decisions.end(),
        [](const DecisionRecord* left, const DecisionRecord* right) {
            return left->id < right->id;
        });

    std::vector<const IntegrationGateEvidence*> gates;
    for (const auto& gate : project->gates()) {
        gates.push_back(&gate);
    }
    std::sort(
        gates.begin(), gates.end(),
        [](const IntegrationGateEvidence* left,
           const IntegrationGateEvidence* right) {
            return left->id < right->id;
        });

    const auto project_node_id =
        "source-integration-project:" + project->id();
    std::ostringstream output;
    output << "{\n"
           << "  \"schema_version\": 1,\n"
           << "  \"terminology\": {\n"
           << "    \"primary_artifact\": \"Composite Source Build\",\n"
           << "    \"final_executable\": \"Custom Recompiled Game EXE\",\n"
           << "    \"runtime_plugin_model\": false\n"
           << "  },\n"
           << "  \"project\": {\n"
           << "    \"id\": ";
    write_string(output, project->id());
    output << ",\n    \"title\": ";
    write_string(output, project->title());
    output << ",\n    \"state\": ";
    write_string(output, to_string(project->state()));
    output << ",\n    \"failure_reason\": ";
    write_string(output, project->failure_reason());
    output << ",\n    \"graph_registered\": "
           << (workspace.graph().find(project_node_id) != nullptr
                   ? "true"
                   : "false")
           << "\n  },\n"
           << "  \"baseline\": {\n"
           << "    \"game_profile\": ";
    write_string(output, project->baseline().game_profile);
    output << ",\n    \"original_exe_sha256\": ";
    write_string(output, project->baseline().original_exe_sha256);
    output << ",\n    \"source_schema_version\": ";
    write_string(output, project->baseline().source_schema_version);
    output << ",\n    \"source_revision\": ";
    write_string(output, project->baseline().source_revision);
    output << ",\n    \"compiler_profile\": ";
    write_string(output, project->baseline().compiler_profile);
    output << ",\n    \"prior_custom_build_id\": ";
    write_string(output, project->baseline().prior_custom_build_id);
    output << "\n  },\n"
           << "  \"governance\": {\n"
           << "    \"maintainers\": ";
    write_string_array(output, project->maintainers(), 6U);
    output << ",\n    \"approval_policy\": ";
    write_string(output, project->approval_policy());
    output << ",\n    \"build_profiles\": ";
    write_string_array(output, project->build_profiles(), 6U);
    output << ",\n    \"test_matrix\": ";
    write_string_array(output, project->test_matrix(), 6U);
    output << "\n  },\n"
           << "  \"modifications\": [";

    if (!modifications.empty()) {
        output << '\n';
        for (std::size_t modification_index = 0U;
             modification_index < modifications.size();
             ++modification_index) {
            const auto& package = *modifications[modification_index];
            std::vector<const SourceChangeUnit*> units;
            for (const auto& unit : package.change_units) {
                units.push_back(&unit);
            }
            std::sort(
                units.begin(), units.end(),
                [](const SourceChangeUnit* left, const SourceChangeUnit* right) {
                    return left->id < right->id;
                });

            output << "    {\n      \"id\": ";
            write_string(output, package.id);
            output << ",\n      \"version\": ";
            write_string(output, package.version);
            output << ",\n      \"title\": ";
            write_string(output, package.title);
            output << ",\n      \"description\": ";
            write_string(output, package.description);
            output << ",\n      \"implementation_kind\": ";
            write_string(output, to_string(package.implementation_kind));
            output << ",\n      \"semantic_intent\": ";
            write_string(output, package.semantic_intent);
            output << ",\n      \"graph_registered\": "
                   << (workspace.graph().find(
                           modification_node_id(package)) != nullptr
                           ? "true"
                           : "false")
                   << ",\n      \"change_units\": [";
            if (!units.empty()) {
                output << '\n';
                for (std::size_t unit_index = 0U;
                     unit_index < units.size();
                     ++unit_index) {
                    const auto& unit = *units[unit_index];
                    std::vector<const SourceSymbolRef*> symbols;
                    for (const auto& symbol : unit.affected_symbols) {
                        symbols.push_back(&symbol);
                    }
                    std::sort(
                        symbols.begin(), symbols.end(),
                        [](const SourceSymbolRef* left,
                           const SourceSymbolRef* right) {
                            return left->id < right->id;
                        });

                    output << "\n        {\n          \"id\": ";
                    write_string(output, unit.id);
                    output << ",\n          \"source_path\": ";
                    write_string(output, unit.source_path);
                    output << ",\n          \"baseline_sha256\": ";
                    write_string(output, unit.baseline_sha256);
                    output << ",\n          \"result_sha256\": ";
                    write_string(output, unit.result_sha256);
                    output << ",\n          \"semantic_intent\": ";
                    write_string(output, unit.semantic_intent);
                    output << ",\n          \"affected_symbols\": [";
                    if (!symbols.empty()) {
                        output << '\n';
                        for (std::size_t symbol_index = 0U;
                             symbol_index < symbols.size();
                             ++symbol_index) {
                            const auto& symbol = *symbols[symbol_index];
                            output << "            {\n"
                                   << "              \"id\": ";
                            write_string(output, symbol.id);
                            output << ",\n              \"kind\": ";
                            write_string(output, to_string(symbol.kind));
                            output << ",\n              \"recovered_name\": ";
                            write_string(output, symbol.recovered_name);
                            output << ",\n              \"rva\": ";
                            if (symbol.rva.has_value()) {
                                output << *symbol.rva;
                            } else {
                                output << "null";
                            }
                            output << ",\n              \"evidence_record_ids\": ";
                            write_string_array(
                                output,
                                symbol.evidence_record_ids,
                                16U);
                            output << "\n            }";
                            if (symbol_index + 1U != symbols.size()) {
                                output << ',';
                            }
                            output << '\n';
                        }
                        output << "          ";
                    }
                    output << "]\n        }";
                    if (unit_index + 1U != units.size()) {
                        output << ',';
                    }
                }
                output << '\n' << "      ";
            }
            output << "],\n      \"dependencies\": [";
            if (!package.dependencies.empty()) {
                output << '\n';
                for (std::size_t index = 0U;
                     index < package.dependencies.size();
                     ++index) {
                    const auto& dependency = package.dependencies[index];
                    output << "        {\n          \"modification_id\": ";
                    write_string(output, dependency.modification_id);
                    output << ",\n          \"version_constraint\": ";
                    write_string(output, dependency.version_constraint);
                    output << ",\n          \"mandatory\": "
                           << (dependency.mandatory ? "true" : "false")
                           << ",\n          \"rationale\": ";
                    write_string(output, dependency.rationale);
                    output << "\n        }";
                    if (index + 1U != package.dependencies.size()) {
                        output << ',';
                    }
                    output << '\n';
                }
                output << "      ";
            }
            output << "],\n      \"configuration_keys\": ";
            write_string_array(output, package.configuration_keys, 8U);
            output << ",\n      \"build_requirements\": ";
            write_string_array(output, package.build_requirements, 8U);
            output << ",\n      \"tests\": [";
            if (!package.tests.empty()) {
                output << '\n';
                for (std::size_t index = 0U; index < package.tests.size(); ++index) {
                    const auto& test = package.tests[index];
                    output << "        {\n          \"id\": ";
                    write_string(output, test.id);
                    output << ",\n          \"layer\": ";
                    write_string(output, to_string(test.layer));
                    output << ",\n          \"profile\": ";
                    write_string(output, test.profile);
                    output << ",\n          \"expected_result\": ";
                    write_string(output, test.expected_result);
                    output << ",\n          \"mandatory\": "
                           << (test.mandatory ? "true" : "false")
                           << "\n        }";
                    if (index + 1U != package.tests.size()) {
                        output << ',';
                    }
                    output << '\n';
                }
                output << "      ";
            }
            output << "],\n      \"evidence_record_ids\": ";
            write_string_array(output, package.evidence_record_ids, 8U);
            output << ",\n      \"save_compatibility\": ";
            write_string(output, package.save_compatibility);
            output << ",\n      \"migration_notes\": ";
            write_string(output, package.migration_notes);
            output << ",\n      \"rollback_instructions\": ";
            write_string(output, package.rollback_instructions);
            output << ",\n      \"known_risks\": ";
            write_string_array(output, package.known_risks, 8U);
            output << ",\n      \"provenance\": {\n"
                   << "        \"authors\": ";
            write_string_array(output, package.provenance.authors, 10U);
            output << ",\n        \"contribution_ledger_refs\": ";
            write_string_array(
                output,
                package.provenance.contribution_ledger_refs,
                10U);
            output << ",\n        \"reviewers\": ";
            write_string_array(output, package.provenance.reviewers, 10U);
            output << ",\n        \"ai_assistance_disclosure\": ";
            write_string(output, package.provenance.ai_assistance_disclosure);
            output << ",\n        \"licence\": ";
            write_string(output, package.provenance.licence);
            output << ",\n        \"redistribution_permission\": ";
            write_string(output, package.provenance.redistribution_permission);
            output << "\n      }\n    }";
            if (modification_index + 1U != modifications.size()) {
                output << ',';
            }
            output << '\n';
        }
        output << "  ";
    }
    output << "],\n  \"dependency_report\": {\n"
           << "    \"passed\": "
           << (project->dependency_report().passed() ? "true" : "false")
           << ",\n    \"blocking_count\": "
           << project->dependency_report().blocking_count()
           << ",\n    \"issues\": [";
    if (!project->dependency_report().issues.empty()) {
        output << '\n';
        for (std::size_t index = 0U;
             index < project->dependency_report().issues.size();
             ++index) {
            const auto& issue = project->dependency_report().issues[index];
            output << "      {\n        \"owner_modification_id\": ";
            write_string(output, issue.owner_modification_id);
            output << ",\n        \"dependency_modification_id\": ";
            write_string(output, issue.dependency_modification_id);
            output << ",\n        \"required_version\": ";
            write_string(output, issue.required_version);
            output << ",\n        \"actual_version\": ";
            write_string(output, issue.actual_version);
            output << ",\n        \"kind\": ";
            write_string(output, to_string(issue.kind));
            output << ",\n        \"blocking\": "
                   << (issue.blocking ? "true" : "false")
                   << ",\n        \"rationale\": ";
            write_string(output, issue.rationale);
            output << "\n      }";
            if (index + 1U != project->dependency_report().issues.size()) {
                output << ',';
            }
            output << '\n';
        }
        output << "    ";
    }
    output << "]\n  },\n"
           << "  \"conflict_register\": {\n"
           << "    \"unresolved_blocking_count\": "
           << project->unresolved_blocking_conflict_count()
           << ",\n    \"records\": [";
    if (!conflicts.empty()) {
        output << '\n';
        for (std::size_t index = 0U; index < conflicts.size(); ++index) {
            const auto& conflict = *conflicts[index];
            output << "      {\n        \"id\": ";
            write_string(output, conflict.id);
            output << ",\n        \"left_modification_id\": ";
            write_string(output, conflict.left_modification_id);
            output << ",\n        \"right_modification_id\": ";
            write_string(output, conflict.right_modification_id);
            output << ",\n        \"class\": ";
            write_string(output, to_string(conflict.conflict_class));
            output << ",\n        \"origin\": ";
            write_string(output, to_string(conflict.origin));
            output << ",\n        \"subject\": ";
            write_string(output, conflict.subject);
            output << ",\n        \"blocking\": "
                   << (conflict.blocking ? "true" : "false")
                   << ",\n        \"status\": ";
            write_string(output, to_string(conflict.status));
            output << ",\n        \"decision_record_id\": ";
            write_string(output, conflict.decision_record_id);
            output << ",\n        \"rationale\": ";
            write_string(output, conflict.rationale);
            output << ",\n        \"resolution\": ";
            write_string(output, conflict.resolution);
            output << "\n      }";
            if (index + 1U != conflicts.size()) {
                output << ',';
            }
            output << '\n';
        }
        output << "    ";
    }
    output << "]\n  },\n"
           << "  \"decision_records\": [";
    if (!decisions.empty()) {
        output << '\n';
        for (std::size_t index = 0U; index < decisions.size(); ++index) {
            const auto& decision = *decisions[index];
            output << "    {\n      \"id\": ";
            write_string(output, decision.id);
            output << ",\n      \"title\": ";
            write_string(output, decision.title);
            output << ",\n      \"rationale\": ";
            write_string(output, decision.rationale);
            output << ",\n      \"authors\": ";
            write_string_array(output, decision.authors, 8U);
            output << ",\n      \"evidence_record_ids\": ";
            write_string_array(output, decision.evidence_record_ids, 8U);
            output << ",\n      \"affected_conflict_ids\": ";
            write_string_array(output, decision.affected_conflict_ids, 8U);
            output << "\n    }";
            if (index + 1U != decisions.size()) {
                output << ',';
            }
            output << '\n';
        }
        output << "  ";
    }
    output << "],\n  \"gates\": [";
    if (!gates.empty()) {
        output << '\n';
        for (std::size_t index = 0U; index < gates.size(); ++index) {
            const auto& gate = *gates[index];
            output << "    {\n      \"id\": ";
            write_string(output, gate.id);
            output << ",\n      \"kind\": ";
            write_string(output, to_string(gate.kind));
            output << ",\n      \"passed\": "
                   << (gate.passed ? "true" : "false")
                   << ",\n      \"summary\": ";
            write_string(output, gate.summary);
            output << ",\n      \"evidence_record_ids\": ";
            write_string_array(output, gate.evidence_record_ids, 8U);
            output << "\n    }";
            if (index + 1U != gates.size()) {
                output << ',';
            }
            output << '\n';
        }
        output << "  ";
    }
    output << "]\n}\n";
    return output.str();
}

} // namespace dmc::rengine::source
