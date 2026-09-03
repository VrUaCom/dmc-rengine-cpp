#include "dmc_rengine/formats/so/graph.hpp"

#include "dmc_rengine/binary/reader.hpp"

#include <algorithm>
#include <utility>

namespace dmc::rengine::formats::so::graph {
namespace {

[[nodiscard]] bool has_error(const std::vector<ParseDiagnostic>& diagnostics) noexcept {
    return std::any_of(diagnostics.begin(), diagnostics.end(), [](const ParseDiagnostic& diagnostic) {
        return diagnostic.severity == ParseSeverity::error;
    });
}

void add_error(std::vector<ParseDiagnostic>& diagnostics,
               std::string code,
               std::string message,
               const std::size_t offset) {
    diagnostics.push_back(ParseDiagnostic{
        ParseSeverity::error,
        std::move(code),
        std::move(message),
        static_cast<std::uint64_t>(offset),
    });
}

[[nodiscard]] bool parse_indexed_block(const binary::Reader& reader,
                                       const std::size_t base,
                                       const std::size_t extent,
                                       const std::size_t header_size,
                                       IndexedBlock& out,
                                       std::vector<ParseDiagnostic>& diagnostics) {
    if (extent < header_size + 2U || !reader.contains(base, extent)) {
        add_error(diagnostics, "so.graph.block_truncated", "SO indexed block is truncated", base);
        return false;
    }

    const auto type = reader.u16_le(base);
    const auto first_entry = reader.u16_le(base + header_size);
    if (!type || !first_entry) {
        add_error(diagnostics, "so.graph.header_truncated", "SO indexed-block header is truncated", base);
        return false;
    }

    out.type = *type;
    out.base_offset = static_cast<std::uint64_t>(base);
    out.extent_size = static_cast<std::uint64_t>(extent);
    out.header_words.clear();
    for (std::size_t offset = 0U; offset < header_size; offset += 2U) {
        const auto word = reader.u16_le(base + offset);
        if (!word) {
            add_error(diagnostics, "so.graph.header_word_truncated", "SO header word is truncated", base + offset);
            return false;
        }
        out.header_words.push_back(*word);
    }

    const auto first_entry_offset = static_cast<std::size_t>(*first_entry);
    if (first_entry_offset < header_size ||
        ((first_entry_offset - header_size) % 2U) != 0U ||
        first_entry_offset >= extent) {
        add_error(diagnostics,
                  "so.graph.offset_table_closure",
                  "SO entry-offset table does not close on its first entry",
                  base + header_size);
        return false;
    }

    const auto count = (first_entry_offset - header_size) / 2U;
    if (count == 0U || count > (extent - header_size) / 2U) {
        add_error(diagnostics, "so.graph.offset_count", "SO entry-offset count is invalid", base + header_size);
        return false;
    }

    out.entry_offsets.clear();
    out.entry_offsets.reserve(count);
    std::uint16_t previous = 0U;
    for (std::size_t index = 0U; index < count; ++index) {
        const auto table_offset = base + header_size + index * 2U;
        const auto value = reader.u16_le(table_offset);
        if (!value) {
            add_error(diagnostics, "so.graph.offset_truncated", "SO entry offset is truncated", table_offset);
            return false;
        }
        if (static_cast<std::size_t>(*value) >= extent || (index != 0U && *value <= previous)) {
            add_error(diagnostics,
                      "so.graph.offset_order",
                      "SO entry offsets are not strictly increasing inside the block",
                      table_offset);
            return false;
        }
        out.entry_offsets.push_back(*value);
        previous = *value;
    }

    if (out.entry_offsets.front() != *first_entry) {
        add_error(diagnostics,
                  "so.graph.first_offset_mismatch",
                  "SO first entry offset is inconsistent with offset-table closure",
                  base + header_size);
        return false;
    }
    return true;
}

} // namespace

bool ParseResult::ok() const noexcept {
    return recognized && !has_error(diagnostics);
}

ParseResult parse(const std::span<const std::byte> bytes) {
    ParseResult result;
    const binary::Reader reader(bytes);
    const auto root_type = reader.u16_le(0U);
    if (!root_type || *root_type != 6U) {
        return result;
    }
    result.recognized = true;

    const auto type8_pointer = reader.u16_le(2U);
    if (!type8_pointer) {
        add_error(result.diagnostics, "so.graph.type8_pointer", "SO type-8 boundary is missing", 2U);
        return result;
    }
    const auto type8_base = static_cast<std::size_t>(*type8_pointer);
    if (type8_base <= type6_header_size || !reader.contains(type8_base, type8_header_size + 2U)) {
        add_error(result.diagnostics,
                  "so.graph.type8_range",
                  "SO type-6 boundary does not point to an in-range second block",
                  2U);
        return result;
    }

    const auto type8 = reader.u16_le(type8_base);
    if (!type8 || *type8 != 8U) {
        add_error(result.diagnostics,
                  "so.graph.type8_identity",
                  "SO type-6 boundary does not point to the observed type-8 companion block",
                  type8_base);
        return result;
    }

    IndexedBlock type6_block;
    if (!parse_indexed_block(reader, 0U, type8_base, type6_header_size, type6_block, result.diagnostics)) {
        return result;
    }

    IndexedBlock type8_block;
    if (!parse_indexed_block(reader,
                             type8_base,
                             bytes.size() - type8_base,
                             type8_header_size,
                             type8_block,
                             result.diagnostics)) {
        return result;
    }

    result.blocks.push_back(std::move(type6_block));
    result.blocks.push_back(std::move(type8_block));
    return result;
}

} // namespace dmc::rengine::formats::so::graph
