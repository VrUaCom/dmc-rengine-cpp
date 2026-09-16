#include "dmc_rengine/exe/code_graph.hpp"

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <span>
#include <vector>

namespace {

using dmc::rengine::exe::CodeGraphBuilder;
using dmc::rengine::exe::PeFunctionRange;
using dmc::rengine::exe::PeFunctionTable;
using dmc::rengine::exe::PeImage;
using dmc::rengine::exe::PeKind;
using dmc::rengine::exe::PeMachine;
using dmc::rengine::exe::PeSection;

// ---------------------------------------------------------------------------
// Synthetic image. `.text` maps RVA 0x1000 to file 0x200; `.rdata` maps RVA
// 0x2000 to file 0x400. All code below is written by this test.
//
// Function A at 0x1000 exercises a forward branch, a local jump, a direct
// call, an indirect call through a data slot, and — crucially — unreachable
// bytes after its last `ret` that would fail to decode. A linear sweep would
// walk into them; a recursive descent must not.
//
// Function B is split: a primary range at 0x1040 and a chained continuation at
// 0x1050, which must be folded into one function.
// ---------------------------------------------------------------------------

constexpr std::uint64_t kImageBase = 0x140000000ULL;

void put(std::vector<std::byte>& bytes, std::size_t offset,
         std::initializer_list<int> values) {
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

[[nodiscard]] PeImage make_image() {
    PeImage image;
    image.kind = PeKind::pe32_plus;
    image.machine = PeMachine::amd64;
    image.image_base = kImageBase;
    image.entry_point_rva = 0x1000U;
    image.size_of_image = 0x3000U;
    image.size_of_headers = 0x200U;
    image.section_count = 2U;
    image.sections.push_back(PeSection{".text", 0x200U, 0x1000U, 0x200U, 0x200U, 0x60000020U});
    image.sections.push_back(PeSection{".rdata", 0x200U, 0x2000U, 0x200U, 0x400U, 0x40000040U});
    return image;
}

[[nodiscard]] std::vector<std::byte> make_code() {
    std::vector<std::byte> bytes(0x600U, std::byte{0});

    // --- function A: 0x1000 .. 0x1030 -----------------------------------
    put(bytes, 0x200U, {0x48, 0x83, 0xEC, 0x20});             // 0x1000 sub rsp, 0x20
    put(bytes, 0x204U, {0x48, 0x8D, 0x0D});                   // 0x1004 lea rcx, [rip+...]
    put_i32(bytes, 0x207U, 0x0FF5);                           //        -> 0x2000
    put(bytes, 0x20BU, {0xE8});                               // 0x100B call
    put_i32(bytes, 0x20CU, 0x30);                             //        -> 0x1040
    put(bytes, 0x210U, {0x75, 0x08});                         // 0x1010 jne -> 0x101A
    put(bytes, 0x212U, {0xFF, 0x15});                         // 0x1012 call [rip+...]
    put_i32(bytes, 0x214U, 0x10E8);                           //        -> slot 0x2100
    put(bytes, 0x218U, {0xEB, 0x06});                         // 0x1018 jmp -> 0x1020
    put(bytes, 0x21AU, {0x48, 0x83, 0xC4, 0x20});             // 0x101A add rsp, 0x20
    put(bytes, 0x21EU, {0xC3});                               // 0x101E ret
    put(bytes, 0x21FU, {0xCC});                               // 0x101F padding
    put(bytes, 0x220U, {0xE8});                               // 0x1020 call
    put_i32(bytes, 0x221U, 0x5B);                             //        -> 0x1080 (a thunk)
    put(bytes, 0x225U, {0xC3});                               // 0x1025 ret

    // Unreachable tail: a VEX encoding the decoder refuses. Reaching it would
    // both add instructions and mark the walk incomplete.
    for (std::size_t offset = 0x226U; offset < 0x230U; offset += 4U) {
        put(bytes, offset, {0xC5, 0xF8, 0x57, 0xC0});
    }

    // --- function B: primary 0x1040, chained continuation 0x1050 ---------
    put(bytes, 0x240U, {0xE9});                               // 0x1040 jmp
    put_i32(bytes, 0x241U, 0x0B);                             //        -> 0x1050
    put(bytes, 0x245U, {0xCC, 0xCC, 0xCC});                   //        padding
    put(bytes, 0x250U, {0x48, 0x8D, 0x0D});                   // 0x1050 lea rcx, [rip+...]
    put_i32(bytes, 0x253U, 0x0FB9);                           //        -> 0x2010
    put(bytes, 0x257U, {0xC3});                               // 0x1057 ret

    // --- an import thunk outside the inventory ---------------------------
    put(bytes, 0x280U, {0xFF, 0x25});                         // 0x1080 jmp [rip+...]
    put_i32(bytes, 0x282U, 0x1082);                           //        -> slot 0x2108

    // --- function C at 0x10A0: a compiled switch -------------------------
    // mov eax, [rax*4 + 0x20C0] loads a table entry; the base register is
    // assumed to hold the image base, so entries are plain RVAs.
    put(bytes, 0x2A0U, {0x8B, 0x04, 0x85});                   // 0x10A0
    put_i32(bytes, 0x2A3U, 0x20C0);                           //        table at 0x20C0
    put(bytes, 0x2A7U, {0xFF, 0xE0});                         // 0x10A7 jmp rax
    put(bytes, 0x2B0U, {0xFF, 0x50, 0x48});                   // 0x10B0 call [rax+0x48]
    put(bytes, 0x2B3U, {0xC3});                               // 0x10B3 case 0 returns
    put(bytes, 0x2B8U, {0xC3});                               // 0x10B8 case 1

    // The table itself, in .rdata at 0x20C0 (file 0x4C0). A third entry would
    // read as zero, which lands outside the function and ends enumeration.
    put_i32(bytes, 0x4C0U, 0x10B0);
    put_i32(bytes, 0x4C4U, 0x10B8);

    return bytes;
}

[[nodiscard]] PeFunctionTable make_function_table() {
    PeFunctionTable table;
    table.functions.push_back(PeFunctionRange{0x1000U, 0x1030U, 0U, false, 0x1000U});
    table.functions.push_back(PeFunctionRange{0x1040U, 0x1050U, 0U, false, 0x1040U});
    // The continuation names its primary, which is what folds them together.
    table.functions.push_back(PeFunctionRange{0x1050U, 0x1060U, 0U, true, 0x1040U});
    table.functions.push_back(PeFunctionRange{0x10A0U, 0x10C0U, 0U, false, 0x10A0U});
    table.primary_functions = 3U;
    table.chained_ranges = 1U;
    return table;
}

void chained_ranges_fold_into_one_function() {
    const auto image = make_image();
    const auto bytes = make_code();
    const auto table = make_function_table();

    const auto graph =
        CodeGraphBuilder::build(std::span<const std::byte>{bytes}, image, table);

    assert(graph.exception_directory_entries == 4U);
    assert(graph.functions.size() == 3U);
    assert(graph.functions_walked == 3U);

    const auto& split = graph.functions[1];
    assert(split.begin_rva == 0x1040U);
    assert(split.ranges.size() == 2U);
    assert(split.ranges[0].begin_rva == 0x1040U);
    assert(split.ranges[1].begin_rva == 0x1050U);
    // Size is the sum of the ranges, not the span between them.
    assert(split.size() == 0x20U);
}

void the_descent_never_walks_unreachable_bytes() {
    const auto image = make_image();
    const auto bytes = make_code();
    const auto graph = CodeGraphBuilder::build(std::span<const std::byte>{bytes}, image,
                                              make_function_table());

    const auto& first = graph.functions[0];

    // Ten reachable instructions; the refusable tail is never touched, so the
    // walk stays complete.
    assert(first.instruction_count == 10U);
    assert(first.complete);
    assert(graph.functions_complete == 3U);
    // 4 + 7 + 5 + 2 + 6 + 2 + 4 + 1 + 5 + 1 bytes. The int3 padding between the
    // two returns is unreachable and stays undecoded.
    assert(first.decoded_bytes == 37U);
    assert(first.coverage() < 1.0);
}

void control_flow_and_references_are_extracted() {
    const auto image = make_image();
    const auto bytes = make_code();
    const auto graph = CodeGraphBuilder::build(std::span<const std::byte>{bytes}, image,
                                              make_function_table());

    const auto& first = graph.functions[0];
    assert(first.call_targets.size() == 2U);
    assert(first.call_targets[0] == 0x1040U);
    assert(first.call_targets[1] == 0x1080U);
    assert(first.data_references.size() == 2U);
    assert(first.data_references[0] == 0x2000U);
    assert(first.data_references[1] == 0x2100U);
    assert(first.indirect_calls == 1U);
    // Function A's indirect call goes through a RIP-relative slot, which has no
    // dispatch offset to record.
    assert(first.indirect_call_displacements.empty());
    assert(first.returns == 2U);
    assert(first.external_jump_targets.empty());

    const auto& second = graph.functions[1];
    assert(second.instruction_count == 3U);
    assert(second.data_references.size() == 1U);
    assert(second.data_references[0] == 0x2010U);
    assert(second.returns == 1U);
    // The jump into its own continuation is local, not an external edge.
    assert(second.external_jump_targets.empty());

    assert(graph.total_instructions == 18U);
    assert(graph.call_edges == 2U);
    // The switch lookup's displacement is a table candidate, not a RIP-relative
    // operand, so it adds no data-reference edge.
    assert(graph.data_reference_edges == 3U);
}

void a_switch_table_is_recovered_and_walked() {
    const auto image = make_image();
    const auto bytes = make_code();
    const auto graph = CodeGraphBuilder::build(std::span<const std::byte>{bytes}, image,
                                              make_function_table());

    const auto& dispatch = graph.functions[2];
    assert(dispatch.begin_rva == 0x10A0U);
    assert(dispatch.switch_tables == 1U);
    assert(dispatch.unresolved_indirect_jumps == 0U);
    assert(dispatch.switch_targets.size() == 2U);
    assert(dispatch.switch_targets[0] == 0x10B0U);
    assert(dispatch.switch_targets[1] == 0x10B8U);

    // Both case blocks were reached only through the table, so their `ret`
    // instructions are part of the walk.
    assert(dispatch.instruction_count == 5U);
    assert(dispatch.returns == 2U);
    assert(dispatch.indirect_jumps == 1U);
    assert(dispatch.complete);

    // The dispatch offset of `call [rax+0x48]` is slot nine of a 64-bit vtable.
    assert(dispatch.indirect_calls == 1U);
    assert(dispatch.indirect_call_displacements.size() == 1U);
    assert(dispatch.indirect_call_displacements[0] == 0x48U);

    assert(graph.switch_tables_recovered == 1U);
    assert(graph.switch_targets_recovered == 2U);
    assert(graph.unresolved_indirect_jumps == 0U);
}

void an_indirect_jump_without_a_valid_table_stays_unresolved() {
    const auto image = make_image();
    auto bytes = make_code();

    // Point the lookup at a table whose entries land outside the function.
    put_i32(bytes, 0x4C0U, 0x9000);
    put_i32(bytes, 0x4C4U, 0x9008);

    const auto graph = CodeGraphBuilder::build(std::span<const std::byte>{bytes}, image,
                                              make_function_table());

    const auto& dispatch = graph.functions[2];
    assert(dispatch.switch_tables == 0U);
    assert(dispatch.switch_targets.empty());
    assert(dispatch.unresolved_indirect_jumps == 1U);

    // The case blocks are now unreachable, so the walk is shorter and the
    // dispatch call inside one of them is never seen.
    assert(dispatch.instruction_count == 2U);
    assert(dispatch.indirect_call_displacements.empty());
    assert(graph.unresolved_indirect_jumps == 1U);
}

void a_memory_indirect_jump_is_never_treated_as_a_switch() {
    const auto image = make_image();
    const auto bytes = make_code();

    // Function at 0x1080 is `jmp [rip+slot]`: a thunk, not a dispatch.
    PeFunctionTable table;
    table.functions.push_back(PeFunctionRange{0x1080U, 0x1090U, 0U, false, 0x1080U});

    const auto graph =
        CodeGraphBuilder::build(std::span<const std::byte>{bytes}, image, table);
    assert(graph.functions[0].indirect_jumps == 1U);
    assert(graph.functions[0].switch_tables == 0U);
    assert(graph.functions[0].unresolved_indirect_jumps == 1U);
}

void a_range_outside_every_section_is_reported_not_walked() {
    const auto image = make_image();
    const auto bytes = make_code();

    PeFunctionTable table;
    table.functions.push_back(PeFunctionRange{0x900000U, 0x900010U, 0U, false, 0x900000U});

    const auto graph =
        CodeGraphBuilder::build(std::span<const std::byte>{bytes}, image, table);
    assert(graph.functions.size() == 1U);
    assert(!graph.functions[0].complete);
    assert(graph.functions_walked == 0U);
    assert(!graph.warnings.empty());
}

void an_undecodable_body_marks_the_walk_incomplete() {
    const auto image = make_image();
    auto bytes = make_code();

    // Replace the reachable body of function B with a VEX encoding.
    put(bytes, 0x240U, {0xC5, 0xF8, 0x57, 0xC0});

    PeFunctionTable table;
    table.functions.push_back(PeFunctionRange{0x1040U, 0x1050U, 0U, false, 0x1040U});

    const auto graph =
        CodeGraphBuilder::build(std::span<const std::byte>{bytes}, image, table);
    assert(graph.functions.size() == 1U);
    assert(!graph.functions[0].complete);
    assert(graph.functions[0].instruction_count == 0U);
    assert(graph.functions_complete == 0U);
}

void a_base_held_in_a_register_makes_an_indexed_access_readable() {
    const auto image = make_image();
    std::vector<std::byte> bytes(0x600U, std::byte{0xCC});

    // .text 0x1100:
    //   48 8d 1d <rel>   lea rbx,[rip+rel]        ; -> .rdata 0x2000
    //   8b 04 8b         mov eax,[rbx+rcx*4]
    //   c3               ret
    put(bytes, 0x300U, {0x48, 0x8D, 0x1D});
    // The operand is relative to the end of the instruction at 0x1107.
    put_i32(bytes, 0x303U, static_cast<std::int32_t>(0x2000) - static_cast<std::int32_t>(0x1107));
    put(bytes, 0x307U, {0x8B, 0x04, 0x8B});
    put(bytes, 0x30AU, {0xC3});

    PeFunctionTable table;
    table.functions.push_back(PeFunctionRange{0x1100U, 0x1110U, 0U, false, 0x1100U});

    const auto graph =
        CodeGraphBuilder::build(std::span<const std::byte>{bytes}, image, table);
    assert(graph.functions.size() == 1U);

    const auto& accesses = graph.functions[0].indexed_accesses;
    assert(accesses.size() == 1U);
    assert(accesses[0].site_rva == 0x1107U);
    assert(accesses[0].base_rva == 0x2000U);
    assert(accesses[0].element_bytes == 4U);
    assert(accesses[0].displacement == 0);
}

void a_call_forfeits_a_volatile_base_and_spares_a_saved_one() {
    const auto image = make_image();

    // The same load-and-use pair with a `call` in between, once through rcx and
    // once through rbx. The Microsoft x64 convention makes rcx volatile and rbx
    // saved, so the call destroys one base and not the other.
    const auto build_with = [&](std::initializer_list<int> lea, std::initializer_list<int> use) {
        std::vector<std::byte> bytes(0x600U, std::byte{0xCC});
        put(bytes, 0x300U, lea);
        put_i32(bytes, 0x303U,
                static_cast<std::int32_t>(0x2000) - static_cast<std::int32_t>(0x1107));
        put(bytes, 0x307U, {0xE8});
        put_i32(bytes, 0x308U, 0);  // call to the instruction after it
        put(bytes, 0x30CU, use);
        put(bytes, 0x30FU, {0xC3});

        PeFunctionTable table;
        table.functions.push_back(PeFunctionRange{0x1100U, 0x1110U, 0U, false, 0x1100U});
        return CodeGraphBuilder::build(std::span<const std::byte>{bytes}, image, table);
    };

    // lea rcx,[rip+rel] ; call ; mov eax,[rcx+rax*4]
    const auto volatile_base = build_with({0x48, 0x8D, 0x0D}, {0x8B, 0x04, 0x81});
    assert(volatile_base.functions[0].indexed_accesses.empty());

    // lea rbx,[rip+rel] ; call ; mov eax,[rbx+rax*4]
    const auto saved_base = build_with({0x48, 0x8D, 0x1D}, {0x8B, 0x04, 0x83});
    assert(saved_base.functions[0].indexed_accesses.size() == 1U);
    assert(saved_base.functions[0].indexed_accesses[0].base_rva == 0x2000U);
}

void overwriting_the_base_register_forfeits_it() {
    const auto image = make_image();
    std::vector<std::byte> bytes(0x600U, std::byte{0xCC});

    //   lea rbx,[rip+rel]
    //   48 89 c3          mov rbx,rax       ; names rbx in a ModRM rm field
    //   8b 04 8b          mov eax,[rbx+rcx*4]
    put(bytes, 0x300U, {0x48, 0x8D, 0x1D});
    put_i32(bytes, 0x303U, static_cast<std::int32_t>(0x2000) - static_cast<std::int32_t>(0x1107));
    put(bytes, 0x307U, {0x48, 0x89, 0xC3});
    put(bytes, 0x30AU, {0x8B, 0x04, 0x8B});
    put(bytes, 0x30DU, {0xC3});

    PeFunctionTable table;
    table.functions.push_back(PeFunctionRange{0x1100U, 0x1110U, 0U, false, 0x1100U});

    const auto graph =
        CodeGraphBuilder::build(std::span<const std::byte>{bytes}, image, table);
    assert(graph.functions.size() == 1U);
    assert(graph.functions[0].indexed_accesses.empty());
}

void a_self_add_doubles_the_index_multiplier() {
    const auto image = make_image();
    std::vector<std::byte> bytes(0x600U, std::byte{0xCC});

    // The form MSVC uses to reach an eighty-byte element:
    //   48 8d 1d <rel>   lea rbx,[rip+rel]     ; -> .rdata 0x2000
    //   48 8d 04 80      lea rax,[rax+rax*4]   ; index * 5
    //   48 03 c0         add rax,rax           ; index * 10
    //   8b 0c c3         mov ecx,[rbx+rax*8]   ; base + index * 80
    put(bytes, 0x300U, {0x48, 0x8D, 0x1D});
    put_i32(bytes, 0x303U, static_cast<std::int32_t>(0x2000) - static_cast<std::int32_t>(0x1107));
    put(bytes, 0x307U, {0x48, 0x8D, 0x04, 0x80});
    put(bytes, 0x30BU, {0x48, 0x03, 0xC0});
    put(bytes, 0x30EU, {0x8B, 0x0C, 0xC3});
    put(bytes, 0x311U, {0xC3});

    PeFunctionTable table;
    table.functions.push_back(PeFunctionRange{0x1100U, 0x1120U, 0U, false, 0x1100U});

    const auto graph =
        CodeGraphBuilder::build(std::span<const std::byte>{bytes}, image, table);
    assert(graph.functions.size() == 1U);

    const auto& accesses = graph.functions[0].indexed_accesses;
    assert(accesses.size() == 1U);
    assert(accesses[0].base_rva == 0x2000U);
    // Five, doubled, scaled by eight. Missing the doubling reports forty and
    // puts every field offset outside the element.
    assert(accesses[0].element_bytes == 80U);
}

void a_shift_multiplies_the_index() {
    const auto image = make_image();
    std::vector<std::byte> bytes(0x600U, std::byte{0xCC});

    //   lea rbx,[rip+rel]
    //   48 c1 e0 02       shl rax,2         ; index * 4
    //   8b 0c c3          mov ecx,[rbx+rax*8]
    put(bytes, 0x300U, {0x48, 0x8D, 0x1D});
    put_i32(bytes, 0x303U, static_cast<std::int32_t>(0x2000) - static_cast<std::int32_t>(0x1107));
    put(bytes, 0x307U, {0x48, 0xC1, 0xE0, 0x02});
    put(bytes, 0x30BU, {0x8B, 0x0C, 0xC3});
    put(bytes, 0x30EU, {0xC3});

    PeFunctionTable table;
    table.functions.push_back(PeFunctionRange{0x1100U, 0x1120U, 0U, false, 0x1100U});

    const auto graph =
        CodeGraphBuilder::build(std::span<const std::byte>{bytes}, image, table);
    assert(graph.functions[0].indexed_accesses.size() == 1U);
    assert(graph.functions[0].indexed_accesses[0].element_bytes == 32U);
}

void a_fact_that_differs_between_paths_does_not_survive_the_join() {
    const auto image = make_image();
    std::vector<std::byte> bytes(0x600U, std::byte{0xCC});

    //   1100: 85 c0             test eax,eax
    //   1102: 74 07             je 0x110b
    //   1104: 48 8d 1d <rel>    lea rbx,[rip -> 0x2000]
    //   110b: 8b 04 83          mov eax,[rbx+rax*4]     <- the join
    //   110e: c3                ret
    //
    // One path gives rbx an address and the other leaves it unknown, so at the
    // join nothing is known about it and the read is not an array access. A
    // walk that took the traces in order would have believed whichever ran
    // last.
    put(bytes, 0x300U, {0x85, 0xC0});
    put(bytes, 0x302U, {0x74, 0x07});
    put(bytes, 0x304U, {0x48, 0x8D, 0x1D});
    put_i32(bytes, 0x307U, static_cast<std::int32_t>(0x2000) - static_cast<std::int32_t>(0x110B));
    put(bytes, 0x30BU, {0x8B, 0x04, 0x83});
    put(bytes, 0x30EU, {0xC3});

    PeFunctionTable table;
    table.functions.push_back(PeFunctionRange{0x1100U, 0x1120U, 0U, false, 0x1100U});

    const auto graph =
        CodeGraphBuilder::build(std::span<const std::byte>{bytes}, image, table);
    assert(graph.functions.size() == 1U);
    assert(graph.functions[0].indexed_accesses.empty());
}

void a_fact_both_paths_agree_on_survives_the_join() {
    const auto image = make_image();
    std::vector<std::byte> bytes(0x600U, std::byte{0xCC});

    //   1100: 85 c0             test eax,eax
    //   1102: 74 07             je 0x110b
    //   1104: 48 8d 1d <rel>    lea rbx,[rip -> 0x2000]
    //   110b: 48 8d 1d <rel>    lea rbx,[rip -> 0x2000]   (the other path)
    //   1112: 8b 04 83          mov eax,[rbx+rax*4]
    //
    // Laid out so both paths reach the read with rbx holding the same address.
    put(bytes, 0x300U, {0x85, 0xC0});
    put(bytes, 0x302U, {0x74, 0x07});
    put(bytes, 0x304U, {0x48, 0x8D, 0x1D});
    put_i32(bytes, 0x307U, static_cast<std::int32_t>(0x2000) - static_cast<std::int32_t>(0x110B));
    put(bytes, 0x30BU, {0x48, 0x8D, 0x1D});
    put_i32(bytes, 0x30EU, static_cast<std::int32_t>(0x2000) - static_cast<std::int32_t>(0x1112));
    put(bytes, 0x312U, {0x8B, 0x04, 0x83});
    put(bytes, 0x315U, {0xC3});

    PeFunctionTable table;
    table.functions.push_back(PeFunctionRange{0x1100U, 0x1120U, 0U, false, 0x1100U});

    const auto graph =
        CodeGraphBuilder::build(std::span<const std::byte>{bytes}, image, table);
    assert(graph.functions[0].indexed_accesses.size() == 1U);
    assert(graph.functions[0].indexed_accesses[0].base_rva == 0x2000U);
}

void a_register_copy_carries_whatever_the_source_held() {
    const auto image = make_image();
    std::vector<std::byte> bytes(0x600U, std::byte{0xCC});

    // An address taken into one register, copied into another, and indexed
    // through the copy. The rule used to carry only the first argument, so the
    // base was lost here for no reason the encoding gives.
    //   48 8d 1d <rel>    lea rbx,[rip -> 0x2000]
    //   48 8b c3          mov rax,rbx
    //   8b 0c 90          mov ecx,[rax+rdx*4]
    put(bytes, 0x300U, {0x48, 0x8D, 0x1D});
    put_i32(bytes, 0x303U, static_cast<std::int32_t>(0x2000) - static_cast<std::int32_t>(0x1107));
    put(bytes, 0x307U, {0x48, 0x8B, 0xC3});
    put(bytes, 0x30AU, {0x8B, 0x0C, 0x90});
    put(bytes, 0x30DU, {0xC3});

    PeFunctionTable table;
    table.functions.push_back(PeFunctionRange{0x1100U, 0x1120U, 0U, false, 0x1100U});

    const auto graph =
        CodeGraphBuilder::build(std::span<const std::byte>{bytes}, image, table);
    assert(graph.functions[0].indexed_accesses.size() == 1U);
    assert(graph.functions[0].indexed_accesses[0].base_rva == 0x2000U);
    assert(graph.functions[0].indexed_accesses[0].element_bytes == 4U);
}

} // namespace

int main() {
    chained_ranges_fold_into_one_function();
    the_descent_never_walks_unreachable_bytes();
    control_flow_and_references_are_extracted();
    a_switch_table_is_recovered_and_walked();
    an_indirect_jump_without_a_valid_table_stays_unresolved();
    a_memory_indirect_jump_is_never_treated_as_a_switch();
    a_range_outside_every_section_is_reported_not_walked();
    an_undecodable_body_marks_the_walk_incomplete();
    a_base_held_in_a_register_makes_an_indexed_access_readable();
    a_call_forfeits_a_volatile_base_and_spares_a_saved_one();
    overwriting_the_base_register_forfeits_it();
    a_self_add_doubles_the_index_multiplier();
    a_shift_multiplies_the_index();
    a_fact_that_differs_between_paths_does_not_survive_the_join();
    a_fact_both_paths_agree_on_survives_the_join();
    a_register_copy_carries_whatever_the_source_held();
    return 0;
}
