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

    /// Byte displacements of `call [reg + disp]` sites, sorted and unique.
    ///
    /// In a vtable dispatch the displacement is the slot offset, so a value of
    /// 0x48 means slot nine. The receiver's type is not known here, which is
    /// why these are displacements rather than resolved targets.
    std::vector<std::uint32_t> indirect_call_displacements;

    /// An instruction that reads `[base + index*scale + disp]` where the base
    /// register was loaded with an image address earlier in the same trace.
    ///
    /// This is the difference between code that *reaches* a table's bytes and
    /// code that *indexes* it. A constant index folded into a displacement and
    /// a direct load of one literal are the same instruction; holding a base
    /// and scaling an index is not, and only this can tell them apart.
    struct IndexedAccess final {
        /// Address of the indexing instruction.
        std::uint32_t site_rva{};
        /// Image address the base register held.
        std::uint32_t base_rva{};
        /// Scale applied to the index: the element size the code assumes.
        std::uint32_t element_bytes{};
        /// Constant added on top of the base, which picks a field.
        std::int32_t displacement{};
        /// Register holding the index. Two reads in one trace that share it and
        /// an element size are walking the same array, which is what makes a
        /// field offset measured rather than inferred from proximity.
        std::uint8_t index_register{};

        friend bool operator==(const IndexedAccess&, const IndexedAccess&) = default;
    };

    /// Indexed reads of a base the walk could follow, in address order.
    std::vector<IndexedAccess> indexed_accesses;

    /// A `call [reg + disp]` whose receiver the walk could follow back to a
    /// static object: `lea reg,[rip+object]`, then a load of the object's first
    /// quadword, which for a polymorphic C++ object is its vtable pointer.
    struct DispatchSite final {
        std::uint32_t site_rva{};
        std::uint32_t displacement{};
        /// Address the dispatched-through pointer was loaded from, or zero when
        /// the walk could not follow it there.
        std::uint32_t receiver_object_rva{};
        /// The pointer was loaded through the register holding the function's
        /// first argument: a call on `this` or on something inside it.
        bool through_this{false};
        /// Offset within that object the pointer was read from. Zero is the
        /// object's own vtable pointer, so the receiver is the object itself; a
        /// non-zero offset is the vtable pointer of whatever sits there, so the
        /// receiver is that member.
        std::uint32_t receiver_field_offset{};
        /// One when the vtable pointer came straight out of the object, which
        /// is an embedded subobject at that offset; two when it came through a
        /// pointer stored there, which is a pointer member.
        std::uint8_t receiver_depth{};

        friend bool operator==(const DispatchSite&, const DispatchSite&) = default;
    };

    /// Virtual dispatch sites whose receiver object the walk could name.
    std::vector<DispatchSite> resolved_dispatch_sites;

    /// A store of a known image address into the object the function was given
    /// as its first argument: `mov [this + offset], reg` with the register
    /// holding an address a RIP-relative `lea` produced.
    ///
    /// A constructor writes its class's vtable at offset zero, and the vtables
    /// of its base subobjects and embedded members at their offsets, so these
    /// stores are the class's layout as the compiler laid it out.
    struct VtableStore final {
        std::uint32_t site_rva{};
        std::uint32_t offset{};
        std::uint32_t stored_rva{};

        friend bool operator==(const VtableStore&, const VtableStore&) = default;
    };

    /// Stores into `this`, in address order.
    std::vector<VtableStore> stores_into_this;

    /// Offsets of memory operands read or written through the register the
    /// Microsoft x64 convention puts the first argument in, sorted and unique.
    ///
    /// Where that argument is `this` — which a vtable binding or a constructor
    /// store establishes and nothing else does — an access at offset K means
    /// the object extends at least K+1 bytes. That is a floor on the class's
    /// size read from code, independent of anything the type information says.
    /// `lea` is excluded: computing an address is not touching what is there,
    /// and a one-past-the-end pointer is an ordinary thing to compute.
    std::vector<std::uint32_t> entry_field_offsets;

    /// A store into the object of a value a call returned: `call F` then
    /// `mov [this + offset], rax`. Where F is a constructor, the field holds a
    /// pointer to an object of the class F builds, which is what a dispatch
    /// through that field reads its vtable from.
    struct PointerStore final {
        std::uint32_t site_rva{};
        std::uint32_t offset{};
        std::uint32_t callee_rva{};

        friend bool operator==(const PointerStore&, const PointerStore&) = default;
    };

    /// Pointer stores into `this`, in address order.
    std::vector<PointerStore> pointer_stores_into_this;

    /// A direct call whose first argument is a constant the code put there.
    /// A factory keyed by a selector is reached this way, and the selector is
    /// what distinguishes one kind of returned object from another.
    struct ConstantArgumentCall final {
        std::uint32_t site_rva{};
        std::uint32_t callee_rva{};
        std::uint32_t argument{};

        friend bool operator==(const ConstantArgumentCall&, const ConstantArgumentCall&) = default;
    };

    /// Calls with a constant first argument, in address order.
    std::vector<ConstantArgumentCall> constant_argument_calls;

    /// Addresses of the instructions the walk decoded, in order. The register
    /// analysis runs over these rather than re-discovering the code, so it
    /// inherits the walk's guarantee that every one of them is a real
    /// instruction boundary.
    std::vector<std::uint32_t> instruction_starts;

    std::uint32_t indirect_calls{};
    std::uint32_t indirect_jumps{};
    std::uint32_t returns{};

    /// Switch dispatch tables recovered behind register-indirect jumps.
    std::uint32_t switch_tables{};
    /// Indirect jumps for which no table could be validated. These remain
    /// genuine holes in the graph rather than assumed-empty ones.
    std::uint32_t unresolved_indirect_jumps{};
    /// Jumps through a memory operand off a register: a virtual call in tail
    /// position, not a switch whose table went missing. Counted apart because
    /// lumping the two together makes a dispatch site look like a failure of
    /// the switch recovery.
    std::uint32_t tail_dispatch_jumps{};
    /// Sorted, unique block addresses reached only through a switch table.
    std::vector<std::uint32_t> switch_targets;

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
    std::size_t indirect_call_sites{};
    std::size_t switch_tables_recovered{};
    std::size_t switch_targets_recovered{};
    std::size_t unresolved_indirect_jumps{};
    std::size_t tail_dispatch_jumps{};

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
