#pragma once

#include "dmc_rengine/exe/pe_image.hpp"

#include <cstddef>
#include <cstdint>
#include <span>
#include <string>
#include <vector>

namespace dmc::rengine::exe {

/// A base recorded in a class's MSVC hierarchy descriptor.
struct RttiBaseClass final {
    std::string decorated_name;
    std::string display_name;
    /// Member displacement of this base within the derived object.
    std::int32_t member_displacement{};
    std::int32_t vbtable_displacement{};
    std::int32_t vbtable_index{};
    std::uint32_t contained_bases{};

    friend bool operator==(const RttiBaseClass&, const RttiBaseClass&) = default;
};

/// One vtable of a class.
///
/// A type under multiple inheritance carries one complete-object locator and
/// one vtable per base subobject, distinguished by the subobject offset the
/// locator records. Collapsing them would lose exactly the layout information
/// that makes the hierarchy usable.
struct RttiVtable final {
    std::uint32_t complete_object_locator_rva{};
    std::uint32_t vtable_rva{};
    std::uint32_t slot_count{};
    /// Offset of this base subobject within the complete object.
    std::uint32_t subobject_offset{};
    std::uint32_t constructor_displacement_offset{};

    friend bool operator==(const RttiVtable&, const RttiVtable&) = default;
};

/// One polymorphic class recovered from MSVC run-time type information.
///
/// Everything here is read from structures the compiler emitted, not inferred:
/// the names are the compiler's own decorated names, and the hierarchy is the
/// one the class hierarchy descriptor declares. Semantics of the class remain
/// unknown until separate evidence establishes them.
struct RttiClass final {
    std::string decorated_name;
    std::string display_name;
    /// False when the decorated name uses constructs the reconstruction does
    /// not fully model, so `display_name` is approximate.
    bool display_name_complete{true};

    std::uint32_t type_descriptor_rva{};
    std::uint32_t class_hierarchy_rva{};

    /// Every vtable the image carries for this type, ordered by subobject
    /// offset and then by locator address.
    std::vector<RttiVtable> vtables;

    bool multiple_inheritance{false};
    bool virtual_inheritance{false};
    bool ambiguous{false};

    /// Hierarchy as declared, most-derived first. Entry zero is the class
    /// itself, which is how MSVC lays the array out.
    std::vector<RttiBaseClass> hierarchy;

    [[nodiscard]] std::size_t base_count() const noexcept {
        return hierarchy.empty() ? 0U : hierarchy.size() - 1U;
    }
};

struct RttiScanResult final {
    /// One entry per distinct type, not per vtable.
    std::vector<RttiClass> classes;
    /// Type descriptors seen, including those with no locator: non-polymorphic
    /// types and types that appear only as a base leave a descriptor behind
    /// without ever getting one of their own.
    std::size_t type_descriptors{};
    std::size_t locators{};
    std::size_t vtables_located{};
    std::vector<std::string> warnings;
};

/// Recovers the MSVC RTTI class graph from a mapped image.
///
/// The scan is structural and read-only. It reports what the compiler recorded
/// and nothing else: a class the scan cannot tie to a vtable is reported with a
/// zero vtable RVA rather than a guessed one.
class RttiScanner final {
public:
    [[nodiscard]] static RttiScanResult scan(std::span<const std::byte> bytes,
                                             const PeImage& image);
};

/// Reconstructs a readable name from an MSVC decorated type name.
///
/// Returns the reconstruction and whether it is complete. Unmodelled
/// constructs are preserved verbatim rather than dropped, so an approximate
/// name never silently loses information.
struct DecoratedName final {
    std::string display;
    bool complete{true};
};

[[nodiscard]] DecoratedName reconstruct_type_name(std::string_view decorated);

} // namespace dmc::rengine::exe
