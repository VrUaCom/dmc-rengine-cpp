#include "dmc_rengine/profiles/dmc3/stage_table_reader.hpp"

#include "dmc_rengine/binary/reader.hpp"

#include <algorithm>
#include <limits>
#include <optional>
#include <string>
#include <utility>

namespace dmc::rengine::profiles::dmc3 {
namespace {

void add_diagnostic(
    StageResourceTableReadResult& result,
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

[[nodiscard]] std::optional<std::string> read_ascii_c_string(
    std::span<const std::byte> bytes,
    std::size_t offset,
    std::size_t max_path_bytes) {
    if (offset >= bytes.size() || max_path_bytes == 0U) {
        return std::nullopt;
    }

    std::string result;
    const auto available = bytes.size() - offset;
    const auto limit = std::min(available, max_path_bytes);
    result.reserve(limit);

    for (std::size_t index = 0U; index < limit; ++index) {
        const auto value = std::to_integer<unsigned char>(bytes[offset + index]);
        if (value == 0U) {
            if (result.empty()) {
                return std::nullopt;
            }
            return result;
        }
        if (value < 0x20U || value > 0x7EU) {
            return std::nullopt;
        }
        result.push_back(static_cast<char>(value));
    }

    return std::nullopt;
}

} // namespace

bool StageResourceTableRowObservation::complete() const noexcept {
    return std::all_of(
        cells.begin(), cells.end(),
        [](const StageResourceTableCellObservation& cell) {
            // Synthetic structural observations may omit Wave-2 kind16 when
            // they are testing only catalog/path behavior. The production
            // reader emits an error if the kind16 field cannot be read, so a
            // canonical executable read still cannot complete without it.
            return cell.valid();
        });
}

std::array<std::string, 4>
StageResourceTableRowObservation::logical_paths() const {
    std::array<std::string, 4> paths{};
    for (std::size_t index = 0U; index < cells.size(); ++index) {
        paths[index] = cells[index].logical_path;
    }
    return paths;
}

bool StageResourceTableReadResult::complete(
    const StageResourceTableDescriptor& descriptor) const noexcept {
    if (!descriptor.valid() || rows.size() != descriptor.row_count) {
        return false;
    }

    if (std::any_of(
            diagnostics.begin(), diagnostics.end(),
            [](const gdspaces::Diagnostic& diagnostic) {
                return diagnostic.severity == gdspaces::DiagnosticSeverity::error;
            })) {
        return false;
    }

    return std::all_of(
        rows.begin(), rows.end(),
        [](const StageResourceTableRowObservation& row) {
            return row.complete();
        });
}

StageResourceTableReadResult StageResourceTableReader::read(
    std::span<const std::byte> executable_bytes,
    const exe::PeImage& image,
    const StageResourceTableDescriptor& descriptor,
    std::size_t max_path_bytes) {
    StageResourceTableReadResult result;

    if (!descriptor.valid()) {
        add_diagnostic(result, gdspaces::DiagnosticSeverity::error,
            "dmc3.stage-table.invalid-descriptor",
            "The DMC3 stage resource table descriptor is invalid.");
        return result;
    }
    if (max_path_bytes == 0U) {
        add_diagnostic(result, gdspaces::DiagnosticSeverity::error,
            "dmc3.stage-table.invalid-path-limit",
            "The stage resource path bound must be greater than zero.");
        return result;
    }
    if (executable_bytes.size() != descriptor.artifact_size) {
        add_diagnostic(result, gdspaces::DiagnosticSeverity::error,
            "dmc3.stage-table.artifact-size-mismatch",
            "The executable byte size does not match the exact artifact described by the stage table evidence.");
        return result;
    }
    if (image.image_base != 0x140000000ULL || image.image_base > descriptor.va ||
        descriptor.va - image.image_base != descriptor.rva) {
        add_diagnostic(result, gdspaces::DiagnosticSeverity::error,
            "dmc3.stage-table.image-mismatch",
            "The supplied PE image does not map the evidenced stage table VA/RVA consistently.");
        return result;
    }

    const auto mapped_file_offset = image.rva_to_file_offset(descriptor.rva);
    const auto mapped_va = image.rva_to_va(descriptor.rva);
    if (!mapped_file_offset.has_value() ||
        static_cast<std::uint64_t>(*mapped_file_offset) != descriptor.file_offset ||
        !mapped_va.has_value() || *mapped_va != descriptor.va) {
        add_diagnostic(result, gdspaces::DiagnosticSeverity::error,
            "dmc3.stage-table.pe-mapping-conflict",
            "PE section mapping does not reproduce the evidenced file offset/RVA/VA tuple for the stage table.");
        return result;
    }

    if (descriptor.file_offset > executable_bytes.size() ||
        descriptor.table_size_bytes() >
            executable_bytes.size() - static_cast<std::size_t>(descriptor.file_offset)) {
        add_diagnostic(result, gdspaces::DiagnosticSeverity::error,
            "dmc3.stage-table.table-out-of-range",
            "The evidenced stage resource table extends outside the executable byte buffer.");
        return result;
    }

    const binary::Reader reader(executable_bytes);
    result.rows.reserve(descriptor.row_count);
    for (std::uint32_t row_index = 0U; row_index < descriptor.row_count; ++row_index) {
        StageResourceTableRowObservation row{.row_index = row_index, .cells = {}};
        for (std::uint32_t column_index = 0U; column_index < row.cells.size(); ++column_index) {
            const auto entry_index = static_cast<std::uint64_t>(row_index) * row.cells.size() + column_index;
            const auto cell_delta = entry_index * descriptor.cell_stride;
            const auto cell_file_offset = descriptor.file_offset + cell_delta;
            const auto cell_rva64 = static_cast<std::uint64_t>(descriptor.rva) + cell_delta;

            auto& cell = row.cells[column_index];
            cell.row_index = row_index;
            cell.column_index = column_index;
            cell.role = descriptor.role_for_column(column_index).value_or(StageResourceRole::script);
            cell.cell_file_offset = cell_file_offset;
            cell.cell_rva = cell_rva64 <= std::numeric_limits<std::uint32_t>::max()
                ? static_cast<std::uint32_t>(cell_rva64) : 0U;
            cell.cell_va = image.image_base + cell_rva64;

            const auto kind_field_offset = cell_file_offset + descriptor.kind16_offset;
            if (kind_field_offset > std::numeric_limits<std::size_t>::max()) {
                add_diagnostic(result, gdspaces::DiagnosticSeverity::error,
                    "dmc3.stage-table.kind-field-overflow",
                    "A stage table kind16 field offset exceeds the host addressable range.");
                continue;
            }
            const auto kind16 = reader.u16_le(static_cast<std::size_t>(kind_field_offset));
            if (!kind16.has_value()) {
                add_diagnostic(result, gdspaces::DiagnosticSeverity::error,
                    "dmc3.stage-table.invalid-kind16",
                    "A Stage resource cell does not contain a readable Wave-2 kind16 field.");
                continue;
            }
            cell.kind16 = *kind16;

            const auto pointer_field_offset = cell_file_offset + descriptor.path_pointer_offset;
            if (pointer_field_offset > std::numeric_limits<std::size_t>::max()) {
                add_diagnostic(result, gdspaces::DiagnosticSeverity::error,
                    "dmc3.stage-table.pointer-field-overflow",
                    "A stage table pointer-field offset exceeds the host addressable range.");
                continue;
            }

            const auto path_pointer = reader.u64_le(static_cast<std::size_t>(pointer_field_offset));
            if (!path_pointer.has_value() || *path_pointer < image.image_base) {
                add_diagnostic(result, gdspaces::DiagnosticSeverity::error,
                    "dmc3.stage-table.invalid-path-pointer",
                    "A stage table cell does not contain a valid in-image 64-bit path pointer.");
                continue;
            }
            cell.path_pointer_va = *path_pointer;

            const auto path_rva64 = *path_pointer - image.image_base;
            if (path_rva64 > std::numeric_limits<std::uint32_t>::max()) {
                add_diagnostic(result, gdspaces::DiagnosticSeverity::error,
                    "dmc3.stage-table.path-rva-overflow",
                    "A stage resource path pointer resolves outside the 32-bit PE RVA space.");
                continue;
            }

            const auto path_file_offset = image.rva_to_file_offset(static_cast<std::uint32_t>(path_rva64));
            if (!path_file_offset.has_value()) {
                add_diagnostic(result, gdspaces::DiagnosticSeverity::error,
                    "dmc3.stage-table.path-unmapped",
                    "A stage resource path pointer does not map to file-backed PE bytes.");
                continue;
            }
            cell.path_file_offset = *path_file_offset;

            const auto path = read_ascii_c_string(executable_bytes, *path_file_offset, max_path_bytes);
            if (!path.has_value()) {
                add_diagnostic(result, gdspaces::DiagnosticSeverity::error,
                    "dmc3.stage-table.invalid-path-string",
                    "A stage resource path pointer does not resolve to a bounded printable NUL-terminated ASCII string.");
                continue;
            }
            cell.logical_path = *path;
        }
        result.rows.push_back(std::move(row));
    }
    return result;
}

} // namespace dmc::rengine::profiles::dmc3
