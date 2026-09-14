#include "dmc_rengine/exe/function_map_report.hpp"

#include "json_writer.hpp"

#include <cstdint>
#include <sstream>

namespace dmc::rengine::exe {
namespace {

using detail::JsonWriter;

void write_summary(JsonWriter& writer, const FunctionMap& map) {
    const auto& summary = map.summary;
    writer.key("summary");
    writer.begin_object();
    writer.member("functions", static_cast<std::uint64_t>(summary.functions));
    writer.member("walks_complete", static_cast<std::uint64_t>(summary.walks_complete));
    writer.member("attributed", static_cast<std::uint64_t>(summary.attributed));
    writer.member("with_virtual_binding",
                  static_cast<std::uint64_t>(summary.with_virtual_binding));
    writer.member("with_import_call", static_cast<std::uint64_t>(summary.with_import_call));
    writer.member("with_string_reference",
                  static_cast<std::uint64_t>(summary.with_string_reference));
    writer.member("import_thunks", static_cast<std::uint64_t>(summary.import_thunks));
    writer.member("external_thunks_resolved",
                  static_cast<std::uint64_t>(summary.external_thunks_resolved));
    writer.member("virtual_dispatch_candidates",
                  static_cast<std::uint64_t>(summary.virtual_dispatch_candidates));
    writer.member("structurally_reachable",
                  static_cast<std::uint64_t>(summary.structurally_reachable));
    writer.member("reachable_from_entry_point",
                  static_cast<std::uint64_t>(summary.reachable_from_entry_point));
    writer.member("reachable_from_export",
                  static_cast<std::uint64_t>(summary.reachable_from_export));
    writer.member("structurally_unreferenced",
                  static_cast<std::uint64_t>(summary.structurally_unreferenced));
    writer.member("strings_recovered", static_cast<std::uint64_t>(summary.strings_recovered));
    writer.end_object();
}

void write_graph(JsonWriter& writer, const CodeGraph& graph) {
    writer.key("code_graph");
    writer.begin_object();
    writer.member("functions_walked", static_cast<std::uint64_t>(graph.functions_walked));
    writer.member("functions_complete", static_cast<std::uint64_t>(graph.functions_complete));
    writer.member("total_instructions", static_cast<std::uint64_t>(graph.total_instructions));
    writer.member("total_decoded_bytes", graph.total_decoded_bytes);
    writer.member("call_edges", static_cast<std::uint64_t>(graph.call_edges));
    writer.member("data_reference_edges",
                  static_cast<std::uint64_t>(graph.data_reference_edges));
    writer.end_object();
}

void write_import_usage(JsonWriter& writer, const FunctionMap& map) {
    writer.key("import_usage");
    writer.begin_array();
    for (const auto& usage : map.import_usage) {
        writer.begin_object();
        writer.member("module", usage.module);
        writer.member("function", usage.function);
        writer.member("calling_functions", static_cast<std::uint64_t>(usage.calling_functions));
        writer.end_object();
    }
    writer.end_array();
}

void write_class_coverage(JsonWriter& writer, const FunctionMap& map) {
    writer.key("class_coverage");
    writer.begin_array();
    for (const auto& entry : map.class_coverage) {
        writer.begin_object();
        writer.member("class", entry.class_display_name);
        writer.member("vtable_slots", static_cast<std::uint64_t>(entry.vtable_slots));
        writer.member("slots_bound_to_functions",
                      static_cast<std::uint64_t>(entry.slots_bound_to_functions));
        writer.member("distinct_functions", static_cast<std::uint64_t>(entry.distinct_functions));
        writer.end_object();
    }
    writer.end_array();
}

void write_function(JsonWriter& writer, const FunctionFacts& facts,
                    const FunctionMapReportOptions& options) {
    writer.begin_object();
    writer.hex_member("begin_rva", facts.begin_rva);
    writer.hex_member("end_rva", facts.end_rva);
    writer.member("size", static_cast<std::uint64_t>(facts.size()));
    writer.member("instructions", static_cast<std::uint64_t>(facts.instruction_count));
    if (!facts.walk_complete) {
        writer.member("walk_complete", false);
    }
    writer.member("callers", static_cast<std::uint64_t>(facts.caller_count));
    writer.member("callees", static_cast<std::uint64_t>(facts.callee_count));
    if (facts.exported) {
        writer.member("export_name", facts.export_name);
    }
    if (facts.import_thunk) {
        writer.member("import_thunk", true);
    }
    if (facts.reachable_from_entry_point) {
        writer.member("reachable_from_entry_point", true);
    }
    if (facts.reachable_from_export) {
        writer.member("reachable_from_export", true);
    }

    if (!facts.virtual_bindings.empty()) {
        writer.key("virtual_bindings");
        writer.begin_array();
        for (const auto& binding : facts.virtual_bindings) {
            writer.begin_object();
            writer.member("class", binding.class_display_name);
            writer.member("vtable_index", static_cast<std::uint64_t>(binding.vtable_index));
            writer.member("slot", static_cast<std::uint64_t>(binding.slot));
            writer.member("subobject_offset",
                          static_cast<std::uint64_t>(binding.subobject_offset));
            writer.end_object();
        }
        writer.end_array();
    }

    if (!facts.imports_called.empty()) {
        writer.key("imports_called");
        writer.begin_array();
        for (const auto& call : facts.imports_called) {
            writer.begin_object();
            writer.member("module", call.module);
            writer.member("function", call.function);
            writer.end_object();
        }
        writer.end_array();
    }

    if (facts.string_reference_count != 0U) {
        writer.member("string_references",
                      static_cast<std::uint64_t>(facts.string_reference_count));
        if (options.include_referenced_strings && !facts.referenced_strings.empty()) {
            writer.key("referenced_strings");
            writer.begin_array();
            for (const auto& text : facts.referenced_strings) {
                writer.string(text);
            }
            writer.end_array();
        }
    }

    writer.end_object();
}

} // namespace

std::string to_json(const ExecutableArtifactIdentity& artifact, const CodeGraph& graph,
                    const FunctionMap& map, const FunctionMapReportOptions& options) {
    std::ostringstream output;
    JsonWriter writer{output};

    writer.begin_object();
    writer.member("schema", "dmc-rengine.function-map.v1");

    writer.key("artifact");
    writer.begin_object();
    writer.member("sha256", artifact.sha256);
    writer.member("size", artifact.size);
    writer.end_object();

    write_summary(writer, map);
    write_graph(writer, graph);
    write_import_usage(writer, map);
    write_class_coverage(writer, map);

    writer.key("functions");
    writer.begin_array();
    std::size_t emitted = 0U;
    for (const auto& facts : map.functions) {
        const bool attributed = !facts.virtual_bindings.empty() || !facts.imports_called.empty() ||
                                facts.string_reference_count != 0U || facts.exported;
        if (!attributed && !options.include_unattributed) {
            continue;
        }
        if (options.function_limit != 0U && emitted >= options.function_limit) {
            break;
        }
        write_function(writer, facts, options);
        ++emitted;
    }
    writer.end_array();
    writer.member("functions_emitted", static_cast<std::uint64_t>(emitted));

    writer.end_object();
    output << '\n';
    return output.str();
}

} // namespace dmc::rengine::exe
