#include "dmc_rengine/gdspaces/nbz_zip_artifact_binding.hpp"

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

constexpr std::uint64_t max_hash_chunk_bytes = 64ULL * 1024ULL * 1024ULL;
constexpr std::size_t central_fixed_size = 46U;
constexpr std::size_t local_fixed_size = 30U;

struct CaptureRange final {
    std::uint64_t offset{};
    std::uint64_t end{};
    std::vector<std::byte> bytes;
};

void add_error(
    NbzZipArtifactBindingResult& result,
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

[[nodiscard]] bool checked_add(
    std::uint64_t left,
    std::uint64_t right,
    std::uint64_t& output) noexcept {
    if (left > std::numeric_limits<std::uint64_t>::max() - right) {
        return false;
    }
    output = left + right;
    return true;
}

[[nodiscard]] bool consume_metadata_budget(
    std::uint64_t amount,
    std::uint64_t limit,
    std::uint64_t& used) noexcept {
    if (used > limit || amount > limit - used) {
        return false;
    }
    used += amount;
    return true;
}

[[nodiscard]] bool append_requested_range(
    std::vector<std::pair<std::uint64_t, std::uint64_t>>& requested,
    std::uint64_t offset,
    std::uint64_t size,
    std::uint64_t archive_size) noexcept {
    if (size == 0U) {
        return true;
    }

    std::uint64_t end{};
    if (!checked_add(offset, size, end) || end > archive_size) {
        return false;
    }
    requested.emplace_back(offset, end);
    return true;
}

[[nodiscard]] std::optional<std::vector<CaptureRange>> make_capture_plan(
    const NbzZipSource& source,
    NbzZipSerializationLimits limits,
    NbzZipArtifactBindingResult& result) {
    const auto& receipt = *source.index_receipt();
    const auto& indexed_entries = source.entries();

    if (limits.max_metadata_bytes < 22U ||
        receipt.computed_central_start > receipt.eocd_offset ||
        receipt.eocd_offset > receipt.archive_size) {
        add_error(
            result,
            "gdspaces.nbz.serialization.metadata-budget",
            "The serialization metadata bounds are invalid for artifact capture.");
        return std::nullopt;
    }
    if (indexed_entries.size() != receipt.walked_entry_count) {
        add_error(
            result,
            "gdspaces.nbz.serialization.index-count-mismatch",
            "Indexed entry count does not match the recovered central-directory walk receipt.");
        return std::nullopt;
    }

    const auto central_size =
        receipt.eocd_offset - receipt.computed_central_start;
    const auto eocd_size = receipt.archive_size - receipt.eocd_offset;

    std::uint64_t metadata_used = 0U;
    if (!consume_metadata_budget(
            central_size, limits.max_metadata_bytes, metadata_used) ||
        !consume_metadata_budget(
            eocd_size, limits.max_metadata_bytes, metadata_used)) {
        add_error(
            result,
            "gdspaces.nbz.serialization.metadata-budget",
            "Central-directory or EOCD metadata exceeds the configured serialization budget.");
        return std::nullopt;
    }

    std::vector<std::pair<std::uint64_t, std::uint64_t>> requested;
    requested.reserve(indexed_entries.size() + 2U);
    if (!append_requested_range(
            requested,
            receipt.computed_central_start,
            central_size,
            receipt.archive_size) ||
        !append_requested_range(
            requested,
            receipt.eocd_offset,
            eocd_size,
            receipt.archive_size)) {
        add_error(
            result,
            "gdspaces.nbz.artifact-binding.capture-range",
            "Central-directory or EOCD capture range exceeds the archive.");
        return std::nullopt;
    }

    for (const auto& indexed : indexed_entries) {
        if (indexed.data_offset < indexed.local_header_offset) {
            add_error(
                result,
                "gdspaces.nbz.serialization.local-prefix-range",
                "Indexed data offset precedes its local-header offset.");
            return std::nullopt;
        }

        const auto local_prefix_size =
            indexed.data_offset - indexed.local_header_offset;
        if (local_prefix_size < local_fixed_size ||
            !consume_metadata_budget(
                local_prefix_size,
                limits.max_metadata_bytes,
                metadata_used) ||
            !append_requested_range(
                requested,
                indexed.local_header_offset,
                local_prefix_size,
                receipt.archive_size)) {
            add_error(
                result,
                "gdspaces.nbz.serialization.metadata-budget",
                "Local-header/name/extra metadata exceeds the configured serialization budget.");
            return std::nullopt;
        }
    }

    std::sort(
        requested.begin(), requested.end(),
        [](const auto& left, const auto& right) {
            if (left.first != right.first) {
                return left.first < right.first;
            }
            return left.second < right.second;
        });

    std::vector<CaptureRange> ranges;
    ranges.reserve(requested.size());
    for (const auto& [offset, end] : requested) {
        if (!ranges.empty() && offset <= ranges.back().end) {
            ranges.back().end = std::max(ranges.back().end, end);
            continue;
        }
        ranges.push_back(CaptureRange{
            .offset = offset,
            .end = end,
            .bytes = {},
        });
    }

    for (auto& range : ranges) {
        const auto size = range.end - range.offset;
        if (size > static_cast<std::uint64_t>(
                std::numeric_limits<std::size_t>::max())) {
            add_error(
                result,
                "gdspaces.nbz.serialization.metadata-budget",
                "A serialization capture range exceeds the host size domain.");
            return std::nullopt;
        }
        range.bytes.resize(static_cast<std::size_t>(size));
    }
    return ranges;
}

void capture_chunk(
    std::span<const std::byte> chunk,
    std::uint64_t chunk_offset,
    std::vector<CaptureRange>& ranges,
    std::size_t& capture_cursor) {
    const auto chunk_end =
        chunk_offset + static_cast<std::uint64_t>(chunk.size());

    while (capture_cursor < ranges.size() &&
           ranges[capture_cursor].end <= chunk_offset) {
        ++capture_cursor;
    }

    auto index = capture_cursor;
    while (index < ranges.size() && ranges[index].offset < chunk_end) {
        auto& range = ranges[index];
        const auto overlap_start = std::max(chunk_offset, range.offset);
        const auto overlap_end = std::min(chunk_end, range.end);
        if (overlap_start < overlap_end) {
            const auto source_offset =
                static_cast<std::size_t>(overlap_start - chunk_offset);
            const auto destination_offset =
                static_cast<std::size_t>(overlap_start - range.offset);
            const auto amount =
                static_cast<std::size_t>(overlap_end - overlap_start);
            std::copy_n(
                chunk.begin() + static_cast<std::ptrdiff_t>(source_offset),
                static_cast<std::ptrdiff_t>(amount),
                range.bytes.begin() +
                    static_cast<std::ptrdiff_t>(destination_offset));
        }
        ++index;
    }

    while (capture_cursor < ranges.size() &&
           ranges[capture_cursor].end <= chunk_end) {
        ++capture_cursor;
    }
}

[[nodiscard]] std::optional<std::string> observe_exact_file(
    const std::filesystem::path& path,
    std::uint64_t expected_size,
    std::uint64_t chunk_bytes,
    std::vector<CaptureRange>& ranges,
    NbzZipArtifactBindingResult& result) {
    if (chunk_bytes == 0U || chunk_bytes > max_hash_chunk_bytes ||
        chunk_bytes > static_cast<std::uint64_t>(
            std::numeric_limits<std::size_t>::max()) ||
        chunk_bytes > static_cast<std::uint64_t>(
            std::numeric_limits<std::streamsize>::max())) {
        add_error(
            result,
            "gdspaces.nbz.artifact-binding.hash-chunk",
            "The streaming observation chunk size is outside the supported product bounds.");
        return std::nullopt;
    }

    std::error_code error;
    const auto size_before = std::filesystem::file_size(path, error);
    if (error || size_before != expected_size) {
        add_error(
            result,
            "gdspaces.nbz.artifact-binding.size-mismatch",
            "The artifact observation saw an archive size different from the expected artifact.");
        return std::nullopt;
    }

    std::ifstream stream(path, std::ios::binary);
    if (!stream) {
        add_error(
            result,
            "gdspaces.nbz.artifact-binding.open-failed",
            "The artifact observation could not open the NBZ archive.");
        return std::nullopt;
    }

    const auto buffer_size = static_cast<std::size_t>(chunk_bytes);
    std::vector<std::byte> buffer(buffer_size);
    core::Sha256Accumulator accumulator;
    std::uint64_t consumed = 0U;
    std::size_t capture_cursor = 0U;

    while (consumed < expected_size) {
        const auto remaining = expected_size - consumed;
        const auto amount = static_cast<std::size_t>(
            std::min<std::uint64_t>(chunk_bytes, remaining));
        stream.read(
            reinterpret_cast<char*>(buffer.data()),
            static_cast<std::streamsize>(amount));
        if (stream.gcount() != static_cast<std::streamsize>(amount)) {
            add_error(
                result,
                "gdspaces.nbz.artifact-binding.short-read",
                "The artifact observation could not read the exact expected archive bytes.");
            return std::nullopt;
        }

        const auto chunk =
            std::span<const std::byte>{buffer.data(), amount};
        if (!accumulator.update(chunk)) {
            add_error(
                result,
                "gdspaces.nbz.artifact-binding.hash-overflow",
                "The archive exceeds the supported SHA-256 byte-count domain.");
            return std::nullopt;
        }

        capture_chunk(chunk, consumed, ranges, capture_cursor);
        consumed += static_cast<std::uint64_t>(amount);
    }

    char extra{};
    stream.read(&extra, 1);
    if (stream.gcount() != 0) {
        add_error(
            result,
            "gdspaces.nbz.artifact-binding.size-changed",
            "The archive grew beyond the expected artifact during observation.");
        return std::nullopt;
    }

    error.clear();
    const auto size_after = std::filesystem::file_size(path, error);
    if (error || size_after != expected_size) {
        add_error(
            result,
            "gdspaces.nbz.artifact-binding.size-changed",
            "The archive size changed during artifact observation.");
        return std::nullopt;
    }

    if (capture_cursor != ranges.size()) {
        add_error(
            result,
            "gdspaces.nbz.artifact-binding.capture-incomplete",
            "The exact artifact observation did not cover every required serialization metadata range.");
        return std::nullopt;
    }

    const auto digest = accumulator.finalize();
    if (!digest.has_value() || accumulator.byte_count() != expected_size) {
        add_error(
            result,
            "gdspaces.nbz.artifact-binding.hash-finalize",
            "The streaming SHA-256 accumulator could not finalize the exact archive.");
        return std::nullopt;
    }
    return digest->hex();
}

[[nodiscard]] bool read_captured(
    const std::vector<CaptureRange>& ranges,
    std::uint64_t offset,
    std::span<std::byte> output) {
    if (output.empty()) {
        return true;
    }

    std::uint64_t end{};
    if (!checked_add(
            offset,
            static_cast<std::uint64_t>(output.size()),
            end)) {
        return false;
    }

    const auto found = std::find_if(
        ranges.begin(), ranges.end(),
        [offset, end](const CaptureRange& range) {
            return offset >= range.offset && end <= range.end;
        });
    if (found == ranges.end()) {
        return false;
    }

    const auto source_offset =
        static_cast<std::size_t>(offset - found->offset);
    std::copy_n(
        found->bytes.begin() +
            static_cast<std::ptrdiff_t>(source_offset),
        static_cast<std::ptrdiff_t>(output.size()),
        output.begin());
    return true;
}

[[nodiscard]] bool bytes_equal_string(
    std::span<const std::byte> bytes,
    std::string_view text) noexcept {
    if (bytes.size() != text.size()) {
        return false;
    }
    for (std::size_t index = 0U; index < bytes.size(); ++index) {
        if (std::to_integer<unsigned char>(bytes[index]) !=
            static_cast<unsigned char>(text[index])) {
            return false;
        }
    }
    return true;
}

[[nodiscard]] bool snapshot_matches_index_observation(
    const NbzZipSource& source,
    const NbzZipSerializationSnapshot& snapshot) noexcept {
    const auto& indexed_entries = source.entries();
    if (snapshot.entries.size() != indexed_entries.size()) {
        return false;
    }

    for (std::size_t index = 0U; index < indexed_entries.size(); ++index) {
        const auto& indexed = indexed_entries[index];
        const auto& observed = snapshot.entries[index];

        if (observed.central_record_bytes.size() < central_fixed_size ||
            observed.local_prefix_bytes.size() < local_fixed_size) {
            return false;
        }

        const auto central =
            std::span<const std::byte>{observed.central_record_bytes};
        const auto local =
            std::span<const std::byte>{observed.local_prefix_bytes};

        const auto central_name_length = u16_le(central, 28U);
        if (u16_le(central, 8U) != indexed.flags ||
            u16_le(central, 10U) != indexed.compression_method ||
            u32_le(central, 16U) != indexed.crc32 ||
            u32_le(central, 20U) != indexed.compressed_size ||
            u32_le(central, 24U) != indexed.uncompressed_size ||
            u32_le(central, 42U) != indexed.local_header_offset ||
            central_name_length != indexed.logical_path.size() ||
            central_fixed_size + central_name_length >
                observed.central_record_bytes.size() ||
            !bytes_equal_string(
                central.subspan(
                    central_fixed_size,
                    central_name_length),
                indexed.logical_path)) {
            return false;
        }

        const auto local_name_length = u16_le(local, 26U);
        const auto local_extra_length = u16_le(local, 28U);
        std::uint64_t expected_local_prefix = local_fixed_size;
        if (!checked_add(
                expected_local_prefix,
                local_name_length,
                expected_local_prefix) ||
            !checked_add(
                expected_local_prefix,
                local_extra_length,
                expected_local_prefix) ||
            expected_local_prefix != observed.local_prefix_bytes.size() ||
            u16_le(local, 6U) != indexed.flags ||
            u16_le(local, 8U) != indexed.compression_method ||
            local_name_length != indexed.logical_path.size() ||
            local_fixed_size + local_name_length >
                observed.local_prefix_bytes.size() ||
            !bytes_equal_string(
                local.subspan(local_fixed_size, local_name_length),
                indexed.logical_path)) {
            return false;
        }
    }
    return true;
}

} // namespace

ArtifactBoundNbzZipSerializationSnapshot::
ArtifactBoundNbzZipSerializationSnapshot(
    evidence::ArtifactIdentity artifact,
    NbzZipSerializationSnapshot serialization,
    std::string observed_sha256)
    : artifact_(std::move(artifact)),
      serialization_(std::move(serialization)),
      observed_sha256_(std::move(observed_sha256)) {}

const evidence::ArtifactIdentity&
ArtifactBoundNbzZipSerializationSnapshot::artifact() const noexcept {
    return artifact_;
}

const NbzZipSerializationSnapshot&
ArtifactBoundNbzZipSerializationSnapshot::serialization() const noexcept {
    return serialization_;
}

std::string_view
ArtifactBoundNbzZipSerializationSnapshot::observed_sha256() const noexcept {
    return observed_sha256_;
}

bool ArtifactBoundNbzZipSerializationSnapshot::valid() const noexcept {
    return artifact_.valid() && serialization_.valid() &&
        artifact_.size == serialization_.archive_size &&
        same_hex_digest(artifact_.sha256, observed_sha256_);
}

bool NbzZipArtifactBindingResult::ok() const noexcept {
    if (!snapshot.has_value() || !snapshot->valid()) {
        return false;
    }
    return std::none_of(
        diagnostics.begin(), diagnostics.end(),
        [](const Diagnostic& diagnostic) {
            return diagnostic.severity == DiagnosticSeverity::error;
        });
}

NbzZipArtifactBindingResult NbzZipArtifactSerializationBinder::bind(
    const NbzZipSource& source,
    const evidence::ArtifactIdentity& expected_artifact,
    NbzZipSerializationLimits serialization_limits,
    NbzZipArtifactBindingLimits binding_limits) {
    NbzZipArtifactBindingResult result;

    if (!expected_artifact.valid()) {
        add_error(
            result,
            "gdspaces.nbz.artifact-binding.invalid-artifact",
            "Artifact binding requires a valid expected ArtifactIdentity.");
        return result;
    }
    if (!source.valid() || !source.index_receipt().has_value() ||
        !source.index_receipt()->valid()) {
        add_error(
            result,
            "gdspaces.nbz.artifact-binding.invalid-source",
            "Artifact binding requires a valid indexed NBZ source.");
        return result;
    }
    if (expected_artifact.size != source.index_receipt()->archive_size) {
        add_error(
            result,
            "gdspaces.nbz.artifact-binding.identity-size",
            "Expected ArtifactIdentity size does not match the indexed NBZ archive size.");
        return result;
    }

    auto ranges = make_capture_plan(
        source, serialization_limits, result);
    if (!ranges.has_value()) {
        return result;
    }

    auto observed_sha256 = observe_exact_file(
        source.archive_path(),
        expected_artifact.size,
        binding_limits.hash_chunk_bytes,
        *ranges,
        result);
    if (!observed_sha256.has_value()) {
        return result;
    }
    if (!same_hex_digest(
            *observed_sha256, expected_artifact.sha256)) {
        add_error(
            result,
            "gdspaces.nbz.artifact-binding.hash-mismatch",
            "Observed SHA-256 does not match the expected ArtifactIdentity.");
        return result;
    }

    NbzZipSerializationReadExact observed_reader =
        [&ranges](std::uint64_t offset, std::span<std::byte> output) {
            return read_captured(*ranges, offset, output);
        };
    auto scan = NbzZipSerializationScanner::scan_with_reader(
        source, observed_reader, serialization_limits);
    result.diagnostics.insert(
        result.diagnostics.end(),
        scan.diagnostics.begin(),
        scan.diagnostics.end());
    if (!scan.ok()) {
        add_error(
            result,
            "gdspaces.nbz.artifact-binding.serialization-scan",
            "The NBZ serialization snapshot could not be produced from the exact hashed byte observation.");
        return result;
    }
    if (!snapshot_matches_index_observation(
            source, *scan.snapshot)) {
        add_error(
            result,
            "gdspaces.nbz.artifact-binding.index-observation-mismatch",
            "The indexed NBZ entry metadata does not match the exact hashed byte observation.");
        return result;
    }

    ArtifactBoundNbzZipSerializationSnapshot bound(
        expected_artifact,
        std::move(*scan.snapshot),
        std::move(*observed_sha256));
    if (!bound.valid()) {
        add_error(
            result,
            "gdspaces.nbz.artifact-binding.invalid-receipt",
            "Artifact-bound NBZ serialization snapshot failed its final receipt invariants.");
        return result;
    }

    result.snapshot = std::move(bound);
    return result;
}

} // namespace dmc::rengine::gdspaces
