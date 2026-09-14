#include "dmc_rengine/exe/code_graph.hpp"

#include "dmc_rengine/exe/x86_decoder.hpp"

#include <algorithm>
#include <map>
#include <cstdint>
#include <vector>

namespace dmc::rengine::exe {
namespace {

constexpr std::size_t kMaxWorklist = 1U << 16U;

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
                walk.data_references.push_back(static_cast<std::uint32_t>(
                    static_cast<std::int64_t>(next) + decoded->displacement));
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
            case X86Flow::jump_indirect:
                ++walk.indirect_jumps;
                fall_through = false;
                break;
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
    }

    return graph;
}

} // namespace dmc::rengine::exe
