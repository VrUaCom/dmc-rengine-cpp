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

    return bytes;
}

[[nodiscard]] PeFunctionTable make_function_table() {
    PeFunctionTable table;
    table.functions.push_back(PeFunctionRange{0x1000U, 0x1030U, 0U, false, 0x1000U});
    table.functions.push_back(PeFunctionRange{0x1040U, 0x1050U, 0U, false, 0x1040U});
    // The continuation names its primary, which is what folds them together.
    table.functions.push_back(PeFunctionRange{0x1050U, 0x1060U, 0U, true, 0x1040U});
    table.primary_functions = 2U;
    table.chained_ranges = 1U;
    return table;
}

void chained_ranges_fold_into_one_function() {
    const auto image = make_image();
    const auto bytes = make_code();
    const auto table = make_function_table();

    const auto graph =
        CodeGraphBuilder::build(std::span<const std::byte>{bytes}, image, table);

    assert(graph.exception_directory_entries == 3U);
    assert(graph.functions.size() == 2U);
    assert(graph.functions_walked == 2U);

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
    assert(graph.functions_complete == 2U);
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
    assert(first.returns == 2U);
    assert(first.external_jump_targets.empty());

    const auto& second = graph.functions[1];
    assert(second.instruction_count == 3U);
    assert(second.data_references.size() == 1U);
    assert(second.data_references[0] == 0x2010U);
    assert(second.returns == 1U);
    // The jump into its own continuation is local, not an external edge.
    assert(second.external_jump_targets.empty());

    assert(graph.total_instructions == 13U);
    assert(graph.call_edges == 2U);
    assert(graph.data_reference_edges == 3U);
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

} // namespace

int main() {
    chained_ranges_fold_into_one_function();
    the_descent_never_walks_unreachable_bytes();
    control_flow_and_references_are_extracted();
    a_range_outside_every_section_is_reported_not_walked();
    an_undecodable_body_marks_the_walk_incomplete();
    return 0;
}
