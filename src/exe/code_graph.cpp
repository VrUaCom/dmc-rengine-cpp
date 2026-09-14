#include "dmc_rengine/exe/code_graph.hpp"

#include <array>

#include "dmc_rengine/exe/x86_decoder.hpp"

#include "pe_byte_access.hpp"

#include <algorithm>
#include <functional>
#include <limits>
#include <map>
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

    // Image addresses currently held in registers, and how many instructions
    // ago each was loaded.
    //
    // This is deliberately not a dataflow analysis. The decoder models no
    // mnemonics, so which instructions write which register is not knowable
    // here; what is knowable is that an instruction naming a register in its
    // ModRM fields may well have written it. Invalidating on that, on every
    // call, and on a bounded age keeps the claim small, and the caller then
    // validates the result against a table it recovered independently. A base
    // that survives all of that and lands on a known table at that table's own
    // element size is not a coincidence.
    constexpr std::uint32_t kMaxBaseAge = 64U;
    constexpr std::uint32_t kMaxMultiplier = 1U << 16U;
    struct HeldValue final {
        std::uint32_t rva{};
        std::uint32_t age{};
        bool held{false};
        /// Constant the register's index value has been multiplied by since it
        /// was an index. A scale field encodes only 1, 2, 4 and 8, so an array
        /// of any other element size is reached by multiplying the index first
        /// — `lea reg,[a+a*2]` for three, then a scale of eight for
        /// twenty-four. Carrying the multiplier is what makes those strides
        /// visible at the point of the read.
        std::uint32_t multiplier{1U};
        /// Address this register's value was *loaded from*, when the walk saw
        /// the load. For a polymorphic object's first quadword that is the
        /// vtable pointer, which is what a virtual call dispatches through.
        std::uint32_t loaded_from{};
        bool is_load{false};
        /// The register still holds the value it had on entry to the function.
        /// For rcx under the Microsoft x64 convention that is the first
        /// argument, which for a method is `this`.
        bool is_entry_value{false};
        /// The register was loaded through the entry value: for `this` that
        /// makes it the object's vtable pointer.
        bool loaded_from_entry_value{false};
    };
    std::array<HeldValue, 16U> held{};
    std::uint32_t traced = 0U;

    const auto forget_all = [&]() {
        held.fill(HeldValue{});
    };
    // A call clobbers the volatile registers and leaves the rest. That is the
    // Microsoft x64 calling convention, which compiler-generated code follows,
    // and it is what lets a `this` pointer copied into a non-volatile register
    // at entry still be `this` after the method has called something.
    const auto forget_volatile = [&]() {
        for (const auto reg : {0U, 1U, 2U, 8U, 9U, 10U, 11U}) {
            held[reg] = HeldValue{};
        }
    };
    const auto forget = [&](std::uint8_t reg) {
        if (reg < held.size()) {
            held[reg] = HeldValue{};
        }
    };
    const auto multiplier_of = [&](std::uint8_t reg) -> std::uint32_t {
        return reg < held.size() ? held[reg].multiplier : 1U;
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

        // A trace root is reached by a branch whose origin is not known here,
        // so nothing a previous trace established still holds. The exception is
        // the function's own entry, where the calling convention says what rcx
        // holds: the first argument, which for a method is `this`.
        forget_all();
        if (rva == walk.begin_rva) {
            constexpr std::uint8_t kFirstArgument = 1U;  // rcx
            held[kFirstArgument].is_entry_value = true;
            held[kFirstArgument].age = traced;
        }

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

            const std::uint32_t next = rva + length;

            // What this instruction computes has to be read before the
            // invalidation below throws its inputs away.
            std::uint32_t produced_multiplier = 0U;
            if (!decoded->two_byte_opcode && decoded->has_modrm) {
                if (decoded->opcode == 0x8DU && decoded->memory_base != X86Instruction::kNoRegister &&
                    decoded->memory_base == decoded->memory_index && decoded->displacement == 0) {
                    // `lea reg,[a + a*k]` is a multiply by k+1.
                    produced_multiplier = multiplier_of(decoded->memory_base) *
                                          (static_cast<std::uint32_t>(decoded->memory_scale) + 1U);
                } else if ((decoded->opcode == 0x69U || decoded->opcode == 0x6BU) &&
                           decoded->immediate > 0) {
                    const auto source = decoded->modrm_mod == 3U
                                            ? multiplier_of(static_cast<std::uint8_t>(
                                                  decoded->modrm_rm))
                                            : 1U;
                    produced_multiplier =
                        source * static_cast<std::uint32_t>(decoded->immediate);
                } else if (decoded->opcode == 0xC1U && decoded->modrm_reg == 4U &&
                           decoded->modrm_mod == 3U && decoded->immediate > 0 &&
                           decoded->immediate < 16) {
                    // `shl reg, imm` is a multiply by a power of two.
                    produced_multiplier = multiplier_of(decoded->rm_operand)
                                          << static_cast<std::uint32_t>(decoded->immediate);
                } else if ((decoded->opcode == 0x01U || decoded->opcode == 0x03U) &&
                           decoded->modrm_mod == 3U &&
                           decoded->reg_operand == decoded->rm_operand) {
                    // `add reg,reg` doubles. MSVC reaches an eighty-byte element
                    // as a multiply by five, this doubling, and a scale of
                    // eight, and missing the doubling puts every field offset
                    // outside the element it computed.
                    produced_multiplier = multiplier_of(decoded->reg_operand) * 2U;
                }
            }

            // A method usually moves `this` out of rcx into a register that
            // survives calls, and every dispatch after that goes through the
            // copy. Following the copy is what makes those visible.
            std::uint8_t entry_copy_destination = X86Instruction::kNoRegister;
            if (!decoded->two_byte_opcode && decoded->modrm_mod == 3U &&
                (decoded->opcode == 0x89U || decoded->opcode == 0x8BU)) {
                const auto source =
                    decoded->opcode == 0x8BU ? decoded->rm_operand : decoded->reg_operand;
                const auto destination =
                    decoded->opcode == 0x8BU ? decoded->reg_operand : decoded->rm_operand;
                if (source < held.size() && held[source].is_entry_value) {
                    entry_copy_destination = destination;
                }
            }

            // Every register this instruction names may have been written by
            // it. Reads are invalidated too, which costs recall and buys the
            // right to make no claim the encoding does not support.
            ++traced;
            if (decoded->has_modrm) {
                forget(decoded->reg_operand);
                if (decoded->modrm_mod == 3U) {
                    forget(decoded->rm_operand);
                }
            } else {
                // push/pop, `mov imm -> reg` and `xchg` write a register named
                // in the low opcode bits and carry no ModRM to read it from.
                const auto low = static_cast<std::uint8_t>(decoded->opcode & 0x07U);
                const bool writes_encoded_register =
                    !decoded->two_byte_opcode &&
                    ((decoded->opcode >= 0x50U && decoded->opcode <= 0x5FU) ||
                     (decoded->opcode >= 0x90U && decoded->opcode <= 0x97U) ||
                     (decoded->opcode >= 0xB0U && decoded->opcode <= 0xBFU));
                if (writes_encoded_register) {
                    forget(low);
                    forget(static_cast<std::uint8_t>(low | 8U));
                }
            }

            // `mov reg, [base]` where the base is a held address: the register
            // now holds whatever that address contains.
            std::uint32_t loaded_from = 0U;
            bool loaded_from_entry_value = false;
            if (!decoded->two_byte_opcode && decoded->opcode == 0x8BU &&
                decoded->modrm_mod != 3U && !decoded->rip_relative &&
                decoded->memory_index == X86Instruction::kNoRegister &&
                decoded->memory_base < held.size() && decoded->displacement == 0) {
                const auto& base = held[decoded->memory_base];
                if (base.held && traced - base.age <= kMaxBaseAge) {
                    loaded_from = base.rva;
                }
                if (base.is_entry_value) {
                    loaded_from_entry_value = true;
                }
            }

            if (decoded->indexed_memory() && decoded->memory_base < held.size()) {
                const auto& base = held[decoded->memory_base];
                if (base.held && traced - base.age <= kMaxBaseAge) {
                    // The element size the code assumes is the scale field and
                    // whatever the index was multiplied by on the way here.
                    const auto element = static_cast<std::uint64_t>(decoded->memory_scale) *
                                         multiplier_of(decoded->memory_index);
                    walk.indexed_accesses.push_back(
                        FunctionWalk::IndexedAccess{rva, base.rva, static_cast<std::uint32_t>(element),
                                                    decoded->displacement, decoded->memory_index});
                }
            }

            if (decoded->rip_relative) {
                const auto target = static_cast<std::uint32_t>(
                    static_cast<std::int64_t>(next) + decoded->displacement);
                walk.data_references.push_back(target);

                if (decoded->rip_relative_lea()) {
                    remember_candidate(target);
                    if (decoded->reg_operand < held.size()) {
                        held[decoded->reg_operand] = HeldValue{target, traced, true, 1U};
                    }
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

            if (entry_copy_destination < held.size()) {
                held[entry_copy_destination] = HeldValue{};
                held[entry_copy_destination].is_entry_value = true;
                held[entry_copy_destination].age = traced;
            }

            if ((loaded_from != 0U || loaded_from_entry_value) &&
                decoded->reg_operand < held.size()) {
                held[decoded->reg_operand] =
                    HeldValue{0U, traced, false, 1U, loaded_from, true, false};
                // The value loaded from `this` is the vtable pointer, which is
                // what the dispatch below reads a slot out of.
                held[decoded->reg_operand].is_entry_value = false;
                if (loaded_from_entry_value) {
                    held[decoded->reg_operand].loaded_from_entry_value = true;
                }
            }

            if (produced_multiplier > 1U && produced_multiplier <= kMaxMultiplier) {
                // `add r/m, reg` and the shift group write the rm operand —
                // for a shift the reg field is the group selector and names no
                // register at all. `lea` and `imul` write the reg operand.
                const auto destination =
                    !decoded->two_byte_opcode &&
                            (decoded->opcode == 0x01U || decoded->opcode == 0xC1U)
                        ? decoded->rm_operand
                        : decoded->reg_operand;
                if (destination < held.size()) {
                    held[destination].multiplier = produced_multiplier;
                    held[destination].age = traced;
                }
            }

            const auto branch_target = [&]() {
                return static_cast<std::uint32_t>(static_cast<std::int64_t>(next) +
                                                  decoded->branch_displacement);
            };

            bool fall_through = true;
            bool forget_after_call = false;
            switch (decoded->flow) {
            case X86Flow::call_direct:
                walk.call_targets.push_back(branch_target());
                forget_volatile();
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
                // Recorded below before the clobber, since the dispatch reads
                // the register it is about to lose.
                forget_after_call = true;
                // A memory-indirect call through a register base carries the
                // dispatch offset in its displacement. Register-direct calls
                // (`call rax`) and RIP-relative ones (import slots) do not.
                if (decoded->has_modrm && !decoded->register_indirect() &&
                    !decoded->rip_relative && decoded->displacement >= 0 &&
                    decoded->displacement % 8 == 0) {
                    walk.indirect_call_displacements.push_back(
                        static_cast<std::uint32_t>(decoded->displacement));

                    std::uint32_t receiver = 0U;
                    bool through_this = false;
                    if (decoded->memory_base < held.size()) {
                        const auto& through = held[decoded->memory_base];
                        if (through.is_load && traced - through.age <= kMaxBaseAge) {
                            receiver = through.loaded_from;
                            through_this = through.loaded_from_entry_value;
                        }
                    }
                    walk.resolved_dispatch_sites.push_back(
                        FunctionWalk::DispatchSite{rva,
                                                   static_cast<std::uint32_t>(decoded->displacement),
                                                   receiver, through_this});
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

            if (forget_after_call) {
                forget_volatile();
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
