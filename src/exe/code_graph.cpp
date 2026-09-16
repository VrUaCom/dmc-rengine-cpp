#include "dmc_rengine/exe/code_graph.hpp"

#include <array>

#include "dmc_rengine/exe/x86_decoder.hpp"

#include "pe_byte_access.hpp"

#include <algorithm>
#include <functional>
#include <limits>
#include <map>
#include <optional>
#include <cstdint>
#include <vector>

namespace dmc::rengine::exe {
namespace {

constexpr std::size_t kMaxWorklist = 1U << 16U;
constexpr std::size_t kMaxSwitchEntries = 4096U;
constexpr std::size_t kMaxTableCandidates = 64U;
constexpr std::size_t kMinSwitchEntries = 2U;

[[nodiscard]] const PeSection* section_for_rva(const PeImage& image, std::uint32_t rva) noexcept {
    for (const auto& section : image.sections) {
        if (section.contains_rva(rva)) {
            return &section;
        }
    }
    return nullptr;
}

void sort_unique(std::vector<std::uint32_t>& values) {
    std::sort(values.begin(), values.end());
    values.erase(std::unique(values.begin(), values.end()), values.end());
}

/// Enumerates a compiled switch table behind a register-indirect jump.
///
/// MSVC x64 emits `lea base, [rip+table]`, indexes a dword array, adds the base
/// back and jumps. So a table entry is an offset from the table base. Some
/// builds store image-base-relative offsets instead, so both readings are tried
/// and the one that validates further wins.
///
/// Validation is what makes an unanchored guess safe: an entry counts only when
/// it lands inside the same function's own ranges. A literal address or an
/// unrelated `lea` produces zero valid entries and is rejected.
struct SwitchTableReading final {
    std::vector<std::uint32_t> targets;
    bool image_base_relative{false};
};

[[nodiscard]] SwitchTableReading read_switch_table(
    std::span<const std::byte> bytes, const PeImage& image, std::uint32_t table_rva,
    const std::function<bool(std::uint32_t)>& owns) {
    const auto enumerate = [&](bool image_base_relative) {
        std::vector<std::uint32_t> targets;
        for (std::size_t index = 0; index < kMaxSwitchEntries; ++index) {
            const auto entry_rva = static_cast<std::uint64_t>(table_rva) + index * 4U;
            if (entry_rva > std::numeric_limits<std::uint32_t>::max()) {
                break;
            }
            const auto offset = image.rva_to_file_offset(static_cast<std::uint32_t>(entry_rva));
            if (!offset.has_value()) {
                break;
            }
            const auto entry = detail::read_u32(bytes, static_cast<std::size_t>(*offset));
            if (!entry.has_value()) {
                break;
            }

            const auto target = image_base_relative
                                    ? *entry
                                    : static_cast<std::uint32_t>(table_rva + *entry);
            if (!owns(target)) {
                break;
            }
            targets.push_back(target);
        }
        return targets;
    };

    SwitchTableReading relative{enumerate(false), false};
    SwitchTableReading absolute{enumerate(true), true};
    return absolute.targets.size() > relative.targets.size() ? absolute : relative;
}

/// Recursive-descent walk of one function across all of its ranges.
/// One register's value, as far as the analysis can say.
///
/// The meet is equality: a fact survives a join only when every path into the
/// block agrees on it. That is what makes a fact at a dispatch site a statement
/// about the programme rather than about the order the walk happened to take.
struct RegisterFact final {
    enum class Kind : std::uint8_t {
        unknown,
        /// An address a RIP-relative `lea` put here.
        image_address,
        /// The value the register held on entry to the function. For rcx under
        /// the Microsoft x64 convention that is the first argument, and in a
        /// method that is `this`.
        entry_value,
        /// Loaded through the entry value at some offset. At offset zero that
        /// is the object's own vtable pointer; at the offset of an embedded
        /// polymorphic member it is that member's, because a member's vtable
        /// pointer sits at its own offset zero.
        loaded_through_entry,
        /// Loaded from a known address.
        loaded_from,
        /// The value a direct call returned. Under the Microsoft x64
        /// convention that is rax, and for a constructor it is the object.
        call_result,
        /// Memory a call has since been run on as its first argument. Where
        /// that call is a constructor the register holds an object of the class
        /// it builds, which is what types a field the pointer is stored in.
        constructed_object,
    };

    Kind kind{Kind::unknown};
    std::uint32_t rva{};
    /// How many loads deep the value is. One is the quadword at `this + k`,
    /// which for an embedded subobject is its vtable pointer. Two is the
    /// quadword that points to, which for a pointer member is the pointee's
    /// vtable pointer.
    std::uint8_t depth{};
    /// What an index in this register has been multiplied by. A scale field
    /// encodes only 1, 2, 4 and 8, so any other element size arrives here.
    std::uint32_t multiplier{1U};

    friend bool operator==(const RegisterFact&, const RegisterFact&) = default;
};

using RegisterState = std::array<RegisterFact, 16U>;

[[nodiscard]] RegisterState meet(const RegisterState& left, const RegisterState& right) {
    RegisterState result;
    for (std::size_t reg = 0; reg < result.size(); ++reg) {
        result[reg] = left[reg] == right[reg] ? left[reg] : RegisterFact{};
    }
    return result;
}

/// Applies one instruction to a register state, optionally recording what the
/// instruction says about the data it touches.
void apply(const X86Instruction& decoded, std::uint32_t rva, RegisterState& state,
           FunctionWalk* emit) {
    constexpr std::uint32_t kMaxMultiplier = 1U << 16U;
    const auto multiplier_of = [&](std::uint8_t reg) -> std::uint32_t {
        return reg < state.size() ? state[reg].multiplier : 1U;
    };

    // Everything this instruction computes is read from the state before the
    // invalidation below throws its inputs away.
    std::uint32_t produced_multiplier = 0U;
    std::uint8_t multiplier_destination = X86Instruction::kNoRegister;
    if (!decoded.two_byte_opcode && decoded.has_modrm) {
        if (decoded.opcode == 0x8DU && decoded.memory_base != X86Instruction::kNoRegister &&
            decoded.memory_base == decoded.memory_index && decoded.displacement == 0) {
            produced_multiplier =
                multiplier_of(decoded.memory_base) * (decoded.memory_scale + 1U);
            multiplier_destination = decoded.reg_operand;
        } else if ((decoded.opcode == 0x69U || decoded.opcode == 0x6BU) && decoded.immediate > 0) {
            const auto source =
                decoded.modrm_mod == 3U ? multiplier_of(decoded.rm_operand) : 1U;
            produced_multiplier = source * static_cast<std::uint32_t>(decoded.immediate);
            multiplier_destination = decoded.reg_operand;
        } else if (decoded.opcode == 0xC1U && decoded.modrm_reg == 4U && decoded.modrm_mod == 3U &&
                   decoded.immediate > 0 && decoded.immediate < 16) {
            produced_multiplier = multiplier_of(decoded.rm_operand)
                                  << static_cast<std::uint32_t>(decoded.immediate);
            multiplier_destination = decoded.rm_operand;
        } else if ((decoded.opcode == 0x01U || decoded.opcode == 0x03U) &&
                   decoded.modrm_mod == 3U && decoded.reg_operand == decoded.rm_operand) {
            produced_multiplier = multiplier_of(decoded.reg_operand) * 2U;
            multiplier_destination = decoded.reg_operand;
        }
    }

    // A register-to-register move carries whatever the source held. A method
    // moving `this` out of rcx into a register that survives calls is the
    // commonest case, and an allocation moved out of rax before the
    // constructor runs on it is the one that types a pointer field.
    std::uint8_t copy_destination = X86Instruction::kNoRegister;
    RegisterFact copied;
    if (!decoded.two_byte_opcode && decoded.modrm_mod == 3U &&
        (decoded.opcode == 0x89U || decoded.opcode == 0x8BU)) {
        const auto source = decoded.opcode == 0x8BU ? decoded.rm_operand : decoded.reg_operand;
        const auto destination = decoded.opcode == 0x8BU ? decoded.reg_operand : decoded.rm_operand;
        if (source < state.size() && state[source].kind != RegisterFact::Kind::unknown) {
            copy_destination = destination;
            copied = state[source];
        }
    }

    // `mov reg, [base]` where the base is known: the register now holds
    // whatever that address contains.
    RegisterFact loaded;
    std::uint8_t load_destination = X86Instruction::kNoRegister;
    if (!decoded.two_byte_opcode && decoded.opcode == 0x8BU && decoded.modrm_mod != 3U &&
        !decoded.rip_relative && decoded.memory_index == X86Instruction::kNoRegister &&
        decoded.memory_base < state.size() && decoded.displacement >= 0) {
        const auto& base = state[decoded.memory_base];
        if (base.kind == RegisterFact::Kind::image_address && decoded.displacement == 0) {
            loaded = RegisterFact{.kind = RegisterFact::Kind::loaded_from, .rva = base.rva};
            load_destination = decoded.reg_operand;
        } else if (base.kind == RegisterFact::Kind::entry_value) {
            loaded = RegisterFact{.kind = RegisterFact::Kind::loaded_through_entry,
                                  .rva = static_cast<std::uint32_t>(decoded.displacement),
                                  .depth = 1U};
            load_destination = decoded.reg_operand;
        } else if (base.kind == RegisterFact::Kind::loaded_through_entry && base.depth == 1U &&
                   decoded.displacement == 0) {
            // A second load off a value read out of the object: the field held
            // a pointer, and this is the pointee's vtable pointer.
            loaded = RegisterFact{.kind = RegisterFact::Kind::loaded_through_entry,
                                  .rva = base.rva,
                                  .depth = 2U};
            load_destination = decoded.reg_operand;
        }
    }

    // ----- what the instruction says about data ----------------------------
    if (emit != nullptr) {
        if (decoded.indexed_memory() && decoded.memory_base < state.size() &&
            state[decoded.memory_base].kind == RegisterFact::Kind::image_address) {
            const auto element = static_cast<std::uint64_t>(decoded.memory_scale) *
                                 multiplier_of(decoded.memory_index);
            emit->indexed_accesses.push_back(
                FunctionWalk::IndexedAccess{rva, state[decoded.memory_base].rva,
                                            static_cast<std::uint32_t>(element),
                                            decoded.displacement, decoded.memory_index});
        }

        // `mov [this + offset], rax` with rax holding what a call returned. In a
        // constructor that is a member being built and stored, so the field
        // holds a pointer to whatever the callee constructs.
        if (!decoded.two_byte_opcode && decoded.opcode == 0x89U && decoded.modrm_mod != 3U &&
            !decoded.rip_relative && decoded.memory_index == X86Instruction::kNoRegister &&
            decoded.memory_base < state.size() && decoded.displacement >= 0 &&
            decoded.reg_operand < state.size() &&
            state[decoded.memory_base].kind == RegisterFact::Kind::entry_value &&
            (state[decoded.reg_operand].kind == RegisterFact::Kind::call_result ||
             state[decoded.reg_operand].kind == RegisterFact::Kind::constructed_object)) {
            emit->pointer_stores_into_this.push_back(
                FunctionWalk::PointerStore{rva, static_cast<std::uint32_t>(decoded.displacement),
                                           state[decoded.reg_operand].rva});
        }

        // `mov [this + offset], reg` with the register holding an address the
        // code took with a `lea`. In a constructor that is a vtable going into
        // its slot, which is the class's layout written out.
        if (!decoded.two_byte_opcode && decoded.opcode == 0x89U && decoded.modrm_mod != 3U &&
            !decoded.rip_relative && decoded.memory_index == X86Instruction::kNoRegister &&
            decoded.memory_base < state.size() && decoded.displacement >= 0 &&
            decoded.reg_operand < state.size() &&
            state[decoded.memory_base].kind == RegisterFact::Kind::entry_value &&
            state[decoded.reg_operand].kind == RegisterFact::Kind::image_address) {
            emit->stores_into_this.push_back(
                FunctionWalk::VtableStore{rva, static_cast<std::uint32_t>(decoded.displacement),
                                          state[decoded.reg_operand].rva});
        }

        if (decoded.flow == X86Flow::call_indirect && decoded.has_modrm &&
            !decoded.register_indirect() && !decoded.rip_relative && decoded.displacement >= 0 &&
            decoded.displacement % 8 == 0) {
            std::uint32_t receiver = 0U;
            bool through_this = false;
            std::uint32_t field_offset = 0U;
            std::uint8_t receiver_depth = 0U;
            if (decoded.memory_base < state.size()) {
                const auto& through = state[decoded.memory_base];
                if (through.kind == RegisterFact::Kind::loaded_from) {
                    receiver = through.rva;
                } else if (through.kind == RegisterFact::Kind::loaded_through_entry) {
                    through_this = true;
                    field_offset = through.rva;
                    receiver_depth = through.depth;
                }
            }
            emit->resolved_dispatch_sites.push_back(
                FunctionWalk::DispatchSite{rva, static_cast<std::uint32_t>(decoded.displacement),
                                           receiver, through_this, field_offset, receiver_depth});
        }
    }

    // ----- invalidation ----------------------------------------------------
    const auto forget = [&](std::uint8_t reg) {
        if (reg < state.size()) {
            state[reg] = RegisterFact{};
        }
    };

    if (decoded.has_modrm) {
        forget(decoded.reg_operand);
        if (decoded.modrm_mod == 3U) {
            forget(decoded.rm_operand);
        }
    } else {
        const auto low = static_cast<std::uint8_t>(decoded.opcode & 0x07U);
        const bool writes_encoded_register =
            !decoded.two_byte_opcode &&
            ((decoded.opcode >= 0x50U && decoded.opcode <= 0x5FU) ||
             (decoded.opcode >= 0x90U && decoded.opcode <= 0x97U) ||
             (decoded.opcode >= 0xB0U && decoded.opcode <= 0xBFU));
        if (writes_encoded_register) {
            forget(low);
            forget(static_cast<std::uint8_t>(low | 8U));
        }
    }

    // A call clobbers the volatile registers and leaves the rest. That is the
    // Microsoft x64 convention, which compiler-generated code follows, and it
    // is what lets a `this` pointer copied into a saved register at entry still
    // be `this` after the method has called something.
    if (decoded.flow == X86Flow::call_direct || decoded.flow == X86Flow::call_indirect) {
        constexpr std::uint8_t kFirstArgument = 1U;  // rcx
        constexpr std::uint8_t kReturnRegister = 0U;  // rax
        const auto receiver = state[kFirstArgument];

        for (const auto reg : {0U, 1U, 2U, 8U, 9U, 10U, 11U}) {
            state[reg] = RegisterFact{};
        }

        if (decoded.flow == X86Flow::call_direct) {
            const auto callee = static_cast<std::uint32_t>(
                static_cast<std::int64_t>(rva) + decoded.length + decoded.branch_displacement);

            // Whatever was passed as the first argument has now had this call
            // run on it. If the callee turns out to be a constructor, every
            // register still holding that same value holds an object of its
            // class — which is how a freshly allocated pointer acquires a type
            // between the allocation and the store that files it away.
            if (receiver.kind == RegisterFact::Kind::call_result ||
                receiver.kind == RegisterFact::Kind::constructed_object) {
                for (auto& fact : state) {
                    if (fact == receiver) {
                        fact = RegisterFact{.kind = RegisterFact::Kind::constructed_object,
                                            .rva = callee};
                    }
                }
            }

            state[kReturnRegister] =
                RegisterFact{.kind = RegisterFact::Kind::call_result, .rva = callee};
        }
    }

    // ----- what the instruction produces -----------------------------------
    if (decoded.rip_relative_lea() && decoded.reg_operand < state.size()) {
        const auto target = static_cast<std::uint32_t>(static_cast<std::int64_t>(rva) +
                                                       decoded.length + decoded.displacement);
        state[decoded.reg_operand] =
            RegisterFact{.kind = RegisterFact::Kind::image_address, .rva = target};
    }
    if (copy_destination < state.size()) {
        state[copy_destination] = copied;
    }
    if (load_destination < state.size()) {
        state[load_destination] = loaded;
    }
    if (produced_multiplier > 1U && produced_multiplier <= kMaxMultiplier &&
        multiplier_destination < state.size()) {
        state[multiplier_destination].multiplier = produced_multiplier;
    }
}

void walk_function(std::span<const std::byte> bytes, const PeImage& image, FunctionWalk& walk,
                   std::vector<std::uint8_t>& visited) {
    if (walk.ranges.empty()) {
        walk.complete = false;
        return;
    }

    std::uint32_t lowest = walk.ranges.front().begin_rva;
    std::uint32_t highest = walk.ranges.front().end_rva;
    for (const auto& range : walk.ranges) {
        lowest = std::min(lowest, range.begin_rva);
        highest = std::max(highest, range.end_rva);
    }

    // Ranges of one function can sit far apart. The span between them is only
    // used as a visited-bitmap index, so cap it rather than allocate wildly.
    constexpr std::uint32_t max_span = 8U * 1024U * 1024U;
    if (highest <= lowest || highest - lowest > max_span) {
        walk.complete = false;
        return;
    }

    visited.assign(static_cast<std::size_t>(highest - lowest), 0U);

    const auto owns = [&](std::uint32_t rva) {
        for (const auto& range : walk.ranges) {
            if (rva >= range.begin_rva && rva < range.end_rva) {
                return true;
            }
        }
        return false;
    };

    // Candidate switch-table bases: every RIP-relative `lea` target seen so
    // far, most recent last. A table base is loaded before the dispatch, often
    // in an earlier basic block, so the candidates outlive a single trace.
    std::vector<std::uint32_t> table_candidates;
    const auto remember_candidate = [&](std::uint32_t candidate) {
        if (!table_candidates.empty() && table_candidates.back() == candidate) {
            return;
        }
        if (table_candidates.size() >= kMaxTableCandidates) {
            table_candidates.erase(table_candidates.begin());
        }
        table_candidates.push_back(candidate);
    };

    std::vector<std::uint32_t> worklist;
    worklist.reserve(walk.ranges.size() + 1U);
    // Every range entry is a trace root: a continuation range is reached by a
    // branch the walk may not be able to follow, so seeding it directly is what
    // keeps split functions fully covered.
    worklist.push_back(walk.begin_rva);
    for (const auto& range : walk.ranges) {
        if (range.begin_rva != walk.begin_rva) {
            worklist.push_back(range.begin_rva);
        }
    }

    while (!worklist.empty()) {
        std::uint32_t rva = worklist.back();
        worklist.pop_back();

        while (owns(rva)) {
            const auto index = static_cast<std::size_t>(rva - lowest);
            if (visited[index] != 0U) {
                break;
            }

            const auto mapped = image.rva_to_file_offset(rva);
            if (!mapped.has_value()) {
                walk.complete = false;
                break;
            }

            const auto decoded = X86LengthDecoder::decode(bytes, static_cast<std::size_t>(*mapped));
            if (!decoded.has_value()) {
                // Fail closed: stop this trace rather than resynchronising on a
                // byte boundary nothing justifies.
                walk.complete = false;
                break;
            }

            const auto length = static_cast<std::uint32_t>(decoded->length);
            if (!owns(rva + length - 1U)) {
                walk.complete = false;
                break;
            }

            for (std::uint32_t byte = 0U; byte < length; ++byte) {
                visited[index + byte] = 1U;
            }
            ++walk.instruction_count;
            walk.decoded_bytes += length;
            walk.instruction_starts.push_back(rva);

            const std::uint32_t next = rva + length;

            if (decoded->rip_relative) {
                const auto target = static_cast<std::uint32_t>(
                    static_cast<std::int64_t>(next) + decoded->displacement);
                walk.data_references.push_back(target);

                if (decoded->rip_relative_lea()) {
                    remember_candidate(target);
                }
            } else if (decoded->displacement_size == 4U && decoded->displacement > 0) {
                // A non-RIP disp32 can be an absolute table RVA: MSVC keeps the
                // image base in a register and indexes `base + disp32`. Only
                // validation can tell that apart from a large struct offset, and
                // validation is what decides.
                const auto candidate = static_cast<std::uint32_t>(decoded->displacement);
                if (candidate >= image.size_of_headers && candidate < image.size_of_image) {
                    remember_candidate(candidate);
                }
            }

            const auto branch_target = [&]() {
                return static_cast<std::uint32_t>(static_cast<std::int64_t>(next) +
                                                  decoded->branch_displacement);
            };

            bool fall_through = true;
            switch (decoded->flow) {
            case X86Flow::call_direct:
                walk.call_targets.push_back(branch_target());
                break;
            case X86Flow::conditional_jump: {
                const auto target = branch_target();
                if (owns(target)) {
                    if (worklist.size() < kMaxWorklist) {
                        worklist.push_back(target);
                    }
                } else {
                    walk.external_jump_targets.push_back(target);
                }
                break;
            }
            case X86Flow::jump_direct: {
                const auto target = branch_target();
                if (owns(target)) {
                    if (worklist.size() < kMaxWorklist) {
                        worklist.push_back(target);
                    }
                } else {
                    // Leaving the function's own ranges is a tail call or a
                    // shared epilogue, not a local branch.
                    walk.external_jump_targets.push_back(target);
                }
                fall_through = false;
                break;
            }
            case X86Flow::jump_indirect: {
                ++walk.indirect_jumps;
                fall_through = false;

                // A register-direct `jmp reg` is the compiled switch form. A
                // memory-indirect jump is a thunk or a virtual dispatch and has
                // no table to read.
                if (!decoded->register_indirect()) {
                    ++walk.unresolved_indirect_jumps;
                    break;
                }

                bool resolved = false;
                for (auto candidate = table_candidates.rbegin();
                     candidate != table_candidates.rend(); ++candidate) {
                    const auto reading = read_switch_table(bytes, image, *candidate, owns);
                    if (reading.targets.size() < kMinSwitchEntries) {
                        continue;
                    }

                    ++walk.switch_tables;
                    for (const auto target : reading.targets) {
                        walk.switch_targets.push_back(target);
                        if (worklist.size() < kMaxWorklist) {
                            worklist.push_back(target);
                        }
                    }
                    resolved = true;
                    break;
                }

                if (!resolved) {
                    ++walk.unresolved_indirect_jumps;
                }
                break;
            }
            case X86Flow::call_indirect:
                ++walk.indirect_calls;
                // A memory-indirect call through a register base carries the
                // dispatch offset in its displacement. Register-direct calls
                // (`call rax`) and RIP-relative ones (import slots) do not.
                if (decoded->has_modrm && !decoded->register_indirect() &&
                    !decoded->rip_relative && decoded->displacement >= 0 &&
                    decoded->displacement % 8 == 0) {
                    walk.indirect_call_displacements.push_back(
                        static_cast<std::uint32_t>(decoded->displacement));
                }
                break;
            case X86Flow::return_:
                ++walk.returns;
                fall_through = false;
                break;
            case X86Flow::interrupt:
                // MSVC pads with int3 and ends unreachable paths with it.
                fall_through = false;
                break;
            case X86Flow::sequential:
                break;
            }

            if (!fall_through) {
                break;
            }
            rva = next;
        }
    }

    sort_unique(walk.call_targets);
    sort_unique(walk.data_references);
    sort_unique(walk.external_jump_targets);
    sort_unique(walk.switch_targets);
    sort_unique(walk.indirect_call_displacements);

    // A trace can be re-entered from several roots, so the same site can be
    // recorded more than once.
    std::sort(walk.indexed_accesses.begin(), walk.indexed_accesses.end(),
              [](const FunctionWalk::IndexedAccess& left,
                 const FunctionWalk::IndexedAccess& right) {
                  if (left.site_rva != right.site_rva) {
                      return left.site_rva < right.site_rva;
                  }
                  return left.base_rva < right.base_rva;
              });
    walk.indexed_accesses.erase(
        std::unique(walk.indexed_accesses.begin(), walk.indexed_accesses.end()),
        walk.indexed_accesses.end());
}

/// Runs the register analysis over the instructions the walk already decoded.
///
/// Separate from the walk on purpose. The walk's job is to find the code, and
/// it is the thing every other measurement rests on; this reads that code again
/// and says what the registers hold at each point, which needs a state per
/// instruction and a meet at every join. Doing it in the walk would have meant
/// facts that depend on which order the traces happened to run.
void analyse_registers(std::span<const std::byte> bytes, const PeImage& image,
                       FunctionWalk& walk) {
    auto& starts = walk.instruction_starts;
    std::sort(starts.begin(), starts.end());
    starts.erase(std::unique(starts.begin(), starts.end()), starts.end());
    if (starts.empty()) {
        return;
    }

    // A function large enough to make the fixpoint expensive is left without
    // register facts rather than allowed to dominate the run.
    constexpr std::size_t kMaxInstructions = 16U * 1024U;
    if (starts.size() > kMaxInstructions) {
        return;
    }

    const auto index_of = [&](std::uint32_t rva) -> std::size_t {
        const auto found = std::lower_bound(starts.begin(), starts.end(), rva);
        if (found == starts.end() || *found != rva) {
            return starts.size();
        }
        return static_cast<std::size_t>(found - starts.begin());
    };

    std::vector<std::optional<X86Instruction>> decoded(starts.size());
    std::vector<std::array<std::size_t, 2U>> successors(
        starts.size(), {starts.size(), starts.size()});

    for (std::size_t position = 0; position < starts.size(); ++position) {
        const auto offset = image.rva_to_file_offset(starts[position]);
        if (!offset.has_value()) {
            continue;
        }
        decoded[position] = X86LengthDecoder::decode(bytes, static_cast<std::size_t>(*offset));
        if (!decoded[position].has_value()) {
            continue;
        }

        const auto& instruction = *decoded[position];
        const auto next = starts[position] + instruction.length;
        const bool falls_through = instruction.flow != X86Flow::jump_direct &&
                                   instruction.flow != X86Flow::jump_indirect &&
                                   instruction.flow != X86Flow::return_ &&
                                   instruction.flow != X86Flow::interrupt;
        if (falls_through) {
            successors[position][0] = index_of(next);
        }
        if (instruction.flow == X86Flow::jump_direct ||
            instruction.flow == X86Flow::conditional_jump) {
            successors[position][1] = index_of(static_cast<std::uint32_t>(
                static_cast<std::int64_t>(next) + instruction.branch_displacement));
        }
    }

    std::vector<RegisterState> incoming(starts.size());
    std::vector<std::uint8_t> reached(starts.size(), 0U);

    // The function's own entry is where the calling convention speaks: rcx holds
    // the first argument. Every other root is reached by a branch this analysis
    // cannot see, so it starts knowing nothing.
    std::vector<std::size_t> pending;
    const auto entry = index_of(walk.begin_rva);
    if (entry < starts.size()) {
        constexpr std::uint8_t kFirstArgument = 1U;  // rcx
        incoming[entry][kFirstArgument] = RegisterFact{.kind = RegisterFact::Kind::entry_value};
        reached[entry] = 1U;
        pending.push_back(entry);
    }
    for (const auto& range : walk.ranges) {
        const auto root = index_of(range.begin_rva);
        if (root < starts.size() && reached[root] == 0U) {
            reached[root] = 1U;
            pending.push_back(root);
        }
    }

    // Facts only ever weaken, so the iteration terminates; the cap is there for
    // a malformed graph rather than for a slow one.
    const auto budget = starts.size() * 64U + 1024U;
    std::size_t applications = 0U;
    while (!pending.empty() && applications < budget) {
        const auto position = pending.back();
        pending.pop_back();
        ++applications;

        if (!decoded[position].has_value()) {
            continue;
        }
        auto state = incoming[position];
        apply(*decoded[position], starts[position], state, nullptr);

        for (const auto successor : successors[position]) {
            if (successor >= starts.size()) {
                continue;
            }
            if (reached[successor] == 0U) {
                incoming[successor] = state;
                reached[successor] = 1U;
                pending.push_back(successor);
                continue;
            }
            const auto merged = meet(incoming[successor], state);
            if (merged != incoming[successor]) {
                incoming[successor] = merged;
                pending.push_back(successor);
            }
        }
    }

    // An instruction the analysis never reached is read with an empty state: it
    // still counts towards the dispatch census, and claims nothing.
    for (std::size_t position = 0; position < starts.size(); ++position) {
        if (!decoded[position].has_value()) {
            continue;
        }
        auto state = incoming[position];
        apply(*decoded[position], starts[position], state, &walk);
    }

    std::sort(walk.indexed_accesses.begin(), walk.indexed_accesses.end(),
              [](const FunctionWalk::IndexedAccess& left,
                 const FunctionWalk::IndexedAccess& right) {
                  return left.site_rva < right.site_rva;
              });
    std::sort(walk.pointer_stores_into_this.begin(), walk.pointer_stores_into_this.end(),
              [](const FunctionWalk::PointerStore& left, const FunctionWalk::PointerStore& right) {
                  return left.site_rva < right.site_rva;
              });
    std::sort(walk.stores_into_this.begin(), walk.stores_into_this.end(),
              [](const FunctionWalk::VtableStore& left, const FunctionWalk::VtableStore& right) {
                  return left.site_rva < right.site_rva;
              });
    std::sort(walk.resolved_dispatch_sites.begin(), walk.resolved_dispatch_sites.end(),
              [](const FunctionWalk::DispatchSite& left, const FunctionWalk::DispatchSite& right) {
                  return left.site_rva < right.site_rva;
              });
}

} // namespace

CodeGraph CodeGraphBuilder::build(std::span<const std::byte> bytes, const PeImage& image,
                                  const PeFunctionTable& functions) {
    CodeGraph graph;
    graph.exception_directory_entries = functions.functions.size();

    // Fold continuation ranges into the function that owns them. Without this
    // the graph would treat each fragment of a split function as a function of
    // its own, and every edge leaving a fragment would look external.
    std::map<std::uint32_t, std::size_t> by_primary;
    for (const auto& range : functions.functions) {
        const auto primary =
            range.primary_begin_rva != 0U ? range.primary_begin_rva : range.begin_rva;

        auto existing = by_primary.find(primary);
        if (existing == by_primary.end()) {
            FunctionWalk walk;
            walk.begin_rva = primary;
            walk.end_rva = range.end_rva;
            existing = by_primary.emplace(primary, graph.functions.size()).first;
            graph.functions.push_back(std::move(walk));
        }

        auto& walk = graph.functions[existing->second];
        walk.ranges.push_back(FunctionCodeRange{range.begin_rva, range.end_rva});
        walk.end_rva = std::max(walk.end_rva, range.end_rva);
    }

    std::vector<std::uint8_t> visited;
    for (auto& walk : graph.functions) {
        std::sort(walk.ranges.begin(), walk.ranges.end(),
                  [](const FunctionCodeRange& left, const FunctionCodeRange& right) {
                      return left.begin_rva < right.begin_rva;
                  });

        if (section_for_rva(image, walk.begin_rva) == nullptr) {
            walk.complete = false;
            graph.warnings.emplace_back("Function entry lies outside every section.");
            continue;
        }

        walk_function(bytes, image, walk, visited);
        analyse_registers(bytes, image, walk);

        ++graph.functions_walked;
        if (walk.complete) {
            ++graph.functions_complete;
        }
        graph.total_instructions += walk.instruction_count;
        graph.total_decoded_bytes += walk.decoded_bytes;
        graph.call_edges += walk.call_targets.size();
        graph.data_reference_edges += walk.data_references.size();
        graph.indirect_call_sites += walk.indirect_calls;
        graph.switch_tables_recovered += walk.switch_tables;
        graph.switch_targets_recovered += walk.switch_targets.size();
        graph.unresolved_indirect_jumps += walk.unresolved_indirect_jumps;
    }

    return graph;
}

} // namespace dmc::rengine::exe
