#include "dmc_rengine/profiles/dmc3/stage_numeric_resolver.hpp"

#include "dmc_rengine/binary/reader.hpp"
#include "dmc_rengine/profiles/dmc3/stage_table.hpp"

#include <limits>
#include <optional>
#include <string>
#include <utility>

namespace dmc::rengine::profiles::dmc3 {
namespace {

void add_diagnostic(
    NumericStageResolution& result,
    gdspaces::DiagnosticSeverity severity,
    std::string code,
    std::string message) {
    result.diagnostics.push_back(gdspaces::Diagnostic{
        .severity = severity,
        .code = std::move(code),
        .message = std::move(message),
        .resource = std::nullopt,
    });
}

[[nodiscard]] std::optional<std::uint32_t> va_to_file_offset(
    const exe::PeImage& image,
    std::uint64_t va) noexcept {
    if (va < image.image_base) {
        return std::nullopt;
    }
    const auto rva64 = va - image.image_base;
    if (rva64 > std::numeric_limits<std::uint32_t>::max()) {
        return std::nullopt;
    }
    return image.rva_to_file_offset(static_cast<std::uint32_t>(rva64));
}

[[nodiscard]] bool identify_descriptor(
    NumericStageResolution& result) noexcept {
    const auto& banks = wave3_stage_resource_banks();
    for (const auto& bank : banks) {
        if (result.descriptor_va < bank.va) {
            continue;
        }
        const auto delta = result.descriptor_va - bank.va;
        const auto row_size = bank.row_size_bytes();
        if (row_size == 0U || delta % row_size != 0U) {
            continue;
        }
        const auto row_index64 = delta / row_size;
        if (row_index64 >= bank.row_count) {
            continue;
        }
        const auto row_index = static_cast<std::uint32_t>(row_index64);
        result.source_table_id = bank.id;
        result.source_row_index = row_index;
        result.descriptor_primary_stage_id =
            runtime_stage_id_for_table_row(bank, row_index);
        return true;
    }
    return false;
}

} // namespace

bool NumericStageResolution::resolved() const noexcept {
    if (source_table_id.empty() || descriptor_va == 0U) {
        return false;
    }
    for (const auto& diagnostic : diagnostics) {
        if (diagnostic.severity == gdspaces::DiagnosticSeverity::error) {
            return false;
        }
    }
    return true;
}

NumericStageResolution StageNumericResolver::resolve(
    std::span<const std::byte> executable_bytes,
    const exe::PeImage& image,
    std::uint16_t stage_id) {
    const auto& topology = wave2_stage_descriptor_universe();
    NumericStageResolution result{
        .requested_stage_id = stage_id,
        .group_index = static_cast<std::uint32_t>(stage_id / 100U),
        .remainder = static_cast<std::uint32_t>(stage_id % 100U),
        .selector_base_va = 0U,
        .selector_slot_va = 0U,
        .descriptor_va = 0U,
        .source_table_id = {},
        .source_row_index = 0U,
        .descriptor_primary_stage_id = std::nullopt,
        .diagnostics = {},
    };

    if (!topology.valid() || result.group_index >= topology.group_base_count) {
        add_diagnostic(result, gdspaces::DiagnosticSeverity::error,
            "dmc3.stage-numeric.group-out-of-range",
            "The numeric Stage ID selects a hundreds group outside the recovered group table.");
        return result;
    }

    const binary::Reader reader(executable_bytes);
    const auto group_entry_va = topology.group_base_table_va +
        static_cast<std::uint64_t>(result.group_index) * 8ULL;
    const auto group_entry_offset = va_to_file_offset(image, group_entry_va);
    if (!group_entry_offset.has_value()) {
        add_diagnostic(result, gdspaces::DiagnosticSeverity::error,
            "dmc3.stage-numeric.group-entry-unmapped",
            "The recovered Stage group-base entry is not file-backed in this PE image.");
        return result;
    }

    const auto selector_base = reader.u64_le(*group_entry_offset);
    if (!selector_base.has_value() || *selector_base == 0U) {
        add_diagnostic(result, gdspaces::DiagnosticSeverity::error,
            "dmc3.stage-numeric.invalid-selector-base",
            "The selected Stage hundreds group does not contain a valid selector-base pointer.");
        return result;
    }
    result.selector_base_va = *selector_base;
    result.selector_slot_va = result.selector_base_va +
        static_cast<std::uint64_t>(result.remainder) * 8ULL;

    const auto selector_slot_offset = va_to_file_offset(image, result.selector_slot_va);
    if (!selector_slot_offset.has_value()) {
        add_diagnostic(result, gdspaces::DiagnosticSeverity::error,
            "dmc3.stage-numeric.selector-slot-unmapped",
            "The selected Stage selector slot is not file-backed in this PE image.");
        return result;
    }

    const auto descriptor_va = reader.u64_le(*selector_slot_offset);
    if (!descriptor_va.has_value() || *descriptor_va == 0U) {
        add_diagnostic(result, gdspaces::DiagnosticSeverity::error,
            "dmc3.stage-numeric.null-descriptor",
            "The selected Stage selector slot does not point to a resource descriptor.");
        return result;
    }
    result.descriptor_va = *descriptor_va;

    if (!identify_descriptor(result)) {
        add_diagnostic(result, gdspaces::DiagnosticSeverity::error,
            "dmc3.stage-numeric.unknown-descriptor",
            "The numeric Stage resolver returned a pointer outside the two recovered descriptor banks.");
        return result;
    }

    if (result.aliases_another_primary_stage()) {
        add_diagnostic(result, gdspaces::DiagnosticSeverity::info,
            "dmc3.stage-numeric.alias",
            "The requested numeric Stage ID resolves to another primary descriptor identity; this is an executable-backed alias/fallback observation.");
    }
    return result;
}

} // namespace dmc::rengine::profiles::dmc3
