#include "dmc_rengine/exe/pe_reader.hpp"
#include "dmc_rengine/exe/rtti_scanner.hpp"

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <span>
#include <string>
#include <string_view>
#include <vector>

namespace {

using dmc::rengine::exe::PeReader;
using dmc::rengine::exe::reconstruct_type_name;
using dmc::rengine::exe::RttiScanner;

// ---------------------------------------------------------------------------
// Synthetic PE32+ carrying invented MSVC RTTI.
//
// It models `Derived : Base` with two complete-object locators, one per base
// subobject, which is the shape multiple inheritance produces. Every structure
// is written by this file; no original executable bytes are used.
// ---------------------------------------------------------------------------

constexpr std::uint64_t kImageBase = 0x140000000ULL;
constexpr std::uint32_t kRdataRva = 0x2000U;
constexpr std::size_t kRdataFile = 0x600U;

[[nodiscard]] constexpr std::size_t file_of(std::uint32_t rdata_rva) {
    return kRdataFile + (rdata_rva - kRdataRva);
}

void write_u16(std::vector<std::byte>& bytes, std::size_t offset, std::uint16_t value) {
    for (std::size_t index = 0; index < 2U; ++index) {
        bytes[offset + index] =
            static_cast<std::byte>((value >> static_cast<unsigned>(index * 8U)) & 0xFFU);
    }
}

void write_u32(std::vector<std::byte>& bytes, std::size_t offset, std::uint32_t value) {
    for (std::size_t index = 0; index < 4U; ++index) {
        bytes[offset + index] =
            static_cast<std::byte>((value >> static_cast<unsigned>(index * 8U)) & 0xFFU);
    }
}

void write_u64(std::vector<std::byte>& bytes, std::size_t offset, std::uint64_t value) {
    for (std::size_t index = 0; index < 8U; ++index) {
        bytes[offset + index] =
            static_cast<std::byte>((value >> static_cast<unsigned>(index * 8U)) & 0xFFU);
    }
}

void write_text(std::vector<std::byte>& bytes, std::size_t offset, std::string_view text) {
    for (std::size_t index = 0; index < text.size(); ++index) {
        bytes[offset + index] = static_cast<std::byte>(text[index]);
    }
    bytes[offset + text.size()] = std::byte{0};
}

[[nodiscard]] std::vector<std::byte> make_fixture() {
    std::vector<std::byte> bytes(0xC00U, std::byte{0});

    bytes[0] = static_cast<std::byte>('M');
    bytes[1] = static_cast<std::byte>('Z');
    write_u32(bytes, 0x3CU, 0x80U);
    write_u32(bytes, 0x80U, 0x00004550U);
    write_u16(bytes, 0x84U, 0x8664U);
    write_u16(bytes, 0x86U, 2U);
    write_u16(bytes, 0x94U, 0x00F0U);

    constexpr std::size_t optional = 0x98U;
    write_u16(bytes, optional, 0x020BU);
    write_u32(bytes, optional + 16U, 0x1000U);
    write_u64(bytes, optional + 24U, kImageBase);
    write_u32(bytes, optional + 56U, 0x3000U);
    write_u32(bytes, optional + 60U, 0x400U);
    write_u16(bytes, optional + 68U, 2U);
    write_u32(bytes, optional + 108U, 16U);

    constexpr std::size_t section_table = 0x188U;
    write_text(bytes, section_table, ".text");
    write_u32(bytes, section_table + 8U, 0x200U);
    write_u32(bytes, section_table + 12U, 0x1000U);
    write_u32(bytes, section_table + 16U, 0x200U);
    write_u32(bytes, section_table + 20U, 0x400U);
    write_u32(bytes, section_table + 36U, 0x60000020U);

    write_text(bytes, section_table + 40U, ".rdata");
    write_u32(bytes, section_table + 48U, 0x600U);
    write_u32(bytes, section_table + 52U, kRdataRva);
    write_u32(bytes, section_table + 56U, 0x600U);
    write_u32(bytes, section_table + 60U, static_cast<std::uint32_t>(kRdataFile));
    write_u32(bytes, section_table + 76U, 0x40000040U);

    // Type descriptors: two pointer-sized header fields, then the name.
    write_text(bytes, file_of(0x2000U) + 16U, ".?AVBase@@");
    write_text(bytes, file_of(0x2040U) + 16U, ".?AVDerived@@");

    // Base class descriptors. Entry zero of a hierarchy is the class itself.
    write_u32(bytes, file_of(0x2080U), 0x2040U);       // Derived
    write_u32(bytes, file_of(0x2080U) + 4U, 1U);       // contained bases
    write_u32(bytes, file_of(0x2080U) + 8U, 0U);       // mdisp
    write_u32(bytes, file_of(0x2080U) + 12U, 0xFFFFFFFFU);  // pdisp = -1
    write_u32(bytes, file_of(0x2080U) + 24U, 0x20C0U); // class hierarchy

    write_u32(bytes, file_of(0x20A0U), 0x2000U);       // Base
    write_u32(bytes, file_of(0x20A0U) + 8U, 8U);       // mdisp
    write_u32(bytes, file_of(0x20A0U) + 12U, 0xFFFFFFFFU);

    // Class hierarchy descriptor for Derived.
    write_u32(bytes, file_of(0x20C0U) + 4U, 1U);       // attributes: multiple inheritance
    write_u32(bytes, file_of(0x20C0U) + 8U, 2U);       // base count
    write_u32(bytes, file_of(0x20C0U) + 12U, 0x20E0U); // base class array
    write_u32(bytes, file_of(0x20E0U), 0x2080U);
    write_u32(bytes, file_of(0x20E0U) + 4U, 0x20A0U);

    // Locator for the primary subobject, and the vtable that references it.
    write_u32(bytes, file_of(0x2100U), 1U);            // signature
    write_u32(bytes, file_of(0x2100U) + 4U, 0U);       // subobject offset
    write_u32(bytes, file_of(0x2100U) + 12U, 0x2040U); // type descriptor
    write_u32(bytes, file_of(0x2100U) + 16U, 0x20C0U); // class hierarchy
    write_u32(bytes, file_of(0x2100U) + 20U, 0x2100U); // self

    write_u64(bytes, file_of(0x2140U), kImageBase + 0x2100U);
    write_u64(bytes, file_of(0x2140U) + 8U, kImageBase + 0x1000U);
    write_u64(bytes, file_of(0x2140U) + 16U, kImageBase + 0x1010U);
    write_u64(bytes, file_of(0x2140U) + 24U, 0U);

    // Locator for the `Base` subobject at offset 8, with its own vtable.
    write_u32(bytes, file_of(0x2180U), 1U);
    write_u32(bytes, file_of(0x2180U) + 4U, 8U);
    write_u32(bytes, file_of(0x2180U) + 12U, 0x2040U);
    write_u32(bytes, file_of(0x2180U) + 16U, 0x20C0U);
    write_u32(bytes, file_of(0x2180U) + 20U, 0x2180U);

    write_u64(bytes, file_of(0x21C0U), kImageBase + 0x2180U);
    write_u64(bytes, file_of(0x21C0U) + 8U, kImageBase + 0x1020U);
    write_u64(bytes, file_of(0x21C0U) + 16U, 0U);

    return bytes;
}

void the_class_graph_is_recovered() {
    const auto bytes = make_fixture();
    const auto parsed = PeReader::read(std::span<const std::byte>{bytes});
    assert(parsed.ok());

    const auto result = RttiScanner::scan(std::span<const std::byte>{bytes}, *parsed.image);

    // Both descriptors are found, but only `Derived` has locators.
    assert(result.type_descriptors == 2U);
    assert(result.locators == 2U);
    assert(result.classes.size() == 1U);

    const auto& derived = result.classes.front();
    assert(derived.decorated_name == ".?AVDerived@@");
    assert(derived.display_name == "Derived");
    assert(derived.display_name_complete);
    assert(derived.type_descriptor_rva == 0x2040U);
    assert(derived.multiple_inheritance);
    assert(!derived.virtual_inheritance);

    assert(derived.hierarchy.size() == 2U);
    assert(derived.base_count() == 1U);
    assert(derived.hierarchy[0].display_name == "Derived");
    assert(derived.hierarchy[1].display_name == "Base");
    assert(derived.hierarchy[1].member_displacement == 8);
    assert(derived.hierarchy[1].vbtable_displacement == -1);
}

void one_type_keeps_every_subobject_vtable() {
    const auto bytes = make_fixture();
    const auto parsed = PeReader::read(std::span<const std::byte>{bytes});
    const auto result = RttiScanner::scan(std::span<const std::byte>{bytes}, *parsed.image);

    const auto& derived = result.classes.front();
    assert(derived.vtables.size() == 2U);
    assert(result.vtables_located == 2U);

    // Ordered by subobject offset, so the primary vtable comes first.
    assert(derived.vtables[0].subobject_offset == 0U);
    assert(derived.vtables[0].complete_object_locator_rva == 0x2100U);
    assert(derived.vtables[0].vtable_rva == 0x2148U);
    assert(derived.vtables[0].slot_count == 2U);

    assert(derived.vtables[1].subobject_offset == 8U);
    assert(derived.vtables[1].vtable_rva == 0x21C8U);
    assert(derived.vtables[1].slot_count == 1U);
}

void a_locator_without_a_matching_descriptor_is_ignored() {
    auto bytes = make_fixture();

    // Repoint the primary locator at an RVA that holds no type descriptor.
    write_u32(bytes, file_of(0x2100U) + 12U, 0x2900U);

    const auto parsed = PeReader::read(std::span<const std::byte>{bytes});
    const auto result = RttiScanner::scan(std::span<const std::byte>{bytes}, *parsed.image);

    assert(result.locators == 1U);
    assert(result.classes.size() == 1U);
    assert(result.classes.front().vtables.size() == 1U);
    assert(result.classes.front().vtables[0].subobject_offset == 8U);
}

void a_locator_that_does_not_point_at_itself_is_rejected() {
    auto bytes = make_fixture();
    write_u32(bytes, file_of(0x2100U) + 20U, 0x2104U);

    const auto parsed = PeReader::read(std::span<const std::byte>{bytes});
    const auto result = RttiScanner::scan(std::span<const std::byte>{bytes}, *parsed.image);

    assert(result.locators == 1U);
}

void a_vtable_stops_at_the_first_non_code_slot() {
    auto bytes = make_fixture();

    // Point the second slot outside the executable section.
    write_u64(bytes, file_of(0x2140U) + 16U, kImageBase + 0x2000U);

    const auto parsed = PeReader::read(std::span<const std::byte>{bytes});
    const auto result = RttiScanner::scan(std::span<const std::byte>{bytes}, *parsed.image);

    const auto& derived = result.classes.front();
    assert(derived.vtables[0].slot_count == 1U);
}

void an_image_without_rtti_reports_nothing() {
    std::vector<std::byte> bytes(0xC00U, std::byte{0});
    bytes[0] = static_cast<std::byte>('M');
    bytes[1] = static_cast<std::byte>('Z');
    write_u32(bytes, 0x3CU, 0x80U);
    write_u32(bytes, 0x80U, 0x00004550U);
    write_u16(bytes, 0x84U, 0x8664U);
    write_u16(bytes, 0x86U, 1U);
    write_u16(bytes, 0x94U, 0x00F0U);
    write_u16(bytes, 0x98U, 0x020BU);
    write_u64(bytes, 0x98U + 24U, kImageBase);
    write_u32(bytes, 0x98U + 56U, 0x3000U);
    write_u32(bytes, 0x98U + 60U, 0x400U);
    write_u32(bytes, 0x98U + 108U, 16U);
    write_text(bytes, 0x188U, ".text");
    write_u32(bytes, 0x188U + 8U, 0x200U);
    write_u32(bytes, 0x188U + 12U, 0x1000U);
    write_u32(bytes, 0x188U + 16U, 0x200U);
    write_u32(bytes, 0x188U + 20U, 0x400U);
    write_u32(bytes, 0x188U + 36U, 0x60000020U);

    const auto parsed = PeReader::read(std::span<const std::byte>{bytes});
    assert(parsed.ok());

    const auto result = RttiScanner::scan(std::span<const std::byte>{bytes}, *parsed.image);
    assert(result.classes.empty());
    assert(result.type_descriptors == 0U);
    assert(!result.warnings.empty());
}

void decorated_names_are_reconstructed() {
    assert(reconstruct_type_name(".?AVCActor@@").display == "CActor");
    assert(reconstruct_type_name(".?AVCActor@@").complete);
    assert(reconstruct_type_name(".?AUIUnknown@@").display == "IUnknown");

    // Qualifiers are recorded innermost first.
    assert(reconstruct_type_name(".?AVFullMotionVideo@DMC3@@").display ==
           "DMC3::FullMotionVideo");
    assert(reconstruct_type_name(".?AVInner@Middle@Outer@@").display ==
           "Outer::Middle::Inner");

    assert(reconstruct_type_name(".?AW4Difficulty@@").display == "Difficulty");
    assert(reconstruct_type_name(".?AV?$CList@VCLockOnTarget@@@@").display ==
           "CList<CLockOnTarget>");
    assert(reconstruct_type_name(".?AV?$CList@VCLockOnTarget@@@@").complete);
    assert(reconstruct_type_name(".?AV?$CPair@HM@@").display == "CPair<int, float>");
    assert(reconstruct_type_name(".?AV?$CHolder@PEAVCActor@@@@").display ==
           "CHolder<CActor*>");
}

void an_unmodelled_name_is_preserved_and_flagged() {
    const auto unknown = reconstruct_type_name("not-a-decorated-name");
    assert(!unknown.complete);
    assert(unknown.display == "not-a-decorated-name");

    // An unmodelled template argument keeps its text rather than vanishing.
    const auto partial = reconstruct_type_name(".?AV?$CBuffer@$$QEAH@@");
    assert(!partial.complete);
    assert(partial.display.find("CBuffer<") == 0U);
}

} // namespace

int main() {
    the_class_graph_is_recovered();
    one_type_keeps_every_subobject_vtable();
    a_locator_without_a_matching_descriptor_is_ignored();
    a_locator_that_does_not_point_at_itself_is_rejected();
    a_vtable_stops_at_the_first_non_code_slot();
    an_image_without_rtti_reports_nothing();
    decorated_names_are_reconstructed();
    an_unmodelled_name_is_preserved_and_flagged();
    return 0;
}
