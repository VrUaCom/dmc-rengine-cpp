#include "dmc_rengine/exe/x86_decoder.hpp"

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <initializer_list>
#include <span>
#include <vector>

namespace {

using dmc::rengine::exe::X86Flow;
using dmc::rengine::exe::X86Instruction;
using dmc::rengine::exe::X86LengthDecoder;

[[nodiscard]] std::vector<std::byte> encode(std::initializer_list<int> values) {
    std::vector<std::byte> bytes;
    bytes.reserve(values.size());
    for (const auto value : values) {
        bytes.push_back(static_cast<std::byte>(static_cast<unsigned char>(value)));
    }
    return bytes;
}

[[nodiscard]] X86Instruction decode_ok(std::initializer_list<int> values) {
    const auto bytes = encode(values);
    const auto decoded = X86LengthDecoder::decode(std::span<const std::byte>{bytes}, 0U);
    assert(decoded.has_value());
    return *decoded;
}

void refuses(std::initializer_list<int> values) {
    const auto bytes = encode(values);
    assert(!X86LengthDecoder::decode(std::span<const std::byte>{bytes}, 0U).has_value());
}

void lengths_match_the_encoding() {
    assert(decode_ok({0xC3}).length == 1U);                          // ret
    assert(decode_ok({0xCC}).length == 1U);                          // int3
    assert(decode_ok({0x90}).length == 1U);                          // nop
    assert(decode_ok({0x0F, 0x1F, 0x00}).length == 3U);              // nopl (%rax)
    assert(decode_ok({0x48, 0x83, 0xEC, 0x20}).length == 4U);        // sub rsp, 0x20
    assert(decode_ok({0xB8, 1, 0, 0, 0}).length == 5U);              // mov eax, imm32
    assert(decode_ok({0x66, 0xB8, 1, 0}).length == 4U);              // mov ax, imm16
    assert(decode_ok({0x48, 0xB8, 1, 2, 3, 4, 5, 6, 7, 8}).length == 10U);  // movabs
    assert(decode_ok({0x48, 0x89, 0x44, 0x24, 0x28}).length == 5U);  // mov [rsp+0x28], rax
    assert(decode_ok({0xF3, 0x0F, 0x10, 0x81, 0xF0, 0, 0, 0}).length == 8U);  // movss
    assert(decode_ok({0x0F, 0x29, 0x74, 0x24, 0x40}).length == 5U);  // movaps [rsp+0x40], xmm6
}

void sib_and_displacement_forms_are_sized() {
    // mod=00 rm=100 selects SIB; SIB base=101 adds a disp32.
    assert(decode_ok({0x8B, 0x04, 0x25, 0, 0, 0, 0}).length == 7U);
    // mod=01 adds a disp8.
    assert(decode_ok({0x8B, 0x41, 0x08}).length == 3U);
    // mod=10 adds a disp32.
    assert(decode_ok({0x8B, 0x81, 0, 1, 0, 0}).length == 6U);
    // mod=11 is register-direct: no displacement at all.
    assert(decode_ok({0x8B, 0xC1}).length == 2U);
}

void rip_relative_operands_are_reported() {
    // lea rcx, [rip+0x10]
    const auto lea = decode_ok({0x48, 0x8D, 0x0D, 0x10, 0, 0, 0});
    assert(lea.length == 7U);
    assert(lea.rip_relative);
    assert(lea.displacement == 0x10);
    assert(lea.flow == X86Flow::sequential);

    // A negative displacement must stay negative.
    const auto backwards = decode_ok({0x48, 0x8D, 0x0D, 0xFC, 0xFF, 0xFF, 0xFF});
    assert(backwards.rip_relative);
    assert(backwards.displacement == -4);

    // mod=01 is not RIP-relative even though the displacement exists.
    assert(!decode_ok({0x8B, 0x41, 0x08}).rip_relative);
}

void direct_branches_carry_their_displacement() {
    const auto call = decode_ok({0xE8, 0x10, 0, 0, 0});
    assert(call.length == 5U);
    assert(call.flow == X86Flow::call_direct);
    assert(call.branch_displacement == 0x10);

    const auto backwards = decode_ok({0xE8, 0x00, 0xFF, 0xFF, 0xFF});
    assert(backwards.branch_displacement == -0x100);

    const auto short_jump = decode_ok({0xEB, 0x05});
    assert(short_jump.length == 2U);
    assert(short_jump.flow == X86Flow::jump_direct);
    assert(short_jump.branch_displacement == 5);

    const auto near_jump = decode_ok({0xE9, 0x20, 0, 0, 0});
    assert(near_jump.flow == X86Flow::jump_direct);

    const auto short_branch = decode_ok({0x75, 0x10});
    assert(short_branch.length == 2U);
    assert(short_branch.flow == X86Flow::conditional_jump);
    assert(short_branch.branch_displacement == 0x10);

    const auto long_branch = decode_ok({0x0F, 0x84, 0x45, 0x01, 0, 0});
    assert(long_branch.length == 6U);
    assert(long_branch.flow == X86Flow::conditional_jump);
    assert(long_branch.branch_displacement == 0x145);
}

void indirect_transfers_are_classified_by_the_group_field() {
    // FF /2 through a RIP-relative slot: the import-call form.
    const auto call = decode_ok({0xFF, 0x15, 0x20, 0, 0, 0});
    assert(call.length == 6U);
    assert(call.flow == X86Flow::call_indirect);
    assert(call.rip_relative);
    assert(call.displacement == 0x20);

    // FF /4 through the same form: the import-thunk jump.
    const auto jump = decode_ok({0xFF, 0x25, 0x20, 0, 0, 0});
    assert(jump.flow == X86Flow::jump_indirect);
    assert(jump.rip_relative);

    // FF /0 is inc, not a transfer.
    assert(decode_ok({0xFF, 0x00}).flow == X86Flow::sequential);

    // Register-indirect call keeps its class without a displacement.
    const auto register_call = decode_ok({0xFF, 0xD0});
    assert(register_call.flow == X86Flow::call_indirect);
    assert(!register_call.rip_relative);
}

void group_immediates_depend_on_the_reg_field() {
    // F7 /0 is test with an imm32; F7 /3 is neg with no immediate.
    assert(decode_ok({0xF7, 0xC1, 1, 0, 0, 0}).length == 6U);
    assert(decode_ok({0xF7, 0xD9}).length == 2U);
    // F6 /0 takes an imm8; F6 /2 does not.
    assert(decode_ok({0xF6, 0xC1, 0x01}).length == 3U);
    assert(decode_ok({0xF6, 0xD1}).length == 2U);
}

void unmodelled_and_truncated_encodings_fail_closed() {
    refuses({0xC5, 0xF8, 0x57, 0xC0});  // 2-byte VEX
    refuses({0xC4, 0xE2, 0x79, 0x18});  // 3-byte VEX
    refuses({0x62, 0xF1, 0x7C, 0x48});  // EVEX
    refuses({0x06});                    // invalid in 64-bit mode
    refuses({0xE8, 0x01});              // truncated rel32
    refuses({0x48, 0x8D, 0x0D, 0x10});  // truncated displacement
    refuses({0x48});                    // REX with no opcode
    refuses({});                        // nothing at all

    // A prefix run longer than the architectural limit is malformed.
    refuses({0x66, 0x66, 0x66, 0x66, 0x66, 0x66, 0x66, 0x66, 0x66, 0x66, 0x66, 0x66, 0x66,
             0x66, 0x66, 0x90});
}

void decoding_respects_the_starting_offset() {
    const auto bytes = encode({0xCC, 0xCC, 0x48, 0x83, 0xEC, 0x20});
    const auto decoded = X86LengthDecoder::decode(std::span<const std::byte>{bytes}, 2U);
    assert(decoded.has_value());
    assert(decoded->length == 4U);

    assert(!X86LengthDecoder::decode(std::span<const std::byte>{bytes}, bytes.size()).has_value());
}

} // namespace

int main() {
    lengths_match_the_encoding();
    sib_and_displacement_forms_are_sized();
    rip_relative_operands_are_reported();
    direct_branches_carry_their_displacement();
    indirect_transfers_are_classified_by_the_group_field();
    group_immediates_depend_on_the_reg_field();
    unmodelled_and_truncated_encodings_fail_closed();
    decoding_respects_the_starting_offset();
    return 0;
}
