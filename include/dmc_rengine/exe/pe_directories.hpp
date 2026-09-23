#pragma once

#include "dmc_rengine/exe/pe_image.hpp"

#include <array>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <vector>

namespace dmc::rengine::exe {

struct PeImportedFunction final {
    std::string name;
    std::uint32_t iat_rva{};
    std::uint16_t hint{};
    std::uint16_t ordinal{};
    bool by_ordinal{false};

    friend bool operator==(const PeImportedFunction&, const PeImportedFunction&) = default;
};

struct PeImportedModule final {
    std::string name;
    std::vector<PeImportedFunction> functions;
    std::uint32_t lookup_table_rva{};
    std::uint32_t address_table_rva{};
    bool delay_loaded{false};

    friend bool operator==(const PeImportedModule&, const PeImportedModule&) = default;
};

struct PeExportedSymbol final {
    std::string name;
    /// Set when the export forwards to another module instead of an address.
    std::string forwarder;
    std::uint32_t ordinal{};
    std::uint32_t rva{};

    [[nodiscard]] bool forwarded() const noexcept {
        return !forwarder.empty();
    }

    friend bool operator==(const PeExportedSymbol&, const PeExportedSymbol&) = default;
};

struct PeExportTable final {
    std::string module_name;
    std::uint32_t ordinal_base{};
    std::uint32_t address_count{};
    std::vector<PeExportedSymbol> symbols;

    friend bool operator==(const PeExportTable&, const PeExportTable&) = default;
};

/// Prologue facts recovered from an x64 `UNWIND_INFO` record.
///
/// The unwind codes describe exactly what the prologue did: which non-volatile
/// registers it pushed, how much stack it reserved, whether it established a
/// frame pointer. That is real per-function structure, recovered without
/// interpreting a single instruction.
struct PeUnwindFrame final {
    std::uint8_t version{};
    std::uint8_t flags{};
    std::uint8_t prolog_size{};
    std::uint8_t code_count{};
    /// Register number establishing the frame pointer, or zero for none.
    std::uint8_t frame_register{};
    std::uint8_t frame_offset{};

    /// Bytes reserved by the prologue's stack allocation operations.
    std::uint32_t stack_allocation{};
    std::uint8_t pushed_registers{};
    std::uint8_t saved_registers{};
    std::uint8_t saved_xmm{};

    bool has_exception_handler{false};
    /// When `has_exception_handler`: the handler routine, and the RVA of the
    /// language-specific data that follows it. Both zero otherwise. The data's
    /// shape depends on the handler - a C++ `FuncInfo` pointer, an SEH scope
    /// table, a stack-cookie offset - so it is recorded raw and interpreted by
    /// whoever knows which handler it is.
    std::uint32_t handler_rva{};
    std::uint32_t handler_data_rva{};
    bool machine_frame{false};
    /// False when the code array could not be walked to its end; the counts
    /// gathered so far remain valid but are incomplete.
    bool decoded{false};

    [[nodiscard]] bool uses_frame_pointer() const noexcept {
        return frame_register != 0U;
    }

    friend bool operator==(const PeUnwindFrame&, const PeUnwindFrame&) = default;
};

/// One `RUNTIME_FUNCTION` of an x64 exception directory.
///
/// The table covers every function the compiler emitted unwind data for, which
/// on a normally built MSVC image is the whole code section. It is therefore
/// the closest thing a stripped binary has to a function inventory — but it is
/// an inventory of *ranges*, not of names or semantics.
struct PeFunctionRange final {
    std::uint32_t begin_rva{};
    std::uint32_t end_rva{};
    std::uint32_t unwind_rva{};

    /// True when the unwind info sets `UNW_FLAG_CHAININFO`: this range is a
    /// continuation of another function rather than a function of its own.
    /// MSVC splits a function across several entries when the linker separates
    /// its blocks, so entry count overstates function count.
    bool chained{false};
    /// Entry range of the function this range belongs to. Equals `begin_rva`
    /// for a primary range.
    std::uint32_t primary_begin_rva{};
    PeUnwindFrame frame{};

    [[nodiscard]] bool primary() const noexcept {
        return !chained;
    }

    [[nodiscard]] std::uint32_t size() const noexcept {
        return end_rva > begin_rva ? end_rva - begin_rva : 0U;
    }

    friend bool operator==(const PeFunctionRange&, const PeFunctionRange&) = default;
};

struct PeFunctionTable final {
    /// One entry per `RUNTIME_FUNCTION`, including chained continuations.
    std::vector<PeFunctionRange> functions;
    std::uint32_t smallest_size{};
    std::uint32_t largest_size{};
    std::uint64_t covered_bytes{};
    bool sorted_by_address{true};

    /// Ranges whose unwind info makes them a function entry.
    std::uint32_t primary_functions{};
    std::uint32_t chained_ranges{};
    /// Ranges whose unwind info could not be read; these are treated as
    /// primary rather than silently attached to a neighbour.
    std::uint32_t unwind_unreadable{};
    /// Functions whose prologue establishes a frame pointer.
    std::uint32_t frame_pointer_functions{};
    /// Functions carrying an exception or termination handler.
    std::uint32_t handler_functions{};
    std::uint64_t total_stack_allocation{};
    std::uint32_t largest_stack_allocation{};

    friend bool operator==(const PeFunctionTable&, const PeFunctionTable&) = default;
};

/// CodeView (RSDS) build identity from the debug directory.
///
/// This is the strongest version fingerprint a shipped binary carries: two
/// builds of the same source differ here. It records where the PDB was, never
/// its contents.
struct PeCodeViewIdentity final {
    std::array<std::uint8_t, 16> guid{};
    std::uint32_t age{};
    std::string pdb_path;

    [[nodiscard]] std::string guid_string() const;

    friend bool operator==(const PeCodeViewIdentity&, const PeCodeViewIdentity&) = default;
};

struct PeDebugEntry final {
    std::uint32_t type{};
    std::uint32_t size_of_data{};
    std::uint32_t address_of_raw_data{};
    std::uint32_t pointer_to_raw_data{};
    std::uint32_t timestamp{};

    friend bool operator==(const PeDebugEntry&, const PeDebugEntry&) = default;
};

[[nodiscard]] std::string_view debug_type_name(std::uint32_t type) noexcept;

struct PeRelocationSummary final {
    std::uint32_t block_count{};
    std::uint64_t entry_count{};
    std::uint64_t absolute_entries{};
    std::uint64_t dir64_entries{};
    std::uint64_t other_entries{};
    std::uint32_t lowest_rva{};
    std::uint32_t highest_rva{};

    friend bool operator==(const PeRelocationSummary&, const PeRelocationSummary&) = default;
};

struct PeTlsDirectory final {
    std::uint64_t raw_data_start_va{};
    std::uint64_t raw_data_end_va{};
    std::uint64_t index_va{};
    std::uint64_t callbacks_va{};
    std::uint32_t zero_fill_size{};
    std::uint32_t characteristics{};
    std::vector<std::uint64_t> callbacks;

    friend bool operator==(const PeTlsDirectory&, const PeTlsDirectory&) = default;
};

struct PeResourceTypeCount final {
    std::uint32_t type_id{};
    std::string type_name;
    std::uint32_t entry_count{};

    friend bool operator==(const PeResourceTypeCount&, const PeResourceTypeCount&) = default;
};

/// Everything the directories of one image describe.
struct PeDirectories final {
    std::vector<PeImportedModule> imports;
    std::optional<PeExportTable> exports;
    std::optional<PeFunctionTable> functions;
    std::vector<PeDebugEntry> debug_entries;
    std::optional<PeCodeViewIdentity> codeview;
    std::optional<PeRelocationSummary> relocations;
    std::optional<PeTlsDirectory> tls;
    std::vector<PeResourceTypeCount> resource_types;

    [[nodiscard]] std::size_t imported_function_count() const noexcept;
};

struct PeDirectoryReadResult final {
    PeDirectories directories;
    std::vector<std::string> errors;
    std::vector<std::string> warnings;

    [[nodiscard]] bool ok() const noexcept {
        return errors.empty();
    }
};

/// Reads the directory tables of an already-parsed image.
///
/// Every table is optional and independently recoverable: a malformed import
/// descriptor does not cost the caller the exception table. Failures are
/// reported per directory rather than aborting the read, because a partially
/// readable image is still evidence.
class PeDirectoryReader final {
public:
    [[nodiscard]] static PeDirectoryReadResult read(std::span<const std::byte> bytes,
                                                    const PeImage& image);
};

} // namespace dmc::rengine::exe
