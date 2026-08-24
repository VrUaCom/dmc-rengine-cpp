#include "dmc_rengine/gdspaces/nbz_zip_artifact_member.hpp"

#include "dmc_rengine/core/raw_deflate.hpp"
#include "dmc_rengine/core/sha256.hpp"

#include <algorithm>
#include <cctype>
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <limits>
#include <span>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace dmc::rengine::gdspaces {
namespace {

constexpr std::uint16_t encrypted_flag = 0x0001U;
constexpr std::uint64_t max_hash_chunk_bytes = 64ULL * 1024ULL * 1024ULL;

void add_error(
    NbzZipArtifactMemberObservationResult& result,
    std::string code,
    std::string message) {
    result.diagnostics.push_back(Diagnostic{
        .severity = DiagnosticSeverity::error,
        .code = std::move(code),
        .message = std::move(message),
        .resource = std::nullopt,
    });
}

[[nodiscard]] unsigned char ascii_lower(unsigned char value) noexcept {
    if (value >= static_cast<unsigned char>('A') &&
        value <= static_cast<unsigned char>('F')) {
        return static_cast<unsigned char>(
            value - static_cast<unsigned char>('A') +
            static_cast<unsigned char>('a'));
    }
    return value;
}

[[nodiscard]] bool same_hex_digest(
    std::string_view left,
    std::string_view right) noexcept {
    if (left.size() != 64U || right.size() != 64U) {
        return false;
    }
    for (std::size_t index = 0U; index < left.size(); ++index) {
        const auto lhs = static_cast<unsigned char>(left[index]);
        const auto rhs = static_cast<unsigned char>(right[index]);
        if (std::isxdigit(lhs) == 0 || std::isxdigit(rhs) == 0 ||
            ascii_lower(lhs) != ascii_lower(rhs)) {
            return false;
        }
    }
    return true;
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

[[nodiscard]] ByteTransform transform_for(std::uint16_t method) noexcept {
    if (method == 0U) {
        return ByteTransform::zip_stored;
    }
    if (method == 8U) {
        return ByteTransform::zip_deflate;
    }
    return ByteTransform::unknown;
}

[[nodiscard]] const NbzZipEntry* find_entry(
    const NbzZipSource& source,
    std::uint32_t central_index) noexcept {
    const auto& entries = source.entries();
    const auto found = std::find_if(
        entries.begin(), entries.end(),
        [central_index](const NbzZipEntry& entry) {
            return entry.central_index == central_index;
        });
    return found == entries.end() ? nullptr : &*found;
}

} // namespace

ArtifactBoundNbzZipMemberObservation::ArtifactBoundNbzZipMemberObservation(
    evidence::ArtifactIdentity artifact,
    NbzZipEntry entry,
    std::vector<std::byte> materialized_bytes,
    ByteProvenance byte_provenance,
    std::string observed_sha256)
    : artifact_(std::move(artifact)),
      entry_(std::move(entry)),
      materialized_bytes_(std::move(materialized_bytes)),
      byte_provenance_(std::move(byte_provenance)),
      observed_sha256_(std::move(observed_sha256)) {}

const evidence::ArtifactIdentity&
ArtifactBoundNbzZipMemberObservation::artifact() const noexcept {
    return artifact_;
}

const NbzZipEntry&
ArtifactBoundNbzZipMemberObservation::entry() const noexcept {
    return entry_;
}

std::span<const std::byte>
ArtifactBoundNbzZipMemberObservation::materialized_bytes() const noexcept {
    return std::span<const std::byte>{materialized_bytes_};
}

const ByteProvenance&
ArtifactBoundNbzZipMemberObservation::byte_provenance() const noexcept {
    return byte_provenance_;
}

std::string_view
ArtifactBoundNbzZipMemberObservation::observed_sha256() const noexcept {
    return observed_sha256_;
}

bool ArtifactBoundNbzZipMemberObservation::valid() const noexcept {
    return artifact_.valid() && !entry_.logical_path.empty() &&
        entry_.uncompressed_size == materialized_bytes_.size() &&
        byte_provenance_.valid() &&
        byte_provenance_.materialized_size == materialized_bytes_.size() &&
        same_hex_digest(artifact_.sha256, observed_sha256_);
}

bool NbzZipArtifactMemberObservationResult::ok() const noexcept {
    if (!observation.has_value() || !observation->valid()) {
        return false;
    }
    return std::none_of(
        diagnostics.begin(), diagnostics.end(),
        [](const Diagnostic& diagnostic) {
            return diagnostic.severity == DiagnosticSeverity::error;
        });
}

NbzZipArtifactMemberObservationResult NbzZipArtifactMemberObserver::observe(
    const NbzZipSource& source,
    const ArtifactBoundNbzZipSerializationSnapshot& bound_snapshot,
    std::uint32_t central_index,
    NbzZipArtifactMemberLimits limits) {
    NbzZipArtifactMemberObservationResult result;

    if (!source.valid() || !source.index_receipt().has_value() ||
        !bound_snapshot.valid() ||
        bound_snapshot.serialization().source_id != source.id() ||
        bound_snapshot.artifact().size != source.index_receipt()->archive_size) {
        add_error(
            result,
            "gdspaces.nbz.artifact-member.invalid-binding",
            "Artifact-bound member observation requires the exact indexed source used by the bound serialization snapshot.");
        return result;
    }

    const auto* entry = find_entry(source, central_index);
    if (entry == nullptr || !entry->valid(bound_snapshot.artifact().size) ||
        entry->directory) {
        add_error(
            result,
            "gdspaces.nbz.artifact-member.invalid-entry",
            "Selected central entry is unavailable, invalid, or a directory.");
        return result;
    }
    if ((entry->flags & encrypted_flag) != 0U ||
        (entry->compression_method != 0U && entry->compression_method != 8U)) {
        add_error(
            result,
            "gdspaces.nbz.artifact-member.unsupported-entry",
            "Selected entry is encrypted or uses an unsupported compression method.");
        return result;
    }
    if (entry->compression_method == 0U &&
        entry->compressed_size != entry->uncompressed_size) {
        add_error(
            result,
            "gdspaces.nbz.artifact-member.store-size-mismatch",
            "STORE entry has different stored and materialized sizes.");
        return result;
    }
    if (entry->compressed_size > limits.max_stored_member_bytes ||
        entry->uncompressed_size > limits.max_materialized_member_bytes ||
        limits.hash_chunk_bytes == 0U ||
        limits.hash_chunk_bytes > max_hash_chunk_bytes ||
        limits.hash_chunk_bytes > static_cast<std::uint64_t>(
            std::numeric_limits<std::size_t>::max()) ||
        limits.hash_chunk_bytes > static_cast<std::uint64_t>(
            std::numeric_limits<std::streamsize>::max())) {
        add_error(
            result,
            "gdspaces.nbz.artifact-member.budget",
            "Selected entry or streaming observation exceeds configured product bounds.");
        return result;
    }

    const auto archive_size = bound_snapshot.artifact().size;
    const auto member_start = entry->data_offset;
    const auto member_end = member_start + entry->compressed_size;
    if (member_end < member_start || member_end > archive_size) {
        add_error(
            result,
            "gdspaces.nbz.artifact-member.range",
            "Selected stored member range exceeds the artifact.");
        return result;
    }

    std::error_code error;
    const auto size_before = std::filesystem::file_size(source.archive_path(), error);
    if (error || size_before != archive_size) {
        add_error(
            result,
            "gdspaces.nbz.artifact-member.size-mismatch",
            "Archive size no longer matches the artifact-bound snapshot.");
        return result;
    }

    std::ifstream stream(source.archive_path(), std::ios::binary);
    if (!stream) {
        add_error(
            result,
            "gdspaces.nbz.artifact-member.open-failed",
            "Unable to open the artifact-bound NBZ archive for observation.");
        return result;
    }

    std::vector<std::byte> stored(entry->compressed_size);
    std::vector<std::byte> buffer(static_cast<std::size_t>(limits.hash_chunk_bytes));
    core::Sha256Accumulator accumulator;
    std::uint64_t consumed = 0U;
    std::uint64_t captured = 0U;

    while (consumed < archive_size) {
        const auto remaining = archive_size - consumed;
        const auto amount = static_cast<std::size_t>(
            std::min<std::uint64_t>(limits.hash_chunk_bytes, remaining));
        stream.read(
            reinterpret_cast<char*>(buffer.data()),
            static_cast<std::streamsize>(amount));
        if (stream.gcount() != static_cast<std::streamsize>(amount)) {
            add_error(
                result,
                "gdspaces.nbz.artifact-member.short-read",
                "Artifact observation could not read the exact expected archive bytes.");
            return result;
        }

        const auto chunk = std::span<const std::byte>{buffer.data(), amount};
        if (!accumulator.update(chunk)) {
            add_error(
                result,
                "gdspaces.nbz.artifact-member.hash-overflow",
                "Artifact exceeds the SHA-256 byte-count domain.");
            return result;
        }

        const auto chunk_end = consumed + static_cast<std::uint64_t>(amount);
        const auto overlap_start = std::max(consumed, member_start);
        const auto overlap_end = std::min(chunk_end, member_end);
        if (overlap_start < overlap_end) {
            const auto source_offset = static_cast<std::size_t>(overlap_start - consumed);
            const auto destination_offset = static_cast<std::size_t>(overlap_start - member_start);
            const auto overlap_size = static_cast<std::size_t>(overlap_end - overlap_start);
            std::copy_n(
                chunk.begin() + static_cast<std::ptrdiff_t>(source_offset),
                static_cast<std::ptrdiff_t>(overlap_size),
                stored.begin() + static_cast<std::ptrdiff_t>(destination_offset));
            captured += static_cast<std::uint64_t>(overlap_size);
        }
        consumed = chunk_end;
    }

    char extra{};
    stream.read(&extra, 1);
    if (stream.gcount() != 0 || captured != entry->compressed_size) {
        add_error(
            result,
            "gdspaces.nbz.artifact-member.observation-incomplete",
            "Artifact observation did not end exactly or did not capture the complete selected member.");
        return result;
    }

    error.clear();
    const auto size_after = std::filesystem::file_size(source.archive_path(), error);
    if (error || size_after != archive_size) {
        add_error(
            result,
            "gdspaces.nbz.artifact-member.size-changed",
            "Archive size changed during selected-member observation.");
        return result;
    }

    const auto digest = accumulator.finalize();
    if (!digest.has_value() || accumulator.byte_count() != archive_size ||
        !same_hex_digest(digest->hex(), bound_snapshot.observed_sha256()) ||
        !same_hex_digest(digest->hex(), bound_snapshot.artifact().sha256)) {
        add_error(
            result,
            "gdspaces.nbz.artifact-member.hash-mismatch",
            "Selected-member observation does not match the artifact-bound archive SHA-256.");
        return result;
    }

    std::vector<std::byte> materialized;
    if (entry->compression_method == 0U) {
        materialized = std::move(stored);
    } else {
        auto inflated = core::RawDeflate::inflate(stored, entry->uncompressed_size);
        if (!inflated.ok()) {
            add_error(
                result,
                std::string{"gdspaces.nbz.artifact-member.deflate."} +
                    core::to_string(inflated.status),
                inflated.detail.empty()
                    ? "Raw DEFLATE materialization failed."
                    : std::move(inflated.detail));
            return result;
        }
        materialized = std::move(inflated.bytes);
    }

    if (materialized.size() != entry->uncompressed_size ||
        crc32_of(materialized) != entry->crc32) {
        add_error(
            result,
            "gdspaces.nbz.artifact-member.crc-or-size",
            "Materialized selected member does not match indexed size/CRC metadata.");
        return result;
    }

    ByteProvenance provenance{
        .kind = entry->compression_method == 0U
            ? ByteOriginKind::direct_source_span
            : ByteOriginKind::transformed_source_span,
        .authority_id = std::string{source.id()},
        .offset = entry->data_offset,
        .stored_size = entry->compressed_size,
        .materialized_size = entry->uncompressed_size,
        .transform = transform_for(entry->compression_method),
        .crc32 = entry->crc32,
    };

    ArtifactBoundNbzZipMemberObservation observation(
        bound_snapshot.artifact(),
        *entry,
        std::move(materialized),
        std::move(provenance),
        digest->hex());
    if (!observation.valid()) {
        add_error(
            result,
            "gdspaces.nbz.artifact-member.invalid-receipt",
            "Artifact-bound selected-member receipt failed its final invariants.");
        return result;
    }

    result.observation = std::move(observation);
    return result;
}

} // namespace dmc::rengine::gdspaces
