#include "dmc_rengine/formats/mod/transform_domain.hpp"

#include "dmc_rengine/binary/reader.hpp"

#include <algorithm>
#include <utility>

namespace dmc::rengine::formats::mod::transform_domain {
namespace {

[[nodiscard]] bool has_error(const std::vector<ParseDiagnostic>& diagnostics) noexcept {
    return std::any_of(diagnostics.begin(), diagnostics.end(), [](const ParseDiagnostic& diagnostic) {
        return diagnostic.severity == ParseSeverity::error;
    });
}

void add_diagnostic(std::vector<ParseDiagnostic>& diagnostics,
                    ParseSeverity severity,
                    std::string code,
                    std::string message,
                    const std::size_t offset) {
    diagnostics.push_back(ParseDiagnostic{
        severity,
        std::move(code),
        std::move(message),
        static_cast<std::uint64_t>(offset),
    });
}

void add_error(std::vector<ParseDiagnostic>& diagnostics,
               std::string code,
               std::string message,
               const std::size_t offset) {
    add_diagnostic(diagnostics, ParseSeverity::error,
                   std::move(code), std::move(message), offset);
}

void add_warning(std::vector<ParseDiagnostic>& diagnostics,
                 std::string code,
                 std::string message,
                 const std::size_t offset) {
    add_diagnostic(diagnostics, ParseSeverity::warning,
                   std::move(code), std::move(message), offset);
}

[[nodiscard]] bool resolve_relative(
    const std::size_t base,
    const std::uint32_t relative,
    const std::size_t payload_size,
    std::size_t& resolved) noexcept {
    if (base > payload_size) return false;
    const auto remaining = payload_size - base;
    if (static_cast<std::size_t>(relative) > remaining) return false;
    resolved = base + static_cast<std::size_t>(relative);
    return true;
}

} // namespace

bool ParseResult::ok() const noexcept {
    return recognized && !has_error(diagnostics);
}

ParseResult parse(const std::span<const std::byte> bytes) {
    ParseResult result;
    const binary::Reader reader(bytes);
    if (!reader.matches(0U, "MOD ")) {
        return result;
    }
    result.recognized = true;

    const auto domain_count = reader.u8(0x11U);
    const auto document_offset = reader.u64_le(0x20U);
    if (!domain_count || !document_offset || *domain_count == 0U) {
        add_error(result.diagnostics,
                  "mod.transform_domain.header",
                  "MOD transform-domain header is incomplete or has zero domain count",
                  0x11U);
        return result;
    }

    result.raw_domain_count = *domain_count;
    result.document_offset = *document_offset;
    if (*document_offset > static_cast<std::uint64_t>(bytes.size())) {
        add_error(result.diagnostics,
                  "mod.transform_domain.document_range",
                  "MOD document pointer is outside the payload",
                  0x20U);
        return result;
    }

    const auto document = static_cast<std::size_t>(*document_offset);
    if (!reader.contains(document, 0x10U)) {
        add_error(result.diagnostics,
                  "mod.transform_domain.document_header",
                  "MOD document header is truncated",
                  document);
        return result;
    }

    const auto table0_rel = reader.u32_le(document);
    const auto table1_rel = reader.u32_le(document + 4U);
    const auto table2_rel = reader.u32_le(document + 8U);
    if (!table0_rel || !table1_rel || !table2_rel) {
        add_error(result.diagnostics,
                  "mod.transform_domain.table_offsets",
                  "MOD transform-domain table offsets are truncated",
                  document);
        return result;
    }

    std::size_t table0{};
    std::size_t table1{};
    std::size_t table2{};
    if (!resolve_relative(document, *table0_rel, bytes.size(), table0) ||
        !resolve_relative(document, *table1_rel, bytes.size(), table1) ||
        !resolve_relative(document, *table2_rel, bytes.size(), table2)) {
        add_error(result.diagnostics,
                  "mod.transform_domain.table_range",
                  "MOD transform-domain relative table pointer escapes the payload",
                  document);
        return result;
    }

    const auto count = static_cast<std::size_t>(*domain_count);
    if (*table0_rel >= *table1_rel || *table1_rel >= *table2_rel ||
        !reader.contains(table0, count) || !reader.contains(table1, count) ||
        table1 - table0 < count || table2 - table1 < count) {
        add_error(result.diagnostics,
                  "mod.transform_domain.table_spans",
                  "MOD transform-domain table spans are inconsistent",
                  document);
        return result;
    }

    result.reference_table.reserve(count);
    result.permutation_table.reserve(count);
    for (std::size_t index = 0U; index < count; ++index) {
        const auto reference = reader.u8(table0 + index);
        const auto permutation = reader.u8(table1 + index);
        if (!reference || !permutation) {
            add_error(result.diagnostics,
                      "mod.transform_domain.table_truncated",
                      "MOD transform-domain table entry is truncated",
                      table0 + index);
            return result;
        }
        result.reference_table.push_back(*reference);
        result.permutation_table.push_back(*permutation);
    }

    std::vector<std::int16_t> inverse(count, static_cast<std::int16_t>(-1));
    result.permutation_is_complete = true;
    for (std::size_t logical_index = 0U; logical_index < count; ++logical_index) {
        const auto physical_index = static_cast<std::size_t>(result.permutation_table[logical_index]);
        if (physical_index >= count || inverse[physical_index] != static_cast<std::int16_t>(-1)) {
            result.permutation_is_complete = false;
            break;
        }
        inverse[physical_index] = static_cast<std::int16_t>(logical_index);
    }
    if (!result.permutation_is_complete) {
        add_warning(result.diagnostics,
                    "mod.transform_domain.permutation_incomplete",
                    "MOD transform-domain permutation is incomplete or contains duplicates",
                    table1);
        return result;
    }

    result.derived_hierarchy_candidate.reserve(count);
    result.hierarchy_candidate_is_acyclic = true;
    for (std::size_t logical_index = 0U; logical_index < count; ++logical_index) {
        const auto raw_reference = result.reference_table[logical_index];
        if (raw_reference == 0xFFU) {
            result.derived_hierarchy_candidate.push_back(static_cast<std::int16_t>(-1));
            continue;
        }

        const auto reference_index = static_cast<std::size_t>(raw_reference);
        if (reference_index >= count || inverse[reference_index] < 0) {
            result.derived_hierarchy_candidate.push_back(static_cast<std::int16_t>(-2));
            result.hierarchy_candidate_is_acyclic = false;
            continue;
        }

        const auto derived = inverse[reference_index];
        result.derived_hierarchy_candidate.push_back(derived);
        if (derived >= static_cast<std::int16_t>(logical_index)) {
            result.hierarchy_candidate_is_acyclic = false;
        }
    }

    if (!result.hierarchy_candidate_is_acyclic) {
        add_warning(result.diagnostics,
                    "mod.transform_domain.hierarchy_candidate_invalid",
                    "MOD derived transform hierarchy candidate is not acyclic",
                    table0);
    }
    return result;
}

} // namespace dmc::rengine::formats::mod::transform_domain
