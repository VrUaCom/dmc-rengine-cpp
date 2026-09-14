#pragma once

#include "dmc_rengine/exe/pe_directories.hpp"
#include "dmc_rengine/exe/pe_image.hpp"

#include <cstddef>
#include <cstdint>
#include <span>
#include <string>
#include <vector>

namespace dmc::rengine::exe {

/// Result of walking one function's reachable instructions.
///
/// The walk is a recursive descent from the function entry, following
/// fall-through and direct branches. It is deliberately not a linear sweep of
/// the function extent: MSVC embeds switch jump tables inside function ranges,
/// and a linear sweep decodes those tables as instructions. A descent reaches
/// only bytes the function can actually execute.
struct FunctionCodeRange final {
    std::uint32_t begin_rva{};
    std::uint32_t end_rva{};

    friend bool operator==(const FunctionCodeRange&, const FunctionCodeRange&) = default;
};

struct FunctionWalk final {
    /// Function entry: the start of its primary unwind range.
    std::uint32_t begin_rva{};
    /// Highest end across the function's ranges. The function need not occupy
    /// this span contiguously.
    std::uint32_t end_rva{};
    /// Every range the unwind data attributes to this function, sorted. A
    /// function split by the linker owns several.
    std::vector<FunctionCodeRange> ranges;

    std::uint32_t instruction_count{};
    std::uint32_t decoded_bytes{};

    /// False when the walk hit a byte sequence the decoder does not model. The
    /// facts already gathered stay valid; the function is simply not fully
    /// covered.
    bool complete{true};

    /// Sorted, unique targets of direct `call` instructions.
    std::vector<std::uint32_t> call_targets;
    /// Sorted, unique targets of RIP-relative memory operands: string
    /// literals, import-address-table slots, statics and jump tables alike.
    std::vector<std::uint32_t> data_references;
    /// Direct branch targets that leave the function's own range, which is how
    /// MSVC emits tail calls and shared epilogues.
    std::vector<std::uint32_t> external_jump_targets;

    std::uint32_t indirect_calls{};
    std::uint32_t indirect_jumps{};
    std::uint32_t returns{};

    /// Sum of the function's range sizes, which is its real code extent.
    [[nodiscard]] std::uint32_t size() const noexcept {
        std::uint32_t total = 0U;
        for (const auto& range : ranges) {
            total += range.end_rva > range.begin_rva ? range.end_rva - range.begin_rva : 0U;
        }
        return total;
    }

    /// Fraction of the function extent the descent actually decoded. Well
    /// below 1.0 usually means embedded data, not a decoder failure.
    [[nodiscard]] double coverage() const noexcept {
        const auto extent = size();
        return extent == 0U ? 0.0 : static_cast<double>(decoded_bytes) / static_cast<double>(extent);
    }
};

struct CodeGraph final {
    /// One entry per *function* — chained continuation ranges are folded into
    /// the function that owns them, so this is shorter than the exception
    /// directory.
    std::vector<FunctionWalk> functions;

    std::size_t exception_directory_entries{};
    std::size_t functions_walked{};
    std::size_t functions_complete{};
    std::size_t total_instructions{};
    std::uint64_t total_decoded_bytes{};
    std::size_t call_edges{};
    std::size_t data_reference_edges{};

    std::vector<std::string> warnings;
};

/// Builds the call and data-reference graph over an unwind-backed function
/// inventory.
class CodeGraphBuilder final {
public:
    [[nodiscard]] static CodeGraph build(std::span<const std::byte> bytes, const PeImage& image,
                                         const PeFunctionTable& functions);
};

} // namespace dmc::rengine::exe
