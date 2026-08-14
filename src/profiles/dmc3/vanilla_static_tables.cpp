#include "dmc_rengine/profiles/dmc3/vanilla_static_tables.hpp"

#include "dmc_rengine/binary/reader.hpp"
#include "dmc_rengine/profiles/dmc3/vanilla_runtime.hpp"

#include <algorithm>
#include <limits>
#include <optional>
#include <string>
#include <utility>

namespace dmc::rengine::profiles::dmc3::vanilla {
namespace {

void add_diagnostic(
    std::vector<gdspaces::Diagnostic>& diagnostics,
    gdspaces::DiagnosticSeverity severity,
    std::string code,
    std::string message) {
    diagnostics.push_back(gdspaces::Diagnostic{
        .severity = severity,
        .code = std::move(code),
        .message = std::move(message),
        .resource = std::nullopt,
    });
}

[[nodiscard]] bool has_error(
    const std::vector<gdspaces::Diagnostic>& diagnostics) noexcept {
    return std::any_of(
        diagnostics.begin(), diagnostics.end(),
        [](const gdspaces::Diagnostic& diagnostic) {
            return diagnostic.severity == gdspaces::DiagnosticSeverity::error;
        });
}

[[nodiscard]] std::optional<std::uint32_t> va_to_rva(
    const exe::PeImage& image,
    std::uint64_t va) noexcept {
    if (va < image.image_base) {
        return std::nullopt;
    }
    const auto rva = va - image.image_base;
    if (rva > std::numeric_limits<std::uint32_t>::max()) {
        return std::nullopt;
    }
    return static_cast<std::uint32_t>(rva);
}

[[nodiscard]] std::optional<std::string> read_ascii_c_string(
    std::span<const std::byte> bytes,
    std::size_t offset,
    std::size_t max_bytes) {
    if (offset >= bytes.size() || max_bytes == 0U) {
        return std::nullopt;
    }

    const auto limit = std::min(max_bytes, bytes.size() - offset);
    std::string value;
    value.reserve(limit);
    for (std::size_t index = 0U; index < limit; ++index) {
        const auto byte = std::to_integer<unsigned char>(bytes[offset + index]);
        if (byte == 0U) {
            return value.empty() ? std::nullopt : std::optional<std::string>{value};
        }
        if (byte < 0x20U || byte > 0x7EU) {
            return std::nullopt;
        }
        value.push_back(static_cast<char>(byte));
    }
    return std::nullopt;
}

[[nodiscard]] std::optional<std::uint32_t> table_file_offset(
    const exe::PeImage& image,
    std::uint64_t table_va) noexcept {
    const auto rva = va_to_rva(image, table_va);
    if (!rva.has_value()) {
        return std::nullopt;
    }
    return image.rva_to_file_offset(*rva);
}

[[nodiscard]] std::optional<std::pair<std::uint64_t, std::string>>
read_pointer_string(
    const binary::Reader& reader,
    std::span<const std::byte> executable_bytes,
    const exe::PeImage& image,
    std::size_t pointer_file_offset,
    std::size_t max_name_bytes) {
    const auto pointer = reader.u64_le(pointer_file_offset);
    if (!pointer.has_value()) {
        return std::nullopt;
    }
    const auto rva = va_to_rva(image, *pointer);
    if (!rva.has_value()) {
        return std::nullopt;
    }
    const auto file_offset = image.rva_to_file_offset(*rva);
    if (!file_offset.has_value()) {
        return std::nullopt;
    }
    const auto name = read_ascii_c_string(
        executable_bytes, *file_offset, max_name_bytes);
    if (!name.has_value()) {
        return std::nullopt;
    }
    return std::pair<std::uint64_t, std::string>{
        static_cast<std::uint64_t>(*file_offset),
        *name,
    };
}

} // namespace

bool MasterResourceCatalogReadResult::complete() const noexcept {
    return names.size() == numeric_resource_resolver.master_name_count &&
        !has_error(diagnostics);
}

bool HdAudioDescriptorObservation::has_explicit_loop() const noexcept {
    return loop_start_ms != hd_audio.no_explicit_loop_sentinel &&
        loop_end_ms != hd_audio.no_explicit_loop_sentinel;
}

bool HdAudioDescriptorTableReadResult::complete() const noexcept {
    return descriptors.size() == hd_audio.record_count &&
        !has_error(diagnostics);
}

MasterResourceCatalogReadResult
VanillaStaticTableReader::read_master_resource_catalog(
    std::span<const std::byte> executable_bytes,
    const exe::PeImage& image,
    std::size_t max_name_bytes) {
    MasterResourceCatalogReadResult result;
    if (max_name_bytes == 0U) {
        add_diagnostic(
            result.diagnostics,
            gdspaces::DiagnosticSeverity::error,
            "dmc3.vanilla.master-catalog.invalid-name-limit",
            "The master resource-name string bound must be greater than zero.");
        return result;
    }

    const auto base_file_offset = table_file_offset(
        image, numeric_resource_resolver.master_name_catalog_va);
    if (!base_file_offset.has_value()) {
        add_diagnostic(
            result.diagnostics,
            gdspaces::DiagnosticSeverity::error,
            "dmc3.vanilla.master-catalog.unmapped-table",
            "The recovered master resource-name catalog VA is not file-backed in this PE image.");
        return result;
    }

    constexpr std::uint32_t stride = 8U;
    const auto required = static_cast<std::uint64_t>(
        numeric_resource_resolver.master_name_count) * stride;
    if (*base_file_offset > executable_bytes.size() ||
        required > executable_bytes.size() - *base_file_offset) {
        add_diagnostic(
            result.diagnostics,
            gdspaces::DiagnosticSeverity::error,
            "dmc3.vanilla.master-catalog.out-of-range",
            "The recovered master resource-name catalog exceeds the supplied executable bytes.");
        return result;
    }

    const binary::Reader reader(executable_bytes);
    result.names.reserve(numeric_resource_resolver.master_name_count);
    for (std::uint32_t index = 0U;
         index < numeric_resource_resolver.master_name_count;
         ++index) {
        const auto pointer_offset = static_cast<std::size_t>(*base_file_offset) +
            static_cast<std::size_t>(index) * stride;
        const auto pointer = reader.u64_le(pointer_offset);
        const auto decoded = read_pointer_string(
            reader, executable_bytes, image, pointer_offset, max_name_bytes);
        if (!pointer.has_value() || !decoded.has_value()) {
            add_diagnostic(
                result.diagnostics,
                gdspaces::DiagnosticSeverity::error,
                "dmc3.vanilla.master-catalog.invalid-entry",
                "A master resource-name pointer does not resolve to a bounded printable string.");
            continue;
        }

        result.names.push_back(MasterResourceNameObservation{
            .index = index,
            .pointer_va = *pointer,
            .string_file_offset = decoded->first,
            .logical_name = std::move(decoded->second),
        });
    }
    return result;
}

HdAudioDescriptorTableReadResult
VanillaStaticTableReader::read_hd_audio_descriptors(
    std::span<const std::byte> executable_bytes,
    const exe::PeImage& image,
    std::size_t max_name_bytes) {
    HdAudioDescriptorTableReadResult result;
    if (max_name_bytes == 0U) {
        add_diagnostic(
            result.diagnostics,
            gdspaces::DiagnosticSeverity::error,
            "dmc3.vanilla.hd-audio.invalid-name-limit",
            "The HD audio descriptor string bound must be greater than zero.");
        return result;
    }

    const auto base_file_offset = table_file_offset(image, hd_audio.table_va);
    if (!base_file_offset.has_value()) {
        add_diagnostic(
            result.diagnostics,
            gdspaces::DiagnosticSeverity::error,
            "dmc3.vanilla.hd-audio.unmapped-table",
            "The recovered OGG descriptor table VA is not file-backed in this PE image.");
        return result;
    }

    const auto required = static_cast<std::uint64_t>(hd_audio.record_count) *
        hd_audio.record_stride;
    if (*base_file_offset > executable_bytes.size() ||
        required > executable_bytes.size() - *base_file_offset) {
        add_diagnostic(
            result.diagnostics,
            gdspaces::DiagnosticSeverity::error,
            "dmc3.vanilla.hd-audio.out-of-range",
            "The recovered OGG descriptor table exceeds the supplied executable bytes.");
        return result;
    }

    const binary::Reader reader(executable_bytes);
    result.descriptors.reserve(hd_audio.record_count);
    for (std::uint32_t index = 0U; index < hd_audio.record_count; ++index) {
        const auto record_offset = static_cast<std::size_t>(*base_file_offset) +
            static_cast<std::size_t>(index) * hd_audio.record_stride;
        const auto filename_pointer = reader.u64_le(
            record_offset + hd_audio_descriptor_abi.filename_pointer_offset);
        const auto decoded = read_pointer_string(
            reader,
            executable_bytes,
            image,
            record_offset + hd_audio_descriptor_abi.filename_pointer_offset,
            max_name_bytes);
        const auto loop_start = reader.u32_le(
            record_offset + hd_audio_descriptor_abi.loop_start_ms_offset);
        const auto loop_end = reader.u32_le(
            record_offset + hd_audio_descriptor_abi.loop_end_ms_offset);

        if (!filename_pointer.has_value() || !decoded.has_value() ||
            !loop_start.has_value() || !loop_end.has_value()) {
            add_diagnostic(
                result.diagnostics,
                gdspaces::DiagnosticSeverity::error,
                "dmc3.vanilla.hd-audio.invalid-entry",
                "An HD audio descriptor could not be decoded from the recovered 0x10-byte ABI.");
            continue;
        }

        result.descriptors.push_back(HdAudioDescriptorObservation{
            .index = index,
            .filename_pointer_va = *filename_pointer,
            .filename_file_offset = decoded->first,
            .filename = std::move(decoded->second),
            .loop_start_ms = *loop_start,
            .loop_end_ms = *loop_end,
        });
    }
    return result;
}

} // namespace dmc::rengine::profiles::dmc3::vanilla
