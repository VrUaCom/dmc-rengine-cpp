#pragma once

#include "dmc_rengine/exe/code_graph.hpp"
#include "dmc_rengine/exe/pe_directories.hpp"
#include "dmc_rengine/exe/pe_image.hpp"
#include "dmc_rengine/exe/rtti_scanner.hpp"

#include <cstddef>
#include <cstdint>
#include <span>
#include <string>
#include <vector>

namespace dmc::rengine::exe {

struct ImportCall final {
    std::string module;
    std::string function;

    friend bool operator==(const ImportCall&, const ImportCall&) = default;
};

/// A function reached through a class vtable slot.
///
/// This is the strongest naming signal a stripped binary offers: the slot is
/// the compiler's own binding of code to a type. It gives the function an
/// owner and an index, not a name or a meaning.
struct VirtualMethodBinding final {
    std::string class_display_name;
    std::uint32_t vtable_index{};
    std::uint32_t slot{};
    std::uint32_t subobject_offset{};

    friend bool operator==(const VirtualMethodBinding&, const VirtualMethodBinding&) = default;
};

/// What is known about one function, from structure alone.
struct FunctionFacts final {
    std::uint32_t begin_rva{};
    std::uint32_t end_rva{};
    std::uint32_t instruction_count{};
    bool walk_complete{true};

    std::uint32_t caller_count{};
    std::uint32_t callee_count{};

    bool reachable_from_entry_point{false};
    bool reachable_from_export{false};
    bool exported{false};
    std::string export_name;

    /// True when the whole function is a single indirect jump through an
    /// import-address-table slot: the linker's import thunk. Most thunks carry
    /// no unwind data and therefore never enter the inventory at all, so this
    /// is normally false even in an image full of them.
    bool import_thunk{false};

    /// Prologue facts from the function's unwind record.
    PeUnwindFrame frame{};

    std::vector<ImportCall> imports_called;
    std::vector<VirtualMethodBinding> virtual_bindings;
    std::vector<std::string> referenced_strings;
    std::uint32_t string_reference_count{};

    [[nodiscard]] std::uint32_t size() const noexcept {
        return end_rva > begin_rva ? end_rva - begin_rva : 0U;
    }

    /// True when nothing in the image structurally refers to this function.
    /// Such a function is reached some other way — a jump table, a data-driven
    /// dispatch, or code the walk could not follow — and is not evidence of
    /// dead code.
    [[nodiscard]] bool structurally_unreferenced() const noexcept {
        return caller_count == 0U && virtual_bindings.empty() && !exported;
    }
};

/// How much of one class's vtable surface resolves to inventoried functions.
struct ClassCodeCoverage final {
    std::string class_display_name;
    std::uint32_t vtable_slots{};
    std::uint32_t slots_bound_to_functions{};
    std::uint32_t distinct_functions{};

    friend bool operator==(const ClassCodeCoverage&, const ClassCodeCoverage&) = default;
};

struct ImportUsage final {
    std::string module;
    std::string function;
    std::uint32_t calling_functions{};

    friend bool operator==(const ImportUsage&, const ImportUsage&) = default;
};

struct FunctionMapSummary final {
    std::size_t functions{};
    std::size_t walks_complete{};
    std::size_t with_virtual_binding{};
    std::size_t with_import_call{};
    std::size_t with_string_reference{};
    std::size_t import_thunks{};
    /// Thunks reached by a call but absent from the unwind inventory, resolved
    /// by decoding their single jump.
    std::size_t external_thunks_resolved{};
    std::size_t reachable_from_entry_point{};
    std::size_t reachable_from_export{};
    /// Functions bound to at least one vtable slot. Virtual dispatch is an
    /// indirect call, so these are reachable in ways a direct-call graph
    /// cannot show.
    std::size_t virtual_dispatch_candidates{};
    /// Union of direct reachability and vtable binding.
    std::size_t structurally_reachable{};
    std::size_t structurally_unreferenced{};
    std::size_t attributed{};
    std::size_t strings_recovered{};
    std::size_t with_frame_pointer{};
    std::size_t with_exception_handler{};
    std::size_t leaf_functions{};
    std::uint64_t total_stack_allocation{};
    std::uint32_t largest_stack_allocation{};
};

struct FunctionMap final {
    std::vector<FunctionFacts> functions;
    FunctionMapSummary summary;
    /// Imports ranked by how many functions call them.
    std::vector<ImportUsage> import_usage;
    /// Classes ranked by vtable surface.
    std::vector<ClassCodeCoverage> class_coverage;
    std::vector<std::string> warnings;
};

struct FunctionMapInputs final {
    const PeImage* image{};
    const PeDirectories* directories{};
    const RttiScanResult* rtti{};
    const CodeGraph* graph{};
};

/// True for a function whose unwind record describes no prologue work at all:
/// no stack reserved, nothing saved, no frame pointer established.
///
/// Vanishingly rare in practice, and structurally so: a function earns an
/// exception-directory entry because it has a prologue worth unwinding. A
/// function with none usually has no entry, which is why import thunks are
/// absent from the inventory entirely.
[[nodiscard]] inline bool is_leaf_frame(const PeUnwindFrame& frame) noexcept {
    return frame.decoded && frame.stack_allocation == 0U && frame.pushed_registers == 0U &&
           frame.saved_registers == 0U && frame.saved_xmm == 0U && !frame.uses_frame_pointer() &&
           !frame.machine_frame;
}

/// Joins the code graph against the import table, the export table and the
/// recovered class graph to say what is known about each function.
///
/// Everything it produces is mechanical. It attributes and counts; it does not
/// name functions or infer what they do.
class FunctionMapBuilder final {
public:
    [[nodiscard]] static FunctionMap build(std::span<const std::byte> bytes,
                                           const FunctionMapInputs& inputs);
};

} // namespace dmc::rengine::exe
