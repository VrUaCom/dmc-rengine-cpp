#include "dmc_rengine/exe/pe_directories.hpp"
#include "dmc_rengine/exe/pe_reader.hpp"

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <initializer_list>
#include <span>
#include <string>
#include <string_view>
#include <vector>

namespace {

using dmc::rengine::exe::PeDirectory;
using dmc::rengine::exe::PeDirectoryReader;
using dmc::rengine::exe::PeReader;

// ---------------------------------------------------------------------------
// Synthetic PE32+ fixture.
//
// Models: two imported modules (one importing by ordinal), an export table
// with one forwarder, a three-entry exception table, an RSDS debug record, one
// relocation block and a TLS directory with a callback. Every value is
// invented; no original executable bytes appear anywhere in this file.
//
// Layout: headers 0x000-0x400, .text at RVA 0x1000 (file 0x400),
// .rdata at RVA 0x2000 (file 0x600), so an .rdata RVA maps to `rva - 0x1A00`.
// ---------------------------------------------------------------------------

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

void put(std::vector<std::byte>& bytes, std::size_t offset, std::initializer_list<int> values) {
    std::size_t index = offset;
    for (const auto value : values) {
        bytes[index++] = static_cast<std::byte>(static_cast<unsigned char>(value));
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
    write_u16(bytes, 0x84U, 0x8664U);      // amd64
    write_u16(bytes, 0x86U, 2U);           // two sections
    write_u32(bytes, 0x88U, 0x5AB4CBC2U);  // timestamp
    write_u16(bytes, 0x94U, 0x00F0U);      // SizeOfOptionalHeader
    write_u16(bytes, 0x96U, 0x0022U);      // characteristics

    constexpr std::size_t optional = 0x98U;
    write_u16(bytes, optional, 0x020BU);
    write_u32(bytes, optional + 4U, 0x200U);    // SizeOfCode
    write_u32(bytes, optional + 8U, 0x600U);    // SizeOfInitializedData
    write_u32(bytes, optional + 16U, 0x1000U);  // entry point
    write_u64(bytes, optional + 24U, 0x140000000ULL);
    write_u32(bytes, optional + 32U, 0x1000U);  // SectionAlignment
    write_u32(bytes, optional + 36U, 0x200U);   // FileAlignment
    write_u32(bytes, optional + 56U, 0x3000U);  // SizeOfImage
    write_u32(bytes, optional + 60U, 0x400U);   // SizeOfHeaders
    write_u32(bytes, optional + 64U, 0x1234U);  // CheckSum
    write_u16(bytes, optional + 68U, 2U);       // subsystem
    write_u16(bytes, optional + 70U, 0x8160U);  // DllCharacteristics
    write_u32(bytes, optional + 108U, 16U);     // NumberOfRvaAndSizes

    const auto directory = [&](std::size_t index, std::uint32_t rva, std::uint32_t size) {
        const std::size_t entry = optional + 112U + index * 8U;
        write_u32(bytes, entry, rva);
        write_u32(bytes, entry + 4U, size);
    };
    directory(0U, 0x2120U, 0x100U);  // export (covers the forwarder string)
    directory(1U, 0x2000U, 60U);     // import
    directory(3U, 0x21C0U, 36U);     // exception
    directory(5U, 0x2260U, 12U);     // base relocation
    directory(6U, 0x2200U, 28U);     // debug
    directory(9U, 0x2280U, 40U);     // tls

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

    // ----- imports -------------------------------------------------------
    // Descriptor 0: ALPHA.dll with two named imports.
    write_u32(bytes, file_of(0x2000U), 0x2040U);        // lookup table
    write_u32(bytes, file_of(0x2000U) + 12U, 0x20C0U);  // module name
    write_u32(bytes, file_of(0x2000U) + 16U, 0x2060U);  // address table
    // Descriptor 1: BETA.dll, one named import and one by ordinal.
    write_u32(bytes, file_of(0x2014U), 0x2080U);
    write_u32(bytes, file_of(0x2014U) + 12U, 0x20D0U);
    write_u32(bytes, file_of(0x2014U) + 16U, 0x20A0U);
    // Descriptor 2 is the all-zero terminator already present.

    write_u64(bytes, file_of(0x2040U), 0x20E0U);
    write_u64(bytes, file_of(0x2040U) + 8U, 0x20F0U);
    write_u64(bytes, file_of(0x2040U) + 16U, 0U);

    write_u64(bytes, file_of(0x2080U), 0x2110U);
    write_u64(bytes, file_of(0x2080U) + 8U, (1ULL << 63U) | 0x0042U);  // by ordinal
    write_u64(bytes, file_of(0x2080U) + 16U, 0U);

    write_text(bytes, file_of(0x20C0U), "ALPHA.dll");
    write_text(bytes, file_of(0x20D0U), "BETA.dll");

    write_u16(bytes, file_of(0x20E0U), 7U);
    write_text(bytes, file_of(0x20E0U) + 2U, "alpha_one");
    write_u16(bytes, file_of(0x20F0U), 8U);
    write_text(bytes, file_of(0x20F0U) + 2U, "alpha_two");
    write_u16(bytes, file_of(0x2110U), 1U);
    write_text(bytes, file_of(0x2110U) + 2U, "beta_one");

    // ----- exports -------------------------------------------------------
    write_u32(bytes, file_of(0x2120U) + 12U, 0x2170U);  // module name
    write_u32(bytes, file_of(0x2120U) + 16U, 1U);       // ordinal base
    write_u32(bytes, file_of(0x2120U) + 20U, 2U);       // address count
    write_u32(bytes, file_of(0x2120U) + 24U, 2U);       // name count
    write_u32(bytes, file_of(0x2120U) + 28U, 0x2150U);  // address table
    write_u32(bytes, file_of(0x2120U) + 32U, 0x2158U);  // name pointer table
    write_u32(bytes, file_of(0x2120U) + 36U, 0x2160U);  // ordinal table

    write_u32(bytes, file_of(0x2150U), 0x1040U);        // real code address
    write_u32(bytes, file_of(0x2150U) + 4U, 0x21A0U);   // inside the directory: a forwarder
    write_u32(bytes, file_of(0x2158U), 0x2180U);
    write_u32(bytes, file_of(0x2158U) + 4U, 0x2190U);
    write_u16(bytes, file_of(0x2160U), 0U);
    write_u16(bytes, file_of(0x2160U) + 2U, 1U);

    write_text(bytes, file_of(0x2170U), "synth.dll");
    write_text(bytes, file_of(0x2180U), "synth_start");
    write_text(bytes, file_of(0x2190U), "synth_forward");
    write_text(bytes, file_of(0x21A0U), "OTHER.other_fn");

    // ----- exception table ------------------------------------------------
    write_u32(bytes, file_of(0x21C0U), 0x1000U);
    write_u32(bytes, file_of(0x21C0U) + 4U, 0x1040U);
    write_u32(bytes, file_of(0x21C0U) + 8U, 0x2300U);
    write_u32(bytes, file_of(0x21C0U) + 12U, 0x1040U);
    write_u32(bytes, file_of(0x21C0U) + 16U, 0x1100U);
    write_u32(bytes, file_of(0x21C0U) + 20U, 0x2310U);
    // Third entry is deliberately degenerate: end == begin.
    write_u32(bytes, file_of(0x21C0U) + 24U, 0x1100U);
    write_u32(bytes, file_of(0x21C0U) + 28U, 0x1100U);
    write_u32(bytes, file_of(0x21C0U) + 32U, 0x2320U);

    // ----- unwind info ----------------------------------------------------
    // Record for the first function: push one register, then reserve 88 bytes.
    put(bytes, file_of(0x2300U), {0x01, 0x12, 0x02, 0x00});
    put(bytes, file_of(0x2300U) + 4U, {0x12, 0x50});  // push non-volatile reg 5
    put(bytes, file_of(0x2300U) + 6U, {0x0E, 0xA2});  // alloc small: 10*8 + 8

    // Record for the second function: establishes a frame pointer and declares
    // an exception handler.
    put(bytes, file_of(0x2310U), {0x09, 0x10, 0x01, 0x25});
    put(bytes, file_of(0x2310U) + 4U, {0x04, 0x03});  // set frame pointer

    // ----- debug ----------------------------------------------------------
    write_u32(bytes, file_of(0x2200U) + 12U, 2U);       // CodeView
    write_u32(bytes, file_of(0x2200U) + 16U, 46U);      // size of data
    write_u32(bytes, file_of(0x2200U) + 20U, 0x2220U);  // rva
    write_u32(bytes, file_of(0x2200U) + 24U,
              static_cast<std::uint32_t>(file_of(0x2220U)));  // file pointer

    write_text(bytes, file_of(0x2220U), "RSDS");
    const std::uint8_t guid[16] = {0x58, 0xCD, 0xAC, 0x8D, 0xB6, 0x89, 0x0C, 0x4E,
                                   0x9B, 0x74, 0xEC, 0x2A, 0x12, 0x92, 0xFC, 0x90};
    for (std::size_t index = 0; index < 16U; ++index) {
        bytes[file_of(0x2220U) + 4U + index] = static_cast<std::byte>(guid[index]);
    }
    write_u32(bytes, file_of(0x2220U) + 20U, 1U);
    write_text(bytes, file_of(0x2220U) + 24U, "C:\\synthetic\\test.pdb");

    // ----- relocations ----------------------------------------------------
    write_u32(bytes, file_of(0x2260U), 0x1000U);  // page RVA
    write_u32(bytes, file_of(0x2260U) + 4U, 12U); // block size
    write_u16(bytes, file_of(0x2260U) + 8U, static_cast<std::uint16_t>((10U << 12U) | 0x0010U));
    write_u16(bytes, file_of(0x2260U) + 10U, 0U); // padding entry, type 0

    // ----- TLS ------------------------------------------------------------
    write_u64(bytes, file_of(0x2280U), 0x140003000ULL);
    write_u64(bytes, file_of(0x2280U) + 8U, 0x140003100ULL);
    write_u64(bytes, file_of(0x2280U) + 16U, 0x140003200ULL);
    write_u64(bytes, file_of(0x2280U) + 24U, 0x1400022C0ULL);  // callbacks
    write_u32(bytes, file_of(0x2280U) + 32U, 0x40U);
    write_u64(bytes, file_of(0x22C0U), 0x140001040ULL);
    write_u64(bytes, file_of(0x22C0U) + 8U, 0U);

    return bytes;
}

[[nodiscard]] dmc::rengine::exe::PeImage read_image(const std::vector<std::byte>& bytes) {
    const auto parsed = PeReader::read(std::span<const std::byte>{bytes});
    assert(parsed.ok());
    return *parsed.image;
}

void header_fields_and_directories_are_parsed() {
    const auto bytes = make_fixture();
    const auto image = read_image(bytes);

    assert(image.timestamp == 0x5AB4CBC2U);
    assert(image.characteristics == 0x0022U);
    assert(image.dll_characteristics == 0x8160U);
    assert(image.section_alignment == 0x1000U);
    assert(image.file_alignment == 0x200U);
    assert(image.checksum == 0x1234U);
    assert(image.size_of_code == 0x200U);
    assert(image.data_directories.size() == 16U);

    assert(image.directory(PeDirectory::import_table).rva == 0x2000U);
    assert(image.directory(PeDirectory::exception_table).size == 36U);
    assert(!image.directory(PeDirectory::clr_runtime_header).present());
}

void imports_are_recovered_including_ordinals() {
    const auto bytes = make_fixture();
    const auto image = read_image(bytes);
    const auto result = PeDirectoryReader::read(std::span<const std::byte>{bytes}, image);

    assert(result.ok());
    const auto& imports = result.directories.imports;
    assert(imports.size() == 2U);
    assert(imports[0].name == "ALPHA.dll");
    assert(imports[0].functions.size() == 2U);
    assert(imports[0].functions[0].name == "alpha_one");
    assert(imports[0].functions[0].hint == 7U);
    assert(!imports[0].functions[0].by_ordinal);
    assert(imports[0].functions[0].iat_rva == 0x2060U);
    assert(imports[0].functions[1].iat_rva == 0x2068U);

    assert(imports[1].name == "BETA.dll");
    assert(imports[1].functions.size() == 2U);
    assert(imports[1].functions[1].by_ordinal);
    assert(imports[1].functions[1].ordinal == 0x42U);
    assert(imports[1].functions[1].name.empty());

    assert(result.directories.imported_function_count() == 4U);
}

void exports_distinguish_addresses_from_forwarders() {
    const auto bytes = make_fixture();
    const auto image = read_image(bytes);
    const auto result = PeDirectoryReader::read(std::span<const std::byte>{bytes}, image);

    assert(result.directories.exports.has_value());
    const auto& table = *result.directories.exports;
    assert(table.module_name == "synth.dll");
    assert(table.ordinal_base == 1U);
    assert(table.symbols.size() == 2U);

    assert(table.symbols[0].name == "synth_start");
    assert(table.symbols[0].ordinal == 1U);
    assert(table.symbols[0].rva == 0x1040U);
    assert(!table.symbols[0].forwarded());

    // An address inside the export directory is a forwarder string.
    assert(table.symbols[1].name == "synth_forward");
    assert(table.symbols[1].forwarded());
    assert(table.symbols[1].forwarder == "OTHER.other_fn");
}

void the_exception_table_is_a_function_inventory() {
    const auto bytes = make_fixture();
    const auto image = read_image(bytes);
    const auto result = PeDirectoryReader::read(std::span<const std::byte>{bytes}, image);

    assert(result.directories.functions.has_value());
    const auto& table = *result.directories.functions;

    // The degenerate third entry is rejected rather than counted as a function.
    assert(table.functions.size() == 2U);
    assert(table.functions[0].begin_rva == 0x1000U);
    assert(table.functions[0].size() == 0x40U);
    assert(table.functions[1].size() == 0xC0U);
    assert(table.covered_bytes == 0x100U);
    assert(table.smallest_size == 0x40U);
    assert(table.largest_size == 0xC0U);
    assert(table.sorted_by_address);
}

void codeview_identity_is_recovered() {
    const auto bytes = make_fixture();
    const auto image = read_image(bytes);
    const auto result = PeDirectoryReader::read(std::span<const std::byte>{bytes}, image);

    assert(result.directories.debug_entries.size() == 1U);
    assert(result.directories.debug_entries[0].type == 2U);
    assert(dmc::rengine::exe::debug_type_name(2U) == "codeview");

    assert(result.directories.codeview.has_value());
    assert(result.directories.codeview->age == 1U);
    assert(result.directories.codeview->pdb_path == "C:\\synthetic\\test.pdb");

    // Windows prints the first three GUID fields byte swapped.
    assert(result.directories.codeview->guid_string() == "8DACCD58-89B6-4E0C-9B74-EC2A1292FC90");
}

void relocations_and_tls_are_summarized() {
    const auto bytes = make_fixture();
    const auto image = read_image(bytes);
    const auto result = PeDirectoryReader::read(std::span<const std::byte>{bytes}, image);

    assert(result.directories.relocations.has_value());
    const auto& relocations = *result.directories.relocations;
    assert(relocations.block_count == 1U);
    assert(relocations.entry_count == 2U);
    assert(relocations.dir64_entries == 1U);
    assert(relocations.absolute_entries == 1U);
    assert(relocations.lowest_rva == 0x1010U);
    assert(relocations.highest_rva == 0x1010U);

    assert(result.directories.tls.has_value());
    assert(result.directories.tls->index_va == 0x140003200ULL);
    assert(result.directories.tls->zero_fill_size == 0x40U);
    assert(result.directories.tls->callbacks.size() == 1U);
    assert(result.directories.tls->callbacks[0] == 0x140001040ULL);
}

void unwind_records_describe_each_prologue() {
    const auto bytes = make_fixture();
    const auto image = read_image(bytes);
    const auto result = PeDirectoryReader::read(std::span<const std::byte>{bytes}, image);

    const auto& table = *result.directories.functions;
    assert(table.functions.size() == 2U);

    const auto& first = table.functions[0].frame;
    assert(first.decoded);
    assert(first.version == 1U);
    assert(first.prolog_size == 0x12U);
    assert(first.pushed_registers == 1U);
    // Eight bytes for the push, eighty-eight for the small allocation.
    assert(first.stack_allocation == 96U);
    assert(!first.uses_frame_pointer());
    assert(!first.has_exception_handler);

    const auto& second = table.functions[1].frame;
    assert(second.decoded);
    assert(second.uses_frame_pointer());
    assert(second.frame_register == 5U);
    assert(second.frame_offset == 2U);
    assert(second.has_exception_handler);
    assert(second.stack_allocation == 0U);
    assert(second.pushed_registers == 0U);

    assert(table.primary_functions == 2U);
    assert(table.chained_ranges == 0U);
    assert(table.frame_pointer_functions == 1U);
    assert(table.handler_functions == 1U);
    assert(table.total_stack_allocation == 96U);
    assert(table.largest_stack_allocation == 96U);
}

void a_large_stack_allocation_is_read_from_its_slot() {
    auto bytes = make_fixture();

    // Replace the first record with a single large allocation of 0x200 slots.
    put(bytes, file_of(0x2300U), {0x01, 0x08, 0x02, 0x00});
    put(bytes, file_of(0x2300U) + 4U, {0x08, 0x01});  // alloc large, info 0
    put(bytes, file_of(0x2300U) + 6U, {0x00, 0x02});  // 0x200 slots

    const auto image = read_image(bytes);
    const auto result = PeDirectoryReader::read(std::span<const std::byte>{bytes}, image);

    const auto& frame = result.directories.functions->functions[0].frame;
    assert(frame.decoded);
    assert(frame.stack_allocation == 0x200U * 8U);
    assert(frame.pushed_registers == 0U);
}

void a_chained_range_is_attributed_to_its_parent() {
    auto bytes = make_fixture();

    // Make the third exception entry a real range that chains to the first.
    write_u32(bytes, file_of(0x21C0U) + 28U, 0x1140U);  // non-degenerate end
    put(bytes, file_of(0x2320U), {0x21, 0x00, 0x00, 0x00});  // version 1, CHAININFO
    write_u32(bytes, file_of(0x2320U) + 4U, 0x1000U);        // parent begin
    write_u32(bytes, file_of(0x2320U) + 8U, 0x1040U);        // parent end
    write_u32(bytes, file_of(0x2320U) + 12U, 0x2300U);       // parent unwind

    const auto image = read_image(bytes);
    const auto result = PeDirectoryReader::read(std::span<const std::byte>{bytes}, image);

    const auto& table = *result.directories.functions;
    assert(table.functions.size() == 3U);
    assert(table.primary_functions == 2U);
    assert(table.chained_ranges == 1U);

    const auto& continuation = table.functions[2];
    assert(continuation.chained);
    assert(!continuation.primary());
    assert(continuation.primary_begin_rva == 0x1000U);

    // A primary range names itself.
    assert(table.functions[0].primary());
    assert(table.functions[0].primary_begin_rva == 0x1000U);
}

void a_truncated_directory_degrades_without_losing_the_others() {
    auto bytes = make_fixture();

    // Point the import directory at an RVA with no file backing. The exception
    // table must still be recovered.
    constexpr std::size_t optional = 0x98U;
    write_u32(bytes, optional + 112U + 8U, 0x0FFF0000U);

    const auto image = read_image(bytes);
    const auto result = PeDirectoryReader::read(std::span<const std::byte>{bytes}, image);

    assert(!result.ok());
    assert(!result.errors.empty());
    assert(result.directories.imports.empty());
    assert(result.directories.functions.has_value());
    assert(result.directories.functions->functions.size() == 2U);
    assert(result.directories.exports.has_value());
}

void an_image_without_directories_is_not_an_error() {
    dmc::rengine::exe::PeImage bare;
    bare.kind = dmc::rengine::exe::PeKind::pe32_plus;
    const std::vector<std::byte> empty;

    const auto result = PeDirectoryReader::read(std::span<const std::byte>{empty}, bare);
    assert(result.ok());
    assert(result.warnings.size() == 1U);
    assert(result.directories.imports.empty());
}

} // namespace

int main() {
    header_fields_and_directories_are_parsed();
    imports_are_recovered_including_ordinals();
    exports_distinguish_addresses_from_forwarders();
    the_exception_table_is_a_function_inventory();
    codeview_identity_is_recovered();
    unwind_records_describe_each_prologue();
    a_large_stack_allocation_is_read_from_its_slot();
    a_chained_range_is_attributed_to_its_parent();
    relocations_and_tls_are_summarized();
    a_truncated_directory_degrades_without_losing_the_others();
    an_image_without_directories_is_not_an_error();
    return 0;
}
