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
    writer.member("with_name_table", static_cast<std::uint64_t>(summary.with_name_table));
    writer.member("constant_index_references",
                  static_cast<std::uint64_t>(summary.constant_index_references));
    writer.member("computed_index_references",
                  static_cast<std::uint64_t>(summary.computed_index_references));
    writer.member("name_tables_referenced",
                  static_cast<std::uint64_t>(summary.name_tables_referenced));
    writer.member("indexed_accesses", static_cast<std::uint64_t>(summary.indexed_accesses));
    writer.member("image_base_indexed_accesses",
                  static_cast<std::uint64_t>(summary.image_base_indexed_accesses));
    writer.member("indexed_table_accesses",
                  static_cast<std::uint64_t>(summary.indexed_table_accesses));
    writer.member("held_base_accesses", static_cast<std::uint64_t>(summary.held_base_accesses));
    writer.member("consistent_array_accesses",
                  static_cast<std::uint64_t>(summary.consistent_array_accesses));
    writer.member("string_scan_accesses",
                  static_cast<std::uint64_t>(summary.string_scan_accesses));
    writer.member("inconsistent_array_accesses",
                  static_cast<std::uint64_t>(summary.inconsistent_array_accesses));
    writer.member("indexed_arrays", static_cast<std::uint64_t>(summary.indexed_arrays));
    writer.member("arrays_with_conflicting_element_size",
                  static_cast<std::uint64_t>(summary.arrays_with_conflicting_element_size));
    writer.member("image_base_groups", static_cast<std::uint64_t>(summary.image_base_groups));
    writer.member("image_base_groups_with_several_reads",
                  static_cast<std::uint64_t>(summary.image_base_groups_with_several_reads));
    writer.member("image_base_groups_spanning_elements",
                  static_cast<std::uint64_t>(summary.image_base_groups_spanning_elements));
    writer.member("image_base_groups_corroborating",
                  static_cast<std::uint64_t>(summary.image_base_groups_corroborating));
    writer.member("dispatch_sites", static_cast<std::uint64_t>(summary.dispatch_sites));
    writer.member("dispatch_sites_on_this",
                  static_cast<std::uint64_t>(summary.dispatch_sites_on_this));
    writer.member("dispatch_sites_in_a_bound_function",
                  static_cast<std::uint64_t>(summary.dispatch_sites_in_a_bound_function));
    writer.member("dispatch_sites_resolved",
                  static_cast<std::uint64_t>(summary.dispatch_sites_resolved));
    writer.member("stores_into_this", static_cast<std::uint64_t>(summary.stores_into_this));
    writer.member("stores_of_a_vtable", static_cast<std::uint64_t>(summary.stores_of_a_vtable));
    writer.member("constructors_identified",
                  static_cast<std::uint64_t>(summary.constructors_identified));
    writer.member("field_layout_entries",
                  static_cast<std::uint64_t>(summary.field_layout_entries));
    writer.member("field_offsets_confirmed_by_rtti",
                  static_cast<std::uint64_t>(summary.field_offsets_confirmed_by_rtti));
    writer.member("name_tables_unreferenced",
                  static_cast<std::uint64_t>(summary.name_tables_unreferenced));
    writer.member("with_vtable_install",
                  static_cast<std::uint64_t>(summary.with_vtable_install));
    writer.member("with_resource_family",
                  static_cast<std::uint64_t>(summary.with_resource_family));
    writer.member("with_indirect_dispatch",
                  static_cast<std::uint64_t>(summary.with_indirect_dispatch));
    writer.member("indirect_call_sites",
                  static_cast<std::uint64_t>(summary.indirect_call_sites));
    writer.member("with_frame_pointer", static_cast<std::uint64_t>(summary.with_frame_pointer));
    writer.member("with_exception_handler",
                  static_cast<std::uint64_t>(summary.with_exception_handler));
    writer.member("leaf_functions", static_cast<std::uint64_t>(summary.leaf_functions));
    writer.member("total_stack_allocation", summary.total_stack_allocation);
    writer.member("largest_stack_allocation",
                  static_cast<std::uint64_t>(summary.largest_stack_allocation));
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
    writer.member("switch_tables_recovered",
                  static_cast<std::uint64_t>(graph.switch_tables_recovered));
    writer.member("switch_targets_recovered",
                  static_cast<std::uint64_t>(graph.switch_targets_recovered));
    writer.member("unresolved_indirect_jumps",
                  static_cast<std::uint64_t>(graph.unresolved_indirect_jumps));
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

void write_resource_families(JsonWriter& writer, const FunctionMap& map) {
    writer.key("resource_family_usage");
    writer.begin_array();
    for (const auto& entry : map.resource_family_usage) {
        writer.begin_object();
        writer.member("family", entry.family);
        writer.member("literals", static_cast<std::uint64_t>(entry.literals));
        writer.member("referencing_functions",
                      static_cast<std::uint64_t>(entry.referencing_functions));
        writer.end_object();
    }
    writer.end_array();
}

void write_name_table_usage(JsonWriter& writer, const FunctionMap& map) {
    writer.key("class_field_layout");
    writer.begin_array();
    for (const auto& field : map.class_field_layout) {
        writer.begin_object();
        writer.member("class", field.class_display_name);
        writer.member("offset", static_cast<std::uint64_t>(field.offset));
        writer.member("member_class", field.member_class_display_name);
        writer.hex_member("site_rva", field.site_rva);
        writer.member("embedded_member", field.embedded_member);
        writer.member("offset_confirmed_by_rtti", field.offset_confirmed_by_rtti);
        writer.end_object();
    }
    writer.end_array();

    writer.key("resolved_dispatches");
    writer.begin_array();
    for (const auto& dispatch : map.resolved_dispatches) {
        writer.begin_object();
        writer.hex_member("site_rva", dispatch.site_rva);
        writer.hex_member("caller_rva", dispatch.caller_rva);
        writer.member("displacement", static_cast<std::uint64_t>(dispatch.displacement));
        writer.member("slot", static_cast<std::uint64_t>(dispatch.slot));
        writer.member("class", dispatch.class_display_name);
        writer.hex_member("target_rva", dispatch.target_rva);
        writer.end_object();
    }
    writer.end_array();

    writer.key("indexed_arrays");
    writer.begin_array();
    // Layout only: where an array is, how wide its element is, and which
    // offsets inside that element the code reads. No contents.
    for (const auto& array : map.indexed_arrays) {
        writer.begin_object();
        writer.hex_member("base_rva", array.base_rva);
        writer.member("element_bytes", static_cast<std::uint64_t>(array.element_bytes));
        writer.member("sites", static_cast<std::uint64_t>(array.sites));
        writer.member("referencing_functions",
                      static_cast<std::uint64_t>(array.referencing_functions));
        writer.member("base_measured", array.base_measured);
        writer.key("field_offsets");
        writer.begin_array();
        for (const auto offset : array.field_offsets) {
            writer.number(static_cast<std::uint64_t>(offset));
        }
        writer.end_array();
        writer.end_object();
    }
    writer.end_array();

    writer.key("name_table_usage");
    writer.begin_array();
    for (const auto& usage : map.name_table_usage) {
        if (usage.referencing_functions == 0U) {
            continue;  // layout is already reported by analyze-exe
        }
        writer.begin_object();
        writer.hex_member("table_base_rva", usage.table_base_rva);
        writer.member("record_layout", usage.record_layout);
        writer.member("element_bytes", static_cast<std::uint64_t>(usage.element_bytes));
        writer.member("entries", static_cast<std::uint64_t>(usage.entries));
        writer.member("referencing_functions",
                      static_cast<std::uint64_t>(usage.referencing_functions));
        writer.member("base_references", static_cast<std::uint64_t>(usage.base_references));
        writer.member("elements_named_by_constant",
                      static_cast<std::uint64_t>(usage.elements_named_by_constant));
        writer.member("indexed_sites", static_cast<std::uint64_t>(usage.indexed_sites));
        writer.end_object();
    }
    writer.end_array();
}

void write_dispatch_slots(JsonWriter& writer, const FunctionMap& map) {
    writer.key("dispatch_slots");
    writer.begin_array();
    for (const auto& entry : map.dispatch_slots) {
        writer.begin_object();
        writer.member("displacement", static_cast<std::uint64_t>(entry.displacement));
        writer.member("slot", static_cast<std::uint64_t>(entry.slot));
        writer.member("call_sites", static_cast<std::uint64_t>(entry.call_sites));
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
        writer.member("install_sites", static_cast<std::uint64_t>(entry.install_sites));
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
    writer.key("frame");
    writer.begin_object();
    writer.member("prolog_size", static_cast<std::uint64_t>(facts.frame.prolog_size));
    writer.member("stack_allocation", static_cast<std::uint64_t>(facts.frame.stack_allocation));
    writer.member("pushed_registers", static_cast<std::uint64_t>(facts.frame.pushed_registers));
    if (facts.frame.saved_registers != 0U) {
        writer.member("saved_registers", static_cast<std::uint64_t>(facts.frame.saved_registers));
    }
    if (facts.frame.saved_xmm != 0U) {
        writer.member("saved_xmm", static_cast<std::uint64_t>(facts.frame.saved_xmm));
    }
    if (facts.frame.uses_frame_pointer()) {
        writer.member("frame_register", static_cast<std::uint64_t>(facts.frame.frame_register));
    }
    if (facts.frame.has_exception_handler) {
        writer.member("exception_handler", true);
    }
    if (!facts.frame.decoded) {
        writer.member("decoded", false);
    }
    writer.end_object();
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

    if (!facts.installs_vtables.empty()) {
        writer.key("installs_vtables");
        writer.begin_array();
        for (const auto& install : facts.installs_vtables) {
            writer.begin_object();
            writer.member("class", install.class_display_name);
            writer.member("vtable_index", static_cast<std::uint64_t>(install.vtable_index));
            writer.hex_member("vtable_rva", install.vtable_rva);
            writer.end_object();
        }
        writer.end_array();
    }

    if (!facts.name_tables.empty()) {
        writer.key("name_tables");
        writer.begin_array();
        for (const auto& reference : facts.name_tables) {
            writer.begin_object();
            writer.hex_member("table_base_rva", reference.table_base_rva);
            if (reference.offset_in_table != 0U) {
                writer.member("offset_in_table",
                              static_cast<std::uint64_t>(reference.offset_in_table));
            }
            writer.member("record_layout", reference.record_layout);
            writer.member("element_bytes", static_cast<std::uint64_t>(reference.element_bytes));
            writer.member("entries", static_cast<std::uint64_t>(reference.entries));
            if (reference.constant_index) {
                writer.member("element_index",
                              static_cast<std::uint64_t>(reference.element_index));
                if (reference.record_layout) {
                    writer.member("field_index",
                                  static_cast<std::uint64_t>(reference.field_index));
                }
            } else {
                writer.member("constant_index", false);
                writer.member("offset_in_element",
                              static_cast<std::uint64_t>(reference.offset_in_element));
            }
            writer.end_object();
        }
        writer.end_array();
    }

    if (!facts.resource_families.empty()) {
        writer.key("resource_families");
        writer.begin_array();
        for (const auto& family : facts.resource_families) {
            writer.string(family);
        }
        writer.end_array();
    }

    if (!facts.indirect_call_displacements.empty()) {
        writer.key("dispatch_displacements");
        writer.begin_array();
        for (const auto displacement : facts.indirect_call_displacements) {
            writer.number(static_cast<std::uint64_t>(displacement));
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
    write_resource_families(writer, map);
    write_dispatch_slots(writer, map);
    write_name_table_usage(writer, map);

    writer.key("functions");
    writer.begin_array();
    std::size_t emitted = 0U;
    for (const auto& facts : map.functions) {
        const bool attributed = !facts.virtual_bindings.empty() || !facts.imports_called.empty() ||
                                facts.string_reference_count != 0U || facts.exported ||
                                !facts.installs_vtables.empty() || !facts.name_tables.empty();
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
