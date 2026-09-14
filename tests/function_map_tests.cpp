#include "dmc_rengine/exe/code_graph.hpp"
#include "dmc_rengine/exe/function_map.hpp"

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <span>
#include <string>
#include <string_view>
#include <vector>

namespace {

using dmc::rengine::exe::CodeGraphBuilder;
using dmc::rengine::exe::FunctionMapBuilder;
using dmc::rengine::exe::FunctionMapInputs;
using dmc::rengine::exe::PeDirectories;
using dmc::rengine::exe::PeExportedSymbol;
using dmc::rengine::exe::PeExportTable;
using dmc::rengine::exe::PeFunctionRange;
using dmc::rengine::exe::PeFunctionTable;
using dmc::rengine::exe::PeImage;
using dmc::rengine::exe::PeImportedFunction;
using dmc::rengine::exe::PeImportedModule;
using dmc::rengine::exe::PeKind;
using dmc::rengine::exe::PeMachine;
using dmc::rengine::exe::PeSection;
using dmc::rengine::exe::RttiClass;
using dmc::rengine::exe::RttiScanResult;
using dmc::rengine::exe::RttiVtable;

// Same synthetic image as the code-graph test: function A at 0x1000 calls
// function B at 0x1040 and an import thunk at 0x1080 that is deliberately
// absent from the inventory. `.rdata` at 0x2000 holds two literals, two
// import-address-table slots and one vtable.

constexpr std::uint64_t kImageBase = 0x140000000ULL;

void put(std::vector<std::byte>& bytes, std::size_t offset, std::initializer_list<int> values) {
    std::size_t index = offset;
    for (const auto value : values) {
        bytes[index++] = static_cast<std::byte>(static_cast<unsigned char>(value));
    }
}

void put_i32(std::vector<std::byte>& bytes, std::size_t offset, std::int32_t value) {
    const auto raw = static_cast<std::uint32_t>(value);
    for (std::size_t index = 0; index < 4U; ++index) {
        bytes[offset + index] =
            static_cast<std::byte>((raw >> static_cast<unsigned>(index * 8U)) & 0xFFU);
    }
}

void put_u64(std::vector<std::byte>& bytes, std::size_t offset, std::uint64_t value) {
    for (std::size_t index = 0; index < 8U; ++index) {
        bytes[offset + index] =
            static_cast<std::byte>((value >> static_cast<unsigned>(index * 8U)) & 0xFFU);
    }
}

void put_text(std::vector<std::byte>& bytes, std::size_t offset, std::string_view text) {
    for (std::size_t index = 0; index < text.size(); ++index) {
        bytes[offset + index] = static_cast<std::byte>(text[index]);
    }
    bytes[offset + text.size()] = std::byte{0};
}

[[nodiscard]] PeImage make_image() {
    PeImage image;
    image.kind = PeKind::pe32_plus;
    image.machine = PeMachine::amd64;
    image.image_base = kImageBase;
    image.entry_point_rva = 0x1000U;
    image.size_of_image = 0x3000U;
    image.section_count = 2U;
    image.sections.push_back(PeSection{".text", 0x200U, 0x1000U, 0x200U, 0x200U, 0x60000020U});
    image.sections.push_back(PeSection{".rdata", 0x200U, 0x2000U, 0x200U, 0x400U, 0x40000040U});
    return image;
}

[[nodiscard]] std::vector<std::byte> make_bytes() {
    std::vector<std::byte> bytes(0x600U, std::byte{0});

    put(bytes, 0x200U, {0x48, 0x83, 0xEC, 0x20});
    put(bytes, 0x204U, {0x48, 0x8D, 0x0D});
    put_i32(bytes, 0x207U, 0x0FF5);              // -> literal at 0x2000
    put(bytes, 0x20BU, {0xE8});
    put_i32(bytes, 0x20CU, 0x30);                // -> function B
    put(bytes, 0x210U, {0x75, 0x08});
    put(bytes, 0x212U, {0xFF, 0x15});
    put_i32(bytes, 0x214U, 0x10E8);              // -> IAT slot 0x2100
    put(bytes, 0x218U, {0xEB, 0x06});
    put(bytes, 0x21AU, {0x48, 0x83, 0xC4, 0x20});
    put(bytes, 0x21EU, {0xC3});
    put(bytes, 0x21FU, {0xCC});
    put(bytes, 0x220U, {0xE8});
    put_i32(bytes, 0x221U, 0x5B);                // -> thunk at 0x1080
    put(bytes, 0x225U, {0xC3});

    put(bytes, 0x240U, {0xE9});
    put_i32(bytes, 0x241U, 0x0B);
    put(bytes, 0x250U, {0x48, 0x8D, 0x0D});
    put_i32(bytes, 0x253U, 0x0FB9);              // -> literal at 0x2010
    put(bytes, 0x257U, {0x48, 0x8D, 0x0D});
    put_i32(bytes, 0x25AU, 0x1122);              // -> the CThing vtable at 0x2180
    put(bytes, 0x25EU, {0xC3});

    put(bytes, 0x280U, {0xFF, 0x25});            // thunk: jmp [rip+...]
    put_i32(bytes, 0x282U, 0x1082);              // -> IAT slot 0x2108

    put_text(bytes, 0x400U, "hello world");      // rva 0x2000
    put_text(bytes, 0x410U, "second string");    // rva 0x2010
    put_u64(bytes, 0x580U, kImageBase + 0x1040U);  // vtable slot at rva 0x2180

    return bytes;
}

[[nodiscard]] PeFunctionTable make_function_table() {
    PeFunctionTable table;

    // Prologue facts as an unwind record would report them.
    dmc::rengine::exe::PeUnwindFrame first{};
    first.version = 1U;
    first.prolog_size = 0x12U;
    first.code_count = 2U;
    first.stack_allocation = 96U;
    first.pushed_registers = 1U;
    first.decoded = true;

    dmc::rengine::exe::PeUnwindFrame second{};
    second.version = 1U;
    second.prolog_size = 0x10U;
    second.code_count = 1U;
    second.frame_register = 5U;
    second.has_exception_handler = true;
    second.decoded = true;

    PeFunctionRange entry{0x1000U, 0x1030U, 0U, false, 0x1000U};
    entry.frame = first;
    PeFunctionRange split{0x1040U, 0x1050U, 0U, false, 0x1040U};
    split.frame = second;
    // A continuation carries its own record, which must not be mistaken for the
    // function's prologue.
    PeFunctionRange continuation{0x1050U, 0x1060U, 0U, true, 0x1040U};
    continuation.frame.stack_allocation = 4096U;
    continuation.frame.decoded = true;

    table.functions.push_back(entry);
    table.functions.push_back(split);
    table.functions.push_back(continuation);
    return table;
}

[[nodiscard]] PeDirectories make_directories() {
    PeDirectories directories;

    PeImportedModule module;
    module.name = "ALPHA.dll";
    module.functions.push_back(PeImportedFunction{"alpha_call", 0x2100U, 0U, 0U, false});
    module.functions.push_back(PeImportedFunction{"alpha_thunked", 0x2108U, 0U, 0U, false});
    directories.imports.push_back(std::move(module));

    PeExportTable exports;
    exports.module_name = "synth.exe";
    exports.ordinal_base = 1U;
    exports.address_count = 1U;
    exports.symbols.push_back(PeExportedSymbol{"synth_main", {}, 1U, 0x1000U});
    directories.exports = std::move(exports);

    return directories;
}

[[nodiscard]] RttiScanResult make_rtti() {
    RttiScanResult rtti;
    RttiClass entry;
    entry.decorated_name = ".?AVCThing@@";
    entry.display_name = "CThing";
    entry.type_descriptor_rva = 0x2140U;
    entry.vtables.push_back(RttiVtable{0x2160U, 0x2180U, 1U, 0U, 0U});
    rtti.classes.push_back(std::move(entry));
    rtti.type_descriptors = 1U;
    rtti.locators = 1U;
    rtti.vtables_located = 1U;
    return rtti;
}

struct Fixture final {
    PeImage image{make_image()};
    std::vector<std::byte> bytes{make_bytes()};
    PeFunctionTable table{make_function_table()};
    PeDirectories directories{make_directories()};
    RttiScanResult rtti{make_rtti()};
    dmc::rengine::exe::CodeGraph graph;

    Fixture() {
        // The exception directory belongs to the directories, exactly as the
        // PE reader produces it; the map reads prologue facts from there.
        directories.functions = table;
        graph = CodeGraphBuilder::build(std::span<const std::byte>{bytes}, image, table);
    }

    [[nodiscard]] dmc::rengine::exe::FunctionMap build() const {
        FunctionMapInputs inputs;
        inputs.image = &image;
        inputs.directories = &directories;
        inputs.rtti = &rtti;
        inputs.graph = &graph;
        return FunctionMapBuilder::build(std::span<const std::byte>{bytes}, inputs);
    }
};

void literals_are_attributed_to_the_functions_that_reference_them() {
    const Fixture fixture;
    const auto map = fixture.build();

    assert(map.functions.size() == 2U);
    assert(map.summary.strings_recovered == 2U);

    const auto& first = map.functions[0];
    assert(first.string_reference_count == 1U);
    assert(first.referenced_strings.size() == 1U);
    assert(first.referenced_strings[0] == "hello world");

    const auto& second = map.functions[1];
    assert(second.string_reference_count == 1U);
    assert(second.referenced_strings[0] == "second string");
    assert(map.summary.with_string_reference == 2U);
}

void imports_are_attributed_directly_and_through_thunks() {
    const Fixture fixture;
    const auto map = fixture.build();

    const auto& first = map.functions[0];
    assert(first.imports_called.size() == 2U);
    assert(first.imports_called[0].module == "ALPHA.dll");
    assert(first.imports_called[0].function == "alpha_call");
    assert(first.imports_called[1].function == "alpha_thunked");

    // The thunk has no unwind data, so it is not a function in the inventory;
    // the import behind it is still recovered.
    assert(map.summary.import_thunks == 0U);
    assert(map.summary.external_thunks_resolved == 1U);
    assert(map.summary.with_import_call == 1U);

    assert(map.import_usage.size() == 2U);
    assert(map.import_usage[0].calling_functions == 1U);
}

void vtable_slots_bind_functions_to_classes() {
    const Fixture fixture;
    const auto map = fixture.build();

    const auto& second = map.functions[1];
    assert(second.virtual_bindings.size() == 1U);
    assert(second.virtual_bindings[0].class_display_name == "CThing");
    assert(second.virtual_bindings[0].vtable_index == 0U);
    assert(second.virtual_bindings[0].slot == 0U);
    assert(map.summary.with_virtual_binding == 1U);
    assert(map.summary.virtual_dispatch_candidates == 1U);

    assert(map.class_coverage.size() == 1U);
    assert(map.class_coverage[0].class_display_name == "CThing");
    assert(map.class_coverage[0].vtable_slots == 1U);
    assert(map.class_coverage[0].slots_bound_to_functions == 1U);
    assert(map.class_coverage[0].distinct_functions == 1U);
}

void call_edges_and_exports_are_recorded() {
    const Fixture fixture;
    const auto map = fixture.build();

    const auto& first = map.functions[0];
    assert(first.exported);
    assert(first.export_name == "synth_main");
    assert(first.caller_count == 0U);
    assert(first.callee_count == 1U);

    const auto& second = map.functions[1];
    assert(second.caller_count == 1U);
    assert(second.callee_count == 0U);
    assert(!second.exported);

    // Nothing calls the exported entry, but it is not unreferenced: the export
    // table refers to it.
    assert(!first.structurally_unreferenced());
    assert(!second.structurally_unreferenced());
    assert(map.summary.structurally_unreferenced == 0U);
}

void reachability_follows_call_edges_from_both_roots() {
    const Fixture fixture;
    const auto map = fixture.build();

    assert(map.functions[0].reachable_from_entry_point);
    assert(map.functions[1].reachable_from_entry_point);
    assert(map.summary.reachable_from_entry_point == 2U);

    assert(map.functions[0].reachable_from_export);
    assert(map.functions[1].reachable_from_export);
    assert(map.summary.reachable_from_export == 2U);

    assert(map.summary.structurally_reachable == 2U);
    assert(map.summary.attributed == 2U);
}

void prologue_facts_come_from_the_primary_range() {
    const Fixture fixture;
    const auto map = fixture.build();

    const auto& first = map.functions[0];
    assert(first.frame.decoded);
    assert(first.frame.prolog_size == 0x12U);
    assert(first.frame.stack_allocation == 96U);
    assert(first.frame.pushed_registers == 1U);
    assert(!first.frame.uses_frame_pointer());

    const auto& second = map.functions[1];
    assert(second.frame.uses_frame_pointer());
    assert(second.frame.frame_register == 5U);
    assert(second.frame.has_exception_handler);

    // The continuation's 4096-byte record belongs to the range, not to the
    // function's prologue.
    assert(second.frame.stack_allocation == 0U);

    assert(map.summary.with_frame_pointer == 1U);
    assert(map.summary.with_exception_handler == 1U);
    assert(map.summary.total_stack_allocation == 96U);
    assert(map.summary.largest_stack_allocation == 96U);

    // Neither function is a leaf: one reserves stack, the other sets a frame
    // pointer.
    assert(map.summary.leaf_functions == 0U);
}

void referencing_a_vtable_marks_a_construction_site() {
    const Fixture fixture;
    const auto map = fixture.build();

    const auto& second = map.functions[1];
    assert(second.installs_vtables.size() == 1U);
    assert(second.installs_vtables[0].class_display_name == "CThing");
    assert(second.installs_vtables[0].vtable_index == 0U);
    assert(second.installs_vtables[0].vtable_rva == 0x2180U);

    assert(map.summary.with_vtable_install == 1U);
    assert(map.class_coverage[0].install_sites == 1U);

    // A vtable reference is an attribution in its own right.
    assert(map.summary.attributed == 2U);

    // The first function references no vtable.
    assert(map.functions[0].installs_vtables.empty());
}

void resource_families_are_read_from_literal_text() {
    using dmc::rengine::exe::resource_family_hints;

    assert(resource_family_hints("demo\\m05_b00\\m05_b00.pac") ==
           std::vector<std::string>{"PAC"});
    assert(resource_family_hints("%sDMC3-%d.nbz") == std::vector<std::string>{"NBZ"});

    // Case is irrelevant, and one literal can name several families.
    assert(resource_family_hints(".PTX") == std::vector<std::string>{"PTX"});
    const auto several = resource_family_hints("convert .mod to .scm");
    assert(several.size() == 2U);
    assert(several[0] == "MOD");
    assert(several[1] == "SCM");

    // A shader path names the shader family, not a resource container.
    assert(resource_family_hints("shaders/hlsl/ps/col.hlsl") ==
           std::vector<std::string>{"SHADER"});

    assert(resource_family_hints("hello world").empty());
    assert(resource_family_hints("").empty());
}

void literal_families_reach_the_summary_and_the_census() {
    const Fixture fixture;
    const auto map = fixture.build();

    // The fixture's literals name no family, so the census stays empty rather
    // than inventing one.
    assert(map.summary.with_resource_family == 0U);
    assert(map.resource_family_usage.empty());
}

void a_constant_table_index_names_one_element() {
    const Fixture fixture;

    // Treat the literal region as a stride-16 table: the function's reference
    // to the second literal is then a constant index of one.
    dmc::rengine::exe::StringTableScanResult tables;
    dmc::rengine::exe::StringTableRun run;
    run.base_rva = 0x2000U;
    run.stride = 16U;
    run.entries = 2U;
    run.first_name = "hello world";
    tables.runs.push_back(run);

    FunctionMapInputs inputs;
    inputs.image = &fixture.image;
    inputs.directories = &fixture.directories;
    inputs.rtti = &fixture.rtti;
    inputs.graph = &fixture.graph;
    inputs.name_tables = &tables;

    const auto map =
        FunctionMapBuilder::build(std::span<const std::byte>{fixture.bytes}, inputs);

    // Function A reaches element 0, function B reaches element 1.
    assert(map.functions[0].name_tables.size() == 1U);
    assert(map.functions[0].name_tables[0].table_base_rva == 0x2000U);
    assert(map.functions[0].name_tables[0].constant_index);
    assert(map.functions[0].name_tables[0].element_index == 0U);

    assert(map.functions[1].name_tables.size() == 1U);
    assert(map.functions[1].name_tables[0].constant_index);
    assert(map.functions[1].name_tables[0].element_index == 1U);
    assert(map.functions[1].name_tables[0].offset_in_table == 16U);

    assert(map.summary.with_name_table == 2U);
    assert(map.summary.constant_index_references == 2U);
    assert(map.summary.computed_index_references == 0U);
    assert(map.summary.name_tables_referenced == 1U);

    assert(map.name_table_usage.size() == 1U);
    assert(map.name_table_usage[0].referencing_functions == 2U);
    assert(map.name_table_usage[0].elements_named_by_constant == 2U);

    // Literals reached through the table are no longer counted as loose
    // literal references.
    assert(map.summary.with_string_reference == 0U);
}

void an_unaligned_offset_is_not_a_constant_index() {
    const Fixture fixture;

    // A stride of 24 makes the second literal land four bytes into element one.
    dmc::rengine::exe::StringTableScanResult tables;
    dmc::rengine::exe::StringTableRun run;
    run.base_rva = 0x2000U;
    run.stride = 12U;
    run.entries = 4U;
    tables.runs.push_back(run);

    FunctionMapInputs inputs;
    inputs.image = &fixture.image;
    inputs.graph = &fixture.graph;
    inputs.name_tables = &tables;

    const auto map =
        FunctionMapBuilder::build(std::span<const std::byte>{fixture.bytes}, inputs);

    const auto& reference = map.functions[1].name_tables.at(0);
    assert(!reference.constant_index);
    assert(reference.element_index == 1U);
    assert(reference.offset_in_element == 4U);
    assert(map.summary.computed_index_references == 1U);
}

void a_missing_graph_is_refused() {
    const Fixture fixture;
    FunctionMapInputs inputs;
    inputs.image = &fixture.image;

    const auto map =
        FunctionMapBuilder::build(std::span<const std::byte>{fixture.bytes}, inputs);
    assert(map.functions.empty());
    assert(!map.warnings.empty());
}

void a_map_without_rtti_or_imports_still_counts_functions() {
    const Fixture fixture;
    FunctionMapInputs inputs;
    inputs.image = &fixture.image;
    inputs.graph = &fixture.graph;

    const auto map =
        FunctionMapBuilder::build(std::span<const std::byte>{fixture.bytes}, inputs);
    assert(map.functions.size() == 2U);
    assert(map.summary.with_virtual_binding == 0U);
    assert(map.summary.with_import_call == 0U);
    // Literals do not depend on the import or type tables.
    assert(map.summary.with_string_reference == 2U);
}

} // namespace

int main() {
    literals_are_attributed_to_the_functions_that_reference_them();
    imports_are_attributed_directly_and_through_thunks();
    vtable_slots_bind_functions_to_classes();
    call_edges_and_exports_are_recorded();
    reachability_follows_call_edges_from_both_roots();
    prologue_facts_come_from_the_primary_range();
    a_constant_table_index_names_one_element();
    an_unaligned_offset_is_not_a_constant_index();
    referencing_a_vtable_marks_a_construction_site();
    resource_families_are_read_from_literal_text();
    literal_families_reach_the_summary_and_the_census();
    a_missing_graph_is_refused();
    a_map_without_rtti_or_imports_still_counts_functions();
    return 0;
}
