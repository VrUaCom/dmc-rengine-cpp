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
#include <string_view>
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
    /// Shortest chain of direct calls from the entry point, or `kUnreached`.
    /// A depth is not an execution order: it says how few calls can reach the
    /// function, not when it runs.
    static constexpr std::uint16_t kUnreached = 0xFFFFU;
    std::uint16_t depth_from_entry{kUnreached};
    /// Reachable from the entry point once every virtual call is assumed to
    /// reach whatever sits at its slot in *any* vtable the image carries.
    /// Nothing says what a receiver's type is, so this is a sound
    /// over-approximation — an upper bound on reachability, not a second
    /// answer. A function this does not reach is not reachable from the entry
    /// point under any assumption this file supports.
    bool reachable_through_dispatch{false};
    /// Reachable when the closure also follows the two edge kinds the file
    /// records besides calls and dispatch: exception funclets of a reached
    /// parent, and function starts whose address a reached function takes.
    /// A superset of `reachable_through_dispatch`; its complement is what
    /// `outside_every_closure` counts.
    bool reachable_through_recorded_edges{false};
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
    /// Dispatch instructions in this function, counted from the same list the
    /// image-wide census uses. The vector above holds only the call-position
    /// offsets and deduplicates them, so it is neither a count nor the same
    /// population — which is exactly the confusion this field exists to end.
    std::uint32_t dispatch_sites{};

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

/// What sits at the far end of a vtable slot.
///
/// Decided by decoding the target's first instruction, which is enough to
/// separate three cases without reading a body. A jump through the import slot
/// the import directory names `_purecall` is the compiler's own marker for a
/// declaration with no definition. A return as the first instruction is a body
/// that does nothing. Everything else has code in it, and this says nothing
/// about what that code does.
enum class VtableSlotKind : std::uint8_t {
    /// Reaches the `_purecall` import: the slot is declared, not defined.
    pure_virtual,
    /// The target's first instruction is a return.
    empty_body,
    implemented,
};

[[nodiscard]] constexpr std::string_view to_string(VtableSlotKind kind) noexcept {
    switch (kind) {
    case VtableSlotKind::pure_virtual: return "pure-virtual";
    case VtableSlotKind::empty_body: return "empty-body";
    case VtableSlotKind::implemented: return "implemented";
    }
    return "implemented";
}

/// One slot of one base class, measured across everything that inherits it.
///
/// A derived class is compared against the base through the vtable of the
/// base's own subobject, at the offset the class hierarchy descriptor records
/// for it — not through the derived class's primary vtable. Those are
/// different tables whenever the base does not sit at offset zero, and
/// comparing them would compare unrelated interfaces.
///
/// The ratio of distinct implementations to classes carrying the slot is the
/// measurement: a slot nearly every class implements differently is where
/// per-class behaviour lives, and a slot they nearly all leave alone is
/// inherited behaviour. Nothing here names what the slot does.
struct BaseSlotOverride final {
    std::string base_display_name;
    std::uint32_t slot{};
    /// What the base itself puts in this slot, and where.
    VtableSlotKind base_kind{VtableSlotKind::implemented};
    std::uint32_t base_target_rva{};

    /// Classes whose subobject vtable for this base reaches this slot.
    std::uint32_t derived_classes{};
    /// Of those, how many leave the base's own target in place. A slot the
    /// base declares pure is never kept, because there is nothing to keep.
    std::uint32_t keep_base_target{};
    std::uint32_t empty_bodies{};
    std::uint32_t pure_virtual{};
    /// Distinct targets with code in them. The linker folds identical
    /// functions, so two classes reaching one target need not have been
    /// written once: this is a lower bound on distinct behaviour.
    std::uint32_t distinct_implementations{};

    friend bool operator==(const BaseSlotOverride&, const BaseSlotOverride&) = default;
};

/// A fixed address the code hands to a direct call as its first argument.
///
/// The Microsoft x64 convention puts the first argument in rcx, which is also
/// where `this` goes, so an address passed there is something the callee
/// operates on. Nothing here says it is a C++ object: a free function's first
/// argument is just an argument. What is measured is the shape — one address,
/// many distinct callees, many distinct callers — which is the shape of an
/// interface over shared state whatever the construct behind it.
///
/// `field_reach` counts only callees dedicated to this block: every image
/// address they are given is this one, and they have no callers beyond those
/// sites, so no other first argument reaches them. Without that restriction a
/// function serving several blocks would lend its deepest offset to all of
/// them.
struct GlobalStateBlock final {
    std::uint32_t base_rva{};
    std::string section;
    std::uint32_t call_sites{};
    std::uint32_t distinct_callees{};
    std::uint32_t distinct_callers{};
    /// Deepest offset a dedicated callee reaches inside it, plus one. Zero when
    /// no callee qualified.
    std::uint32_t field_reach{};
    /// Distance to the next address the code addresses this way, or zero for
    /// the last. Not a size: two addresses can be parts of one thing, which is
    /// exactly what the flag below records.
    std::uint32_t bytes_to_next_block{};
    /// The reach runs past the next block's address, so that address is a field
    /// inside this one. Two measurements from unrelated sources disagreeing
    /// this way is informative rather than wrong.
    bool reach_runs_past_the_next_block{false};
    /// Class of the constructor seen running on it, where one was.
    std::string constructed_class;

    friend bool operator==(const GlobalStateBlock&, const GlobalStateBlock&) = default;
};

/// A floor on how large one class's objects are.
///
/// Two independent sources, and neither reads a field's contents. The class
/// hierarchy descriptor places each base subobject at a recorded displacement,
/// and a base carrying a vtable occupies at least the eight bytes of that
/// pointer, so the deepest such base plus eight is a floor. Separately, a
/// method the compiler bound into the class's vtable receives the object as its
/// first argument, so a memory operand at offset K within it means the object
/// reaches at least K+1 bytes.
///
/// A function whose code sits in several ranges is
/// analysed with each range's start knowing nothing, because a range can be
/// entered by a branch the analysis cannot see; a field touched only in such a
/// range therefore does not count. That is one more reason this is a floor.
///
/// The code source is all but guaranteed to be the larger of the two, because
/// MSVC lays base subobjects before members: the deepest member sits past the
/// deepest base whenever a class has any member at all. Agreement is therefore
/// not evidence; what the two together give is a floor plus a check that the
/// pair is ordered the way the layout rule requires.
struct ClassSizeFloor final {
    std::string class_display_name;
    /// The larger of the two sources.
    std::uint32_t floor_bytes{};
    /// Deepest base subobject carrying a vtable, plus its eight bytes.
    std::uint32_t floor_from_bases{};
    std::string deepest_base_display_name;
    /// Deepest offset any bound method reaches inside the object, plus one.
    std::uint32_t floor_from_field_access{};

    /// Functions that say anything about this class's extent, and how many of
    /// them independently reach at least half the floor. A floor one function
    /// alone supports is a different kind of claim from one a dozen agree on,
    /// and this is the difference rather than a filter applied to it.
    std::uint32_t functions_speaking{};
    std::uint32_t functions_reaching_half{};

    friend bool operator==(const ClassSizeFloor&, const ClassSizeFloor&) = default;
};

/// A run of consecutive function addresses sitting in data, outside every
/// vtable the type information locates.
///
/// A vtable is one of these, so the interesting ones are what is left after
/// every located vtable's whole extent is excluded — not merely its base. A
/// slot holding something the function inventory does not cover splits a
/// vtable into fragments whose bases are not the vtable's, and counting those
/// as tables invents a dispatch mechanism out of a gap in the inventory.
struct FunctionPointerRun final {
    std::uint32_t base_rva{};
    std::uint32_t entries{};
    /// Entries naming a function that nothing else in the image reaches.
    std::uint32_t entries_reaching_nothing_else{};
    /// Functions whose code takes the run's address. Zero means the run is
    /// reached from outside the inventory, if at all.
    std::uint32_t referencing_functions{};

    friend bool operator==(const FunctionPointerRun&, const FunctionPointerRun&) = default;
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
    /// Entries of the finished table sharing a base at different element sizes.
    /// Held bases are resolved before they reach it — see below — so a non-zero
    /// value here now means image-base groups, whose base is only an upper bound
    /// and which are grouped per function and index register, so one base can
    /// legitimately appear twice.
    std::size_t arrays_with_conflicting_element_size{};
    /// Bases read at more than one element size where every smaller reading is
    /// the raw SIB scale and divides the largest. A multiplier can be missed but
    /// not invented, so those readings are one array and the largest is its
    /// element size.
    std::size_t arrays_resolved_by_a_missed_multiplier{};
    /// Bases where that does not hold, so nothing says which reading is right.
    /// Such a base is left out of the layout rather than given a size it may
    /// not have, and its accesses are counted below.
    std::size_t arrays_with_a_real_size_conflict{};
    std::size_t accesses_on_a_conflicted_base{};
    /// Image-base reads grouped by function, index register and element size.
    /// A group whose reads span more than one element means the index register
    /// was reused for another array, which is what makes grouping by address
    /// proximity alone unsound.
    std::size_t image_base_groups{};
    std::size_t image_base_groups_with_several_reads{};
    std::size_t image_base_groups_spanning_elements{};
    /// Of those, the ones whose reads do not all fall in one section, which
    /// cannot be one array whatever else is true. The rest span more than an
    /// element and still sit in one section, so nothing in the encoding decides
    /// between one array read at several constant indices and a register reused
    /// for another: they are dropped as undecided rather than as reuse.
    std::size_t image_base_groups_reads_in_several_sections{};
    std::size_t image_base_groups_undecided{};
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
    /// Sites whose receiver came out of the second, third or fourth argument
    /// rather than out of `this`. The convention names those registers, so this
    /// is a classification the file supports rather than a guess; what the
    /// argument points at still takes a caller to say.
    std::size_t dispatch_sites_on_an_argument{};
    /// Sites where the analysis has nothing for the register the vtable is read
    /// through, and sites where it came from an address the code took or a
    /// fixed one it loaded. With the two above, these four partition
    /// `dispatch_sites` exactly — which is the point of emitting them: a census
    /// that does not add up to its own total is a census with a bug in it.
    std::size_t dispatch_sites_with_an_unnamed_receiver{};
    std::size_t dispatch_sites_on_a_fixed_or_taken_address{};
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
    /// Every slot of every vtable in the image, split by what its target is.
    /// The pure count is the size of the image's declared-but-undefined
    /// surface; the empty count is how much of its polymorphism does nothing.
    std::size_t vtable_slots_classified{};
    std::size_t vtable_slots_pure_virtual{};
    std::size_t vtable_slots_empty_body{};
    std::size_t vtable_slots_implemented{};
    std::size_t vtable_slot_implementations{};
    /// Base-and-slot pairs measured, and the base-to-derived pairings that
    /// could not be measured because the class carries no vtable at the offset
    /// its own hierarchy descriptor records for that base.
    /// Reachability as a bracket rather than a single number. The lower bound
    /// is `reachable_from_entry_point`, which follows direct calls only; the
    /// upper bound assumes every virtual call reaches every vtable's slot.
    /// The truth is between, and how wide the gap is measures how much of the
    /// image's control flow is decided at run time.
    /// The startup path: what direct calls alone reach from the entry point.
    /// Sound throughout, since it assumes nothing about dispatch.
    std::size_t startup_path_functions{};
    std::uint16_t startup_path_deepest{};
    std::size_t startup_path_modules{};
    std::size_t startup_path_import_symbols{};
    /// Functions on the path that construct a class, and dispatch sites on it.
    /// Both say where the sound path stops being able to follow the program.
    std::size_t startup_path_constructors{};
    /// Counted from the same list as `dispatch_sites`, so the two agree. A
    /// narrower count of only the call-position ones misses the tail-position
    /// virtual calls, which are dispatch by another instruction.
    std::size_t startup_path_dispatch_sites{};
    std::size_t startup_path_dispatch_sites_in_tail_position{};
    /// Functions on the path the compiler bound into a vtable. Resolving a
    /// dispatch needs the enclosing function to belong to a class, so a zero
    /// here is why none of the path's dispatches resolves.
    std::size_t startup_path_functions_bound_to_a_class{};
    /// Functions the path reaches only through a dispatch whose target is
    /// determined. Each one extends the sound closure by an argued step rather
    /// than by an assumption.
    std::size_t startup_path_extended_by_resolved_dispatch{};
    std::size_t reachable_through_dispatch{};
    std::size_t outside_every_closure{};
    /// Reached by the widest closure but not by calls and dispatch alone.
    std::size_t reached_only_through_funclets_or_taken_addresses{};
    /// Parent-to-funclet edges read from C++ FuncInfo and SEH scope tables.
    std::size_t funclet_edges{};
    /// C++ FuncInfo structures recognised by their magic number.
    std::size_t funcinfo_structures{};
    /// SEH scope tables accepted after every entry validated against its owner.
    std::size_t scope_tables{};
    /// Function-to-function edges from a RIP-relative `lea` of a function start.
    std::size_t taken_address_edges{};
    std::size_t dispatch_slots_reached{};
    /// Vtables installed by code the direct-call closure reaches. Restricting
    /// dispatch to those is the standard way to tighten the upper bound; on
    /// this image it does almost nothing, because construction is itself
    /// behind dispatch.
    std::size_t vtables_instantiated_by_reachable_code{};
    std::size_t vtables_located{};
    /// Fixed addresses handed to a call as its first argument, and how the
    /// measurement was narrowed to make each reach sound.
    std::size_t global_state_blocks{};
    std::size_t global_block_call_sites{};
    std::size_t global_blocks_with_a_field_reach{};
    std::size_t global_blocks_reach_within_the_gap{};
    std::size_t global_blocks_reach_past_the_gap{};
    std::size_t global_blocks_with_a_class{};
    /// Call sites whose callee serves more than one block, or has callers
    /// beyond these sites, so its reach belongs to no one block.
    std::size_t global_block_sites_pooled{};
    std::size_t global_block_sites_with_other_callers{};
    /// Classes given a size floor, and how well supported each is.
    std::size_t class_size_floors{};
    std::size_t size_floors_above_a_vtable_pointer{};
    std::size_t size_floors_corroborated{};
    std::size_t size_floors_on_a_lone_outlier{};
    /// Classes where the base-placement floor is the deeper of the two. Not a
    /// contradiction — both are floors and the larger stands — but it marks
    /// where the code source said less than the type information did, which is
    /// the case for a class whose methods only ever read its vtable pointer.
    std::size_t size_floors_where_bases_say_more{};
    std::size_t function_pointer_runs{};
    std::size_t function_pointer_run_entries{};
    std::size_t base_slots_measured{};
    std::size_t base_pairings_without_a_vtable{};
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
    /// Per-base, per-slot override census, by base then slot.
    std::vector<BaseSlotOverride> base_slot_overrides;
    /// Function-address runs in data outside every located vtable.
    std::vector<FunctionPointerRun> function_pointer_runs;
    /// Size floors by class, largest first.
    std::vector<ClassSizeFloor> class_size_floors;
    /// Fixed addresses the code operates on, most-referenced first.
    std::vector<GlobalStateBlock> global_state_blocks;
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
