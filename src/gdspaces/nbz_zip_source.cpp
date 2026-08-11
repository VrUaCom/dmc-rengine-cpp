#include "dmc_rengine/gdspaces/nbz_zip_source.hpp"

#include "dmc_rengine/gdspaces/classifier.hpp"

#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <fstream>
#include <limits>
#include <span>
#include <string>
#include <utility>
#include <vector>

namespace dmc::rengine::gdspaces {
namespace {

constexpr std::uint32_t eocd_signature = 0x06054B50U;
constexpr std::uint32_t central_signature = 0x02014B50U;
constexpr std::uint32_t local_signature = 0x04034B50U;
constexpr std::size_t eocd_fixed_size = 22U;
constexpr std::size_t central_fixed_size = 46U;
constexpr std::size_t local_fixed_size = 30U;
constexpr std::size_t max_zip_comment = 65535U;

[[nodiscard]] std::uint16_t u16_le(
    std::span<const std::byte> bytes,
    std::size_t offset) noexcept {
    return static_cast<std::uint16_t>(
        std::to_integer<std::uint8_t>(bytes[offset]) |
        (static_cast<std::uint16_t>(
             std::to_integer<std::uint8_t>(bytes[offset + 1U])) << 8U));
}

[[nodiscard]] std::uint32_t u32_le(
    std::span<const std::byte> bytes,
    std::size_t offset) noexcept {
    return static_cast<std::uint32_t>(
        std::to_integer<std::uint8_t>(bytes[offset]) |
        (static_cast<std::uint32_t>(
             std::to_integer<std::uint8_t>(bytes[offset + 1U])) << 8U) |
        (static_cast<std::uint32_t>(
             std::to_integer<std::uint8_t>(bytes[offset + 2U])) << 16U) |
        (static_cast<std::uint32_t>(
             std::to_integer<std::uint8_t>(bytes[offset + 3U])) << 24U));
}

[[nodiscard]] bool read_exact(
    std::ifstream& stream,
    std::uint64_t offset,
    std::span<std::byte> output) {
    if (offset > static_cast<std::uint64_t>(
            std::numeric_limits<std::streamoff>::max()) ||
        output.size() > static_cast<std::size_t>(
            std::numeric_limits<std::streamsize>::max())) {
        return false;
    }

    stream.clear();
    stream.seekg(static_cast<std::streamoff>(offset), std::ios::beg);
    if (!stream) {
        return false;
    }
    if (output.empty()) {
        return true;
    }

    stream.read(
        reinterpret_cast<char*>(output.data()),
        static_cast<std::streamsize>(output.size()));
    return stream.good() ||
        stream.gcount() == static_cast<std::streamsize>(output.size());
}

void add_diagnostic(
    std::vector<Diagnostic>& diagnostics,
    DiagnosticSeverity severity,
    std::string code,
    std::string message,
    std::optional<ResourceId> resource = std::nullopt) {
    diagnostics.push_back(Diagnostic{
        .severity = severity,
        .code = std::move(code),
        .message = std::move(message),
        .resource = std::move(resource),
    });
}

[[nodiscard]] bool has_error(const std::vector<Diagnostic>& diagnostics) noexcept {
    return std::any_of(
        diagnostics.begin(), diagnostics.end(),
        [](const Diagnostic& diagnostic) {
            return diagnostic.severity == DiagnosticSeverity::error;
        });
}

[[nodiscard]] std::string chain_for(std::uint32_t central_index) {
    return "nbz[" + std::to_string(central_index) + "]";
}

[[nodiscard]] ResourceId id_for(
    std::string_view source_id,
    const NbzZipEntry& entry) {
    return ResourceId{
        .source_id = std::string{source_id},
        .logical_path = entry.logical_path,
        .container_chain = chain_for(entry.central_index),
        .offset = 0U,
        .size = entry.uncompressed_size,
    };
}

[[nodiscard]] std::uint32_t crc32_of(std::span<const std::byte> bytes) noexcept {
    std::uint32_t crc = 0xFFFFFFFFU;
    for (const auto value : bytes) {
        crc ^= std::to_integer<std::uint8_t>(value);
        for (unsigned bit = 0U; bit < 8U; ++bit) {
            const auto mask = static_cast<std::uint32_t>(
                0U - static_cast<std::uint32_t>(crc & 1U));
            crc = (crc >> 1U) ^ (0xEDB88320U & mask);
        }
    }
    return ~crc;
}

[[nodiscard]] std::optional<std::uint64_t> checked_add(
    std::uint64_t left,
    std::uint64_t right) noexcept {
    if (left > std::numeric_limits<std::uint64_t>::max() - right) {
        return std::nullopt;
    }
    return left + right;
}

} // namespace

bool NbzZipEntry::directory() const noexcept {
    return !logical_path.empty() && logical_path.back() == '/';
}

bool NbzZipEntry::encrypted() const noexcept {
    return (flags & 0x0001U) != 0U;
}

NbzZipSource::NbzZipSource(
    std::string source_id,
    std::filesystem::path archive_path,
    std::string profile)
    : source_id_(std::move(source_id)),
      archive_path_(std::move(archive_path)),
      profile_(std::move(profile)) {
    build_index();
}

std::string_view NbzZipSource::id() const noexcept {
    return source_id_;
}

std::string_view NbzZipSource::kind() const noexcept {
    return "nbz-zip";
}

std::vector<ResourceRef> NbzZipSource::enumerate() const {
    std::vector<ResourceRef> resources;
    if (!valid()) {
        return resources;
    }

    resources.reserve(entries_.size());
    for (const auto& entry : entries_) {
        if (entry.directory()) {
            continue;
        }

        const auto classification = ResourceClassifier::classify(entry.logical_path);
        resources.push_back(ResourceRef{
            .id = id_for(source_id_, entry),
            .display_name = std::filesystem::path(entry.logical_path).filename().string(),
            .format = classification.format,
            .profile = profile_,
            .synthetic_name = false,
            .container = classification.container,
        });
    }
    return resources;
}

std::optional<ResourcePayload> NbzZipSource::read(
    const ResourceId& resource) const {
    if (!valid() || resource.source_id != source_id_ || !resource.valid()) {
        return std::nullopt;
    }

    const auto* entry = find_entry(resource);
    if (entry == nullptr || entry->directory()) {
        return std::nullopt;
    }

    const auto initial = ResourceClassifier::classify(entry->logical_path);
    ResourcePayload payload{
        .resource = ResourceRef{
            .id = id_for(source_id_, *entry),
            .display_name = std::filesystem::path(entry->logical_path).filename().string(),
            .format = initial.format,
            .profile = profile_,
            .synthetic_name = false,
            .container = initial.container,
        },
        .bytes = {},
        .diagnostics = {},
        .byte_provenance = std::nullopt,
    };

    const auto transform = entry->compression_method == 0U
        ? ByteTransform::zip_stored
        : entry->compression_method == 8U
            ? ByteTransform::zip_deflate
            : ByteTransform::unknown;
    payload.byte_provenance = ByteProvenance{
        .kind = entry->compression_method == 0U
            ? ByteOriginKind::direct_source_span
            : ByteOriginKind::transformed_source_span,
        .authority_id = source_id_,
        .offset = entry->data_offset,
        .stored_size = entry->compressed_size,
        .materialized_size = entry->uncompressed_size,
        .transform = entry->compression_method == 0U
            ? ByteTransform::none
            : transform,
        .crc32 = entry->crc32,
    };

    if (entry->encrypted()) {
        add_diagnostic(
            payload.diagnostics,
            DiagnosticSeverity::error,
            "gdspaces.nbz.encrypted-entry-unsupported",
            "Encrypted ZIP/NBZ entries are not materialized by the read-only source.",
            payload.resource.id);
        return payload;
    }

    if (entry->compression_method == 8U) {
        add_diagnostic(
            payload.diagnostics,
            DiagnosticSeverity::error,
            "gdspaces.nbz.deflate-materialization-pending",
            "The entry is indexed with verified storage provenance, but DEFLATE materialization is intentionally gated until the decompressor path is promoted.",
            payload.resource.id);
        return payload;
    }

    if (entry->compression_method != 0U) {
        add_diagnostic(
            payload.diagnostics,
            DiagnosticSeverity::error,
            "gdspaces.nbz.compression-method-unsupported",
            "The ZIP/NBZ entry uses a compression method not yet evidenced and promoted for production materialization.",
            payload.resource.id);
        return payload;
    }

    if (entry->compressed_size != entry->uncompressed_size) {
        add_diagnostic(
            payload.diagnostics,
            DiagnosticSeverity::error,
            "gdspaces.nbz.stored-size-mismatch",
            "A stored ZIP entry reports different compressed and uncompressed sizes.",
            payload.resource.id);
        return payload;
    }

    std::ifstream stream(archive_path_, std::ios::binary);
    if (!stream) {
        add_diagnostic(
            payload.diagnostics,
            DiagnosticSeverity::error,
            "gdspaces.nbz.open-failed",
            "The NBZ archive could not be reopened for entry materialization.",
            payload.resource.id);
        return payload;
    }

    payload.bytes.resize(entry->uncompressed_size);
    if (!read_exact(
            stream,
            entry->data_offset,
            std::span<std::byte>{payload.bytes})) {
        payload.bytes.clear();
        add_diagnostic(
            payload.diagnostics,
            DiagnosticSeverity::error,
            "gdspaces.nbz.entry-read-failed",
            "The stored ZIP/NBZ entry could not be read completely.",
            payload.resource.id);
        return payload;
    }

    if (crc32_of(std::span<const std::byte>{payload.bytes}) != entry->crc32) {
        payload.bytes.clear();
        add_diagnostic(
            payload.diagnostics,
            DiagnosticSeverity::error,
            "gdspaces.nbz.crc32-mismatch",
            "The materialized stored ZIP/NBZ entry failed CRC32 validation.",
            payload.resource.id);
        return payload;
    }

    const auto classification = ResourceClassifier::classify(
        entry->logical_path,
        std::span<const std::byte>{payload.bytes});
    payload.resource.format = classification.format;
    payload.resource.container = classification.container;
    return payload;
}

bool NbzZipSource::valid() const noexcept {
    return !source_id_.empty() && !profile_.empty() && archive_size_ != 0U &&
        !has_error(diagnostics_);
}

const std::filesystem::path& NbzZipSource::archive_path() const noexcept {
    return archive_path_;
}

const std::vector<NbzZipEntry>& NbzZipSource::entries() const noexcept {
    return entries_;
}

const std::vector<Diagnostic>& NbzZipSource::diagnostics() const noexcept {
    return diagnostics_;
}

void NbzZipSource::build_index() {
    entries_.clear();
    diagnostics_.clear();
    archive_size_ = 0U;

    if (source_id_.empty() || profile_.empty()) {
        add_diagnostic(
            diagnostics_,
            DiagnosticSeverity::error,
            "gdspaces.nbz.invalid-source-config",
            "NBZ ZIP source id and profile must be non-empty.");
        return;
    }

    std::error_code error;
    const auto raw_size = std::filesystem::file_size(archive_path_, error);
    if (error || raw_size < eocd_fixed_size) {
        add_diagnostic(
            diagnostics_,
            DiagnosticSeverity::error,
            "gdspaces.nbz.invalid-archive-size",
            "The NBZ archive is missing or too small to contain a ZIP EOCD record.");
        return;
    }
    archive_size_ = raw_size;

    std::ifstream stream(archive_path_, std::ios::binary);
    if (!stream) {
        add_diagnostic(
            diagnostics_,
            DiagnosticSeverity::error,
            "gdspaces.nbz.open-failed",
            "The NBZ archive could not be opened read-only.");
        return;
    }

    const auto tail_size = static_cast<std::size_t>(std::min<std::uint64_t>(
        archive_size_, eocd_fixed_size + max_zip_comment));
    std::vector<std::byte> tail(tail_size);
    if (!read_exact(
            stream,
            archive_size_ - tail_size,
            std::span<std::byte>{tail})) {
        add_diagnostic(
            diagnostics_,
            DiagnosticSeverity::error,
            "gdspaces.nbz.tail-read-failed",
            "The ZIP EOCD search window could not be read.");
        return;
    }

    std::optional<std::size_t> eocd_index;
    for (std::size_t index = tail.size() - eocd_fixed_size + 1U;
         index-- > 0U;) {
        if (u32_le(std::span<const std::byte>{tail}, index) != eocd_signature) {
            continue;
        }
        const auto comment_length = u16_le(
            std::span<const std::byte>{tail}, index + 20U);
        if (index + eocd_fixed_size + comment_length == tail.size()) {
            eocd_index = index;
            break;
        }
    }

    if (!eocd_index.has_value()) {
        add_diagnostic(
            diagnostics_,
            DiagnosticSeverity::error,
            "gdspaces.nbz.eocd-not-found",
            "No bounded standard ZIP EOCD record was found at the end of the NBZ archive.");
        return;
    }

    const auto eocd = std::span<const std::byte>{tail}.subspan(*eocd_index);
    const auto disk_number = u16_le(eocd, 4U);
    const auto central_disk = u16_le(eocd, 6U);
    const auto disk_entries = u16_le(eocd, 8U);
    const auto total_entries = u16_le(eocd, 10U);
    const auto central_size = u32_le(eocd, 12U);
    const auto central_offset = u32_le(eocd, 16U);

    if (disk_number != 0U || central_disk != 0U || disk_entries != total_entries) {
        add_diagnostic(
            diagnostics_,
            DiagnosticSeverity::error,
            "gdspaces.nbz.multidisk-unsupported",
            "Multi-disk ZIP archives are outside the evidenced DMC3 NBZ backend contract.");
        return;
    }

    if (total_entries == 0xFFFFU || central_size == 0xFFFFFFFFU ||
        central_offset == 0xFFFFFFFFU) {
        add_diagnostic(
            diagnostics_,
            DiagnosticSeverity::error,
            "gdspaces.nbz.zip64-unresolved",
            "ZIP64 sentinel values were encountered; ZIP64 behavior is not promoted for DMC3 NBZ yet.");
        return;
    }

    const auto central_end = checked_add(central_offset, central_size);
    if (!central_end.has_value() || *central_end > archive_size_) {
        add_diagnostic(
            diagnostics_,
            DiagnosticSeverity::error,
            "gdspaces.nbz.central-range-invalid",
            "The ZIP central-directory range extends outside the NBZ archive.");
        return;
    }

    entries_.reserve(total_entries);
    std::uint64_t cursor = central_offset;
    for (std::uint32_t central_index = 0U;
         central_index < total_entries;
         ++central_index) {
        std::array<std::byte, central_fixed_size> header{};
        if (!read_exact(stream, cursor, std::span<std::byte>{header}) ||
            u32_le(std::span<const std::byte>{header}, 0U) != central_signature) {
            add_diagnostic(
                diagnostics_,
                DiagnosticSeverity::error,
                "gdspaces.nbz.central-header-invalid",
                "A ZIP central-directory entry is truncated or has an invalid signature.");
            return;
        }

        const auto flags = u16_le(header, 8U);
        const auto method = u16_le(header, 10U);
        const auto crc32 = u32_le(header, 16U);
        const auto compressed_size = u32_le(header, 20U);
        const auto uncompressed_size = u32_le(header, 24U);
        const auto name_length = u16_le(header, 28U);
        const auto extra_length = u16_le(header, 30U);
        const auto comment_length = u16_le(header, 32U);
        const auto disk_start = u16_le(header, 34U);
        const auto local_header_offset = u32_le(header, 42U);

        if (disk_start != 0U || compressed_size == 0xFFFFFFFFU ||
            uncompressed_size == 0xFFFFFFFFU ||
            local_header_offset == 0xFFFFFFFFU) {
            add_diagnostic(
                diagnostics_,
                DiagnosticSeverity::error,
                "gdspaces.nbz.zip64-entry-unresolved",
                "A ZIP central entry requires ZIP64 or another disk, which is not promoted for DMC3 NBZ yet.");
            return;
        }

        const auto variable_size = static_cast<std::uint64_t>(name_length) +
            extra_length + comment_length;
        const auto next_cursor = checked_add(
            cursor,
            central_fixed_size + variable_size);
        if (!next_cursor.has_value() || *next_cursor > *central_end) {
            add_diagnostic(
                diagnostics_,
                DiagnosticSeverity::error,
                "gdspaces.nbz.central-entry-range-invalid",
                "A ZIP central entry's variable fields exceed the central-directory range.");
            return;
        }

        std::vector<std::byte> name_bytes(name_length);
        if (name_length != 0U && !read_exact(
                stream,
                cursor + central_fixed_size,
                std::span<std::byte>{name_bytes})) {
            add_diagnostic(
                diagnostics_,
                DiagnosticSeverity::error,
                "gdspaces.nbz.central-name-read-failed",
                "A ZIP central entry file name could not be read.");
            return;
        }

        std::string logical_path;
        logical_path.reserve(name_bytes.size());
        bool embedded_nul = false;
        for (const auto value : name_bytes) {
            const auto byte = std::to_integer<unsigned char>(value);
            if (byte == 0U) {
                embedded_nul = true;
                break;
            }
            logical_path.push_back(static_cast<char>(byte));
        }
        if (logical_path.empty() || embedded_nul) {
            add_diagnostic(
                diagnostics_,
                DiagnosticSeverity::error,
                "gdspaces.nbz.invalid-entry-name",
                "A ZIP central entry has an empty or embedded-NUL file name.");
            return;
        }

        std::array<std::byte, local_fixed_size> local{};
        if (!read_exact(
                stream,
                local_header_offset,
                std::span<std::byte>{local}) ||
            u32_le(std::span<const std::byte>{local}, 0U) != local_signature) {
            add_diagnostic(
                diagnostics_,
                DiagnosticSeverity::error,
                "gdspaces.nbz.local-header-invalid",
                "A ZIP central entry does not resolve to a valid local-file header.");
            return;
        }

        const auto local_flags = u16_le(local, 6U);
        const auto local_method = u16_le(local, 8U);
        const auto local_name_length = u16_le(local, 26U);
        const auto local_extra_length = u16_le(local, 28U);
        if (local_flags != flags || local_method != method) {
            add_diagnostic(
                diagnostics_,
                DiagnosticSeverity::error,
                "gdspaces.nbz.central-local-mismatch",
                "ZIP central and local headers disagree on flags or compression method.");
            return;
        }

        const auto data_offset = checked_add(
            local_header_offset,
            local_fixed_size +
                static_cast<std::uint64_t>(local_name_length) +
                local_extra_length);
        if (!data_offset.has_value()) {
            add_diagnostic(
                diagnostics_,
                DiagnosticSeverity::error,
                "gdspaces.nbz.data-offset-overflow",
                "A ZIP local-file data offset overflowed the supported range.");
            return;
        }
        const auto data_end = checked_add(*data_offset, compressed_size);
        if (!data_end.has_value() || *data_end > archive_size_) {
            add_diagnostic(
                diagnostics_,
                DiagnosticSeverity::error,
                "gdspaces.nbz.data-range-invalid",
                "A ZIP entry compressed-data span extends outside the NBZ archive.");
            return;
        }

        if (local_name_length != name_length) {
            add_diagnostic(
                diagnostics_,
                DiagnosticSeverity::error,
                "gdspaces.nbz.local-name-length-mismatch",
                "ZIP central and local headers disagree on file-name length.");
            return;
        }
        std::vector<std::byte> local_name_bytes(local_name_length);
        if (local_name_length != 0U && !read_exact(
                stream,
                static_cast<std::uint64_t>(local_header_offset) + local_fixed_size,
                std::span<std::byte>{local_name_bytes})) {
            add_diagnostic(
                diagnostics_,
                DiagnosticSeverity::error,
                "gdspaces.nbz.local-name-read-failed",
                "A ZIP local-header file name could not be read.");
            return;
        }
        if (local_name_bytes != name_bytes) {
            add_diagnostic(
                diagnostics_,
                DiagnosticSeverity::error,
                "gdspaces.nbz.central-local-name-mismatch",
                "ZIP central and local headers disagree on the exact entry name bytes.");
            return;
        }

        entries_.push_back(NbzZipEntry{
            .central_index = central_index,
            .logical_path = std::move(logical_path),
            .flags = flags,
            .compression_method = method,
            .crc32 = crc32,
            .compressed_size = compressed_size,
            .uncompressed_size = uncompressed_size,
            .local_header_offset = local_header_offset,
            .data_offset = *data_offset,
        });
        cursor = *next_cursor;
    }

    if (cursor != *central_end) {
        add_diagnostic(
            diagnostics_,
            DiagnosticSeverity::warning,
            "gdspaces.nbz.central-tail-unparsed",
            "The indexed central entries do not consume the complete EOCD-declared central-directory span.");
    }
}

const NbzZipEntry* NbzZipSource::find_entry(
    const ResourceId& resource) const noexcept {
    for (const auto& entry : entries_) {
        if (id_for(source_id_, entry).canonical() == resource.canonical()) {
            return &entry;
        }
    }
    return nullptr;
}

} // namespace dmc::rengine::gdspaces
