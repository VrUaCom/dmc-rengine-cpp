#include "dmc_rengine/exe/x86_decoder.hpp"

#include <array>

namespace dmc::rengine::exe {
namespace {

/// Immediate-size codes used by the opcode tables.
enum class Imm : std::uint8_t {
    none,
    byte,      // imm8
    word,      // imm16
    zdword,    // imm32, narrowed to imm16 by a 0x66 prefix
    qword,     // imm64 when REX.W is set, otherwise zdword
    enter,     // imm16 followed by imm8
    moffs,     // address-sized offset: 8 bytes in 64-bit mode
    group_f6,  // imm8 for /0 and /1 only
    group_f7,  // zdword for /0 and /1 only
    invalid,
};

struct OpcodeTraits final {
    bool modrm{false};
    Imm immediate{Imm::none};
};

// Bytes that act as prefixes rather than opcodes.
[[nodiscard]] constexpr bool is_legacy_prefix(std::uint8_t value) noexcept {
    switch (value) {
    case 0xF0U:  // lock
    case 0xF2U:  // repne
    case 0xF3U:  // rep
    case 0x2EU:
    case 0x36U:
    case 0x3EU:
    case 0x26U:
    case 0x64U:
    case 0x65U:  // segment overrides
    case 0x66U:  // operand size
    case 0x67U:  // address size
        return true;
    default:
        return false;
    }
}

/// One-byte opcode map.
[[nodiscard]] constexpr std::array<OpcodeTraits, 256> build_one_byte_map() noexcept {
    std::array<OpcodeTraits, 256> map{};

    // Arithmetic/logic blocks: r/m,r and r,r/m take ModRM; the AL/eAX forms
    // take an immediate instead. 0x06/0x07, 0x0E, 0x16/0x17, 0x1E/0x1F,
    // 0x27, 0x2F, 0x37 and 0x3F are invalid in 64-bit mode.
    for (std::uint8_t block = 0U; block < 8U; ++block) {
        const std::size_t base = static_cast<std::size_t>(block) * 8U;
        map[base + 0U] = OpcodeTraits{true, Imm::none};
        map[base + 1U] = OpcodeTraits{true, Imm::none};
        map[base + 2U] = OpcodeTraits{true, Imm::none};
        map[base + 3U] = OpcodeTraits{true, Imm::none};
        map[base + 4U] = OpcodeTraits{false, Imm::byte};
        map[base + 5U] = OpcodeTraits{false, Imm::zdword};
        map[base + 6U] = OpcodeTraits{false, Imm::invalid};
        map[base + 7U] = OpcodeTraits{false, Imm::invalid};
    }
    // 0x0F is the two-byte escape, handled before this table is consulted.
    map[0x0FU] = OpcodeTraits{false, Imm::none};

    for (std::size_t index = 0x40U; index <= 0x5FU; ++index) {
        map[index] = OpcodeTraits{false, Imm::none};  // REX (filtered earlier), push, pop
    }

    map[0x60U] = OpcodeTraits{false, Imm::invalid};
    map[0x61U] = OpcodeTraits{false, Imm::invalid};
    map[0x62U] = OpcodeTraits{false, Imm::invalid};  // EVEX: not modelled
    map[0x63U] = OpcodeTraits{true, Imm::none};      // movsxd
    map[0x68U] = OpcodeTraits{false, Imm::zdword};
    map[0x69U] = OpcodeTraits{true, Imm::zdword};
    map[0x6AU] = OpcodeTraits{false, Imm::byte};
    map[0x6BU] = OpcodeTraits{true, Imm::byte};
    for (std::size_t index = 0x6CU; index <= 0x6FU; ++index) {
        map[index] = OpcodeTraits{false, Imm::none};
    }

    for (std::size_t index = 0x70U; index <= 0x7FU; ++index) {
        map[index] = OpcodeTraits{false, Imm::byte};  // jcc rel8
    }

    map[0x80U] = OpcodeTraits{true, Imm::byte};
    map[0x81U] = OpcodeTraits{true, Imm::zdword};
    map[0x82U] = OpcodeTraits{false, Imm::invalid};
    map[0x83U] = OpcodeTraits{true, Imm::byte};
    for (std::size_t index = 0x84U; index <= 0x8FU; ++index) {
        map[index] = OpcodeTraits{true, Imm::none};  // test, xchg, mov, lea, pop r/m
    }

    for (std::size_t index = 0x90U; index <= 0x99U; ++index) {
        map[index] = OpcodeTraits{false, Imm::none};
    }
    map[0x9AU] = OpcodeTraits{false, Imm::invalid};
    for (std::size_t index = 0x9BU; index <= 0x9FU; ++index) {
        map[index] = OpcodeTraits{false, Imm::none};
    }

    for (std::size_t index = 0xA0U; index <= 0xA3U; ++index) {
        map[index] = OpcodeTraits{false, Imm::moffs};
    }
    for (std::size_t index = 0xA4U; index <= 0xA7U; ++index) {
        map[index] = OpcodeTraits{false, Imm::none};
    }
    map[0xA8U] = OpcodeTraits{false, Imm::byte};
    map[0xA9U] = OpcodeTraits{false, Imm::zdword};
    for (std::size_t index = 0xAAU; index <= 0xAFU; ++index) {
        map[index] = OpcodeTraits{false, Imm::none};
    }

    for (std::size_t index = 0xB0U; index <= 0xB7U; ++index) {
        map[index] = OpcodeTraits{false, Imm::byte};
    }
    for (std::size_t index = 0xB8U; index <= 0xBFU; ++index) {
        map[index] = OpcodeTraits{false, Imm::qword};  // mov r64, imm64 under REX.W
    }

    map[0xC0U] = OpcodeTraits{true, Imm::byte};
    map[0xC1U] = OpcodeTraits{true, Imm::byte};
    map[0xC2U] = OpcodeTraits{false, Imm::word};
    map[0xC3U] = OpcodeTraits{false, Imm::none};
    map[0xC4U] = OpcodeTraits{false, Imm::invalid};  // VEX: not modelled
    map[0xC5U] = OpcodeTraits{false, Imm::invalid};  // VEX: not modelled
    map[0xC6U] = OpcodeTraits{true, Imm::byte};
    map[0xC7U] = OpcodeTraits{true, Imm::zdword};
    map[0xC8U] = OpcodeTraits{false, Imm::enter};
    map[0xC9U] = OpcodeTraits{false, Imm::none};
    map[0xCAU] = OpcodeTraits{false, Imm::word};
    map[0xCBU] = OpcodeTraits{false, Imm::none};
    map[0xCCU] = OpcodeTraits{false, Imm::none};
    map[0xCDU] = OpcodeTraits{false, Imm::byte};
    map[0xCEU] = OpcodeTraits{false, Imm::invalid};
    map[0xCFU] = OpcodeTraits{false, Imm::none};

    for (std::size_t index = 0xD0U; index <= 0xD3U; ++index) {
        map[index] = OpcodeTraits{true, Imm::none};
    }
    map[0xD4U] = OpcodeTraits{false, Imm::invalid};
    map[0xD5U] = OpcodeTraits{false, Imm::invalid};
    map[0xD6U] = OpcodeTraits{false, Imm::invalid};
    map[0xD7U] = OpcodeTraits{false, Imm::none};
    for (std::size_t index = 0xD8U; index <= 0xDFU; ++index) {
        map[index] = OpcodeTraits{true, Imm::none};  // x87
    }

    for (std::size_t index = 0xE0U; index <= 0xE3U; ++index) {
        map[index] = OpcodeTraits{false, Imm::byte};
    }
    for (std::size_t index = 0xE4U; index <= 0xE7U; ++index) {
        map[index] = OpcodeTraits{false, Imm::byte};
    }
    map[0xE8U] = OpcodeTraits{false, Imm::zdword};
    map[0xE9U] = OpcodeTraits{false, Imm::zdword};
    map[0xEAU] = OpcodeTraits{false, Imm::invalid};
    map[0xEBU] = OpcodeTraits{false, Imm::byte};
    for (std::size_t index = 0xECU; index <= 0xEFU; ++index) {
        map[index] = OpcodeTraits{false, Imm::none};
    }

    map[0xF1U] = OpcodeTraits{false, Imm::none};
    map[0xF4U] = OpcodeTraits{false, Imm::none};
    map[0xF5U] = OpcodeTraits{false, Imm::none};
    map[0xF6U] = OpcodeTraits{true, Imm::group_f6};
    map[0xF7U] = OpcodeTraits{true, Imm::group_f7};
    for (std::size_t index = 0xF8U; index <= 0xFDU; ++index) {
        map[index] = OpcodeTraits{false, Imm::none};
    }
    map[0xFEU] = OpcodeTraits{true, Imm::none};
    map[0xFFU] = OpcodeTraits{true, Imm::none};

    return map;
}

/// Two-byte (0x0F xx) opcode map.
[[nodiscard]] constexpr std::array<OpcodeTraits, 256> build_two_byte_map() noexcept {
    std::array<OpcodeTraits, 256> map{};

    // Default for this map is ModRM with no immediate, which covers the bulk of
    // the SSE and bit-manipulation space. Exceptions follow.
    for (std::size_t index = 0; index < 256U; ++index) {
        map[index] = OpcodeTraits{true, Imm::none};
    }

    map[0x04U] = OpcodeTraits{false, Imm::invalid};
    map[0x05U] = OpcodeTraits{false, Imm::none};  // syscall
    map[0x06U] = OpcodeTraits{false, Imm::none};  // clts
    map[0x07U] = OpcodeTraits{false, Imm::none};  // sysret
    map[0x08U] = OpcodeTraits{false, Imm::none};  // invd
    map[0x09U] = OpcodeTraits{false, Imm::none};  // wbinvd
    map[0x0AU] = OpcodeTraits{false, Imm::invalid};
    map[0x0BU] = OpcodeTraits{false, Imm::none};  // ud2
    map[0x0CU] = OpcodeTraits{false, Imm::invalid};
    map[0x0EU] = OpcodeTraits{false, Imm::none};  // femms
    map[0x0FU] = OpcodeTraits{true, Imm::byte};   // 3DNow!

    for (std::size_t index = 0x24U; index <= 0x27U; ++index) {
        map[index] = OpcodeTraits{false, Imm::invalid};
    }
    for (std::size_t index = 0x30U; index <= 0x37U; ++index) {
        map[index] = OpcodeTraits{false, Imm::none};  // wrmsr, rdtsc, rdmsr, ...
    }
    // 0x38 and 0x3A are three-byte escapes, handled before this table.
    map[0x39U] = OpcodeTraits{false, Imm::invalid};
    map[0x3BU] = OpcodeTraits{false, Imm::invalid};
    for (std::size_t index = 0x3CU; index <= 0x3FU; ++index) {
        map[index] = OpcodeTraits{false, Imm::invalid};
    }

    map[0x70U] = OpcodeTraits{true, Imm::byte};
    map[0x71U] = OpcodeTraits{true, Imm::byte};
    map[0x72U] = OpcodeTraits{true, Imm::byte};
    map[0x73U] = OpcodeTraits{true, Imm::byte};
    map[0x77U] = OpcodeTraits{false, Imm::none};  // emms
    map[0x78U] = OpcodeTraits{true, Imm::none};
    map[0x7AU] = OpcodeTraits{false, Imm::invalid};
    map[0x7BU] = OpcodeTraits{false, Imm::invalid};

    for (std::size_t index = 0x80U; index <= 0x8FU; ++index) {
        map[index] = OpcodeTraits{false, Imm::zdword};  // jcc rel32
    }

    map[0xA0U] = OpcodeTraits{false, Imm::none};  // push fs
    map[0xA1U] = OpcodeTraits{false, Imm::none};  // pop fs
    map[0xA2U] = OpcodeTraits{false, Imm::none};  // cpuid
    map[0xA4U] = OpcodeTraits{true, Imm::byte};   // shld imm8
    map[0xA6U] = OpcodeTraits{false, Imm::invalid};
    map[0xA7U] = OpcodeTraits{false, Imm::invalid};
    map[0xA8U] = OpcodeTraits{false, Imm::none};  // push gs
    map[0xA9U] = OpcodeTraits{false, Imm::none};  // pop gs
    map[0xAAU] = OpcodeTraits{false, Imm::none};  // rsm
    map[0xACU] = OpcodeTraits{true, Imm::byte};   // shrd imm8

    map[0xBAU] = OpcodeTraits{true, Imm::byte};  // group 8
    map[0xC2U] = OpcodeTraits{true, Imm::byte};  // cmpps
    map[0xC4U] = OpcodeTraits{true, Imm::byte};  // pinsrw
    map[0xC5U] = OpcodeTraits{true, Imm::byte};  // pextrw
    map[0xC6U] = OpcodeTraits{true, Imm::byte};  // shufps
    for (std::size_t index = 0xC8U; index <= 0xCFU; ++index) {
        map[index] = OpcodeTraits{false, Imm::none};  // bswap
    }
    map[0xFFU] = OpcodeTraits{false, Imm::invalid};

    return map;
}

constexpr auto kOneByte = build_one_byte_map();
constexpr auto kTwoByte = build_two_byte_map();

struct Cursor final {
    std::span<const std::byte> bytes;
    std::size_t position{};

    [[nodiscard]] bool available(std::size_t count) const noexcept {
        return count <= bytes.size() - position;
    }

    [[nodiscard]] std::uint8_t peek() const noexcept {
        return std::to_integer<std::uint8_t>(bytes[position]);
    }

    [[nodiscard]] std::uint8_t take() noexcept {
        return std::to_integer<std::uint8_t>(bytes[position++]);
    }
};

[[nodiscard]] std::int32_t read_i32(std::span<const std::byte> bytes,
                                    std::size_t offset) noexcept {
    std::uint32_t value = 0U;
    for (std::size_t index = 0; index < 4U; ++index) {
        value |= static_cast<std::uint32_t>(std::to_integer<std::uint8_t>(bytes[offset + index]))
            << static_cast<unsigned>(index * 8U);
    }
    return static_cast<std::int32_t>(value);
}

[[nodiscard]] std::size_t immediate_size(Imm code, bool operand_size_override, bool rex_w,
                                         std::uint8_t modrm_reg) noexcept {
    switch (code) {
    case Imm::none: return 0U;
    case Imm::byte: return 1U;
    case Imm::word: return 2U;
    case Imm::zdword: return operand_size_override && !rex_w ? 2U : 4U;
    case Imm::qword: return rex_w ? 8U : (operand_size_override ? 2U : 4U);
    case Imm::enter: return 3U;
    case Imm::moffs: return 8U;
    case Imm::group_f6: return modrm_reg <= 1U ? 1U : 0U;
    case Imm::group_f7:
        return modrm_reg <= 1U ? (operand_size_override && !rex_w ? 2U : 4U) : 0U;
    case Imm::invalid: return 0U;
    }
    return 0U;
}

[[nodiscard]] X86Flow classify(std::uint8_t opcode, bool two_byte, bool has_modrm,
                               std::uint8_t modrm_reg) noexcept {
    if (two_byte) {
        return opcode >= 0x80U && opcode <= 0x8FU ? X86Flow::conditional_jump
                                                  : X86Flow::sequential;
    }

    switch (opcode) {
    case 0xE8U: return X86Flow::call_direct;
    case 0xE9U:
    case 0xEBU: return X86Flow::jump_direct;
    case 0xC2U:
    case 0xC3U:
    case 0xCAU:
    case 0xCBU: return X86Flow::return_;
    case 0xCCU:
    case 0xCDU:
    case 0xCEU: return X86Flow::interrupt;
    case 0xFFU:
        if (!has_modrm) {
            return X86Flow::sequential;
        }
        // FF /2 and /3 call; FF /4 and /5 jump.
        if (modrm_reg == 2U || modrm_reg == 3U) {
            return X86Flow::call_indirect;
        }
        if (modrm_reg == 4U || modrm_reg == 5U) {
            return X86Flow::jump_indirect;
        }
        return X86Flow::sequential;
    default:
        break;
    }

    if (opcode >= 0x70U && opcode <= 0x7FU) {
        return X86Flow::conditional_jump;
    }
    return X86Flow::sequential;
}

} // namespace

std::optional<X86Instruction> X86LengthDecoder::decode(std::span<const std::byte> bytes,
                                                       std::size_t offset) noexcept {
    if (offset >= bytes.size()) {
        return std::nullopt;
    }

    Cursor cursor{bytes, offset};
    bool operand_size_override = false;
    bool rex_w = false;

    // Legacy prefixes, then at most one REX byte immediately before the opcode.
    // An instruction longer than the architectural 15-byte limit is malformed.
    constexpr std::size_t max_prefix_bytes = 14U;
    std::size_t prefixes = 0U;
    while (cursor.available(1U) && is_legacy_prefix(cursor.peek())) {
        if (cursor.peek() == 0x66U) {
            operand_size_override = true;
        }
        static_cast<void>(cursor.take());
        if (++prefixes > max_prefix_bytes) {
            return std::nullopt;
        }
    }

    if (cursor.available(1U) && (cursor.peek() & 0xF0U) == 0x40U) {
        const auto rex = cursor.take();
        rex_w = (rex & 0x08U) != 0U;
    }

    if (!cursor.available(1U)) {
        return std::nullopt;
    }

    X86Instruction instruction;
    auto opcode = cursor.take();
    OpcodeTraits traits{};

    if (opcode == 0x0FU) {
        if (!cursor.available(1U)) {
            return std::nullopt;
        }
        const auto second = cursor.take();
        instruction.two_byte_opcode = true;

        if (second == 0x38U || second == 0x3AU) {
            // Three-byte maps: both take ModRM, and the 0x3A map adds an imm8.
            if (!cursor.available(1U)) {
                return std::nullopt;
            }
            static_cast<void>(cursor.take());
            traits = OpcodeTraits{true, second == 0x3AU ? Imm::byte : Imm::none};
            opcode = second;
        } else {
            traits = kTwoByte[second];
            opcode = second;
        }
    } else {
        traits = kOneByte[opcode];
    }

    if (traits.immediate == Imm::invalid) {
        return std::nullopt;
    }

    instruction.opcode = opcode;
    instruction.has_modrm = traits.modrm;

    if (traits.modrm) {
        if (!cursor.available(1U)) {
            return std::nullopt;
        }

        const auto modrm = cursor.take();
        const auto mod = static_cast<std::uint8_t>(modrm >> 6U);
        const auto rm = static_cast<std::uint8_t>(modrm & 0x07U);
        instruction.modrm_reg = static_cast<std::uint8_t>((modrm >> 3U) & 0x07U);
        instruction.modrm_mod = mod;
        instruction.modrm_rm = rm;

        std::size_t displacement_bytes = 0U;
        bool sib = false;

        if (mod != 3U) {
            if (rm == 4U) {
                sib = true;
            } else if (mod == 0U && rm == 5U) {
                // The one RIP-relative form in 64-bit addressing.
                instruction.rip_relative = true;
                displacement_bytes = 4U;
            }

            if (mod == 1U) {
                displacement_bytes = 1U;
            } else if (mod == 2U) {
                displacement_bytes = 4U;
            }
        }

        if (sib) {
            if (!cursor.available(1U)) {
                return std::nullopt;
            }
            const auto base = static_cast<std::uint8_t>(cursor.take() & 0x07U);
            if (mod == 0U && base == 5U) {
                displacement_bytes = 4U;
            }
        }

        if (displacement_bytes != 0U) {
            if (!cursor.available(displacement_bytes)) {
                return std::nullopt;
            }
            instruction.displacement_size = static_cast<std::uint8_t>(displacement_bytes);
            if (displacement_bytes == 4U) {
                instruction.displacement = read_i32(bytes, cursor.position);
            } else {
                instruction.displacement =
                    static_cast<std::int8_t>(std::to_integer<std::uint8_t>(bytes[cursor.position]));
            }
            cursor.position += displacement_bytes;
        }
    }

    const auto immediate =
        immediate_size(traits.immediate, operand_size_override, rex_w, instruction.modrm_reg);
    if (immediate != 0U) {
        if (!cursor.available(immediate)) {
            return std::nullopt;
        }

        instruction.flow = classify(opcode, instruction.two_byte_opcode, instruction.has_modrm,
                                    instruction.modrm_reg);
        switch (instruction.flow) {
        case X86Flow::call_direct:
        case X86Flow::jump_direct:
        case X86Flow::conditional_jump:
            instruction.branch_displacement =
                immediate == 1U
                    ? static_cast<std::int8_t>(std::to_integer<std::uint8_t>(bytes[cursor.position]))
                    : (immediate == 4U ? read_i32(bytes, cursor.position) : 0);
            break;
        default:
            break;
        }

        cursor.position += immediate;
    } else {
        instruction.flow = classify(opcode, instruction.two_byte_opcode, instruction.has_modrm,
                                    instruction.modrm_reg);
    }

    const auto length = cursor.position - offset;
    if (length == 0U || length > 15U) {
        return std::nullopt;
    }

    instruction.length = static_cast<std::uint8_t>(length);
    return instruction;
}

} // namespace dmc::rengine::exe
