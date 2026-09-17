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
using dmc::rengine::exe::RttiBaseClass;
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

void an_address_pointing_at_a_nul_is_not_a_table_reference() {
    Fixture fixture;

    // The linker folds an empty string literal into any NUL byte it can find,
    // and a name table's padding is full of them. Such a reference arrives with
    // a table's coordinates and means nothing about the table: in the retail
    // image four functions pass one address inside a record's padding to a
    // "%s%s%s%s" call, and it resolves to "".
    const auto offset = fixture.image.rva_to_file_offset(0x2010U);
    assert(offset.has_value());
    fixture.bytes[static_cast<std::size_t>(*offset)] = std::byte{0};

    dmc::rengine::exe::StringTableScanResult tables;
    dmc::rengine::exe::StringTableRun run;
    run.base_rva = 0x2000U;
    run.stride = 16U;
    run.entries = 2U;
    tables.runs.push_back(run);

    FunctionMapInputs inputs;
    inputs.image = &fixture.image;
    inputs.graph = &fixture.graph;
    inputs.name_tables = &tables;

    const auto map =
        FunctionMapBuilder::build(std::span<const std::byte>{fixture.bytes}, inputs);

    // Function A still reaches element 0; function B's reference is dropped.
    assert(map.functions[0].name_tables.size() == 1U);
    assert(map.functions[1].name_tables.empty());
    assert(map.name_table_usage[0].elements_named_by_constant == 1U);
}

void an_indexed_array_carries_its_element_size_and_fields() {
    Fixture fixture;

    // Overwrite function B's body with a base load, an index multiply and two
    // reads at different offsets inside a 24-byte element:
    //   48 8d 1d <rel>   lea rbx,[rip+rel]        ; -> .rdata 0x2000
    //   48 8d 04 40      lea rax,[rax+rax*2]      ; index * 3
    //   8b 0c c3         mov ecx,[rbx+rax*8]      ; + 0
    //   8b 4c c3 08      mov ecx,[rbx+rax*8+8]    ; + 8
    //   c3               ret
    const auto body = fixture.image.rva_to_file_offset(0x1040U);
    assert(body.has_value());
    const auto at = static_cast<std::size_t>(*body);
    put(fixture.bytes, at, {0x48, 0x8D, 0x1D});
    put_i32(fixture.bytes, at + 3U,
            static_cast<std::int32_t>(0x2000) - static_cast<std::int32_t>(0x1047));
    put(fixture.bytes, at + 7U, {0x48, 0x8D, 0x04, 0x40});
    put(fixture.bytes, at + 11U, {0x8B, 0x0C, 0xC3});
    put(fixture.bytes, at + 14U, {0x8B, 0x4C, 0xC3, 0x08});
    put(fixture.bytes, at + 18U, {0xC3});

    PeFunctionTable table;
    table.functions.push_back(PeFunctionRange{0x1040U, 0x1060U, 0U, false, 0x1040U});
    const auto graph =
        CodeGraphBuilder::build(std::span<const std::byte>{fixture.bytes}, fixture.image, table);

    FunctionMapInputs inputs;
    inputs.image = &fixture.image;
    inputs.graph = &graph;
    const auto map =
        FunctionMapBuilder::build(std::span<const std::byte>{fixture.bytes}, inputs);

    assert(map.indexed_arrays.size() == 1U);
    const auto& array = map.indexed_arrays[0];
    assert(array.base_rva == 0x2000U);
    // A scale of eight against an index already multiplied by three: an element
    // size no scale field alone can express.
    assert(array.element_bytes == 24U);
    assert(array.sites == 2U);
    const std::vector<std::uint32_t> expected_fields{0U, 8U};
    assert(array.field_offsets == expected_fields);
    assert(map.summary.consistent_array_accesses == 2U);
    assert(map.summary.inconsistent_array_accesses == 0U);
}

void a_field_offset_outside_the_element_is_not_an_array() {
    Fixture fixture;

    // The same pair, but the second read is 8 bytes into a 4-byte element,
    // which cannot be a field of it. The element size or the base is wrong, so
    // the site is counted against the inference rather than believed.
    //   lea rbx,[rip+rel]
    //   8b 4c 9b 08       mov ecx,[rbx+rbx*4+8]  -- no: use rax as index
    const auto body = fixture.image.rva_to_file_offset(0x1040U);
    assert(body.has_value());
    const auto at = static_cast<std::size_t>(*body);
    put(fixture.bytes, at, {0x48, 0x8D, 0x1D});
    put_i32(fixture.bytes, at + 3U,
            static_cast<std::int32_t>(0x2000) - static_cast<std::int32_t>(0x1047));
    put(fixture.bytes, at + 7U, {0x8B, 0x4C, 0x83, 0x08});  // mov ecx,[rbx+rax*4+8]
    put(fixture.bytes, at + 11U, {0xC3});

    PeFunctionTable table;
    table.functions.push_back(PeFunctionRange{0x1040U, 0x1060U, 0U, false, 0x1040U});
    const auto graph =
        CodeGraphBuilder::build(std::span<const std::byte>{fixture.bytes}, fixture.image, table);

    FunctionMapInputs inputs;
    inputs.image = &fixture.image;
    inputs.graph = &graph;
    const auto map =
        FunctionMapBuilder::build(std::span<const std::byte>{fixture.bytes}, inputs);

    assert(map.indexed_arrays.empty());
    assert(map.summary.inconsistent_array_accesses == 1U);
    assert(map.summary.consistent_array_accesses == 0U);
}

void image_base_reads_sharing_an_index_register_are_one_array() {
    Fixture fixture;

    // Two reads against a register holding the image base, in one function,
    // through one index register, at one element size. Their addresses differ
    // by four, which is inside the eight-byte element, so they are two fields
    // of one array and the lower address bounds where it starts.
    //   48 8d 1d <rel>    lea rbx,[rip+rel]        ; -> RVA 0 (__ImageBase)
    //   8b 8c c3 00 20 00 00   mov ecx,[rbx+rax*8+0x2000]
    //   8b 8c c3 04 20 00 00   mov ecx,[rbx+rax*8+0x2004]
    const auto body = fixture.image.rva_to_file_offset(0x1040U);
    assert(body.has_value());
    const auto at = static_cast<std::size_t>(*body);
    put(fixture.bytes, at, {0x48, 0x8D, 0x1D});
    put_i32(fixture.bytes, at + 3U, -static_cast<std::int32_t>(0x1047));
    put(fixture.bytes, at + 7U, {0x8B, 0x8C, 0xC3});
    put_i32(fixture.bytes, at + 10U, 0x2000);
    put(fixture.bytes, at + 14U, {0x8B, 0x8C, 0xC3});
    put_i32(fixture.bytes, at + 17U, 0x2004);
    put(fixture.bytes, at + 21U, {0xC3});

    PeFunctionTable table;
    table.functions.push_back(PeFunctionRange{0x1040U, 0x1060U, 0U, false, 0x1040U});
    const auto graph =
        CodeGraphBuilder::build(std::span<const std::byte>{fixture.bytes}, fixture.image, table);

    FunctionMapInputs inputs;
    inputs.image = &fixture.image;
    inputs.graph = &graph;
    const auto map =
        FunctionMapBuilder::build(std::span<const std::byte>{fixture.bytes}, inputs);

    assert(map.summary.image_base_groups == 1U);
    assert(map.summary.image_base_groups_with_several_reads == 1U);
    assert(map.summary.image_base_groups_spanning_elements == 0U);
    assert(map.indexed_arrays.size() == 1U);

    const auto& array = map.indexed_arrays[0];
    assert(array.base_rva == 0x2000U);
    assert(array.element_bytes == 8U);
    // The base is an upper bound: the array may begin earlier, and only the
    // offsets between the reads are measured.
    assert(!array.base_measured);
    const std::vector<std::uint32_t> expected{0U, 4U};
    assert(array.field_offsets == expected);
}

void image_base_reads_spanning_elements_are_two_arrays_not_one() {
    Fixture fixture;

    // The same shape, but the two addresses are 0x40 apart, well beyond the
    // eight-byte element. One index register was reused for a second array, so
    // nothing here says these are fields of anything, and grouping them by how
    // close their addresses are would invent a layout.
    const auto body = fixture.image.rva_to_file_offset(0x1040U);
    assert(body.has_value());
    const auto at = static_cast<std::size_t>(*body);
    put(fixture.bytes, at, {0x48, 0x8D, 0x1D});
    put_i32(fixture.bytes, at + 3U, -static_cast<std::int32_t>(0x1047));
    put(fixture.bytes, at + 7U, {0x8B, 0x8C, 0xC3});
    put_i32(fixture.bytes, at + 10U, 0x2000);
    put(fixture.bytes, at + 14U, {0x8B, 0x8C, 0xC3});
    put_i32(fixture.bytes, at + 17U, 0x2040);
    put(fixture.bytes, at + 21U, {0xC3});

    PeFunctionTable table;
    table.functions.push_back(PeFunctionRange{0x1040U, 0x1060U, 0U, false, 0x1040U});
    const auto graph =
        CodeGraphBuilder::build(std::span<const std::byte>{fixture.bytes}, fixture.image, table);

    FunctionMapInputs inputs;
    inputs.image = &fixture.image;
    inputs.graph = &graph;
    const auto map =
        FunctionMapBuilder::build(std::span<const std::byte>{fixture.bytes}, inputs);

    assert(map.summary.image_base_groups_with_several_reads == 1U);
    assert(map.summary.image_base_groups_spanning_elements == 1U);
    assert(map.indexed_arrays.empty());
}

void a_virtual_call_on_this_resolves_to_a_class_and_a_target() {
    Fixture fixture;

    // Function B is a virtual method of the fixture's class: the RTTI vtable
    // binds it into a slot. Give it a call on `this` at slot one, and the
    // receiver follows from the calling convention rather than from analysis —
    // rcx holds the first argument, the load off it is the vtable pointer, and
    // the method's own binding says which vtable that is.
    //   48 8b 01          mov rax,[rcx]
    //   ff 10             call QWORD PTR [rax]
    //   c3                ret
    const auto body = fixture.image.rva_to_file_offset(0x1040U);
    assert(body.has_value());
    const auto at = static_cast<std::size_t>(*body);
    put(fixture.bytes, at, {0x48, 0x8B, 0x01});
    put(fixture.bytes, at + 3U, {0xFF, 0x10});
    put(fixture.bytes, at + 5U, {0xC3});

    PeFunctionTable table;
    table.functions.push_back(PeFunctionRange{0x1000U, 0x1040U, 0U, false, 0x1000U});
    table.functions.push_back(PeFunctionRange{0x1040U, 0x1060U, 0U, false, 0x1040U});
    const auto graph =
        CodeGraphBuilder::build(std::span<const std::byte>{fixture.bytes}, fixture.image, table);

    FunctionMapInputs inputs;
    inputs.image = &fixture.image;
    inputs.rtti = &fixture.rtti;
    inputs.graph = &graph;
    const auto map =
        FunctionMapBuilder::build(std::span<const std::byte>{fixture.bytes}, inputs);

    assert(map.summary.dispatch_sites == 1U);
    assert(map.summary.dispatch_sites_on_this == 1U);
    assert(map.summary.dispatch_sites_in_a_bound_function == 1U);
    assert(map.summary.dispatch_sites_resolved == 1U);

    assert(map.resolved_dispatches.size() == 1U);
    const auto& dispatch = map.resolved_dispatches[0];
    assert(dispatch.caller_rva == 0x1040U);
    assert(dispatch.displacement == 0U);
    assert(dispatch.slot == 0U);
    assert(dispatch.class_display_name == "CThing");
    // The one slot of the fixture's vtable holds function B itself, so the
    // method's own binding resolves the call back to it.
    assert(dispatch.target_rva == 0x1040U);
}

void a_dispatch_on_something_other_than_this_is_not_resolved() {
    Fixture fixture;

    // The same call, but the vtable pointer comes from rdx rather than from the
    // register the convention puts the first argument in. rdx is the second
    // argument, so the site is classified as being on an argument rather than
    // on `this` — and still left unresolved, because what a caller passes there
    // is not something this function says.
    //   48 8b 02          mov rax,[rdx]
    //   ff 10             call QWORD PTR [rax]
    const auto body = fixture.image.rva_to_file_offset(0x1040U);
    assert(body.has_value());
    const auto at = static_cast<std::size_t>(*body);
    put(fixture.bytes, at, {0x48, 0x8B, 0x02});
    put(fixture.bytes, at + 3U, {0xFF, 0x10});
    put(fixture.bytes, at + 5U, {0xC3});

    PeFunctionTable table;
    table.functions.push_back(PeFunctionRange{0x1040U, 0x1060U, 0U, false, 0x1040U});
    const auto graph =
        CodeGraphBuilder::build(std::span<const std::byte>{fixture.bytes}, fixture.image, table);

    FunctionMapInputs inputs;
    inputs.image = &fixture.image;
    inputs.rtti = &fixture.rtti;
    inputs.graph = &graph;
    const auto map =
        FunctionMapBuilder::build(std::span<const std::byte>{fixture.bytes}, inputs);

    assert(map.summary.dispatch_sites == 1U);
    assert(map.summary.dispatch_sites_on_this == 0U);
    assert(map.summary.dispatch_sites_on_an_argument == 1U);
    assert(map.resolved_dispatches.empty());
}

void a_constructor_store_names_the_class_and_its_layout() {
    Fixture fixture;

    // A constructor writes its class's vtable at offset zero, which is what
    // identifies it, and whatever else it writes into the object names what the
    // object contains.
    //   48 8d 05 <rel>    lea rax,[rip -> the CThing vtable at 0x2180]
    //   48 89 01          mov [rcx],rax            ; offset 0: this is CThing
    //   48 8d 05 <rel>    lea rax,[rip -> 0x2180]
    //   48 89 41 10       mov [rcx+0x10],rax       ; offset 16: a base subobject
    //   c3                ret
    const auto body = fixture.image.rva_to_file_offset(0x1040U);
    assert(body.has_value());
    const auto at = static_cast<std::size_t>(*body);
    put(fixture.bytes, at, {0x48, 0x8D, 0x05});
    put_i32(fixture.bytes, at + 3U,
            static_cast<std::int32_t>(0x2180) - static_cast<std::int32_t>(0x1047));
    put(fixture.bytes, at + 7U, {0x48, 0x89, 0x01});
    put(fixture.bytes, at + 10U, {0x48, 0x8D, 0x05});
    put_i32(fixture.bytes, at + 13U,
            static_cast<std::int32_t>(0x2180) - static_cast<std::int32_t>(0x1051));
    put(fixture.bytes, at + 17U, {0x48, 0x89, 0x41, 0x10});
    put(fixture.bytes, at + 21U, {0xC3});

    PeFunctionTable table;
    table.functions.push_back(PeFunctionRange{0x1040U, 0x1060U, 0U, false, 0x1040U});
    const auto graph =
        CodeGraphBuilder::build(std::span<const std::byte>{fixture.bytes}, fixture.image, table);

    FunctionMapInputs inputs;
    inputs.image = &fixture.image;
    inputs.rtti = &fixture.rtti;
    inputs.graph = &graph;
    const auto map =
        FunctionMapBuilder::build(std::span<const std::byte>{fixture.bytes}, inputs);

    assert(map.summary.stores_into_this == 2U);
    assert(map.summary.stores_of_a_vtable == 2U);
    assert(map.summary.constructors_identified == 1U);
    assert(map.functions[0].constructs_class == "CThing");

    // Only the non-zero offset is layout; offset zero says what the object is.
    assert(map.class_field_layout.size() == 1U);
    const auto& field = map.class_field_layout[0];
    assert(field.class_display_name == "CThing");
    assert(field.offset == 16U);
    assert(field.member_class_display_name == "CThing");
    // Same class, so a base subobject rather than something the object holds.
    assert(!field.embedded_member);
    // The fixture's RTTI records a subobject offset of zero, which is not 16,
    // so the store stands alone rather than being confirmed twice over.
    assert(!field.offset_confirmed_by_rtti);
}

void a_store_of_something_that_is_not_a_vtable_is_not_layout() {
    Fixture fixture;

    // The same shape, but the address stored is a string literal rather than a
    // vtable. It is counted as a store into the object and goes no further.
    const auto body = fixture.image.rva_to_file_offset(0x1040U);
    assert(body.has_value());
    const auto at = static_cast<std::size_t>(*body);
    put(fixture.bytes, at, {0x48, 0x8D, 0x05});
    put_i32(fixture.bytes, at + 3U,
            static_cast<std::int32_t>(0x2000) - static_cast<std::int32_t>(0x1047));
    put(fixture.bytes, at + 7U, {0x48, 0x89, 0x01});
    put(fixture.bytes, at + 10U, {0xC3});

    PeFunctionTable table;
    table.functions.push_back(PeFunctionRange{0x1040U, 0x1060U, 0U, false, 0x1040U});
    const auto graph =
        CodeGraphBuilder::build(std::span<const std::byte>{fixture.bytes}, fixture.image, table);

    FunctionMapInputs inputs;
    inputs.image = &fixture.image;
    inputs.rtti = &fixture.rtti;
    inputs.graph = &graph;
    const auto map =
        FunctionMapBuilder::build(std::span<const std::byte>{fixture.bytes}, inputs);

    assert(map.summary.stores_into_this == 1U);
    assert(map.summary.stores_of_a_vtable == 0U);
    assert(map.summary.constructors_identified == 0U);
    assert(map.class_field_layout.empty());
}

void a_dispatch_through_a_base_subobject_uses_that_subobject_vtable() {
    Fixture fixture;

    // Give the fixture's class a second vtable sitting 16 bytes into the
    // object, as multiple inheritance does, and have its method dispatch
    // through that offset. The RTTI records where each vtable sits, so the call
    // resolves into the subobject's vtable rather than the class's primary one
    // — which is the whole point: at that offset a different function runs.
    fixture.rtti.classes[0].vtables.push_back(
        RttiVtable{0x2160U, 0x21A0U, 1U, 16U, 0U});

    // rva 0x21A0 lives at file offset 0x5A0; point its one slot at function A.
    put_u64(fixture.bytes, 0x5A0U, kImageBase + 0x1000U);

    //   48 8b 41 10       mov rax,[rcx+0x10]     ; the subobject's vtable
    //   ff 10             call QWORD PTR [rax]   ; slot 0
    const auto body = fixture.image.rva_to_file_offset(0x1040U);
    assert(body.has_value());
    const auto at = static_cast<std::size_t>(*body);
    put(fixture.bytes, at, {0x48, 0x8B, 0x41, 0x10});
    put(fixture.bytes, at + 4U, {0xFF, 0x10});
    put(fixture.bytes, at + 6U, {0xC3});

    PeFunctionTable table;
    table.functions.push_back(PeFunctionRange{0x1000U, 0x1040U, 0U, false, 0x1000U});
    table.functions.push_back(PeFunctionRange{0x1040U, 0x1060U, 0U, false, 0x1040U});
    const auto graph =
        CodeGraphBuilder::build(std::span<const std::byte>{fixture.bytes}, fixture.image, table);

    FunctionMapInputs inputs;
    inputs.image = &fixture.image;
    inputs.rtti = &fixture.rtti;
    inputs.graph = &graph;
    const auto map =
        FunctionMapBuilder::build(std::span<const std::byte>{fixture.bytes}, inputs);

    assert(map.summary.dispatch_sites_on_this == 1U);
    assert(map.summary.dispatch_sites_resolved == 1U);
    assert(map.summary.dispatch_sites_on_a_member == 1U);

    assert(map.resolved_dispatches.size() == 1U);
    const auto& dispatch = map.resolved_dispatches[0];
    assert(dispatch.receiver_field_offset == 16U);
    assert(dispatch.slot == 0U);
    // Function A, from the subobject's vtable — not function B, which slot 0 of
    // the primary vtable holds.
    assert(dispatch.target_rva == 0x1000U);
}

void a_dispatch_through_a_pointer_member_is_counted_not_resolved() {
    Fixture fixture;

    // Two loads: the field holds a pointer, and the vtable comes from what it
    // points at. The enclosing class's layout describes the pointer, not the
    // object, so nothing here says what is on the other end.
    //   48 8b 41 10       mov rax,[rcx+0x10]     ; the pointer
    //   48 8b 10          mov rdx,[rax]          ; the pointee's vtable
    //   ff 12             call QWORD PTR [rdx]
    const auto body = fixture.image.rva_to_file_offset(0x1040U);
    assert(body.has_value());
    const auto at = static_cast<std::size_t>(*body);
    put(fixture.bytes, at, {0x48, 0x8B, 0x41, 0x10});
    put(fixture.bytes, at + 4U, {0x48, 0x8B, 0x10});
    put(fixture.bytes, at + 7U, {0xFF, 0x12});
    put(fixture.bytes, at + 9U, {0xC3});

    PeFunctionTable table;
    table.functions.push_back(PeFunctionRange{0x1040U, 0x1060U, 0U, false, 0x1040U});
    const auto graph =
        CodeGraphBuilder::build(std::span<const std::byte>{fixture.bytes}, fixture.image, table);

    FunctionMapInputs inputs;
    inputs.image = &fixture.image;
    inputs.rtti = &fixture.rtti;
    inputs.graph = &graph;
    const auto map =
        FunctionMapBuilder::build(std::span<const std::byte>{fixture.bytes}, inputs);

    assert(map.summary.dispatch_sites_on_this == 1U);
    assert(map.summary.dispatch_sites_through_a_pointer_member == 1U);
    assert(map.summary.dispatch_sites_resolved == 0U);
    assert(map.resolved_dispatches.empty());
}

void a_pointer_stored_into_this_records_its_callee() {
    Fixture fixture;

    // `call` then a store of what it returned into the object. Where the callee
    // constructs something the field's type would follow; where it is an
    // allocator, as it is throughout the retail image, the store types nothing
    // and is counted rather than believed.
    // `this` goes into a saved register first, because rcx is volatile and the
    // call would take it — which is what real constructors do and why the
    // pattern is visible at all.
    //   48 8b d9          mov rbx,rcx
    //   e8 <rel>          call 0x1000
    //   48 89 43 20       mov [rbx+0x20],rax
    //   c3                ret
    const auto body = fixture.image.rva_to_file_offset(0x1040U);
    assert(body.has_value());
    const auto at = static_cast<std::size_t>(*body);
    put(fixture.bytes, at, {0x48, 0x8B, 0xD9});
    put(fixture.bytes, at + 3U, {0xE8});
    put_i32(fixture.bytes, at + 4U,
            static_cast<std::int32_t>(0x1000) - static_cast<std::int32_t>(0x1048));
    put(fixture.bytes, at + 8U, {0x48, 0x89, 0x43, 0x20});
    put(fixture.bytes, at + 12U, {0xC3});

    PeFunctionTable table;
    table.functions.push_back(PeFunctionRange{0x1040U, 0x1060U, 0U, false, 0x1040U});
    const auto graph =
        CodeGraphBuilder::build(std::span<const std::byte>{fixture.bytes}, fixture.image, table);

    assert(graph.functions[0].pointer_stores_into_this.size() == 1U);
    assert(graph.functions[0].pointer_stores_into_this[0].offset == 32U);
    assert(graph.functions[0].pointer_stores_into_this[0].callee_rva == 0x1000U);

    FunctionMapInputs inputs;
    inputs.image = &fixture.image;
    inputs.rtti = &fixture.rtti;
    inputs.graph = &graph;
    const auto map =
        FunctionMapBuilder::build(std::span<const std::byte>{fixture.bytes}, inputs);
    assert(map.summary.pointer_stores_into_this == 1U);
    assert(map.summary.pointer_stores_from_a_constructor == 0U);
}

// Lay a base and a derived class over the fixture so the slot census has
// something to compare. `.text` gets two bodies the census must recognise
// without reading them: a bare return, and a jump through the import slot the
// import directory names `_purecall`.
void add_slot_census_shapes(Fixture& fixture) {
    put(fixture.bytes, 0x290U, {0xC3});          // rva 0x1090: an empty body
    put(fixture.bytes, 0x2A0U, {0xFF, 0x25});    // rva 0x10A0: jmp [rip+disp]
    put_i32(fixture.bytes, 0x2A2U, 0x106A);      // -> IAT slot at rva 0x2110

    fixture.directories.imports[0].functions.push_back(
        PeImportedFunction{"_purecall", 0x2110U, 0U, 0U, false});

    // CBase's vtable at rva 0x2188: code, pure, empty.
    put_u64(fixture.bytes, 0x588U, kImageBase + 0x1000U);
    put_u64(fixture.bytes, 0x590U, kImageBase + 0x10A0U);
    put_u64(fixture.bytes, 0x598U, kImageBase + 0x1090U);

    // CDerived's vtable at rva 0x21C0: its own code, then the empty body
    // twice. Slot 1 fills in what the base declared pure; slot 2 is the base's
    // own target left in place.
    put_u64(fixture.bytes, 0x5C0U, kImageBase + 0x1040U);
    put_u64(fixture.bytes, 0x5C8U, kImageBase + 0x1090U);
    put_u64(fixture.bytes, 0x5D0U, kImageBase + 0x1090U);

    RttiClass base;
    base.decorated_name = ".?AVCBase@@";
    base.display_name = "CBase";
    base.vtables.push_back(RttiVtable{0x2168U, 0x2188U, 3U, 0U, 0U});
    base.hierarchy.push_back(RttiBaseClass{".?AVCBase@@", "CBase", 0, 0, 0, 0U});
    fixture.rtti.classes.push_back(std::move(base));

    RttiClass derived;
    derived.decorated_name = ".?AVCDerived@@";
    derived.display_name = "CDerived";
    derived.vtables.push_back(RttiVtable{0x21A0U, 0x21C0U, 3U, 0U, 0U});
    derived.hierarchy.push_back(RttiBaseClass{".?AVCDerived@@", "CDerived", 0, 0, 0, 1U});
    derived.hierarchy.push_back(RttiBaseClass{".?AVCBase@@", "CBase", 0, 0, 0, 0U});
    fixture.rtti.classes.push_back(std::move(derived));
}

void a_slot_is_classified_by_what_its_target_starts_with() {
    Fixture fixture;
    add_slot_census_shapes(fixture);

    FunctionMapInputs inputs;
    inputs.image = &fixture.image;
    inputs.rtti = &fixture.rtti;
    inputs.directories = &fixture.directories;
    inputs.graph = &fixture.graph;
    const auto map =
        FunctionMapBuilder::build(std::span<const std::byte>{fixture.bytes}, inputs);

    // One slot from CThing, three each from CBase and CDerived.
    assert(map.summary.vtable_slots_classified == 7U);
    // Only CBase slot 1 reaches the import; CDerived fills that slot in.
    assert(map.summary.vtable_slots_pure_virtual == 1U);
    // CBase slot 2 and both of CDerived's tail slots.
    assert(map.summary.vtable_slots_empty_body == 3U);
    assert(map.summary.vtable_slots_implemented == 3U);
    // Those three reach two addresses: CThing and CDerived share one.
    assert(map.summary.vtable_slot_implementations == 2U);
}

void a_base_slot_records_what_its_inheritors_put_there() {
    Fixture fixture;
    add_slot_census_shapes(fixture);

    FunctionMapInputs inputs;
    inputs.image = &fixture.image;
    inputs.rtti = &fixture.rtti;
    inputs.directories = &fixture.directories;
    inputs.graph = &fixture.graph;
    const auto map =
        FunctionMapBuilder::build(std::span<const std::byte>{fixture.bytes}, inputs);

    // CThing has no inheritors, so only CBase's three slots are measured.
    assert(map.summary.base_slots_measured == 3U);
    assert(map.base_slot_overrides.size() == 3U);
    assert(map.summary.base_pairings_without_a_vtable == 0U);

    const auto& code = map.base_slot_overrides[0];
    assert(code.base_display_name == "CBase");
    assert(code.slot == 0U);
    assert(code.base_kind == dmc::rengine::exe::VtableSlotKind::implemented);
    assert(code.base_target_rva == 0x1000U);
    assert(code.derived_classes == 1U);
    assert(code.keep_base_target == 0U);
    assert(code.distinct_implementations == 1U);

    // A slot the base declares pure is never kept: there is nothing to keep.
    const auto& pure = map.base_slot_overrides[1];
    assert(pure.slot == 1U);
    assert(pure.base_kind == dmc::rengine::exe::VtableSlotKind::pure_virtual);
    assert(pure.keep_base_target == 0U);
    assert(pure.empty_bodies == 1U);
    assert(pure.pure_virtual == 0U);
    assert(pure.distinct_implementations == 0U);

    const auto& inherited = map.base_slot_overrides[2];
    assert(inherited.slot == 2U);
    assert(inherited.base_kind == dmc::rengine::exe::VtableSlotKind::empty_body);
    assert(inherited.base_target_rva == 0x1090U);
    assert(inherited.keep_base_target == 1U);
    assert(inherited.empty_bodies == 1U);
    assert(inherited.distinct_implementations == 0U);
}

void a_base_is_compared_through_its_own_subobject_vtable() {
    Fixture fixture;
    add_slot_census_shapes(fixture);

    // Move CDerived's CBase subobject sixteen bytes in, as a second base would
    // put it, and give the class a primary vtable of its own holding something
    // else entirely. Comparing primary tables would read that unrelated table;
    // the hierarchy's own recorded displacement is what says which one to use.
    fixture.rtti.classes.back().hierarchy[1].member_displacement = 16;
    fixture.rtti.classes.back().vtables[0].subobject_offset = 16U;
    fixture.rtti.classes.back().vtables.push_back(
        RttiVtable{0x21A8U, 0x21E0U, 1U, 0U, 0U});
    put_u64(fixture.bytes, 0x5E0U, kImageBase + 0x1090U);  // rva 0x21E0

    FunctionMapInputs inputs;
    inputs.image = &fixture.image;
    inputs.rtti = &fixture.rtti;
    inputs.directories = &fixture.directories;
    inputs.graph = &fixture.graph;
    const auto map =
        FunctionMapBuilder::build(std::span<const std::byte>{fixture.bytes}, inputs);

    assert(map.base_slot_overrides.size() == 3U);
    // Slot 0 still reads CDerived's own function out of the subobject table at
    // rva 0x21C0, not the empty body sitting in the primary one.
    const auto& code = map.base_slot_overrides[0];
    assert(code.derived_classes == 1U);
    assert(code.distinct_implementations == 1U);
    assert(code.empty_bodies == 0U);
}

void a_base_with_no_vtable_at_its_recorded_offset_is_not_measured() {
    Fixture fixture;
    add_slot_census_shapes(fixture);

    // The hierarchy says the base sits at +32 but no vtable is recorded there,
    // so there is no table to compare against. Reported rather than guessed at.
    fixture.rtti.classes.back().hierarchy[1].member_displacement = 32;

    FunctionMapInputs inputs;
    inputs.image = &fixture.image;
    inputs.rtti = &fixture.rtti;
    inputs.directories = &fixture.directories;
    inputs.graph = &fixture.graph;
    const auto map =
        FunctionMapBuilder::build(std::span<const std::byte>{fixture.bytes}, inputs);

    assert(map.summary.base_pairings_without_a_vtable == 1U);
    assert(map.base_slot_overrides.empty());
    // The classification is a property of the tables alone, so it is unmoved.
    assert(map.summary.vtable_slots_classified == 7U);
}

void a_purecall_thunk_is_named_by_the_import_table_not_its_shape() {
    Fixture fixture;
    add_slot_census_shapes(fixture);
    // Same jump, same encoding, different import. Nothing about the shape of a
    // thunk says pure-virtual; only the name on the other end does.
    fixture.directories.imports[0].functions.back().name = "alpha_other";

    FunctionMapInputs inputs;
    inputs.image = &fixture.image;
    inputs.rtti = &fixture.rtti;
    inputs.directories = &fixture.directories;
    inputs.graph = &fixture.graph;
    const auto map =
        FunctionMapBuilder::build(std::span<const std::byte>{fixture.bytes}, inputs);

    assert(map.summary.vtable_slots_pure_virtual == 0U);
    assert(map.summary.vtable_slots_implemented == 4U);
    assert(map.base_slot_overrides[1].base_kind ==
           dmc::rengine::exe::VtableSlotKind::implemented);
}

// A third function at rva 0x10C0 that nothing calls, plus the function table
// covering it. Everything that follows is about how it can be reached.
[[nodiscard]] PeFunctionTable table_with_a_third_function(Fixture& fixture) {
    put(fixture.bytes, 0x2C0U, {0xC3});
    auto table = fixture.table;
    table.functions.push_back(PeFunctionRange{0x10C0U, 0x10D0U, 0U, false, 0x10C0U});
    return table;
}

// Make function B dispatch through slot 2 of whatever it was given:
//   ff 50 10   call QWORD PTR [rax+0x10]
// It replaces the second `lea` in B's body, so the padding keeps the walk
// landing on B's closing return.
void make_b_dispatch_through_slot_two(Fixture& fixture) {
    put(fixture.bytes, 0x257U, {0xFF, 0x50, 0x10, 0x90, 0x90, 0x90, 0x90});
}

void the_dispatch_bound_reaches_what_direct_calls_cannot() {
    Fixture fixture;
    add_slot_census_shapes(fixture);
    const auto table = table_with_a_third_function(fixture);
    make_b_dispatch_through_slot_two(fixture);
    // CBase slot 2 now names the third function.
    put_u64(fixture.bytes, 0x598U, kImageBase + 0x10C0U);

    const auto graph =
        CodeGraphBuilder::build(std::span<const std::byte>{fixture.bytes}, fixture.image, table);
    FunctionMapInputs inputs;
    inputs.image = &fixture.image;
    inputs.rtti = &fixture.rtti;
    inputs.directories = &fixture.directories;
    inputs.graph = &graph;
    const auto map =
        FunctionMapBuilder::build(std::span<const std::byte>{fixture.bytes}, inputs);

    assert(map.functions.size() == 3U);
    // Direct calls reach the entry point and what it calls, and stop there.
    assert(map.summary.reachable_from_entry_point == 2U);
    // The dispatch in B reads slot 2 of something, and slot 2 of a vtable in
    // the image holds the third function. Nothing says the receiver is a
    // CBase; the bound assumes it could be.
    assert(map.summary.reachable_through_dispatch == 3U);
    assert(map.summary.outside_every_closure == 0U);
    assert(map.summary.dispatch_slots_reached == 1U);
}

void without_a_dispatch_the_bound_is_the_direct_closure() {
    Fixture fixture;
    add_slot_census_shapes(fixture);
    const auto table = table_with_a_third_function(fixture);
    put_u64(fixture.bytes, 0x598U, kImageBase + 0x10C0U);
    // Same vtable, same third function, but nothing dispatches. The bound must
    // not reach it: sitting in a vtable is not by itself a path from the entry
    // point.
    const auto graph =
        CodeGraphBuilder::build(std::span<const std::byte>{fixture.bytes}, fixture.image, table);
    FunctionMapInputs inputs;
    inputs.image = &fixture.image;
    inputs.rtti = &fixture.rtti;
    inputs.directories = &fixture.directories;
    inputs.graph = &graph;
    const auto map =
        FunctionMapBuilder::build(std::span<const std::byte>{fixture.bytes}, inputs);

    assert(map.summary.reachable_through_dispatch == 2U);
    assert(map.summary.outside_every_closure == 1U);
    assert(map.summary.dispatch_slots_reached == 0U);
}

void a_slot_declared_pure_is_not_a_candidate_target() {
    Fixture fixture;
    add_slot_census_shapes(fixture);
    const auto table = table_with_a_third_function(fixture);
    make_b_dispatch_through_slot_two(fixture);

    // Slot 2 of CBase now reaches the _purecall thunk instead. A pure slot
    // reaches the runtime's abort path, not the program, so the bound must not
    // treat it as somewhere control can go.
    put_u64(fixture.bytes, 0x598U, kImageBase + 0x10A0U);
    // And slot 2 of CDerived is the third function, so there is still something
    // at that slot for the bound to find if it looked past the pure one.
    put_u64(fixture.bytes, 0x5D0U, kImageBase + 0x10C0U);

    const auto graph =
        CodeGraphBuilder::build(std::span<const std::byte>{fixture.bytes}, fixture.image, table);
    FunctionMapInputs inputs;
    inputs.image = &fixture.image;
    inputs.rtti = &fixture.rtti;
    inputs.directories = &fixture.directories;
    inputs.graph = &graph;
    const auto map =
        FunctionMapBuilder::build(std::span<const std::byte>{fixture.bytes}, inputs);

    // CDerived's slot 2 still names it, so it is reached — through the vtable
    // that defines the slot, not through the one that only declares it.
    assert(map.summary.reachable_through_dispatch == 3U);
    // CBase now declares both slot 1 and slot 2 pure.
    assert(map.summary.vtable_slots_pure_virtual == 2U);
}

// Lay three consecutive function addresses at rva 0x21D0 and give CDerived a
// vtable whose declared extent covers them.
void put_addresses_at_21d0(Fixture& fixture, std::uint32_t slot_count) {
    put_u64(fixture.bytes, 0x5C0U, kImageBase + 0x1040U);
    put_u64(fixture.bytes, 0x5C8U, kImageBase + 0x1090U);  // not in the inventory
    put_u64(fixture.bytes, 0x5D0U, kImageBase + 0x1000U);
    put_u64(fixture.bytes, 0x5D8U, kImageBase + 0x1040U);
    put_u64(fixture.bytes, 0x5E0U, kImageBase + 0x10C0U);
    fixture.rtti.classes.back().vtables[0].slot_count = slot_count;
}

void a_run_inside_a_vtable_is_not_a_table() {
    Fixture fixture;
    add_slot_census_shapes(fixture);
    const auto table = table_with_a_third_function(fixture);
    // Five slots: rva 0x21C0 through 0x21E8. Slot 1 holds something the
    // inventory does not cover, which splits the table into fragments, and the
    // second fragment starts at 0x21D0 — not at the vtable's base. Excluding
    // vtables by base alone would call that fragment a table of its own.
    put_addresses_at_21d0(fixture, 5U);

    const auto graph =
        CodeGraphBuilder::build(std::span<const std::byte>{fixture.bytes}, fixture.image, table);
    FunctionMapInputs inputs;
    inputs.image = &fixture.image;
    inputs.rtti = &fixture.rtti;
    inputs.directories = &fixture.directories;
    inputs.graph = &graph;
    const auto map =
        FunctionMapBuilder::build(std::span<const std::byte>{fixture.bytes}, inputs);

    assert(map.function_pointer_runs.empty());
    assert(map.summary.function_pointer_runs == 0U);
}

void the_same_addresses_outside_a_vtable_are_a_table() {
    Fixture fixture;
    add_slot_census_shapes(fixture);
    const auto table = table_with_a_third_function(fixture);
    // Identical bytes; only the declared extent changes. One slot means the
    // vtable ends at 0x21C8 and the addresses at 0x21D0 are outside it.
    put_addresses_at_21d0(fixture, 1U);

    const auto graph =
        CodeGraphBuilder::build(std::span<const std::byte>{fixture.bytes}, fixture.image, table);
    FunctionMapInputs inputs;
    inputs.image = &fixture.image;
    inputs.rtti = &fixture.rtti;
    inputs.directories = &fixture.directories;
    inputs.graph = &graph;
    const auto map =
        FunctionMapBuilder::build(std::span<const std::byte>{fixture.bytes}, inputs);

    assert(map.function_pointer_runs.size() == 1U);
    const auto& run = map.function_pointer_runs[0];
    assert(run.base_rva == 0x21D0U);
    assert(run.entries == 3U);
    // Nothing in the fixture's code takes the run's address.
    assert(run.referencing_functions == 0U);
}

void a_bound_method_reaching_into_this_floors_the_class_size() {
    Fixture fixture;
    // Function B is slot 0 of CThing's vtable. Replace its body so it touches
    // offset 0x40 of whatever it was given and returns:
    //   48 8b 41 40   mov rax,[rcx+0x40]
    //   c3            ret
    put(fixture.bytes, 0x240U, {0x48, 0x8B, 0x41, 0x40, 0xC3});

    const auto graph =
        CodeGraphBuilder::build(std::span<const std::byte>{fixture.bytes}, fixture.image,
                                fixture.table);
    FunctionMapInputs inputs;
    inputs.image = &fixture.image;
    inputs.rtti = &fixture.rtti;
    inputs.graph = &graph;
    const auto map =
        FunctionMapBuilder::build(std::span<const std::byte>{fixture.bytes}, inputs);

    assert(map.class_size_floors.size() == 1U);
    const auto& floor = map.class_size_floors[0];
    assert(floor.class_display_name == "CThing");
    // The first argument to a method the compiler bound into the vtable is the
    // object, so reaching offset 0x40 means at least 0x41 bytes.
    assert(floor.floor_from_field_access == 0x41U);
    // Only its own vtable pointer, since the fixture's class has no bases.
    assert(floor.floor_from_bases == 8U);
    assert(floor.floor_bytes == 0x41U);
    assert(floor.functions_speaking == 1U);
}

void a_base_at_a_displacement_floors_the_class_size() {
    Fixture fixture;
    // A second vtable 128 bytes into the object, and a hierarchy that records
    // a base sitting there. A base carrying a vtable occupies at least its
    // eight bytes, so the object reaches 136 whatever the code does.
    fixture.rtti.classes[0].vtables.push_back(RttiVtable{0x2160U, 0x21A0U, 1U, 128U, 0U});
    put_u64(fixture.bytes, 0x5A0U, kImageBase + 0x1000U);
    fixture.rtti.classes[0].hierarchy.push_back(
        RttiBaseClass{".?AVCThing@@", "CThing", 0, 0, 0, 0U});
    fixture.rtti.classes[0].hierarchy.push_back(
        RttiBaseClass{".?AVCUnder@@", "CUnder", 128, 0, 0, 0U});

    FunctionMapInputs inputs;
    inputs.image = &fixture.image;
    inputs.rtti = &fixture.rtti;
    inputs.graph = &fixture.graph;
    const auto map =
        FunctionMapBuilder::build(std::span<const std::byte>{fixture.bytes}, inputs);

    const auto& floor = map.class_size_floors[0];
    assert(floor.class_display_name == "CThing");
    assert(floor.floor_from_bases == 136U);
    assert(floor.deepest_base_display_name == "CUnder");
    assert(floor.floor_bytes == 136U);
}

void a_base_the_class_carries_no_vtable_for_does_not_floor_it() {
    Fixture fixture;
    // The hierarchy records a base at 128 but the class has no vtable there,
    // so nothing says the subobject occupies anything. An empty base at a
    // recorded displacement takes no room, and assuming eight bytes would
    // invent them.
    fixture.rtti.classes[0].hierarchy.push_back(
        RttiBaseClass{".?AVCThing@@", "CThing", 0, 0, 0, 0U});
    fixture.rtti.classes[0].hierarchy.push_back(
        RttiBaseClass{".?AVCUnder@@", "CUnder", 128, 0, 0, 0U});

    FunctionMapInputs inputs;
    inputs.image = &fixture.image;
    inputs.rtti = &fixture.rtti;
    inputs.graph = &fixture.graph;
    const auto map =
        FunctionMapBuilder::build(std::span<const std::byte>{fixture.bytes}, inputs);

    assert(map.class_size_floors[0].floor_from_bases == 8U);
}

void a_method_bound_into_a_subobject_reaches_further_into_the_object() {
    Fixture fixture;
    // Bind function B at subobject offset 128. It is handed a pointer to the
    // subobject, so an access at 0x40 inside it lands at 128 + 0x40 of the
    // complete object.
    fixture.rtti.classes[0].vtables.clear();
    fixture.rtti.classes[0].vtables.push_back(RttiVtable{0x2160U, 0x2180U, 1U, 128U, 0U});
    put(fixture.bytes, 0x240U, {0x48, 0x8B, 0x41, 0x40, 0xC3});

    const auto graph =
        CodeGraphBuilder::build(std::span<const std::byte>{fixture.bytes}, fixture.image,
                                fixture.table);
    FunctionMapInputs inputs;
    inputs.image = &fixture.image;
    inputs.rtti = &fixture.rtti;
    inputs.graph = &graph;
    const auto map =
        FunctionMapBuilder::build(std::span<const std::byte>{fixture.bytes}, inputs);

    assert(map.class_size_floors[0].floor_from_field_access == 128U + 0x41U);
}

void an_address_computed_off_this_is_not_a_field_access() {
    Fixture fixture;
    // `lea rax,[rcx+0x40]` computes an address without touching what is there,
    // and a one-past-the-end pointer is an ordinary thing to compute.
    put(fixture.bytes, 0x240U, {0x48, 0x8D, 0x41, 0x40, 0xC3});

    const auto graph =
        CodeGraphBuilder::build(std::span<const std::byte>{fixture.bytes}, fixture.image,
                                fixture.table);
    FunctionMapInputs inputs;
    inputs.image = &fixture.image;
    inputs.rtti = &fixture.rtti;
    inputs.graph = &graph;
    const auto map =
        FunctionMapBuilder::build(std::span<const std::byte>{fixture.bytes}, inputs);

    assert(map.class_size_floors[0].floor_from_field_access == 0U);
    assert(map.class_size_floors[0].floor_bytes == 8U);
}

void an_address_passed_first_marks_a_block_the_code_operates_on() {
    Fixture fixture;
    // Function A takes the address of the literal at rva 0x2000 into rcx and
    // calls function B with it. The convention puts the first argument there,
    // so B operates on whatever sits at that address.
    FunctionMapInputs inputs;
    inputs.image = &fixture.image;
    inputs.rtti = &fixture.rtti;
    inputs.graph = &fixture.graph;
    const auto map =
        FunctionMapBuilder::build(std::span<const std::byte>{fixture.bytes}, inputs);

    assert(map.global_state_blocks.size() == 1U);
    const auto& block = map.global_state_blocks[0];
    assert(block.base_rva == 0x2000U);
    assert(block.section == ".rdata");
    assert(block.call_sites == 1U);
    assert(block.distinct_callees == 1U);
    assert(block.distinct_callers == 1U);
    // Nothing else is addressed this way, so there is no next block.
    assert(block.bytes_to_next_block == 0U);
    assert(!block.reach_runs_past_the_next_block);
}

void a_callee_serving_one_block_lends_it_the_reach() {
    Fixture fixture;
    // Give B a body that reaches offset 0x20 of what it was given and returns.
    // B is called once, by A, with the address at rva 0x2000 — so every call
    // that reaches B passes that block, and the reach is the block's.
    put(fixture.bytes, 0x240U, {0x48, 0x8B, 0x41, 0x20, 0xC3});

    const auto graph =
        CodeGraphBuilder::build(std::span<const std::byte>{fixture.bytes}, fixture.image,
                                fixture.table);
    FunctionMapInputs inputs;
    inputs.image = &fixture.image;
    inputs.rtti = &fixture.rtti;
    inputs.graph = &graph;
    const auto map =
        FunctionMapBuilder::build(std::span<const std::byte>{fixture.bytes}, inputs);

    assert(map.global_state_blocks.size() == 1U);
    assert(map.global_state_blocks[0].field_reach == 0x21U);
    assert(map.summary.global_blocks_with_a_field_reach == 1U);
}

void a_callee_with_another_caller_lends_its_reach_to_nobody() {
    Fixture fixture;
    put(fixture.bytes, 0x240U, {0x48, 0x8B, 0x41, 0x20, 0xC3});
    // A second call to B, from the third function, with something the walk
    // cannot name. B now serves two first arguments, so its deepest offset
    // belongs to neither and lending it to the block would overstate the block.
    //   e8 rel32   call B
    //   c3         ret
    put(fixture.bytes, 0x2C0U, {0xE8});
    put_i32(fixture.bytes, 0x2C1U, -0x85);  // 0x10C5 - 0x85 = 0x1040
    put(fixture.bytes, 0x2C5U, {0xC3});
    auto table = fixture.table;
    table.functions.push_back(PeFunctionRange{0x10C0U, 0x10D0U, 0U, false, 0x10C0U});

    const auto graph =
        CodeGraphBuilder::build(std::span<const std::byte>{fixture.bytes}, fixture.image, table);
    FunctionMapInputs inputs;
    inputs.image = &fixture.image;
    inputs.rtti = &fixture.rtti;
    inputs.graph = &graph;
    const auto map =
        FunctionMapBuilder::build(std::span<const std::byte>{fixture.bytes}, inputs);

    assert(map.global_state_blocks.size() == 1U);
    assert(map.global_state_blocks[0].field_reach == 0U);
    assert(map.summary.global_block_sites_with_other_callers == 1U);
}

void an_interior_address_of_this_is_still_this() {
    Fixture fixture;
    // A second vtable sixteen bytes into the object, as multiple inheritance
    // lays one out, and a method that takes the address of that subobject
    // before dispatching through it:
    //   48 8d 41 10       lea rax,[rcx+0x10]     ; the subobject's address
    //   48 8b 10          mov rdx,[rax]          ; its vtable
    //   ff 12             call QWORD PTR [rdx]   ; slot 0
    fixture.rtti.classes[0].vtables.push_back(RttiVtable{0x2160U, 0x21A0U, 1U, 16U, 0U});
    put_u64(fixture.bytes, 0x5A0U, kImageBase + 0x1000U);

    const auto body = fixture.image.rva_to_file_offset(0x1040U);
    assert(body.has_value());
    const auto at = static_cast<std::size_t>(*body);
    put(fixture.bytes, at, {0x48, 0x8D, 0x41, 0x10});
    put(fixture.bytes, at + 4U, {0x48, 0x8B, 0x10});
    put(fixture.bytes, at + 7U, {0xFF, 0x12});
    put(fixture.bytes, at + 9U, {0xC3});

    PeFunctionTable table;
    table.functions.push_back(PeFunctionRange{0x1000U, 0x1040U, 0U, false, 0x1000U});
    table.functions.push_back(PeFunctionRange{0x1040U, 0x1060U, 0U, false, 0x1040U});
    const auto graph =
        CodeGraphBuilder::build(std::span<const std::byte>{fixture.bytes}, fixture.image, table);

    FunctionMapInputs inputs;
    inputs.image = &fixture.image;
    inputs.rtti = &fixture.rtti;
    inputs.graph = &graph;
    const auto map =
        FunctionMapBuilder::build(std::span<const std::byte>{fixture.bytes}, inputs);

    // Taking the address of something inside the object does not stop it being
    // the object, so the site is on `this` at offset sixteen and resolves into
    // the vtable sitting there.
    assert(map.summary.dispatch_sites_on_this == 1U);
    assert(map.summary.dispatch_sites_on_a_member == 1U);
    assert(map.resolved_dispatches.size() == 1U);
    assert(map.resolved_dispatches[0].receiver_field_offset == 16U);
    assert(map.resolved_dispatches[0].target_rva == 0x1000U);
}

void a_store_through_an_interior_address_lands_at_its_own_offset() {
    Fixture fixture;
    // `lea rax,[rcx+0x20]` then a vtable stored through rax is a store at
    // offset 32, not at offset 0. Reading it as 0 would name the function a
    // constructor of whatever class that vtable belongs to.
    //   48 8d 41 20       lea rax,[rcx+0x20]
    //   48 8d 0d ...      lea rcx,[rip+...]      ; the CThing vtable at 0x2180
    //   48 89 08          mov [rax],rcx
    const auto body = fixture.image.rva_to_file_offset(0x1040U);
    assert(body.has_value());
    const auto at = static_cast<std::size_t>(*body);
    put(fixture.bytes, at, {0x48, 0x8D, 0x41, 0x20});
    put(fixture.bytes, at + 4U, {0x48, 0x8D, 0x0D});
    // The lea ends at rva 0x104B, so 0x1135 reaches the CThing vtable at 0x2180.
    put_i32(fixture.bytes, at + 7U, 0x1135);
    put(fixture.bytes, at + 11U, {0x48, 0x89, 0x08});
    put(fixture.bytes, at + 14U, {0xC3});

    PeFunctionTable table;
    table.functions.push_back(PeFunctionRange{0x1040U, 0x1060U, 0U, false, 0x1040U});
    const auto graph =
        CodeGraphBuilder::build(std::span<const std::byte>{fixture.bytes}, fixture.image, table);

    FunctionMapInputs inputs;
    inputs.image = &fixture.image;
    inputs.rtti = &fixture.rtti;
    inputs.graph = &graph;
    const auto map =
        FunctionMapBuilder::build(std::span<const std::byte>{fixture.bytes}, inputs);

    assert(map.summary.stores_into_this == 1U);
    // The stored address really is a vtable, so the only thing deciding whether
    // this function is a constructor is the offset the store landed at.
    assert(map.summary.stores_of_a_vtable == 1U);
    // It landed at 32, so it names a member rather than the object itself.
    assert(map.functions[0].constructs_class.empty());
}

void a_constant_moved_into_an_extended_register_leaves_the_others_alone() {
    Fixture fixture;
    // The shape the image is full of, and the one that exposed the bug:
    //   48 8b 01          mov rax,[rcx]          ; the vtable
    //   41 b8 7c 02 00 00 mov r8d,0x27c          ; a third argument
    //   ff 50 08          call QWORD PTR [rax+8] ; slot 1
    // `b8 +r` carries its register in the opcode, so reading it without REX.B
    // records a write to rax and destroys the vtable fact sitting there.
    fixture.rtti.classes[0].vtables[0].slot_count = 2U;
    put_u64(fixture.bytes, 0x588U, kImageBase + 0x1000U);  // slot 1 -> function A

    const auto body = fixture.image.rva_to_file_offset(0x1040U);
    assert(body.has_value());
    const auto at = static_cast<std::size_t>(*body);
    put(fixture.bytes, at, {0x48, 0x8B, 0x01});
    put(fixture.bytes, at + 3U, {0x41, 0xB8, 0x7C, 0x02, 0x00, 0x00});
    put(fixture.bytes, at + 9U, {0xFF, 0x50, 0x08});
    put(fixture.bytes, at + 12U, {0xC3});

    PeFunctionTable table;
    table.functions.push_back(PeFunctionRange{0x1000U, 0x1040U, 0U, false, 0x1000U});
    table.functions.push_back(PeFunctionRange{0x1040U, 0x1060U, 0U, false, 0x1040U});
    const auto graph =
        CodeGraphBuilder::build(std::span<const std::byte>{fixture.bytes}, fixture.image, table);

    FunctionMapInputs inputs;
    inputs.image = &fixture.image;
    inputs.rtti = &fixture.rtti;
    inputs.graph = &graph;
    const auto map =
        FunctionMapBuilder::build(std::span<const std::byte>{fixture.bytes}, inputs);

    // rax still holds the vtable, so the dispatch is on `this` and resolves.
    assert(map.summary.dispatch_sites_on_this == 1U);
    assert(map.resolved_dispatches.size() == 1U);
    assert(map.resolved_dispatches[0].slot == 1U);
    assert(map.resolved_dispatches[0].target_rva == 0x1000U);
}

void a_constant_in_an_extended_register_is_not_a_constant_first_argument() {
    Fixture fixture;
    // `41 b9 imm32` is `mov r9d, imm`, not `mov ecx, imm`. Reading the opcode
    // without REX.B puts the constant in rcx and reports the following call as
    // taking a constant first argument, which it does not.
    //   41 b9 05 00 00 00 mov r9d,5
    //   e8 rel32          call function A
    const auto body = fixture.image.rva_to_file_offset(0x1040U);
    assert(body.has_value());
    const auto at = static_cast<std::size_t>(*body);
    put(fixture.bytes, at, {0x41, 0xB9, 0x05, 0x00, 0x00, 0x00});
    put(fixture.bytes, at + 6U, {0xE8});
    put_i32(fixture.bytes, at + 7U, -0x4B);  // 0x104B - 0x4B = 0x1000
    put(fixture.bytes, at + 11U, {0xC3});

    PeFunctionTable table;
    table.functions.push_back(PeFunctionRange{0x1000U, 0x1040U, 0U, false, 0x1000U});
    table.functions.push_back(PeFunctionRange{0x1040U, 0x1060U, 0U, false, 0x1040U});
    const auto graph =
        CodeGraphBuilder::build(std::span<const std::byte>{fixture.bytes}, fixture.image, table);

    FunctionMapInputs inputs;
    inputs.image = &fixture.image;
    inputs.rtti = &fixture.rtti;
    inputs.graph = &graph;
    const auto map =
        FunctionMapBuilder::build(std::span<const std::byte>{fixture.bytes}, inputs);

    assert(map.summary.constant_argument_calls == 0U);
    assert(map.constant_argument_callees.empty());
}

void a_base_read_at_two_sizes_is_one_array_when_the_smaller_is_the_bare_scale() {
    Fixture fixture;
    // Two reads off the same held base. The first multiplies its index by three
    // before the scale of eight, so the element is 24; the second sees no
    // multiplier, so it reads 8 straight off the encoding. A multiplier can be
    // missed but not invented, so these are one array of 24-byte elements — and
    // the second read's displacement of 16, which would be past an 8-byte
    // element, lands inside a 24-byte one.
    //   48 8d 05 ..       lea rax,[rip+..]        ; the array base at 0x2000
    //   48 8d 14 52       lea rdx,[rdx+rdx*2]     ; index times three
    //   8b 0c d0          mov ecx,[rax+rdx*8]     ; element 24, field 0
    //   8b 4c c8 10       mov ecx,[rax+rcx*8+16]  ; element 8 read, field 16
    const auto body = fixture.image.rva_to_file_offset(0x1040U);
    assert(body.has_value());
    const auto at = static_cast<std::size_t>(*body);
    put(fixture.bytes, at, {0x48, 0x8D, 0x05});
    put_i32(fixture.bytes, at + 3U, 0x0FB9);  // 0x1047 + 0xFB9 = 0x2000
    put(fixture.bytes, at + 7U, {0x48, 0x8D, 0x14, 0x52});
    put(fixture.bytes, at + 11U, {0x8B, 0x0C, 0xD0});
    put(fixture.bytes, at + 14U, {0x8B, 0x4C, 0xC8, 0x10});
    put(fixture.bytes, at + 18U, {0xC3});

    PeFunctionTable table;
    table.functions.push_back(PeFunctionRange{0x1040U, 0x1060U, 0U, false, 0x1040U});
    const auto graph =
        CodeGraphBuilder::build(std::span<const std::byte>{fixture.bytes}, fixture.image, table);

    FunctionMapInputs inputs;
    inputs.image = &fixture.image;
    inputs.graph = &graph;
    const auto map =
        FunctionMapBuilder::build(std::span<const std::byte>{fixture.bytes}, inputs);

    assert(map.summary.arrays_resolved_by_a_missed_multiplier == 1U);
    assert(map.summary.arrays_with_a_real_size_conflict == 0U);
    // Both reads are consistent with the resolved element size, and neither is
    // reported as landing outside its element.
    assert(map.summary.consistent_array_accesses == 2U);
    assert(map.summary.inconsistent_array_accesses == 0U);
    assert(map.indexed_arrays.size() == 1U);
    assert(map.indexed_arrays[0].element_bytes == 24U);
    const std::vector<std::uint32_t> expected_fields{0U, 16U};
    assert(map.indexed_arrays[0].field_offsets == expected_fields);
}

void a_base_whose_sizes_do_not_divide_is_left_out_of_the_layout() {
    Fixture fixture;
    // Elements of 24 and 16 off one base. Neither divides the other and both
    // carry a multiplier, so nothing says which reading is right. The base is
    // left out rather than given a size it may not have.
    //   48 8d 05 ..       lea rax,[rip+..]
    //   48 8d 14 52       lea rdx,[rdx+rdx*2]     ; times three, scale 8 -> 24
    //   8b 0c d0          mov ecx,[rax+rdx*8]
    //   48 8d 0c 49       lea rcx,[rcx+rcx*2]     ; times three
    //   8b 14 88          mov edx,[rax+rcx*4]     ; times three, scale 4 -> 12
    const auto body = fixture.image.rva_to_file_offset(0x1040U);
    assert(body.has_value());
    const auto at = static_cast<std::size_t>(*body);
    put(fixture.bytes, at, {0x48, 0x8D, 0x05});
    put_i32(fixture.bytes, at + 3U, 0x0FB9);
    put(fixture.bytes, at + 7U, {0x48, 0x8D, 0x14, 0x52});
    put(fixture.bytes, at + 11U, {0x8B, 0x0C, 0xD0});
    put(fixture.bytes, at + 14U, {0x48, 0x8D, 0x0C, 0x49});
    put(fixture.bytes, at + 18U, {0x8B, 0x14, 0x88});
    put(fixture.bytes, at + 21U, {0xC3});

    PeFunctionTable table;
    table.functions.push_back(PeFunctionRange{0x1040U, 0x1060U, 0U, false, 0x1040U});
    const auto graph =
        CodeGraphBuilder::build(std::span<const std::byte>{fixture.bytes}, fixture.image, table);

    FunctionMapInputs inputs;
    inputs.image = &fixture.image;
    inputs.graph = &graph;
    const auto map =
        FunctionMapBuilder::build(std::span<const std::byte>{fixture.bytes}, inputs);

    assert(map.summary.arrays_with_a_real_size_conflict == 1U);
    assert(map.summary.arrays_resolved_by_a_missed_multiplier == 0U);
    assert(map.summary.accesses_on_a_conflicted_base == 2U);
    assert(map.indexed_arrays.empty());
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
    an_address_pointing_at_a_nul_is_not_a_table_reference();
    an_indexed_array_carries_its_element_size_and_fields();
    a_field_offset_outside_the_element_is_not_an_array();
    image_base_reads_sharing_an_index_register_are_one_array();
    image_base_reads_spanning_elements_are_two_arrays_not_one();
    a_virtual_call_on_this_resolves_to_a_class_and_a_target();
    a_dispatch_on_something_other_than_this_is_not_resolved();
    a_constructor_store_names_the_class_and_its_layout();
    a_store_of_something_that_is_not_a_vtable_is_not_layout();
    a_dispatch_through_a_base_subobject_uses_that_subobject_vtable();
    a_dispatch_through_a_pointer_member_is_counted_not_resolved();
    a_pointer_stored_into_this_records_its_callee();
    referencing_a_vtable_marks_a_construction_site();
    resource_families_are_read_from_literal_text();
    literal_families_reach_the_summary_and_the_census();
    a_slot_is_classified_by_what_its_target_starts_with();
    a_base_slot_records_what_its_inheritors_put_there();
    a_base_is_compared_through_its_own_subobject_vtable();
    a_base_with_no_vtable_at_its_recorded_offset_is_not_measured();
    a_purecall_thunk_is_named_by_the_import_table_not_its_shape();
    the_dispatch_bound_reaches_what_direct_calls_cannot();
    without_a_dispatch_the_bound_is_the_direct_closure();
    a_slot_declared_pure_is_not_a_candidate_target();
    a_run_inside_a_vtable_is_not_a_table();
    the_same_addresses_outside_a_vtable_are_a_table();
    a_bound_method_reaching_into_this_floors_the_class_size();
    a_base_at_a_displacement_floors_the_class_size();
    a_base_the_class_carries_no_vtable_for_does_not_floor_it();
    a_method_bound_into_a_subobject_reaches_further_into_the_object();
    an_address_computed_off_this_is_not_a_field_access();
    an_address_passed_first_marks_a_block_the_code_operates_on();
    a_callee_serving_one_block_lends_it_the_reach();
    a_callee_with_another_caller_lends_its_reach_to_nobody();
    an_interior_address_of_this_is_still_this();
    a_store_through_an_interior_address_lands_at_its_own_offset();
    a_constant_moved_into_an_extended_register_leaves_the_others_alone();
    a_constant_in_an_extended_register_is_not_a_constant_first_argument();
    a_base_read_at_two_sizes_is_one_array_when_the_smaller_is_the_bare_scale();
    a_base_whose_sizes_do_not_divide_is_left_out_of_the_layout();
    a_missing_graph_is_refused();
    a_map_without_rtti_or_imports_still_counts_functions();
    return 0;
}
