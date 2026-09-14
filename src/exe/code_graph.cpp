#include "dmc_rengine/exe/code_graph.hpp"

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
        graph.switch_tables_recovered += walk.switch_tables;
        graph.switch_targets_recovered += walk.switch_targets.size();
        graph.unresolved_indirect_jumps += walk.unresolved_indirect_jumps;
    }

    return graph;
}

} // namespace dmc::rengine::exe
