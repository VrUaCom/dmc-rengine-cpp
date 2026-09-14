#pragma once

#include <cstddef>
#include <cstdint>
#include <optional>
#include <span>
#include <string_view>

namespace dmc::rengine::exe {

/// Control-flow role of a decoded instruction.
enum class X86Flow {
    sequential,
    call_direct,
    call_indirect,
    jump_direct,
    jump_indirect,
    conditional_jump,
    return_,
    interrupt,
};

[[nodiscard]] constexpr std::string_view to_string(X86Flow flow) noexcept {
    switch (flow) {
    case X86Flow::sequential: return "sequential";
    case X86Flow::call_direct: return "call-direct";
    case X86Flow::call_indirect: return "call-indirect";
    case X86Flow::jump_direct: return "jump-direct";
    case X86Flow::jump_indirect: return "jump-indirect";
    case X86Flow::conditional_jump: return "conditional-jump";
    case X86Flow::return_: return "return";
    case X86Flow::interrupt: return "interrupt";
    }
    return "sequential";
}

/// One decoded instruction: its extent, its control-flow role, and the operand
/// references a code walk needs.
///
/// This is a *length* decoder, not a disassembler. It answers where the
/// instruction ends, whether it transfers control and where to, and whether it
/// addresses memory relative to RIP. It deliberately does not model mnemonics
/// or operand semantics, because the analyses built on it do not need them and
/// a smaller decoder is a decoder that can be trusted.
struct X86Instruction final {
    std::uint8_t length{};
    X86Flow flow{X86Flow::sequential};

    bool has_modrm{false};
    /// True when the memory operand is `[rip + displacement]`.
    bool rip_relative{false};
    std::int32_t displacement{};
    /// Width of the encoded displacement in bytes: 0, 1 or 4. A four-byte
    /// displacement on a non-RIP operand can be an absolute table address when
    /// the base register holds the image base, which is how MSVC x64 reaches a
    /// switch table.
    std::uint8_t displacement_size{};

    /// Signed displacement of a direct branch, valid when `flow` is a direct
    /// call, direct jump or conditional jump. The target is
    /// `address + length + branch_displacement`.
    std::int32_t branch_displacement{};

    std::uint8_t opcode{};
    bool two_byte_opcode{false};
    /// ModRM reg field, which selects the operation for group opcodes such as
    /// `FF /2` (indirect call) and `FF /4` (indirect jump).
    std::uint8_t modrm_reg{};
    /// ModRM mod field. Value 3 is register-direct, which distinguishes
    /// `jmp rax` from `jmp [rax]`.
    std::uint8_t modrm_mod{};
    /// ModRM rm field, without the REX.B extension bit.
    std::uint8_t modrm_rm{};

    /// Sentinel for a register role the encoding leaves empty.
    static constexpr std::uint8_t kNoRegister = 16U;

    /// Register the ModRM reg field names, 0-15, with REX.R applied. For the
    /// forms that matter here — `lea reg, [mem]` and `mov reg, [mem]` — this is
    /// the destination.
    std::uint8_t reg_operand{kNoRegister};
    /// Base register of the memory operand, 0-15, or `kNoRegister` when the
    /// encoding has none: a RIP-relative operand, a SIB with no base, or a
    /// register-direct instruction.
    std::uint8_t memory_base{kNoRegister};
    /// Index register of the memory operand, or `kNoRegister` when there is no
    /// SIB or the SIB names no index.
    std::uint8_t memory_index{kNoRegister};
    /// Scale applied to the index register: 1, 2, 4 or 8. An indexed read of a
    /// table of fixed-width elements carries the element size here.
    std::uint8_t memory_scale{1U};

    [[nodiscard]] bool transfers_control() const noexcept {
        return flow != X86Flow::sequential;
    }

    /// True for `lea reg, [rip + displacement]`, the form MSVC uses to load a
    /// switch table base and to take the address of a literal.
    [[nodiscard]] bool rip_relative_lea() const noexcept {
        return !two_byte_opcode && opcode == 0x8DU && rip_relative;
    }

    /// True for a memory operand of the form `[base + index*scale + disp]`,
    /// which is how code reads an element out of a table whose base it holds.
    [[nodiscard]] bool indexed_memory() const noexcept {
        return memory_base != kNoRegister && memory_index != kNoRegister;
    }

    /// True for a register-direct indirect transfer such as `jmp rax`, which is
    /// how a compiled switch dispatches.
    [[nodiscard]] bool register_indirect() const noexcept {
        return has_modrm && modrm_mod == 3U;
    }
};

/// Bounded x86-64 instruction length decoder.
///
/// Fail-closed: anything it does not model — VEX and EVEX encodings, opcodes
/// invalid in 64-bit mode, a truncated instruction — returns no value rather
/// than a guessed length. A caller walking code must stop at that point
/// instead of resynchronising on a byte boundary it cannot justify.
class X86LengthDecoder final {
public:
    /// Decodes the instruction beginning at `offset`.
    [[nodiscard]] static std::optional<X86Instruction> decode(std::span<const std::byte> bytes,
                                                              std::size_t offset) noexcept;
};

} // namespace dmc::rengine::exe
