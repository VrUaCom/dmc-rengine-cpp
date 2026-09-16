#pragma once

#include "dmc_rengine/exe/code_graph.hpp"
#include "dmc_rengine/exe/pe_directories.hpp"
#include "dmc_rengine/exe/pe_image.hpp"
#include "dmc_rengine/exe/rtti_scanner.hpp"
#include "dmc_rengine/exe/string_table_scanner.hpp"

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

/// A class vtable whose address this function writes or takes.
///
/// Installing a vtable is what a constructor does, so this is the strongest
/// mechanical pointer to construction and destruction code that a stripped
/// binary offers. It identifies the class, not the operation: an initializer, a
/// destructor and a placement helper all reference the same table.
struct VtableInstall final {
    std::string class_display_name;
    std::uint32_t vtable_index{};
    std::uint32_t vtable_rva{};

    friend bool operator==(const VtableInstall&, const VtableInstall&) = default;
};

/// Resource families a literal's text names.
///
/// Purely a property of the string: a literal containing ".pac" names the PAC
/// family. It says nothing about what the referencing function does with it,
/// and a match is a lead rather than a conclusion.
[[nodiscard]] std::vector<std::string> resource_family_hints(std::string_view literal);

/// A recovered name table this function addresses.
///
/// A table is indexed from its base, so a reference is the link between code
/// and the resource names it resolves. A reference landing inside the span
/// rather than on the base is equally real: the compiler folds a constant index
/// into the displacement.
struct NameTableReference final {
    std::uint32_t table_base_rva{};
    /// Byte offset of the reference within the table; zero means the base.
    std::uint32_t offset_in_table{};
    /// True for a multi-field record layout, false for a constant-stride run.
    bool record_layout{false};
    /// Stride for a run, record size for a record layout.
    std::uint32_t element_bytes{};
    std::uint32_t entries{};

    /// True when the offset is a whole multiple of the element size, so the
    /// reference names one element outright. A compiler folds a constant index
    /// into the displacement, which is why a constant lookup is readable here
    /// and a computed one is not.
    bool constant_index{false};
    /// Element the reference names, valid when `constant_index` holds.
    std::uint32_t element_index{};
    /// For a record layout, the field within the record the offset falls in.
    std::uint32_t field_index{};
    std::uint32_t offset_in_element{};

    friend bool operator==(const NameTableReference&, const NameTableReference&) = default;
};

/// What is known about one function, from structure alone.
struct FunctionFacts final {
    /// Class whose vtable this function writes at offset zero of the object it
    /// was given, which makes it a constructor or destructor of that class.
    std::string constructs_class;
    std::uint32_t begin_rva{};
    std::uint32_t end_rva{};
    std::uint32_t instruction_count{};
    bool walk_complete{true};

    std::uint32_t caller_count{};
    std::uint32_t callee_count{};
    /// Functions this one calls directly, sorted and unique. Carried through
    /// so the call graph can be queried rather than only counted: bounding a
    /// subsystem means asking what reaches an import, which is a question about
    /// edges.
    std::vector<std::uint32_t> calls;

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

    /// Class vtables whose addresses this function references.
    std::vector<VtableInstall> installs_vtables;
    /// Name tables this function addresses.
    std::vector<NameTableReference> name_tables;
    /// Resource families named by literals this function references.
    std::vector<std::string> resource_families;
    /// Dispatch offsets of `call [reg + disp]` sites within this function.
    std::vector<std::uint32_t> indirect_call_displacements;

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
    /// Functions that reference one of this class's vtables.
    std::uint32_t install_sites{};

    friend bool operator==(const ClassCodeCoverage&, const ClassCodeCoverage&) = default;
};

struct ResourceFamilyUsage final {
    std::string family;
    std::uint32_t literals{};
    std::uint32_t referencing_functions{};

    friend bool operator==(const ResourceFamilyUsage&, const ResourceFamilyUsage&) = default;
};

/// Census of dispatch offsets across all indirect call sites.
struct DispatchSlotUsage final {
    std::uint32_t displacement{};
    /// Slot index the displacement implies for a 64-bit vtable.
    std::uint32_t slot{};
    std::uint32_t call_sites{};

    friend bool operator==(const DispatchSlotUsage&, const DispatchSlotUsage&) = default;
};

/// How a recovered name table is reached from code.
struct NameTableUsage final {
    std::uint32_t table_base_rva{};
    bool record_layout{false};
    std::uint32_t element_bytes{};
    std::uint32_t entries{};
    std::uint32_t referencing_functions{};
    /// References landing on the base rather than inside the span.
    std::uint32_t base_references{};
    /// Distinct elements named outright by a constant index.
    std::uint32_t elements_named_by_constant{};
    /// Instructions that index this run as an array: a register holding its
    /// base, scaled by its own element size.
    ///
    /// This is the difference between code reaching a run's bytes and code
    /// treating it as a table. A constant index folded into a displacement is
    /// indistinguishable from a direct load of the literal at that offset; a
    /// held base with a scaled index is not.
    std::uint32_t indexed_sites{};

    friend bool operator==(const NameTableUsage&, const NameTableUsage&) = default;
};

/// An array in data that code walks with a scaled index.
///
/// Recovered from instruction encodings alone: the base a register was seen to
/// hold, the element size the scale and any index multiplier imply, and the
/// displacements used against it, which are offsets of fields within the
/// element. A displacement must land inside the element to be counted, which is
/// a free consistency check on the element size.
struct IndexedArray final {
    std::uint32_t base_rva{};
    std::uint32_t element_bytes{};
    std::uint32_t sites{};
    std::uint32_t referencing_functions{};
    /// Sorted, unique field offsets observed, each below `element_bytes`.
    std::vector<std::uint32_t> field_offsets;
    /// True when a register was seen holding this base, so the base is where
    /// the array starts. False when it was reached against the image base, in
    /// which case the array's start is folded into the displacement and this
    /// is only the lowest address observed — an upper bound on the base, with
    /// the field offsets measured relative to it.
    bool base_measured{true};

    friend bool operator==(const IndexedArray&, const IndexedArray&) = default;
};

/// A virtual call whose receiver the walk could name, resolved to a target.
///
/// The receiver is `this`: the dispatch reads the vtable pointer out of the
/// register the Microsoft x64 convention puts the first argument in, or out of
/// a copy of it. The enclosing function's own class then says which vtable, and
/// the displacement says which slot, so the target is read rather than guessed.
struct ResolvedDispatch final {
    std::uint32_t site_rva{};
    std::uint32_t caller_rva{};
    std::uint32_t displacement{};
    std::uint32_t slot{};
    std::string class_display_name;
    std::uint32_t target_rva{};
    /// Offset within the enclosing object the receiver was read from. Zero is a
    /// call on the object itself; anything else is a call on the member sitting
    /// there, whose class the constructor's own stores named.
    std::uint32_t receiver_field_offset{};

    friend bool operator==(const ResolvedDispatch&, const ResolvedDispatch&) = default;
};

/// A vtable a constructor writes into its object, read as class layout.
///
/// A constructor writes its own class's vtable at offset zero and the vtables
/// of whatever sits inside the object at their offsets. Where the vtable
/// belongs to another class the offset names an embedded member and its type;
/// where it belongs to the same class it is a base subobject of that class's
/// own hierarchy, and the RTTI's recorded subobject offset is a second,
/// independent statement of where it sits.
struct ClassFieldLayout final {
    std::string class_display_name;
    std::uint32_t offset{};
    std::string member_class_display_name;
    /// The vtable the constructor actually wrote here. For a base subobject
    /// that is a secondary vtable of the class itself, which is the one a call
    /// through this offset dispatches into — not the class's primary vtable.
    std::uint32_t member_vtable_rva{};
    std::uint32_t site_rva{};
    /// The member's class differs from the constructor's, so the offset names
    /// something the object contains rather than something it is.
    bool embedded_member{false};
    /// The RTTI records this vtable's subobject offset, and it is the offset
    /// the store used.
    bool offset_confirmed_by_rtti{false};

    friend bool operator==(const ClassFieldLayout&, const ClassFieldLayout&) = default;
};

/// A function the code calls with a constant first argument, and the constants
/// it is called with.
///
/// Says nothing about what the constant means. It is a measurement of which
/// functions are parameterised by a small value and what values exist, which
/// bounds an enumeration without naming it.
struct ConstantArgumentCallee final {
    std::uint32_t callee_rva{};
    std::uint32_t call_sites{};
    std::uint32_t distinct_arguments{};
    /// The constants seen, ascending, capped so one callee cannot fill the
    /// report.
    std::vector<std::uint32_t> arguments;

    friend bool operator==(const ConstantArgumentCallee&, const ConstantArgumentCallee&) = default;
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
    std::size_t with_name_table{};
    std::size_t constant_index_references{};
    std::size_t computed_index_references{};
    std::size_t name_tables_referenced{};
    /// Instructions indexing a base the walk could follow, and how many of
    /// those reach a recovered run or record at its own element size.
    std::size_t indexed_accesses{};
    std::size_t image_base_indexed_accesses{};
    std::size_t indexed_table_accesses{};
    /// Accesses on a base other than the image base, split by whether the
    /// displacement lands inside the element. A negative displacement is the
    /// inlined character scan over a string, not an array walk; one at or past
    /// the element means the element size or the base is wrong, and is the
    /// error bar on this inference.
    std::size_t held_base_accesses{};
    std::size_t consistent_array_accesses{};
    std::size_t string_scan_accesses{};
    std::size_t inconsistent_array_accesses{};
    std::size_t indexed_arrays{};
    /// Bases read at more than one element size, which is a contradiction: at
    /// least one of the readings is wrong. Reported rather than resolved, since
    /// nothing here says which.
    std::size_t arrays_with_conflicting_element_size{};
    /// Image-base reads grouped by function, index register and element size.
    /// A group whose reads span more than one element means the index register
    /// was reused for another array, which is what makes grouping by address
    /// proximity alone unsound.
    std::size_t image_base_groups{};
    std::size_t image_base_groups_with_several_reads{};
    std::size_t image_base_groups_spanning_elements{};
    /// Groups that reproduce a base and element size a register was also seen
    /// holding: two routes to the same array, which is corroboration rather
    /// than a second array.
    std::size_t image_base_groups_corroborating{};
    /// Virtual dispatch through a register base, and how far each site could be
    /// followed. A site is resolved only when the receiver is `this`, the
    /// enclosing function belongs to exactly one class, and that class's vtable
    /// has the slot.
    std::size_t dispatch_sites{};
    std::size_t dispatch_sites_on_this{};
    std::size_t dispatch_sites_in_a_bound_function{};
    std::size_t dispatch_sites_resolved{};
    /// Of those, the ones whose receiver is a member of the enclosing object
    /// rather than the object itself.
    std::size_t dispatch_sites_on_a_member{};
    /// Sites whose receiver came out of a pointer stored in the object rather
    /// than out of the object itself. The vtable read is the pointee's, so the
    /// enclosing class's layout says nothing about it and these are left alone.
    std::size_t dispatch_sites_through_a_pointer_member{};
    /// Stores into the object of a value a call returned, and how many of those
    /// callees are constructors. A store of freshly allocated memory types
    /// nothing until the constructor that runs on it is followed too.
    std::size_t pointer_stores_into_this{};
    std::size_t pointer_stores_from_a_constructor{};
    /// Calls whose first argument is a constant the code put there, and how
    /// many distinct functions they reach.
    std::size_t constant_argument_calls{};
    std::size_t constant_argument_callees{};
    /// Stores of an address the code took with a `lea` into the object a
    /// function was given, and how many of those land on a known vtable. A
    /// store at offset zero identifies the function as a constructor or
    /// destructor of that vtable's class.
    std::size_t stores_into_this{};
    std::size_t stores_of_a_vtable{};
    std::size_t constructors_identified{};
    std::size_t field_layout_entries{};
    std::size_t field_offsets_confirmed_by_rtti{};
    std::size_t name_tables_unreferenced{};
    std::size_t with_vtable_install{};
    std::size_t with_resource_family{};
    std::size_t with_indirect_dispatch{};
    std::size_t indirect_call_sites{};
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
    /// Resource families ranked by referencing functions.
    std::vector<ResourceFamilyUsage> resource_family_usage;
    /// Dispatch offsets ranked by call sites.
    std::vector<DispatchSlotUsage> dispatch_slots;
    /// Recovered name tables and how many functions reach each.
    std::vector<NameTableUsage> name_table_usage;
    /// Arrays in data the code walks with a scaled index, most-used first.
    std::vector<IndexedArray> indexed_arrays;
    /// Class layout read out of constructor stores, by class then offset.
    std::vector<ClassFieldLayout> class_field_layout;
    /// Functions called with a constant first argument, most-called first.
    std::vector<ConstantArgumentCallee> constant_argument_callees;
    /// Virtual calls resolved to a class and a target, in address order.
    std::vector<ResolvedDispatch> resolved_dispatches;
    std::vector<std::string> warnings;
};

struct FunctionMapInputs final {
    const PeImage* image{};
    const PeDirectories* directories{};
    const RttiScanResult* rtti{};
    const CodeGraph* graph{};
    /// Optional: recovered name tables, to link code to the resource names it
    /// resolves.
    const StringTableScanResult* name_tables{};
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
