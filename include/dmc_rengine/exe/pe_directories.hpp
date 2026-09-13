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

    [[nodiscard]] std::uint32_t size() const noexcept {
        return end_rva > begin_rva ? end_rva - begin_rva : 0U;
    }

    friend bool operator==(const PeFunctionRange&, const PeFunctionRange&) = default;
};

struct PeFunctionTable final {
    std::vector<PeFunctionRange> functions;
    std::uint32_t smallest_size{};
    std::uint32_t largest_size{};
    std::uint64_t covered_bytes{};
    bool sorted_by_address{true};

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
