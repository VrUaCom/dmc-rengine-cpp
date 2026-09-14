#include "dmc_rengine/exe/pe_directories.hpp"

#include "pe_byte_access.hpp"

#include <algorithm>
#include <array>
#include <cstdio>
#include <limits>
#include <map>
#include <string>
#include <utility>

namespace dmc::rengine::exe {
namespace {

using detail::has_range;
using detail::read_cstring;
using detail::read_u8;
using detail::read_u16;
using detail::read_u32;
using detail::read_u64;

// Safety ceilings. A malformed or hostile image must cost bounded work, so
// every walk stops at a limit far above anything a real toolchain emits.
constexpr std::size_t kMaxImportModules = 4096U;
constexpr std::size_t kMaxImportsPerModule = 65536U;
constexpr std::size_t kMaxExports = 65536U;
constexpr std::size_t kMaxFunctions = 1U << 21U;
constexpr std::size_t kMaxDebugEntries = 256U;
constexpr std::size_t kMaxRelocationBlocks = 1U << 20U;
constexpr std::size_t kMaxTlsCallbacks = 4096U;
constexpr std::size_t kMaxResourceEntries = 65536U;
constexpr std::size_t kResourceDepthLimit = 4U;

/// Resolves an RVA to a readable file offset, or nothing when the RVA has no
/// file-backed data (a virtual-only tail, for instance).
[[nodiscard]] std::optional<std::size_t> offset_of(std::span<const std::byte> bytes,
                                                   const PeImage& image, std::uint32_t rva) {
    const auto mapped = image.rva_to_file_offset(rva);
    if (!mapped.has_value()) {
        return std::nullopt;
    }
    const auto offset = static_cast<std::size_t>(*mapped);
    if (offset >= bytes.size()) {
        return std::nullopt;
    }
    return offset;
}

[[nodiscard]] std::optional<std::string> string_at_rva(std::span<const std::byte> bytes,
                                                       const PeImage& image, std::uint32_t rva) {
    const auto offset = offset_of(bytes, image, rva);
    if (!offset.has_value()) {
        return std::nullopt;
    }
    return read_cstring(bytes, *offset);
}

[[nodiscard]] std::uint32_t va_to_rva(const PeImage& image, std::uint64_t va) noexcept {
    if (va <= image.image_base) {
        return 0U;
    }
    const auto delta = va - image.image_base;
    if (delta > std::numeric_limits<std::uint32_t>::max()) {
        return 0U;
    }
    return static_cast<std::uint32_t>(delta);
}

void read_import_thunks(std::span<const std::byte> bytes, const PeImage& image,
                        PeImportedModule& module, std::uint32_t lookup_rva,
                        std::uint32_t address_rva, PeDirectoryReadResult& result) {
    // Prefer the lookup table: it survives binding, while the address table is
    // overwritten with resolved pointers in a bound image.
    const std::uint32_t walk_rva = lookup_rva != 0U ? lookup_rva : address_rva;
    if (walk_rva == 0U) {
        result.warnings.emplace_back("Import '" + module.name + "' has no thunk table.");
        return;
    }

    const auto base_offset = offset_of(bytes, image, walk_rva);
    if (!base_offset.has_value()) {
        result.warnings.emplace_back("Import '" + module.name +
                                     "' thunk table does not map to file data.");
        return;
    }

    const bool wide = image.kind == PeKind::pe32_plus;
    const std::size_t entry_size = wide ? 8U : 4U;
    const std::uint64_t ordinal_flag = wide ? (1ULL << 63U) : (1ULL << 31U);

    for (std::size_t index = 0; index < kMaxImportsPerModule; ++index) {
        const std::size_t entry_offset = *base_offset + index * entry_size;
        const auto raw = wide ? read_u64(bytes, entry_offset)
                              : [&]() -> std::optional<std::uint64_t> {
            const auto narrow = read_u32(bytes, entry_offset);
            if (!narrow.has_value()) {
                return std::nullopt;
            }
            return static_cast<std::uint64_t>(*narrow);
        }();

        if (!raw.has_value()) {
            result.warnings.emplace_back("Import '" + module.name + "' thunk table is truncated.");
            return;
        }
        if (*raw == 0U) {
            return;
        }

        PeImportedFunction function;
        function.iat_rva =
            address_rva == 0U
                ? 0U
                : static_cast<std::uint32_t>(address_rva + index * entry_size);

        if ((*raw & ordinal_flag) != 0U) {
            function.by_ordinal = true;
            function.ordinal = static_cast<std::uint16_t>(*raw & 0xFFFFU);
        } else {
            const auto hint_name_rva = static_cast<std::uint32_t>(*raw & 0x7FFFFFFFU);
            const auto hint_offset = offset_of(bytes, image, hint_name_rva);
            if (!hint_offset.has_value()) {
                result.warnings.emplace_back("Import '" + module.name +
                                             "' names an unmapped hint/name entry.");
                continue;
            }
            function.hint = read_u16(bytes, *hint_offset).value_or(0U);
            auto name = read_cstring(bytes, *hint_offset + 2U);
            if (!name.has_value()) {
                result.warnings.emplace_back("Import '" + module.name +
                                             "' has an unterminated function name.");
                continue;
            }
            function.name = std::move(*name);
        }

        module.functions.push_back(std::move(function));
    }

    result.warnings.emplace_back("Import '" + module.name + "' exceeded the thunk safety limit.");
}

void read_imports(std::span<const std::byte> bytes, const PeImage& image,
                  PeDirectoryReadResult& result) {
    const auto directory = image.directory(PeDirectory::import_table);
    if (!directory.present()) {
        return;
    }

    const auto base_offset = offset_of(bytes, image, directory.rva);
    if (!base_offset.has_value()) {
        result.errors.emplace_back("Import directory does not map to file data.");
        return;
    }

    constexpr std::size_t descriptor_size = 20U;
    for (std::size_t index = 0; index < kMaxImportModules; ++index) {
        const std::size_t offset = *base_offset + index * descriptor_size;
        const auto lookup_rva = read_u32(bytes, offset);
        const auto name_rva = read_u32(bytes, offset + 12U);
        const auto address_rva = read_u32(bytes, offset + 16U);
        if (!lookup_rva.has_value() || !name_rva.has_value() || !address_rva.has_value()) {
            result.errors.emplace_back("Import descriptor table is truncated.");
            return;
        }
        if (*lookup_rva == 0U && *name_rva == 0U && *address_rva == 0U) {
            return;
        }

        PeImportedModule module;
        module.lookup_table_rva = *lookup_rva;
        module.address_table_rva = *address_rva;
        auto name = string_at_rva(bytes, image, *name_rva);
        module.name = name.value_or("<unnamed>");
        if (!name.has_value()) {
            result.warnings.emplace_back("Import descriptor has an unreadable module name.");
        }

        read_import_thunks(bytes, image, module, *lookup_rva, *address_rva, result);
        result.directories.imports.push_back(std::move(module));
    }

    result.warnings.emplace_back("Import descriptor table exceeded the safety limit.");
}

void read_delay_imports(std::span<const std::byte> bytes, const PeImage& image,
                        PeDirectoryReadResult& result) {
    const auto directory = image.directory(PeDirectory::delay_import_descriptor);
    if (!directory.present()) {
        return;
    }

    const auto base_offset = offset_of(bytes, image, directory.rva);
    if (!base_offset.has_value()) {
        result.warnings.emplace_back("Delay-import directory does not map to file data.");
        return;
    }

    constexpr std::size_t descriptor_size = 32U;
    for (std::size_t index = 0; index < kMaxImportModules; ++index) {
        const std::size_t offset = *base_offset + index * descriptor_size;
        const auto attributes = read_u32(bytes, offset);
        const auto name_field = read_u32(bytes, offset + 4U);
        const auto address_field = read_u32(bytes, offset + 12U);
        const auto lookup_field = read_u32(bytes, offset + 16U);
        if (!attributes.has_value() || !name_field.has_value() || !address_field.has_value() ||
            !lookup_field.has_value()) {
            result.warnings.emplace_back("Delay-import descriptor table is truncated.");
            return;
        }
        if (*name_field == 0U && *address_field == 0U && *lookup_field == 0U) {
            return;
        }

        // Attribute bit 0 marks RVA-based descriptors. Older linkers stored
        // virtual addresses in the same fields, so convert when it is clear.
        const bool rva_based = (*attributes & 1U) != 0U;
        const auto to_rva = [&](std::uint32_t field) {
            return rva_based ? field : va_to_rva(image, field);
        };

        PeImportedModule module;
        module.delay_loaded = true;
        module.lookup_table_rva = to_rva(*lookup_field);
        module.address_table_rva = to_rva(*address_field);
        auto name = string_at_rva(bytes, image, to_rva(*name_field));
        module.name = name.value_or("<unnamed>");
        if (!name.has_value()) {
            result.warnings.emplace_back("Delay-import descriptor has an unreadable module name.");
        }

        read_import_thunks(bytes, image, module, module.lookup_table_rva,
                           module.address_table_rva, result);
        result.directories.imports.push_back(std::move(module));
    }

    result.warnings.emplace_back("Delay-import descriptor table exceeded the safety limit.");
}

void read_exports(std::span<const std::byte> bytes, const PeImage& image,
                  PeDirectoryReadResult& result) {
    const auto directory = image.directory(PeDirectory::export_table);
    if (!directory.present()) {
        return;
    }

    const auto base_offset = offset_of(bytes, image, directory.rva);
    if (!base_offset.has_value()) {
        result.errors.emplace_back("Export directory does not map to file data.");
        return;
    }

    const auto name_rva = read_u32(bytes, *base_offset + 12U);
    const auto ordinal_base = read_u32(bytes, *base_offset + 16U);
    const auto address_count = read_u32(bytes, *base_offset + 20U);
    const auto name_count = read_u32(bytes, *base_offset + 24U);
    const auto address_table_rva = read_u32(bytes, *base_offset + 28U);
    const auto name_table_rva = read_u32(bytes, *base_offset + 32U);
    const auto ordinal_table_rva = read_u32(bytes, *base_offset + 36U);
    if (!name_rva.has_value() || !ordinal_base.has_value() || !address_count.has_value() ||
        !name_count.has_value() || !address_table_rva.has_value() ||
        !name_table_rva.has_value() || !ordinal_table_rva.has_value()) {
        result.errors.emplace_back("Export directory is truncated.");
        return;
    }

    if (*address_count > kMaxExports || *name_count > kMaxExports) {
        result.errors.emplace_back("Export table exceeds the safety limit.");
        return;
    }

    PeExportTable table;
    table.ordinal_base = *ordinal_base;
    table.address_count = *address_count;
    table.module_name = string_at_rva(bytes, image, *name_rva).value_or("<unnamed>");

    const auto address_offset = offset_of(bytes, image, *address_table_rva);
    const auto name_offset = offset_of(bytes, image, *name_table_rva);
    const auto ordinal_offset = offset_of(bytes, image, *ordinal_table_rva);
    if (!address_offset.has_value()) {
        result.errors.emplace_back("Export address table does not map to file data.");
        return;
    }

    const std::uint64_t directory_begin = directory.rva;
    const std::uint64_t directory_end = directory_begin + directory.size;

    for (std::size_t index = 0; index < static_cast<std::size_t>(*name_count); ++index) {
        if (!name_offset.has_value() || !ordinal_offset.has_value()) {
            result.warnings.emplace_back("Export name table is missing; named exports skipped.");
            break;
        }

        const auto symbol_name_rva = read_u32(bytes, *name_offset + index * 4U);
        const auto ordinal_index = read_u16(bytes, *ordinal_offset + index * 2U);
        if (!symbol_name_rva.has_value() || !ordinal_index.has_value()) {
            result.warnings.emplace_back("Export name or ordinal table is truncated.");
            break;
        }
        if (static_cast<std::uint32_t>(*ordinal_index) >= *address_count) {
            result.warnings.emplace_back("Export ordinal index is out of range.");
            continue;
        }

        const auto function_rva =
            read_u32(bytes, *address_offset + static_cast<std::size_t>(*ordinal_index) * 4U);
        if (!function_rva.has_value()) {
            result.warnings.emplace_back("Export address table is truncated.");
            break;
        }

        PeExportedSymbol symbol;
        symbol.ordinal = *ordinal_base + *ordinal_index;
        symbol.rva = *function_rva;
        symbol.name = string_at_rva(bytes, image, *symbol_name_rva).value_or("<unnamed>");

        // An address inside the export directory is a forwarder string, not code.
        if (*function_rva >= directory_begin && *function_rva < directory_end) {
            symbol.forwarder = string_at_rva(bytes, image, *function_rva).value_or(std::string{});
        }

        table.symbols.push_back(std::move(symbol));
    }

    result.directories.exports = std::move(table);
}

/// Reads one `UNWIND_INFO` and, when it chains, the parent `RUNTIME_FUNCTION`.
///
/// Layout: a four-byte header, then `CountOfCodes` two-byte unwind codes padded
/// to an even count, then — when `UNW_FLAG_CHAININFO` is set — the parent
/// entry. Handler data occupies the same slot for the handler flags, so the
/// chained form is the only one this reads.
[[nodiscard]] std::optional<std::uint32_t> read_chained_parent(std::span<const std::byte> bytes,
                                                               const PeImage& image,
                                                               std::uint32_t unwind_rva,
                                                               bool& readable) {
    readable = false;
    const auto offset = offset_of(bytes, image, unwind_rva);
    if (!offset.has_value()) {
        return std::nullopt;
    }

    const auto version_flags = read_u8(bytes, *offset);
    const auto code_count = read_u8(bytes, *offset + 2U);
    if (!version_flags.has_value() || !code_count.has_value()) {
        return std::nullopt;
    }
    readable = true;

    constexpr std::uint8_t chain_info = 0x04U;
    const auto flags = static_cast<std::uint8_t>(*version_flags >> 3U);
    if ((flags & chain_info) == 0U) {
        return std::nullopt;
    }

    const std::size_t padded_codes = (static_cast<std::size_t>(*code_count) + 1U) & ~std::size_t{1U};
    const std::size_t parent_offset = *offset + 4U + padded_codes * 2U;
    const auto parent_begin = read_u32(bytes, parent_offset);
    if (!parent_begin.has_value()) {
        return std::nullopt;
    }
    return *parent_begin;
}

/// Groups continuation ranges under the function they belong to.
void resolve_chained_functions(std::span<const std::byte> bytes, const PeImage& image,
                               PeFunctionTable& table, PeDirectoryReadResult& result) {
    std::map<std::uint32_t, std::size_t> by_begin;
    for (std::size_t index = 0; index < table.functions.size(); ++index) {
        by_begin.emplace(table.functions[index].begin_rva, index);
    }

    std::vector<std::optional<std::uint32_t>> parents(table.functions.size());
    for (std::size_t index = 0; index < table.functions.size(); ++index) {
        bool readable = false;
        parents[index] =
            read_chained_parent(bytes, image, table.functions[index].unwind_rva, readable);
        if (!readable) {
            ++table.unwind_unreadable;
        }
    }

    for (std::size_t index = 0; index < table.functions.size(); ++index) {
        auto& range = table.functions[index];

        // Follow the chain to the function entry. The depth guard keeps a
        // malformed or circular chain from costing unbounded work.
        std::size_t current = index;
        std::size_t depth = 0U;
        constexpr std::size_t max_chain_depth = 64U;
        while (parents[current].has_value() && depth < max_chain_depth) {
            const auto parent = by_begin.find(*parents[current]);
            if (parent == by_begin.end() || parent->second == current) {
                break;
            }
            current = parent->second;
            ++depth;
        }

        if (depth >= max_chain_depth) {
            result.warnings.emplace_back("Unwind chain exceeded the depth limit.");
        }

        range.chained = current != index;
        range.primary_begin_rva = table.functions[current].begin_rva;
        if (range.chained) {
            ++table.chained_ranges;
        } else {
            ++table.primary_functions;
        }
    }
}

void read_exception_table(std::span<const std::byte> bytes, const PeImage& image,
                          PeDirectoryReadResult& result) {
    const auto directory = image.directory(PeDirectory::exception_table);
    if (!directory.present()) {
        return;
    }
    if (image.machine != PeMachine::amd64) {
        result.warnings.emplace_back(
            "Exception directory present but the 12-byte RUNTIME_FUNCTION layout is x64-specific.");
        return;
    }

    const auto base_offset = offset_of(bytes, image, directory.rva);
    if (!base_offset.has_value()) {
        result.errors.emplace_back("Exception directory does not map to file data.");
        return;
    }

    constexpr std::size_t entry_size = 12U;
    if (directory.size % entry_size != 0U) {
        result.warnings.emplace_back(
            "Exception directory size is not a whole number of RUNTIME_FUNCTION entries.");
    }

    const std::size_t entry_count = static_cast<std::size_t>(directory.size) / entry_size;
    if (entry_count > kMaxFunctions) {
        result.errors.emplace_back("Exception directory exceeds the safety limit.");
        return;
    }

    PeFunctionTable table;
    table.functions.reserve(entry_count);
    table.smallest_size = std::numeric_limits<std::uint32_t>::max();

    std::uint32_t previous_begin = 0U;
    for (std::size_t index = 0; index < entry_count; ++index) {
        const std::size_t offset = *base_offset + index * entry_size;
        const auto begin = read_u32(bytes, offset);
        const auto end = read_u32(bytes, offset + 4U);
        const auto unwind = read_u32(bytes, offset + 8U);
        if (!begin.has_value() || !end.has_value() || !unwind.has_value()) {
            result.warnings.emplace_back("Exception directory is truncated.");
            break;
        }
        if (*end <= *begin) {
            result.warnings.emplace_back("Exception entry has a non-increasing range.");
            continue;
        }

        if (*begin < previous_begin) {
            table.sorted_by_address = false;
        }
        previous_begin = *begin;

        const PeFunctionRange range{*begin, *end, *unwind};
        table.smallest_size = std::min(table.smallest_size, range.size());
        table.largest_size = std::max(table.largest_size, range.size());
        table.covered_bytes += range.size();
        table.functions.push_back(range);
    }

    if (table.functions.empty()) {
        table.smallest_size = 0U;
    }

    resolve_chained_functions(bytes, image, table, result);
    result.directories.functions = std::move(table);
}

void read_debug(std::span<const std::byte> bytes, const PeImage& image,
                PeDirectoryReadResult& result) {
    const auto directory = image.directory(PeDirectory::debug);
    if (!directory.present()) {
        return;
    }

    const auto base_offset = offset_of(bytes, image, directory.rva);
    if (!base_offset.has_value()) {
        result.warnings.emplace_back("Debug directory does not map to file data.");
        return;
    }

    constexpr std::size_t entry_size = 28U;
    const std::size_t entry_count =
        std::min(static_cast<std::size_t>(directory.size) / entry_size, kMaxDebugEntries);

    for (std::size_t index = 0; index < entry_count; ++index) {
        const std::size_t offset = *base_offset + index * entry_size;
        const auto timestamp = read_u32(bytes, offset + 4U);
        const auto type = read_u32(bytes, offset + 12U);
        const auto size_of_data = read_u32(bytes, offset + 16U);
        const auto address_of_raw = read_u32(bytes, offset + 20U);
        const auto pointer_to_raw = read_u32(bytes, offset + 24U);
        if (!type.has_value() || !size_of_data.has_value() || !address_of_raw.has_value() ||
            !pointer_to_raw.has_value() || !timestamp.has_value()) {
            result.warnings.emplace_back("Debug directory is truncated.");
            break;
        }

        result.directories.debug_entries.push_back(
            PeDebugEntry{*type, *size_of_data, *address_of_raw, *pointer_to_raw, *timestamp});

        // Type 2 is CodeView. Only RSDS (PDB 7.0) carries the GUID/age identity.
        if (*type != 2U || *size_of_data < 25U) {
            continue;
        }

        const auto data_offset = static_cast<std::size_t>(*pointer_to_raw);
        if (!has_range(bytes, data_offset, static_cast<std::size_t>(*size_of_data))) {
            result.warnings.emplace_back("CodeView record lies outside the file.");
            continue;
        }

        const auto signature = read_u32(bytes, data_offset);
        if (!signature.has_value() || *signature != 0x53445352U) {
            result.warnings.emplace_back("CodeView record is not an RSDS entry.");
            continue;
        }

        PeCodeViewIdentity identity;
        for (std::size_t byte_index = 0; byte_index < identity.guid.size(); ++byte_index) {
            identity.guid[byte_index] =
                detail::read_u8(bytes, data_offset + 4U + byte_index).value_or(0U);
        }
        identity.age = read_u32(bytes, data_offset + 20U).value_or(0U);
        identity.pdb_path = read_cstring(bytes, data_offset + 24U,
                                         static_cast<std::size_t>(*size_of_data))
                                .value_or(std::string{});
        result.directories.codeview = std::move(identity);
    }
}

void read_relocations(std::span<const std::byte> bytes, const PeImage& image,
                      PeDirectoryReadResult& result) {
    const auto directory = image.directory(PeDirectory::base_relocation_table);
    if (!directory.present()) {
        return;
    }

    const auto base_offset = offset_of(bytes, image, directory.rva);
    if (!base_offset.has_value()) {
        result.warnings.emplace_back("Relocation directory does not map to file data.");
        return;
    }

    PeRelocationSummary summary;
    summary.lowest_rva = std::numeric_limits<std::uint32_t>::max();

    std::size_t consumed = 0U;
    std::size_t blocks = 0U;
    while (consumed + 8U <= static_cast<std::size_t>(directory.size) &&
           blocks < kMaxRelocationBlocks) {
        const std::size_t offset = *base_offset + consumed;
        const auto page_rva = read_u32(bytes, offset);
        const auto block_size = read_u32(bytes, offset + 4U);
        if (!page_rva.has_value() || !block_size.has_value()) {
            result.warnings.emplace_back("Relocation block header is truncated.");
            break;
        }
        if (*block_size < 8U || consumed + *block_size > directory.size) {
            result.warnings.emplace_back("Relocation block declares an inconsistent size.");
            break;
        }

        const std::size_t entries = (static_cast<std::size_t>(*block_size) - 8U) / 2U;
        for (std::size_t index = 0; index < entries; ++index) {
            const auto entry = read_u16(bytes, offset + 8U + index * 2U);
            if (!entry.has_value()) {
                result.warnings.emplace_back("Relocation block is truncated.");
                break;
            }

            const auto type = static_cast<std::uint16_t>(*entry >> 12U);
            const auto item_rva =
                static_cast<std::uint32_t>(*page_rva + (*entry & 0x0FFFU));
            ++summary.entry_count;
            switch (type) {
            case 0U: ++summary.absolute_entries; break;
            case 10U: ++summary.dir64_entries; break;
            default: ++summary.other_entries; break;
            }

            // Type 0 is padding and points nowhere meaningful.
            if (type != 0U) {
                summary.lowest_rva = std::min(summary.lowest_rva, item_rva);
                summary.highest_rva = std::max(summary.highest_rva, item_rva);
            }
        }

        consumed += *block_size;
        ++blocks;
    }

    summary.block_count = static_cast<std::uint32_t>(blocks);
    if (summary.entry_count == summary.absolute_entries) {
        summary.lowest_rva = 0U;
    }
    result.directories.relocations = summary;
}

void read_tls(std::span<const std::byte> bytes, const PeImage& image,
              PeDirectoryReadResult& result) {
    const auto directory = image.directory(PeDirectory::tls_table);
    if (!directory.present()) {
        return;
    }

    const auto base_offset = offset_of(bytes, image, directory.rva);
    if (!base_offset.has_value()) {
        result.warnings.emplace_back("TLS directory does not map to file data.");
        return;
    }

    const bool wide = image.kind == PeKind::pe32_plus;
    const auto read_pointer = [&](std::size_t offset) -> std::optional<std::uint64_t> {
        if (wide) {
            return read_u64(bytes, offset);
        }
        const auto narrow = read_u32(bytes, offset);
        if (!narrow.has_value()) {
            return std::nullopt;
        }
        return static_cast<std::uint64_t>(*narrow);
    };

    const std::size_t pointer_size = wide ? 8U : 4U;
    PeTlsDirectory tls;
    tls.raw_data_start_va = read_pointer(*base_offset).value_or(0U);
    tls.raw_data_end_va = read_pointer(*base_offset + pointer_size).value_or(0U);
    tls.index_va = read_pointer(*base_offset + pointer_size * 2U).value_or(0U);
    tls.callbacks_va = read_pointer(*base_offset + pointer_size * 3U).value_or(0U);
    tls.zero_fill_size = read_u32(bytes, *base_offset + pointer_size * 4U).value_or(0U);
    tls.characteristics = read_u32(bytes, *base_offset + pointer_size * 4U + 4U).value_or(0U);

    if (tls.callbacks_va != 0U) {
        const auto callback_offset = offset_of(bytes, image, va_to_rva(image, tls.callbacks_va));
        if (callback_offset.has_value()) {
            for (std::size_t index = 0; index < kMaxTlsCallbacks; ++index) {
                const auto callback = read_pointer(*callback_offset + index * pointer_size);
                if (!callback.has_value() || *callback == 0U) {
                    break;
                }
                tls.callbacks.push_back(*callback);
            }
        } else {
            result.warnings.emplace_back("TLS callback array does not map to file data.");
        }
    }

    result.directories.tls = std::move(tls);
}

/// Counts leaf entries below one resource directory node.
[[nodiscard]] std::uint32_t count_resource_leaves(std::span<const std::byte> bytes,
                                                  const PeImage& image, std::uint32_t root_rva,
                                                  std::uint32_t node_rva, std::size_t depth,
                                                  std::size_t& budget) {
    if (depth >= kResourceDepthLimit || budget == 0U) {
        return 0U;
    }

    const auto offset = offset_of(bytes, image, node_rva);
    if (!offset.has_value()) {
        return 0U;
    }

    const auto named = read_u16(bytes, *offset + 12U);
    const auto identified = read_u16(bytes, *offset + 14U);
    if (!named.has_value() || !identified.has_value()) {
        return 0U;
    }

    const std::size_t total = static_cast<std::size_t>(*named) + *identified;
    std::uint32_t leaves = 0U;
    for (std::size_t index = 0; index < total && budget > 0U; ++index) {
        --budget;
        const std::size_t entry_offset = *offset + 16U + index * 8U;
        const auto payload = read_u32(bytes, entry_offset + 4U);
        if (!payload.has_value()) {
            break;
        }

        // The high bit marks a subdirectory; otherwise the entry is a data leaf.
        if ((*payload & 0x80000000U) != 0U) {
            leaves += count_resource_leaves(bytes, image, root_rva,
                                            root_rva + (*payload & 0x7FFFFFFFU), depth + 1U,
                                            budget);
        } else {
            ++leaves;
        }
    }
    return leaves;
}

[[nodiscard]] std::string_view resource_type_name(std::uint32_t type_id) noexcept {
    switch (type_id) {
    case 1U: return "CURSOR";
    case 2U: return "BITMAP";
    case 3U: return "ICON";
    case 4U: return "MENU";
    case 5U: return "DIALOG";
    case 6U: return "STRING";
    case 7U: return "FONTDIR";
    case 8U: return "FONT";
    case 9U: return "ACCELERATOR";
    case 10U: return "RCDATA";
    case 11U: return "MESSAGETABLE";
    case 12U: return "GROUP_CURSOR";
    case 14U: return "GROUP_ICON";
    case 16U: return "VERSION";
    case 17U: return "DLGINCLUDE";
    case 19U: return "PLUGPLAY";
    case 20U: return "VXD";
    case 21U: return "ANICURSOR";
    case 22U: return "ANIICON";
    case 23U: return "HTML";
    case 24U: return "MANIFEST";
    default: return "";
    }
}

void read_resources(std::span<const std::byte> bytes, const PeImage& image,
                    PeDirectoryReadResult& result) {
    const auto directory = image.directory(PeDirectory::resource_table);
    if (!directory.present()) {
        return;
    }

    const auto root_offset = offset_of(bytes, image, directory.rva);
    if (!root_offset.has_value()) {
        result.warnings.emplace_back("Resource directory does not map to file data.");
        return;
    }

    const auto named = read_u16(bytes, *root_offset + 12U);
    const auto identified = read_u16(bytes, *root_offset + 14U);
    if (!named.has_value() || !identified.has_value()) {
        result.warnings.emplace_back("Resource root directory is truncated.");
        return;
    }

    std::size_t budget = kMaxResourceEntries;
    const std::size_t total = static_cast<std::size_t>(*named) + *identified;
    for (std::size_t index = 0; index < total; ++index) {
        const std::size_t entry_offset = *root_offset + 16U + index * 8U;
        const auto name_field = read_u32(bytes, entry_offset);
        const auto payload = read_u32(bytes, entry_offset + 4U);
        if (!name_field.has_value() || !payload.has_value()) {
            result.warnings.emplace_back("Resource root entry is truncated.");
            break;
        }

        PeResourceTypeCount entry;
        // A high bit in the name field means a string name rather than an id.
        if ((*name_field & 0x80000000U) == 0U) {
            entry.type_id = *name_field;
            entry.type_name = std::string{resource_type_name(entry.type_id)};
        } else {
            entry.type_name = "<named>";
        }

        entry.entry_count =
            (*payload & 0x80000000U) != 0U
                ? count_resource_leaves(bytes, image, directory.rva,
                                        directory.rva + (*payload & 0x7FFFFFFFU), 1U, budget)
                : 1U;
        result.directories.resource_types.push_back(std::move(entry));
    }
}

} // namespace

std::string PeCodeViewIdentity::guid_string() const {
    // Windows prints the CodeView GUID with the first three fields byte
    // swapped; matching that spelling is what makes the value comparable with
    // symbol-server and debugger output.
    std::array<char, 40> buffer{};
    const int written = std::snprintf(
        buffer.data(), buffer.size(), "%02X%02X%02X%02X-%02X%02X-%02X%02X-%02X%02X-%02X%02X%02X%02X%02X%02X",
        guid[3], guid[2], guid[1], guid[0], guid[5], guid[4], guid[7], guid[6], guid[8], guid[9],
        guid[10], guid[11], guid[12], guid[13], guid[14], guid[15]);
    if (written <= 0) {
        return {};
    }
    return std::string{buffer.data(), static_cast<std::size_t>(written)};
}

std::string_view debug_type_name(std::uint32_t type) noexcept {
    switch (type) {
    case 0U: return "unknown";
    case 1U: return "coff";
    case 2U: return "codeview";
    case 3U: return "fpo";
    case 4U: return "misc";
    case 5U: return "exception";
    case 6U: return "fixup";
    case 7U: return "omap-to-src";
    case 8U: return "omap-from-src";
    case 9U: return "borland";
    case 10U: return "reserved10";
    case 11U: return "clsid";
    case 12U: return "vc-feature";
    case 13U: return "pogo";
    case 14U: return "iltcg";
    case 15U: return "mpx";
    case 16U: return "repro";
    case 20U: return "ex-dllcharacteristics";
    default: return "unrecognized";
    }
}

std::size_t PeDirectories::imported_function_count() const noexcept {
    std::size_t total = 0U;
    for (const auto& module : imports) {
        total += module.functions.size();
    }
    return total;
}

PeDirectoryReadResult PeDirectoryReader::read(std::span<const std::byte> bytes,
                                              const PeImage& image) {
    PeDirectoryReadResult result;
    if (image.data_directories.empty()) {
        result.warnings.emplace_back("Image declares no data directories.");
        return result;
    }

    read_imports(bytes, image, result);
    read_delay_imports(bytes, image, result);
    read_exports(bytes, image, result);
    read_exception_table(bytes, image, result);
    read_debug(bytes, image, result);
    read_relocations(bytes, image, result);
    read_tls(bytes, image, result);
    read_resources(bytes, image, result);
    return result;
}

} // namespace dmc::rengine::exe
