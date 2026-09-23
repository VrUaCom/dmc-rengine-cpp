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
    writer.member("arrays_resolved_by_a_missed_multiplier",
                  static_cast<std::uint64_t>(summary.arrays_resolved_by_a_missed_multiplier));
    writer.member("arrays_with_a_real_size_conflict",
                  static_cast<std::uint64_t>(summary.arrays_with_a_real_size_conflict));
    writer.member("accesses_on_a_conflicted_base",
                  static_cast<std::uint64_t>(summary.accesses_on_a_conflicted_base));
        writer.member("arrays_with_conflicting_element_size",
                  static_cast<std::uint64_t>(summary.arrays_with_conflicting_element_size));
    writer.member("image_base_groups", static_cast<std::uint64_t>(summary.image_base_groups));
    writer.member("image_base_groups_with_several_reads",
                  static_cast<std::uint64_t>(summary.image_base_groups_with_several_reads));
    writer.member("image_base_groups_reads_in_several_sections",
                  static_cast<std::uint64_t>(summary.image_base_groups_reads_in_several_sections));
    writer.member("image_base_groups_undecided",
                  static_cast<std::uint64_t>(summary.image_base_groups_undecided));
        writer.member("image_base_groups_spanning_elements",
                  static_cast<std::uint64_t>(summary.image_base_groups_spanning_elements));
    writer.member("image_base_groups_corroborating",
                  static_cast<std::uint64_t>(summary.image_base_groups_corroborating));
    writer.member("dispatch_sites", static_cast<std::uint64_t>(summary.dispatch_sites));
    writer.member("dispatch_sites_with_an_unnamed_receiver",
                  static_cast<std::uint64_t>(summary.dispatch_sites_with_an_unnamed_receiver));
    writer.member("dispatch_sites_on_a_fixed_or_taken_address",
                  static_cast<std::uint64_t>(summary.dispatch_sites_on_a_fixed_or_taken_address));
        writer.member("dispatch_sites_on_an_argument",
                  static_cast<std::uint64_t>(summary.dispatch_sites_on_an_argument));
        writer.member("dispatch_sites_on_this",
                  static_cast<std::uint64_t>(summary.dispatch_sites_on_this));
    writer.member("dispatch_sites_in_a_bound_function",
                  static_cast<std::uint64_t>(summary.dispatch_sites_in_a_bound_function));
    writer.member("dispatch_sites_resolved",
                  static_cast<std::uint64_t>(summary.dispatch_sites_resolved));
    writer.member("dispatch_sites_on_a_member",
                  static_cast<std::uint64_t>(summary.dispatch_sites_on_a_member));
    writer.member("dispatch_sites_through_a_pointer_member",
                  static_cast<std::uint64_t>(summary.dispatch_sites_through_a_pointer_member));
    writer.member("pointer_stores_into_this",
                  static_cast<std::uint64_t>(summary.pointer_stores_into_this));
    writer.member("pointer_stores_from_a_constructor",
                  static_cast<std::uint64_t>(summary.pointer_stores_from_a_constructor));
    writer.member("constant_argument_calls",
                  static_cast<std::uint64_t>(summary.constant_argument_calls));
    writer.member("constant_argument_callees",
                  static_cast<std::uint64_t>(summary.constant_argument_callees));
    writer.member("stores_into_this", static_cast<std::uint64_t>(summary.stores_into_this));
    writer.member("stores_of_a_vtable", static_cast<std::uint64_t>(summary.stores_of_a_vtable));
    writer.member("constructors_identified",
                  static_cast<std::uint64_t>(summary.constructors_identified));
    writer.member("field_layout_entries",
                  static_cast<std::uint64_t>(summary.field_layout_entries));
    writer.member("vtable_slots_classified",
                  static_cast<std::uint64_t>(summary.vtable_slots_classified));
    writer.member("vtable_slots_pure_virtual",
                  static_cast<std::uint64_t>(summary.vtable_slots_pure_virtual));
    writer.member("vtable_slots_empty_body",
                  static_cast<std::uint64_t>(summary.vtable_slots_empty_body));
    writer.member("vtable_slots_implemented",
                  static_cast<std::uint64_t>(summary.vtable_slots_implemented));
    writer.member("vtable_slot_implementations",
                  static_cast<std::uint64_t>(summary.vtable_slot_implementations));
    writer.member("startup_path_functions",
                  static_cast<std::uint64_t>(summary.startup_path_functions));
    writer.member("startup_path_deepest",
                  static_cast<std::uint64_t>(summary.startup_path_deepest));
    writer.member("startup_path_modules",
                  static_cast<std::uint64_t>(summary.startup_path_modules));
    writer.member("startup_path_import_symbols",
                  static_cast<std::uint64_t>(summary.startup_path_import_symbols));
    writer.member("startup_path_constructors",
                  static_cast<std::uint64_t>(summary.startup_path_constructors));
    writer.member("startup_path_dispatch_sites_in_tail_position",
                  static_cast<std::uint64_t>(summary.startup_path_dispatch_sites_in_tail_position));
    writer.member("startup_path_functions_bound_to_a_class",
                  static_cast<std::uint64_t>(summary.startup_path_functions_bound_to_a_class));
    writer.member("startup_path_extended_by_resolved_dispatch",
                  static_cast<std::uint64_t>(summary.startup_path_extended_by_resolved_dispatch));
        writer.member("startup_path_dispatch_sites",
                  static_cast<std::uint64_t>(summary.startup_path_dispatch_sites));
        writer.member("reachable_through_dispatch",
                  static_cast<std::uint64_t>(summary.reachable_through_dispatch));
    writer.member("outside_every_closure",
                  static_cast<std::uint64_t>(summary.outside_every_closure));
    writer.member("reached_only_through_funclets_or_taken_addresses",
                  static_cast<std::uint64_t>(
                      summary.reached_only_through_funclets_or_taken_addresses));
    writer.member("funclet_edges", static_cast<std::uint64_t>(summary.funclet_edges));
    writer.member("funcinfo_structures",
                  static_cast<std::uint64_t>(summary.funcinfo_structures));
    writer.member("scope_tables", static_cast<std::uint64_t>(summary.scope_tables));
    writer.member("taken_address_edges",
                  static_cast<std::uint64_t>(summary.taken_address_edges));
    writer.member("dispatch_slots_reached",
                  static_cast<std::uint64_t>(summary.dispatch_slots_reached));
    writer.member("vtables_located", static_cast<std::uint64_t>(summary.vtables_located));
    writer.member("vtables_instantiated_by_reachable_code",
                  static_cast<std::uint64_t>(summary.vtables_instantiated_by_reachable_code));
    writer.member("global_state_blocks",
                  static_cast<std::uint64_t>(summary.global_state_blocks));
    writer.member("global_block_call_sites",
                  static_cast<std::uint64_t>(summary.global_block_call_sites));
    writer.member("global_blocks_with_a_field_reach",
                  static_cast<std::uint64_t>(summary.global_blocks_with_a_field_reach));
    writer.member("global_blocks_reach_within_the_gap",
                  static_cast<std::uint64_t>(summary.global_blocks_reach_within_the_gap));
    writer.member("global_blocks_reach_past_the_gap",
                  static_cast<std::uint64_t>(summary.global_blocks_reach_past_the_gap));
    writer.member("global_blocks_with_a_class",
                  static_cast<std::uint64_t>(summary.global_blocks_with_a_class));
    writer.member("global_block_sites_pooled",
                  static_cast<std::uint64_t>(summary.global_block_sites_pooled));
    writer.member("global_block_sites_with_other_callers",
                  static_cast<std::uint64_t>(summary.global_block_sites_with_other_callers));
        writer.member("class_size_floors",
                  static_cast<std::uint64_t>(summary.class_size_floors));
    writer.member("size_floors_above_a_vtable_pointer",
                  static_cast<std::uint64_t>(summary.size_floors_above_a_vtable_pointer));
    writer.member("size_floors_corroborated",
                  static_cast<std::uint64_t>(summary.size_floors_corroborated));
    writer.member("size_floors_on_a_lone_outlier",
                  static_cast<std::uint64_t>(summary.size_floors_on_a_lone_outlier));
    writer.member("size_floors_where_bases_say_more",
                  static_cast<std::uint64_t>(summary.size_floors_where_bases_say_more));
        writer.member("function_pointer_runs",
                  static_cast<std::uint64_t>(summary.function_pointer_runs));
    writer.member("function_pointer_run_entries",
                  static_cast<std::uint64_t>(summary.function_pointer_run_entries));
        writer.member("base_slots_measured",
                  static_cast<std::uint64_t>(summary.base_slots_measured));
    writer.member("base_pairings_without_a_vtable",
                  static_cast<std::uint64_t>(summary.base_pairings_without_a_vtable));
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
    writer.member("tail_dispatch_jumps",
                  static_cast<std::uint64_t>(graph.tail_dispatch_jumps));
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
    writer.key("constant_argument_callees");
    writer.begin_array();
    // Which functions take a constant first argument and what constants
    // reach them. What the constant means is not stated here.
    constexpr std::size_t kReportedCallees = 64U;
    for (std::size_t index = 0; index < map.constant_argument_callees.size() &&
                                index < kReportedCallees;
         ++index) {
        const auto& callee = map.constant_argument_callees[index];
        writer.begin_object();
        writer.hex_member("callee_rva", callee.callee_rva);
        writer.member("call_sites", static_cast<std::uint64_t>(callee.call_sites));
        writer.member("distinct_arguments",
                      static_cast<std::uint64_t>(callee.distinct_arguments));
        writer.key("arguments");
        writer.begin_array();
        for (const auto argument : callee.arguments) {
            writer.number(static_cast<std::uint64_t>(argument));
        }
        writer.end_array();
        writer.end_object();
    }
    writer.end_array();

    writer.key("class_field_layout");
    writer.begin_array();
    for (const auto& field : map.class_field_layout) {
        writer.begin_object();
        writer.member("class", field.class_display_name);
        writer.member("offset", static_cast<std::uint64_t>(field.offset));
        writer.member("member_class", field.member_class_display_name);
    writer.hex_member("member_vtable_rva", field.member_vtable_rva);
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
    writer.member("receiver_field_offset",
                  static_cast<std::uint64_t>(dispatch.receiver_field_offset));
        writer.hex_member("target_rva", dispatch.target_rva);
        writer.end_object();
    }
    writer.end_array();

    writer.key("global_state_blocks");
    writer.begin_array();
    // Addresses and how much code operates on them. No byte at any of these
    // addresses is read.
    for (const auto& block : map.global_state_blocks) {
        writer.begin_object();
        writer.hex_member("base_rva", block.base_rva);
        writer.member("section", block.section);
        writer.member("call_sites", static_cast<std::uint64_t>(block.call_sites));
        writer.member("distinct_callees", static_cast<std::uint64_t>(block.distinct_callees));
        writer.member("distinct_callers", static_cast<std::uint64_t>(block.distinct_callers));
        writer.member("field_reach", static_cast<std::uint64_t>(block.field_reach));
        writer.member("bytes_to_next_block",
                      static_cast<std::uint64_t>(block.bytes_to_next_block));
        writer.member("reach_runs_past_the_next_block", block.reach_runs_past_the_next_block);
        writer.member("constructed_class", block.constructed_class);
        writer.end_object();
    }
    writer.end_array();

    writer.key("class_size_floors");
    writer.begin_array();
    // A floor, not a size, and no field's contents are read to produce it.
    for (const auto& floor : map.class_size_floors) {
        writer.begin_object();
        writer.member("class", floor.class_display_name);
        writer.member("floor_bytes", static_cast<std::uint64_t>(floor.floor_bytes));
        writer.member("floor_from_bases", static_cast<std::uint64_t>(floor.floor_from_bases));
        writer.member("deepest_base", floor.deepest_base_display_name);
        writer.member("floor_from_field_access",
                      static_cast<std::uint64_t>(floor.floor_from_field_access));
        writer.member("functions_speaking", static_cast<std::uint64_t>(floor.functions_speaking));
        writer.member("functions_reaching_half",
                      static_cast<std::uint64_t>(floor.functions_reaching_half));
        writer.end_object();
    }
    writer.end_array();

    writer.key("function_pointer_runs");
    writer.begin_array();
    for (const auto& run : map.function_pointer_runs) {
        writer.begin_object();
        writer.hex_member("base_rva", run.base_rva);
        writer.member("entries", static_cast<std::uint64_t>(run.entries));
        writer.member("entries_reaching_nothing_else",
                      static_cast<std::uint64_t>(run.entries_reaching_nothing_else));
        writer.member("referencing_functions",
                      static_cast<std::uint64_t>(run.referencing_functions));
        writer.end_object();
    }
    writer.end_array();

    writer.key("base_slot_overrides");
    writer.begin_array();
    // Layout and linkage only: which base declares a slot, how many classes
    // inherit it, and how many different targets they put there. No slot is
    // named and no body is described.
    for (const auto& record : map.base_slot_overrides) {
        writer.begin_object();
        writer.member("base_class", record.base_display_name);
        writer.member("slot", static_cast<std::uint64_t>(record.slot));
        writer.member("base_kind", std::string{to_string(record.base_kind)});
        writer.hex_member("base_target_rva", record.base_target_rva);
        writer.member("derived_classes", static_cast<std::uint64_t>(record.derived_classes));
        writer.member("keep_base_target", static_cast<std::uint64_t>(record.keep_base_target));
        writer.member("empty_bodies", static_cast<std::uint64_t>(record.empty_bodies));
        writer.member("pure_virtual", static_cast<std::uint64_t>(record.pure_virtual));
        writer.member("distinct_implementations",
                      static_cast<std::uint64_t>(record.distinct_implementations));
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
    if (!facts.calls.empty()) {
        writer.key("calls");
        writer.begin_array();
        for (const auto target : facts.calls) {
            writer.hex(target);
        }
        writer.end_array();
    }
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
    if (facts.depth_from_entry != dmc::rengine::exe::FunctionFacts::kUnreached) {
        writer.member("depth_from_entry", static_cast<std::uint64_t>(facts.depth_from_entry));
    }
    // The count the image-wide census is made of, per function, so a consumer
    // can add the detail up and check the summary against it. A counter that
    // measures something other than its name is not caught by any arithmetic
    // identity; it is caught by measuring the same thing twice.
    if (facts.dispatch_sites != 0U) {
        writer.member("dispatch_sites", static_cast<std::uint64_t>(facts.dispatch_sites));
    }
    // The interesting minority is what the bound does *not* reach, so that is
    // what gets a member; emitting the majority flag would say nothing.
    // Two minorities: reached only once funclets and taken addresses are
    // followed, and reached by nothing the file records at all.
    if (!facts.reachable_through_dispatch && facts.reachable_through_recorded_edges) {
        writer.member("reached_only_through_funclets_or_taken_addresses", true);
    }
    if (!facts.reachable_through_recorded_edges) {
        writer.member("outside_every_closure", true);
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
