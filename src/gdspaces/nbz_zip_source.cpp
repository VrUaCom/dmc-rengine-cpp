#include "dmc_rengine/gdspaces/nbz_zip_source.hpp"

#include "dmc_rengine/core/raw_deflate.hpp"
#include "dmc_rengine/gdspaces/classifier.hpp"

#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <fstream>
#include <limits>
#include <optional>
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
constexpr std::uint16_t encrypted_flag = 0x0001U;

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

[[nodiscard]] bool has_error(
    const std::vector<Diagnostic>& diagnostics) noexcept {
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

[[nodiscard]] std::uint32_t crc32_of(
    std::span<const std::byte> bytes) noexcept {
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

[[nodiscard]] ByteTransform transform_for(std::uint16_t method) noexcept {
    if (method == 0U) {
        return ByteTransform::zip_stored;
    }
    if (method == 8U) {
        return ByteTransform::zip_deflate;
    }
    return ByteTransform::unknown;
}

[[nodiscard]] bool is_zip64_sentinel(
    std::uint16_t entries,
    std::uint32_t size,
    std::uint32_t offset) noexcept {
    return entries == 0xFFFFU || size == 0xFFFFFFFFU || offset == 0xFFFFFFFFU;
}

} // namespace

bool NbzZipEntry::valid(std::uint64_t archive_size) const noexcept {
    if (logical_path.empty()) {
        return false;
    }
    if (local_header_offset > archive_size ||
        local_fixed_size > archive_size - local_header_offset) {
        return false;
    }
    return data_offset <= archive_size &&
        compressed_size <= archive_size - data_offset;
}

bool NbzZipIndexReceipt::valid() const noexcept {
    return archive_size >= eocd_fixed_size &&
        eocd_offset <= archive_size - eocd_fixed_size &&
        computed_central_start <= eocd_offset &&
        declared_central_size == eocd_offset - computed_central_start;
}

NbzZipSource::NbzZipSource(
    std::string source_id,
    std::filesystem::path archive_path,
    NbzZipLimits limits)
    : source_id_(std::move(source_id)),
      archive_path_(std::move(archive_path)),
      limits_(limits) {
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
        if (entry.directory) {
            continue;
        }
        const auto classification = ResourceClassifier::classify(entry.logical_path);
        resources.push_back(ResourceRef{
            .id = id_for(source_id_, entry),
            .display_name = std::filesystem::path(entry.logical_path).filename().string(),
            .format = classification.format,
            .profile = std::string(to_string(classification.profile)),
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
    if (entry == nullptr) {
        return std::nullopt;
    }

    const auto initial_classification = ResourceClassifier::classify(entry->logical_path);
    ResourcePayload payload{
        .resource = ResourceRef{
            .id = resource,
            .display_name = std::filesystem::path(entry->logical_path).filename().string(),
            .format = initial_classification.format,
            .profile = std::string(to_string(initial_classification.profile)),
            .synthetic_name = false,
            .container = initial_classification.container,
        },
        .bytes = {},
        .diagnostics = {},
        .byte_provenance = ByteProvenance{
            .kind = entry->compression_method == 0U
                ? ByteOriginKind::direct_source_span
                : ByteOriginKind::transformed_source_span,
            .authority_id = source_id_,
            .offset = entry->data_offset,
            .stored_size = entry->compressed_size,
            .materialized_size = entry->uncompressed_size,
            .transform = transform_for(entry->compression_method),
            .crc32 = entry->crc32,
        },
    };

    if (entry->directory) {
        add_diagnostic(
            payload.diagnostics,
            DiagnosticSeverity::error,
            "gdspaces.nbz.safe.directory-read",
            "Directory entries are indexed for topology but are not readable resources.",
            resource);
        return payload;
    }
    if ((entry->flags & encrypted_flag) != 0U) {
        add_diagnostic(
            payload.diagnostics,
            DiagnosticSeverity::error,
            "gdspaces.nbz.safe.encrypted-entry",
            "Encrypted ZIP/NBZ members are outside the current product reader.",
            resource);
        return payload;
    }
    if (entry->compression_method != 0U && entry->compression_method != 8U) {
        add_diagnostic(
            payload.diagnostics,
            DiagnosticSeverity::error,
            "gdspaces.nbz.safe.compression-method-unsupported",
            "The product NBZ reader currently supports STORE and raw DEFLATE only.",
            resource);
        return payload;
    }
    if (entry->compression_method == 0U &&
        entry->compressed_size != entry->uncompressed_size) {
        add_diagnostic(
            payload.diagnostics,
            DiagnosticSeverity::error,
            "gdspaces.nbz.safe.store-size-mismatch",
            "A STORE member has different stored and materialized sizes.",
            resource);
        return payload;
    }
    if (entry->compressed_size > limits_.max_stored_member_bytes) {
        add_diagnostic(
            payload.diagnostics,
            DiagnosticSeverity::error,
            "gdspaces.nbz.safe.stored-member-budget",
            "The member's stored byte size exceeds the configured product materialization budget.",
            resource);
        return payload;
    }
    if (entry->uncompressed_size > limits_.max_materialized_member_bytes) {
        add_diagnostic(
            payload.diagnostics,
            DiagnosticSeverity::error,
            "gdspaces.nbz.safe.materialized-member-budget",
            "The member's materialized byte size exceeds the configured product materialization budget.",
            resource);
        return payload;
    }

    std::ifstream stream(archive_path_, std::ios::binary);
    if (!stream) {
        add_diagnostic(
            payload.diagnostics,
            DiagnosticSeverity::error,
            "gdspaces.nbz.open-failed",
            "Unable to reopen the NBZ archive for member materialization.",
            resource);
        return payload;
    }

    std::vector<std::byte> stored(entry->compressed_size);
    if (!read_exact(stream, entry->data_offset, stored)) {
        add_diagnostic(
            payload.diagnostics,
            DiagnosticSeverity::error,
            "gdspaces.nbz.read-failed",
            "Unable to read the complete stored NBZ member bytes.",
            resource);
        return payload;
    }

    if (entry->compression_method == 0U) {
        payload.bytes = std::move(stored);
    } else {
        auto inflated = core::RawDeflate::inflate(stored, entry->uncompressed_size);
        if (!inflated.ok()) {
            add_diagnostic(
                payload.diagnostics,
                DiagnosticSeverity::error,
                std::string{"gdspaces.nbz.safe.deflate."} +
                    core::to_string(inflated.status),
                inflated.detail.empty()
                    ? "Raw DEFLATE materialization failed."
                    : std::move(inflated.detail),
                resource);
            return payload;
        }
        payload.bytes = std::move(inflated.bytes);
    }

    if (crc32_of(payload.bytes) != entry->crc32) {
        payload.bytes.clear();
        add_diagnostic(
            payload.diagnostics,
            DiagnosticSeverity::error,
            "gdspaces.nbz.safe.crc-mismatch",
            "Materialized member bytes do not match the central-directory CRC32.",
            resource);
        return payload;
    }

    const auto classification = ResourceClassifier::classify(
        entry->logical_path,
        std::span<const std::byte>{payload.bytes});
    payload.resource.format = classification.format;
    payload.resource.profile = std::string(to_string(classification.profile));
    payload.resource.container = classification.container;
    return payload;
}

const std::filesystem::path& NbzZipSource::archive_path() const noexcept {
    return archive_path_;
}

bool NbzZipSource::valid() const noexcept {
    return !source_id_.empty() && archive_size_ >= eocd_fixed_size &&
        index_receipt_.has_value() && index_receipt_->valid() &&
        !has_error(diagnostics_);
}

const std::vector<NbzZipEntry>& NbzZipSource::entries() const noexcept {
    return entries_;
}

const std::vector<Diagnostic>& NbzZipSource::diagnostics() const noexcept {
    return diagnostics_;
}

const std::optional<NbzZipIndexReceipt>& NbzZipSource::index_receipt() const noexcept {
    return index_receipt_;
}

void NbzZipSource::build_index() {
    entries_.clear();
    diagnostics_.clear();
    index_receipt_.reset();
    archive_size_ = 0U;

    if (source_id_.empty()) {
        add_diagnostic(
            diagnostics_,
            DiagnosticSeverity::error,
            "gdspaces.nbz.invalid-source-id",
            "NBZ source ID must not be empty.");
        return;
    }

    std::error_code error;
    const auto raw_size = std::filesystem::file_size(archive_path_, error);
    if (error || raw_size < eocd_fixed_size) {
        add_diagnostic(
            diagnostics_,
            DiagnosticSeverity::error,
            "gdspaces.nbz.archive-size",
            "NBZ archive is unavailable or too small for ZIP EOCD.");
        return;
    }
    archive_size_ = static_cast<std::uint64_t>(raw_size);

    std::ifstream stream(archive_path_, std::ios::binary);
    if (!stream) {
        add_diagnostic(
            diagnostics_,
            DiagnosticSeverity::error,
            "gdspaces.nbz.open-failed",
            "Unable to open NBZ archive for indexing.");
        return;
    }

    const auto tail_size = static_cast<std::size_t>(std::min<std::uint64_t>(
        archive_size_, eocd_fixed_size + max_zip_comment));
    std::vector<std::byte> tail(tail_size);
    const auto tail_start = archive_size_ - tail_size;
    if (!read_exact(stream, tail_start, tail)) {
        add_diagnostic(
            diagnostics_,
            DiagnosticSeverity::error,
            "gdspaces.nbz.eocd-tail-read",
            "Unable to read the bounded ZIP EOCD search tail.");
        return;
    }

    std::optional<std::size_t> eocd_in_tail;
    for (std::size_t index = tail.size() - eocd_fixed_size + 1U;
         index-- > 0U;) {
        if (u32_le(tail, index) != eocd_signature) {
            continue;
        }
        const auto comment_length = u16_le(tail, index + 20U);
        if (index + eocd_fixed_size + comment_length == tail.size()) {
            eocd_in_tail = index;
            break;
        }
    }
    if (!eocd_in_tail.has_value()) {
        add_diagnostic(
            diagnostics_,
            DiagnosticSeverity::error,
            "gdspaces.nbz.eocd-not-found",
            "No strict end-of-archive ZIP EOCD record was found.");
        return;
    }

    const auto eocd = *eocd_in_tail;
    const auto eocd_absolute = tail_start + eocd;
    const auto disk_number = u16_le(tail, eocd + 4U);
    const auto central_disk = u16_le(tail, eocd + 6U);
    const auto disk_entries = u16_le(tail, eocd + 8U);
    const auto total_entries = u16_le(tail, eocd + 10U);
    const auto central_size = u32_le(tail, eocd + 12U);
    const auto declared_central_offset = u32_le(tail, eocd + 16U);

    // SafeProductValidation: current product source intentionally accepts a
    // bounded classic single-disk subset. This is not an original-game claim.
    if (disk_number != 0U || central_disk != 0U ||
        disk_entries != total_entries) {
        add_diagnostic(
            diagnostics_,
            DiagnosticSeverity::error,
            "gdspaces.nbz.safe.multi-disk",
            "Multi-disk ZIP/NBZ archives are outside the current product reader.");
        return;
    }
    if (is_zip64_sentinel(total_entries, central_size, declared_central_offset)) {
        add_diagnostic(
            diagnostics_,
            DiagnosticSeverity::error,
            "gdspaces.nbz.safe.zip64-unresolved",
            "ZIP64 sentinel values are outside the current product reader.");
        return;
    }

    // Recovered original-walk authority: central start is derived backwards
    // from the absolute EOCD position and central-directory size. The EOCD
    // central-offset field is preserved as a validation/receipt dimension but
    // is not used as the seek authority.
    if (central_size > eocd_absolute) {
        add_diagnostic(
            diagnostics_,
            DiagnosticSeverity::error,
            "gdspaces.nbz.central-size-out-of-range",
            "The central-directory size extends before the archive start.");
        return;
    }
    const auto computed_central_start =
        eocd_absolute - static_cast<std::uint64_t>(central_size);

    index_receipt_ = NbzZipIndexReceipt{
        .archive_size = archive_size_,
        .eocd_offset = eocd_absolute,
        .computed_central_start = computed_central_start,
        .declared_central_offset = declared_central_offset,
        .declared_central_size = central_size,
        .declared_entry_count = total_entries,
        .walked_entry_count = 0U,
        .central_offset_matches =
            declared_central_offset == computed_central_start,
        .entry_count_matches = false,
    };

    if (!index_receipt_->central_offset_matches) {
        add_diagnostic(
            diagnostics_,
            DiagnosticSeverity::warning,
            "gdspaces.nbz.compat.central-offset-mismatch",
            "EOCD central-offset metadata differs from the recovered DMC3 central-start derivation; computed start remains traversal authority.");
    }

    std::uint64_t cursor = computed_central_start;
    std::uint32_t walked = 0U;
    while (cursor < eocd_absolute) {
        // SafeProductValidation: the recovered walk is signature/boundary
        // driven, so the product needs an independent traversal-work budget.
        if (walked >= limits_.max_central_entries) {
            add_diagnostic(
                diagnostics_,
                DiagnosticSeverity::error,
                "gdspaces.nbz.safe.central-entry-budget",
                "The signature-bounded central-directory walk exceeds the configured product entry-count safety budget.");
            return;
        }
        if (eocd_absolute - cursor < central_fixed_size) {
            add_diagnostic(
                diagnostics_,
                DiagnosticSeverity::error,
                "gdspaces.nbz.central-record-truncated",
                "Central-directory bytes end with a truncated record before EOCD.");
            return;
        }

        std::array<std::byte, central_fixed_size> central{};
        if (!read_exact(stream, cursor, central) ||
            u32_le(central, 0U) != central_signature) {
            add_diagnostic(
                diagnostics_,
                DiagnosticSeverity::error,
                "gdspaces.nbz.central-signature",
                "Unexpected record signature while walking central-directory bytes toward EOCD.");
            return;
        }

        const auto flags = u16_le(central, 8U);
        const auto method = u16_le(central, 10U);
        const auto crc = u32_le(central, 16U);
        const auto compressed_size = u32_le(central, 20U);
        const auto uncompressed_size = u32_le(central, 24U);
        const auto name_length = u16_le(central, 28U);
        const auto extra_length = u16_le(central, 30U);
        const auto comment_length = u16_le(central, 32U);
        const auto disk_start = u16_le(central, 34U);
        const auto local_header_offset = u32_le(central, 42U);

        if (disk_start != 0U) {
            add_diagnostic(
                diagnostics_,
                DiagnosticSeverity::error,
                "gdspaces.nbz.safe.multi-disk-entry",
                "A central-directory entry references another disk.");
            return;
        }
        if (compressed_size == 0xFFFFFFFFU ||
            uncompressed_size == 0xFFFFFFFFU ||
            local_header_offset == 0xFFFFFFFFU) {
            add_diagnostic(
                diagnostics_,
                DiagnosticSeverity::error,
                "gdspaces.nbz.safe.zip64-entry-unresolved",
                "A central-directory entry uses ZIP64 sentinel fields.");
            return;
        }

        const auto variable_size =
            static_cast<std::uint64_t>(name_length) + extra_length + comment_length;
        const auto record_size = checked_add(central_fixed_size, variable_size);
        if (!record_size.has_value() ||
            *record_size > eocd_absolute - cursor) {
            add_diagnostic(
                diagnostics_,
                DiagnosticSeverity::error,
                "gdspaces.nbz.central-record-range",
                "A central-directory record extends beyond the recovered EOCD boundary.");
            return;
        }

        std::vector<std::byte> name_bytes(name_length);
        if (!read_exact(stream, cursor + central_fixed_size, name_bytes)) {
            add_diagnostic(
                diagnostics_,
                DiagnosticSeverity::error,
                "gdspaces.nbz.central-name-read",
                "Unable to read a complete central-directory filename.");
            return;
        }
        std::string logical_path;
        logical_path.reserve(name_bytes.size());
        for (const auto value : name_bytes) {
            logical_path.push_back(static_cast<char>(
                std::to_integer<std::uint8_t>(value)));
        }
        if (logical_path.empty()) {
            add_diagnostic(
                diagnostics_,
                DiagnosticSeverity::error,
                "gdspaces.nbz.empty-name",
                "A central-directory entry has an empty filename.");
            return;
        }

        std::array<std::byte, local_fixed_size> local{};
        if (!read_exact(stream, local_header_offset, local) ||
            u32_le(local, 0U) != local_signature) {
            add_diagnostic(
                diagnostics_,
                DiagnosticSeverity::error,
                "gdspaces.nbz.local-header",
                "A central-directory entry does not reference a readable local header.");
            return;
        }

        const auto local_flags = u16_le(local, 6U);
        const auto local_method = u16_le(local, 8U);
        const auto local_name_length = u16_le(local, 26U);
        const auto local_extra_length = u16_le(local, 28U);

        // SafeProductValidation: exact local/central agreement is retained as
        // hardening, not mislabeled as recovered original acceptance behavior.
        if (local_flags != flags || local_method != method) {
            add_diagnostic(
                diagnostics_,
                DiagnosticSeverity::error,
                "gdspaces.nbz.safe.central-local-metadata-mismatch",
                "Local and central ZIP metadata disagree on flags or method.");
            return;
        }

        std::vector<std::byte> local_name(local_name_length);
        if (!read_exact(
                stream,
                static_cast<std::uint64_t>(local_header_offset) + local_fixed_size,
                local_name) ||
            local_name != name_bytes) {
            add_diagnostic(
                diagnostics_,
                DiagnosticSeverity::error,
                "gdspaces.nbz.safe.central-local-name-mismatch",
                "Local and central ZIP filenames are not byte-identical.");
            return;
        }

        const auto local_variable = checked_add(local_fixed_size, local_name_length);
        if (!local_variable.has_value()) {
            add_diagnostic(
                diagnostics_,
                DiagnosticSeverity::error,
                "gdspaces.nbz.local-range-overflow",
                "Local-header variable-length arithmetic overflowed.");
            return;
        }
        const auto local_total = checked_add(*local_variable, local_extra_length);
        if (!local_total.has_value()) {
            add_diagnostic(
                diagnostics_,
                DiagnosticSeverity::error,
                "gdspaces.nbz.local-range-overflow",
                "Local-header variable-length arithmetic overflowed.");
            return;
        }
        const auto data_offset = checked_add(local_header_offset, *local_total);
        if (!data_offset.has_value() || *data_offset > archive_size_ ||
            compressed_size > archive_size_ - *data_offset) {
            add_diagnostic(
                diagnostics_,
                DiagnosticSeverity::error,
                "gdspaces.nbz.entry-data-range",
                "A member's stored byte span lies outside the archive.");
            return;
        }

        // SafeProductValidation: local member bytes must end before the
        // recovered central-directory byte domain begins. Without this check a
        // crafted central size could make materialization consume metadata.
        if (*data_offset > computed_central_start ||
            compressed_size > computed_central_start - *data_offset) {
            add_diagnostic(
                diagnostics_,
                DiagnosticSeverity::error,
                "gdspaces.nbz.safe.member-overlaps-central-directory",
                "A local member's stored byte span reaches into the central-directory byte domain.");
            return;
        }

        const auto directory = !logical_path.empty() &&
            (logical_path.back() == '/' || logical_path.back() == '\\');
        entries_.push_back(NbzZipEntry{
            .central_index = walked,
            .logical_path = std::move(logical_path),
            .flags = flags,
            .compression_method = method,
            .crc32 = crc,
            .compressed_size = compressed_size,
            .uncompressed_size = uncompressed_size,
            .local_header_offset = local_header_offset,
            .data_offset = *data_offset,
            .directory = directory,
        });
        if (!entries_.back().valid(archive_size_)) {
            add_diagnostic(
                diagnostics_,
                DiagnosticSeverity::error,
                "gdspaces.nbz.entry-invalid",
                "A decoded NBZ entry violates bounded archive invariants.");
            return;
        }

        ++walked;
        cursor += *record_size;
    }

    index_receipt_->walked_entry_count = walked;
    index_receipt_->entry_count_matches = walked == total_entries;
    if (!index_receipt_->entry_count_matches) {
        add_diagnostic(
            diagnostics_,
            DiagnosticSeverity::warning,
            "gdspaces.nbz.compat.entry-count-mismatch",
            "EOCD declared entry count differs from the signature-bounded central-directory walk; walked records remain traversal authority.");
    }
}

const NbzZipEntry* NbzZipSource::find_entry(
    const ResourceId& resource) const noexcept {
    const auto iterator = std::find_if(
        entries_.begin(), entries_.end(),
        [this, &resource](const NbzZipEntry& entry) {
            return id_for(source_id_, entry).canonical() == resource.canonical();
        });
    return iterator == entries_.end() ? nullptr : &*iterator;
}

} // namespace dmc::rengine::gdspaces
