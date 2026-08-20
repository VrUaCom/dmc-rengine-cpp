#include "dmc_rengine/gdspaces/nbz_zip_repacker.hpp"

#include "dmc_rengine/core/raw_deflate.hpp"
#include "dmc_rengine/core/sha256.hpp"

#include <algorithm>
#include <array>
#include <cctype>
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <limits>
#include <numeric>
#include <span>
#include <string>
#include <utility>
#include <vector>

namespace dmc::rengine::gdspaces {
namespace {

constexpr std::uint32_t local_signature = 0x04034B50U;
constexpr std::uint32_t central_signature = 0x02014B50U;
constexpr std::uint32_t eocd_signature = 0x06054B50U;
constexpr std::uint32_t descriptor_signature = 0x08074B50U;
constexpr std::size_t local_fixed_size = 30U;
constexpr std::size_t central_fixed_size = 46U;
constexpr std::size_t eocd_fixed_size = 22U;
constexpr std::uint16_t data_descriptor_flag = 0x0008U;
constexpr std::uint16_t encrypted_flag = 0x0001U;
constexpr std::uint64_t max_io_chunk_bytes = 64ULL * 1024ULL * 1024ULL;
constexpr std::uint64_t zip32_sentinel = 0xFFFFFFFFULL;

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

void put_u32_le(
    std::span<std::byte> bytes,
    std::size_t offset,
    std::uint32_t value) noexcept {
    for (unsigned shift = 0U; shift < 32U; shift += 8U) {
        bytes[offset + shift / 8U] =
            static_cast<std::byte>((value >> shift) & 0xFFU);
    }
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

[[nodiscard]] bool fits_zip32(std::uint64_t value) noexcept {
    return value < zip32_sentinel;
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

[[nodiscard]] bool has_error(
    const std::vector<Diagnostic>& diagnostics) noexcept {
    return std::any_of(
        diagnostics.begin(), diagnostics.end(),
        [](const Diagnostic& diagnostic) {
            return diagnostic.severity == DiagnosticSeverity::error;
        });
}

[[nodiscard]] NbzZipRetailRepackResult failure(
    NbzZipRetailRepackStatus status,
    std::string detail) {
    return NbzZipRetailRepackResult{
        .status = status,
        .receipt = std::nullopt,
        .detail = std::move(detail),
    };
}

struct TempFileGuard final {
    std::filesystem::path path;
    bool committed{};

    ~TempFileGuard() {
        if (!committed && !path.empty()) {
            std::error_code error;
            std::filesystem::remove(path, error);
        }
    }
};

class OutputState final {
public:
    OutputState(
        const std::filesystem::path& path,
        std::size_t chunk_bytes)
        : stream_(path, std::ios::binary | std::ios::trunc),
          chunk_bytes_(chunk_bytes) {}

    [[nodiscard]] bool valid() const noexcept {
        return static_cast<bool>(stream_);
    }

    [[nodiscard]] bool write(std::span<const std::byte> bytes) {
        std::size_t cursor = 0U;
        while (cursor < bytes.size()) {
            const auto amount = std::min(chunk_bytes_, bytes.size() - cursor);
            const auto part = bytes.subspan(cursor, amount);
            stream_.write(
                reinterpret_cast<const char*>(part.data()),
                static_cast<std::streamsize>(part.size()));
            if (!stream_ || !sha_.update(part)) {
                return false;
            }
            cursor += amount;
            size_ += static_cast<std::uint64_t>(amount);
        }
        return true;
    }

    [[nodiscard]] std::uint64_t size() const noexcept {
        return size_;
    }

    [[nodiscard]] std::optional<std::string> finalize_sha() const {
        const auto digest = sha_.finalize();
        if (!digest.has_value() || sha_.byte_count() != size_) {
            return std::nullopt;
        }
        return digest->hex();
    }

    [[nodiscard]] bool close() {
        stream_.flush();
        const bool good = static_cast<bool>(stream_);
        stream_.close();
        return good;
    }

private:
    std::ofstream stream_;
    std::size_t chunk_bytes_{};
    core::Sha256Accumulator sha_;
    std::uint64_t size_{};
};

class InputState final {
public:
    InputState(
        const std::filesystem::path& path,
        std::size_t chunk_bytes)
        : stream_(path, std::ios::binary),
          buffer_(chunk_bytes) {}

    [[nodiscard]] bool valid() const noexcept {
        return static_cast<bool>(stream_);
    }

    [[nodiscard]] bool consume(
        std::uint64_t count,
        OutputState* output = nullptr) {
        while (count != 0U) {
            const auto amount = static_cast<std::size_t>(
                std::min<std::uint64_t>(count, buffer_.size()));
            stream_.read(
                reinterpret_cast<char*>(buffer_.data()),
                static_cast<std::streamsize>(amount));
            if (stream_.gcount() != static_cast<std::streamsize>(amount)) {
                return false;
            }
            const auto chunk =
                std::span<const std::byte>{buffer_.data(), amount};
            if (!sha_.update(chunk) ||
                (output != nullptr && !output->write(chunk))) {
                return false;
            }
            cursor_ += static_cast<std::uint64_t>(amount);
            count -= static_cast<std::uint64_t>(amount);
        }
        return true;
    }

    [[nodiscard]] std::optional<std::vector<std::byte>> capture(
        std::size_t count) {
        std::vector<std::byte> bytes(count);
        if (count == 0U) {
            return bytes;
        }
        stream_.read(
            reinterpret_cast<char*>(bytes.data()),
            static_cast<std::streamsize>(count));
        if (stream_.gcount() != static_cast<std::streamsize>(count) ||
            !sha_.update(bytes)) {
            return std::nullopt;
        }
        cursor_ += static_cast<std::uint64_t>(count);
        return bytes;
    }

    [[nodiscard]] bool no_extra_byte() {
        char extra{};
        stream_.read(&extra, 1);
        return stream_.gcount() == 0;
    }

    [[nodiscard]] std::uint64_t cursor() const noexcept {
        return cursor_;
    }

    [[nodiscard]] std::optional<std::string> finalize_sha() const {
        const auto digest = sha_.finalize();
        if (!digest.has_value() || sha_.byte_count() != cursor_) {
            return std::nullopt;
        }
        return digest->hex();
    }

private:
    std::ifstream stream_;
    std::vector<std::byte> buffer_;
    core::Sha256Accumulator sha_;
    std::uint64_t cursor_{};
};

struct PhysicalRegion final {
    std::uint64_t old_offset{};
    std::uint64_t old_size{};
    std::vector<std::uint32_t> central_indices;
    const NbzZipMemberReplacement* replacement{};
    bool changed{};
    std::uint16_t flags{};
    std::uint16_t method{};
    std::uint32_t old_crc{};
    std::uint32_t old_compressed_size{};
    std::uint32_t old_uncompressed_size{};
    std::vector<std::byte> patched_local_prefix;
    std::vector<std::byte> encoded_payload;
    std::uint32_t new_crc{};
    std::uint32_t new_compressed_size{};
    std::uint32_t new_uncompressed_size{};
    std::uint32_t new_offset{};
};

[[nodiscard]] std::span<const std::byte> replacement_payload(
    const PhysicalRegion& region) noexcept {
    if (region.method == 0U) {
        return std::span<const std::byte>{region.replacement->materialized_bytes};
    }
    return std::span<const std::byte>{region.encoded_payload};
}

[[nodiscard]] bool patch_local_metadata(
    PhysicalRegion& region,
    const NbzZipEntry& indexed) {
    if (region.patched_local_prefix.size() < local_fixed_size ||
        u32_le(region.patched_local_prefix, 0U) != local_signature ||
        u16_le(region.patched_local_prefix, 6U) != indexed.flags ||
        u16_le(region.patched_local_prefix, 8U) != indexed.compression_method) {
        return false;
    }

    const auto local_crc = u32_le(region.patched_local_prefix, 14U);
    const auto local_compressed = u32_le(region.patched_local_prefix, 18U);
    const auto local_uncompressed = u32_le(region.patched_local_prefix, 22U);
    const bool descriptor = (indexed.flags & data_descriptor_flag) != 0U;

    if (descriptor) {
        const bool all_zero = local_crc == 0U &&
            local_compressed == 0U && local_uncompressed == 0U;
        const bool mirrors_central = local_crc == indexed.crc32 &&
            local_compressed == indexed.compressed_size &&
            local_uncompressed == indexed.uncompressed_size;
        if (!all_zero && !mirrors_central) {
            return false;
        }
        if (mirrors_central) {
            put_u32_le(region.patched_local_prefix, 14U, region.new_crc);
            put_u32_le(
                region.patched_local_prefix, 18U, region.new_compressed_size);
            put_u32_le(
                region.patched_local_prefix, 22U, region.new_uncompressed_size);
        }
        return true;
    }

    if (local_crc != indexed.crc32 ||
        local_compressed != indexed.compressed_size ||
        local_uncompressed != indexed.uncompressed_size) {
        return false;
    }
    put_u32_le(region.patched_local_prefix, 14U, region.new_crc);
    put_u32_le(region.patched_local_prefix, 18U, region.new_compressed_size);
    put_u32_le(region.patched_local_prefix, 22U, region.new_uncompressed_size);
    return true;
}

[[nodiscard]] bool same_replacement_bytes(
    const NbzZipMemberReplacement& left,
    const NbzZipMemberReplacement& right) noexcept {
    return left.materialized_bytes == right.materialized_bytes;
}

[[nodiscard]] std::vector<std::byte> make_descriptor(
    bool with_signature,
    std::uint32_t crc,
    std::uint32_t compressed,
    std::uint32_t uncompressed) {
    std::vector<std::byte> bytes(with_signature ? 16U : 12U);
    std::size_t cursor = 0U;
    if (with_signature) {
        put_u32_le(bytes, 0U, descriptor_signature);
        cursor = 4U;
    }
    put_u32_le(bytes, cursor, crc);
    put_u32_le(bytes, cursor + 4U, compressed);
    put_u32_le(bytes, cursor + 8U, uncompressed);
    return bytes;
}

[[nodiscard]] std::optional<std::size_t> descriptor_size(
    std::span<const std::byte> probe,
    const PhysicalRegion& region) noexcept {
    if (probe.size() >= 16U &&
        u32_le(probe, 0U) == descriptor_signature &&
        u32_le(probe, 4U) == region.old_crc &&
        u32_le(probe, 8U) == region.old_compressed_size &&
        u32_le(probe, 12U) == region.old_uncompressed_size) {
        return 16U;
    }
    if (probe.size() >= 12U &&
        u32_le(probe, 0U) == region.old_crc &&
        u32_le(probe, 4U) == region.old_compressed_size &&
        u32_le(probe, 8U) == region.old_uncompressed_size) {
        return 12U;
    }
    return std::nullopt;
}

[[nodiscard]] std::string chain_for(std::uint32_t central_index) {
    return "nbz[" + std::to_string(central_index) + "]";
}

} // namespace

bool NbzZipRetailRepackReceipt::valid() const noexcept {
    if (source_sha256.size() != 64U || output_sha256.size() != 64U ||
        source_size < eocd_fixed_size || output_size < eocd_fixed_size ||
        !fits_zip32(central_offset) || !fits_zip32(central_size) ||
        static_cast<std::uint64_t>(central_offset) + central_size > output_size) {
        return false;
    }
    for (std::size_t index = 0U; index < entries.size(); ++index) {
        const auto& entry = entries[index];
        if (entry.central_index != static_cast<std::uint32_t>(index) ||
            (entry.changed && entry.compression_method != 0U &&
             entry.compression_method != 8U) ||
            !fits_zip32(entry.local_header_offset)) {
            return false;
        }
    }
    return true;
}

NbzZipRetailRepackResult NbzZipRetailRepacker::write(
    const NbzZipSource& source,
    const ArtifactBoundNbzZipSerializationSnapshot& bound,
    std::span<const NbzZipMemberReplacement> replacements,
    const std::filesystem::path& destination,
    NbzZipRetailRepackLimits limits) {
    if (!source.valid() || !source.index_receipt().has_value()) {
        return failure(
            NbzZipRetailRepackStatus::invalid_source,
            "Retail repacking requires a valid indexed NBZ source.");
    }
    if (!bound.valid() ||
        bound.serialization().source_id != source.id() ||
        bound.artifact().size != source.index_receipt()->archive_size ||
        bound.serialization().archive_size != bound.artifact().size ||
        bound.serialization().entries.size() != source.entries().size()) {
        return failure(
            NbzZipRetailRepackStatus::invalid_binding,
            "Retail repacking requires an artifact-bound serialization snapshot for this exact indexed source.");
    }
    if (limits.io_chunk_bytes == 0U ||
        limits.io_chunk_bytes > max_io_chunk_bytes ||
        limits.io_chunk_bytes > static_cast<std::uint64_t>(
            std::numeric_limits<std::size_t>::max()) ||
        limits.io_chunk_bytes > static_cast<std::uint64_t>(
            std::numeric_limits<std::streamsize>::max())) {
        return failure(
            NbzZipRetailRepackStatus::invalid_destination,
            "The retail-repack I/O chunk size is outside supported product bounds.");
    }

    std::error_code path_error;
    const auto source_absolute = std::filesystem::absolute(
        source.archive_path(), path_error).lexically_normal();
    if (path_error) {
        return failure(
            NbzZipRetailRepackStatus::invalid_destination,
            "The source archive path could not be normalized.");
    }
    path_error.clear();
    const auto destination_absolute = std::filesystem::absolute(
        destination, path_error).lexically_normal();
    if (path_error || destination.empty() ||
        source_absolute == destination_absolute ||
        std::filesystem::exists(destination, path_error)) {
        return failure(
            NbzZipRetailRepackStatus::invalid_destination,
            "Retail repacking requires a distinct, non-existing destination path.");
    }

    const auto temp_path = std::filesystem::path{
        destination.string() + ".dmc-rengine-repack.tmp"};
    path_error.clear();
    if (std::filesystem::exists(temp_path, path_error)) {
        return failure(
            NbzZipRetailRepackStatus::invalid_destination,
            "The deterministic temporary retail-repack path already exists.");
    }
    TempFileGuard temp_guard{.path = temp_path, .committed = false};

    const auto chunk_bytes = static_cast<std::size_t>(limits.io_chunk_bytes);
    InputState input(source.archive_path(), chunk_bytes);
    if (!input.valid()) {
        return failure(
            NbzZipRetailRepackStatus::source_open_failed,
            "The exact source archive could not be opened for streaming repack observation.");
    }
    OutputState output(temp_path, chunk_bytes);
    if (!output.valid()) {
        return failure(
            NbzZipRetailRepackStatus::destination_open_failed,
            "The temporary retail-repack destination could not be opened.");
    }

    const auto& indexed_entries = source.entries();
    const auto& snapshot = bound.serialization();
    std::vector<const NbzZipMemberReplacement*> replacement_by_index(
        indexed_entries.size(), nullptr);
    for (const auto& replacement : replacements) {
        if (replacement.central_index >= indexed_entries.size() ||
            indexed_entries[replacement.central_index].directory) {
            return failure(
                NbzZipRetailRepackStatus::invalid_replacement,
                "A retail replacement targets an unknown or directory central identity.");
        }
        auto*& slot = replacement_by_index[replacement.central_index];
        if (slot != nullptr) {
            return failure(
                NbzZipRetailRepackStatus::duplicate_replacement,
                "A central identity appears more than once in the replacement set.");
        }
        slot = &replacement;
    }

    // Zero-edit repack is a strict identity copy. This is an important no-loss
    // invariant: no metadata is normalized merely because the writer was used.
    if (replacements.empty()) {
        if (!input.consume(bound.artifact().size, &output) ||
            !input.no_extra_byte()) {
            return failure(
                NbzZipRetailRepackStatus::source_read_failed,
                "The complete source artifact could not be streamed into the identity output.");
        }
        const auto observed = input.finalize_sha();
        if (!observed.has_value() ||
            !same_hex_digest(*observed, bound.artifact().sha256)) {
            return failure(
                NbzZipRetailRepackStatus::source_artifact_mismatch,
                "The source bytes changed after artifact binding; identity repack was discarded.");
        }
        const auto output_sha = output.finalize_sha();
        const auto output_size = output.size();
        if (!output_sha.has_value() || !output.close()) {
            return failure(
                NbzZipRetailRepackStatus::destination_write_failed,
                "The byte-identical retail output could not be finalized.");
        }

        NbzZipSource reopened(
            std::string{source.id()} + "::repack-validation", temp_path);
        if (!reopened.valid() || reopened.entries().size() != indexed_entries.size()) {
            return failure(
                NbzZipRetailRepackStatus::canonical_reopen_failed,
                "The byte-identical temporary retail output did not reopen through canonical NbzZipSource.");
        }

        NbzZipRetailRepackReceipt receipt{
            .source_sha256 = bound.artifact().sha256,
            .output_sha256 = *output_sha,
            .source_size = bound.artifact().size,
            .output_size = output_size,
            .central_offset = static_cast<std::uint32_t>(
                source.index_receipt()->computed_central_start),
            .central_size = source.index_receipt()->declared_central_size,
            .entries = {},
        };
        receipt.entries.reserve(indexed_entries.size());
        for (const auto& entry : indexed_entries) {
            receipt.entries.push_back(NbzZipRetailRepackEntryReceipt{
                .central_index = entry.central_index,
                .changed = false,
                .compression_method = entry.compression_method,
                .crc32 = entry.crc32,
                .compressed_size = entry.compressed_size,
                .uncompressed_size = entry.uncompressed_size,
                .local_header_offset = entry.local_header_offset,
            });
        }
        if (!receipt.valid() ||
            !same_hex_digest(receipt.source_sha256, receipt.output_sha256) ||
            receipt.source_size != receipt.output_size) {
            return failure(
                NbzZipRetailRepackStatus::canonical_validation_failed,
                "Identity retail repack did not preserve the complete artifact identity.");
        }

        path_error.clear();
        std::filesystem::rename(temp_path, destination, path_error);
        if (path_error) {
            return failure(
                NbzZipRetailRepackStatus::destination_commit_failed,
                "The validated temporary retail output could not be committed to its destination.");
        }
        temp_guard.committed = true;
        return NbzZipRetailRepackResult{
            .status = NbzZipRetailRepackStatus::ok,
            .receipt = std::move(receipt),
            .detail = {},
        };
    }

    std::vector<std::size_t> physical_order(indexed_entries.size());
    std::iota(physical_order.begin(), physical_order.end(), 0U);
    std::sort(
        physical_order.begin(), physical_order.end(),
        [&snapshot](std::size_t left, std::size_t right) {
            const auto lhs = snapshot.entries[left].local_record_offset;
            const auto rhs = snapshot.entries[right].local_record_offset;
            if (lhs != rhs) {
                return lhs < rhs;
            }
            return left < right;
        });

    std::vector<PhysicalRegion> regions;
    std::vector<std::size_t> region_by_index(
        indexed_entries.size(), std::numeric_limits<std::size_t>::max());
    for (const auto raw_index : physical_order) {
        const auto local_offset = snapshot.entries[raw_index].local_record_offset;
        if (regions.empty() || regions.back().old_offset != local_offset) {
            regions.push_back(PhysicalRegion{
                .old_offset = local_offset,
                .old_size = snapshot.entries[raw_index].local_region_size,
                .central_indices = {},
            });
        } else if (regions.back().old_size !=
                   snapshot.entries[raw_index].local_region_size) {
            return failure(
                NbzZipRetailRepackStatus::invalid_binding,
                "Aliased central identities disagree on the physical local-region span.");
        }
        regions.back().central_indices.push_back(
            static_cast<std::uint32_t>(raw_index));
        region_by_index[raw_index] = regions.size() - 1U;
    }

    std::uint64_t new_local_cursor = snapshot.prefix_size;
    for (auto& region : regions) {
        const auto representative_index = region.central_indices.front();
        const auto& representative = indexed_entries[representative_index];
        const auto& preserved = snapshot.entries[representative_index];
        region.flags = representative.flags;
        region.method = representative.compression_method;
        region.old_crc = representative.crc32;
        region.old_compressed_size = representative.compressed_size;
        region.old_uncompressed_size = representative.uncompressed_size;

        const NbzZipMemberReplacement* selected = nullptr;
        std::size_t replacement_count = 0U;
        for (const auto central_index : region.central_indices) {
            const auto& indexed = indexed_entries[central_index];
            if (indexed.flags != representative.flags ||
                indexed.compression_method != representative.compression_method ||
                indexed.crc32 != representative.crc32 ||
                indexed.compressed_size != representative.compressed_size ||
                indexed.uncompressed_size != representative.uncompressed_size) {
                return failure(
                    NbzZipRetailRepackStatus::alias_replacement_conflict,
                    "Aliased central identities do not share one coherent stored-member metadata contract.");
            }
            const auto* candidate = replacement_by_index[central_index];
            if (candidate != nullptr) {
                ++replacement_count;
                if (selected == nullptr) {
                    selected = candidate;
                } else if (!same_replacement_bytes(*selected, *candidate)) {
                    return failure(
                        NbzZipRetailRepackStatus::alias_replacement_conflict,
                        "Aliased central identities were given different replacement materialized bytes.");
                }
            }
        }
        if (replacement_count != 0U &&
            replacement_count != region.central_indices.size()) {
            return failure(
                NbzZipRetailRepackStatus::alias_replacement_incomplete,
                "Every central identity sharing a physical local record must explicitly acknowledge the same replacement.");
        }

        region.changed = selected != nullptr;
        region.replacement = selected;
        if (region.changed) {
            if ((region.flags & encrypted_flag) != 0U ||
                (region.method != 0U && region.method != 8U)) {
                return failure(
                    NbzZipRetailRepackStatus::unsupported_compression_method,
                    "Changed retail members currently require unencrypted STORE or raw-DEFLATE source methods.");
            }
            if (!fits_zip32(selected->materialized_bytes.size())) {
                return failure(
                    NbzZipRetailRepackStatus::zip32_overflow,
                    "Replacement materialized bytes exceed the classic ZIP32 size domain.");
            }

            region.new_crc = crc32_of(selected->materialized_bytes);
            region.new_uncompressed_size = static_cast<std::uint32_t>(
                selected->materialized_bytes.size());
            if (region.method == 8U) {
                auto encoded = core::RawDeflate::deflate_stored(
                    selected->materialized_bytes);
                if (!encoded.ok()) {
                    return failure(
                        NbzZipRetailRepackStatus::canonical_validation_failed,
                        encoded.detail.empty()
                            ? "Raw-DEFLATE replacement authoring failed."
                            : std::move(encoded.detail));
                }
                region.encoded_payload = std::move(encoded.bytes);
            }
            const auto payload = replacement_payload(region);
            if (!fits_zip32(payload.size())) {
                return failure(
                    NbzZipRetailRepackStatus::zip32_overflow,
                    "Replacement stored bytes exceed the classic ZIP32 size domain.");
            }
            region.new_compressed_size = static_cast<std::uint32_t>(payload.size());
            region.patched_local_prefix = preserved.local_prefix_bytes;
            if (!patch_local_metadata(region, representative)) {
                return failure(
                    NbzZipRetailRepackStatus::unresolved_local_metadata,
                    "Changed local ZIP metadata is not in a safely rewritable zero-or-central-mirror form.");
            }
        } else {
            region.new_crc = region.old_crc;
            region.new_compressed_size = region.old_compressed_size;
            region.new_uncompressed_size = region.old_uncompressed_size;
        }

        if (!fits_zip32(new_local_cursor)) {
            return failure(
                NbzZipRetailRepackStatus::zip32_overflow,
                "A rebuilt local-header offset reaches the classic ZIP32 sentinel domain.");
        }
        region.new_offset = static_cast<std::uint32_t>(new_local_cursor);

        std::uint64_t next_cursor{};
        const auto new_region_size = region.changed
            ? region.old_size - region.old_compressed_size +
                region.new_compressed_size
            : region.old_size;
        if (!checked_add(new_local_cursor, new_region_size, next_cursor) ||
            !fits_zip32(next_cursor)) {
            return failure(
                NbzZipRetailRepackStatus::zip32_overflow,
                "Rebuilt local-region layout exceeds the classic ZIP32 address domain.");
        }
        new_local_cursor = next_cursor;
    }

    if (snapshot.prefix_size > bound.artifact().size ||
        !input.consume(snapshot.prefix_size, &output)) {
        return failure(
            NbzZipRetailRepackStatus::source_read_failed,
            "The opaque source prefix could not be copied from the repack observation.");
    }

    for (const auto& region : regions) {
        if (input.cursor() != region.old_offset) {
            return failure(
                NbzZipRetailRepackStatus::invalid_binding,
                "Physical local regions are not contiguous with the preserved source topology.");
        }
        if (!region.changed) {
            if (!input.consume(region.old_size, &output)) {
                return failure(
                    NbzZipRetailRepackStatus::source_read_failed,
                    "An unchanged physical local region could not be copied byte-for-byte.");
            }
            continue;
        }

        const auto& representative =
            indexed_entries[region.central_indices.front()];
        const auto prefix_size = region.patched_local_prefix.size();
        if (prefix_size > region.old_size ||
            representative.compressed_size > region.old_size - prefix_size) {
            return failure(
                NbzZipRetailRepackStatus::invalid_binding,
                "Changed local region does not contain its preserved prefix and stored data span.");
        }
        const auto tail_size = region.old_size - prefix_size -
            representative.compressed_size;

        if (!input.consume(prefix_size) ||
            !output.write(region.patched_local_prefix) ||
            !input.consume(representative.compressed_size) ||
            !output.write(replacement_payload(region))) {
            return failure(
                NbzZipRetailRepackStatus::destination_write_failed,
                "Changed local prefix or replacement stored payload could not be streamed safely.");
        }

        if ((region.flags & data_descriptor_flag) == 0U) {
            if (!input.consume(tail_size, &output)) {
                return failure(
                    NbzZipRetailRepackStatus::source_read_failed,
                    "Opaque post-data bytes could not be preserved after the changed member.");
            }
            continue;
        }

        if (tail_size < 12U) {
            return failure(
                NbzZipRetailRepackStatus::unresolved_data_descriptor,
                "A bit-3 changed member has no room for a classic ZIP data descriptor.");
        }
        const auto probe_size = static_cast<std::size_t>(
            std::min<std::uint64_t>(16U, tail_size));
        auto probe = input.capture(probe_size);
        if (!probe.has_value()) {
            return failure(
                NbzZipRetailRepackStatus::source_read_failed,
                "The original data-descriptor boundary could not be observed.");
        }
        const auto old_descriptor_size = descriptor_size(*probe, region);
        if (!old_descriptor_size.has_value()) {
            return failure(
                NbzZipRetailRepackStatus::unresolved_data_descriptor,
                "The original bit-3 descriptor is neither the supported signed nor unsigned classic form.");
        }
        const bool signed_descriptor = *old_descriptor_size == 16U;
        const auto rewritten_descriptor = make_descriptor(
            signed_descriptor,
            region.new_crc,
            region.new_compressed_size,
            region.new_uncompressed_size);
        if (!output.write(rewritten_descriptor)) {
            return failure(
                NbzZipRetailRepackStatus::destination_write_failed,
                "The rewritten data descriptor could not be emitted.");
        }
        if (probe->size() > *old_descriptor_size &&
            !output.write(std::span<const std::byte>{*probe}.subspan(
                *old_descriptor_size))) {
            return failure(
                NbzZipRetailRepackStatus::destination_write_failed,
                "Opaque bytes immediately following the data descriptor could not be preserved.");
        }
        const auto remaining_tail =
            tail_size - static_cast<std::uint64_t>(probe_size);
        if (!input.consume(remaining_tail, &output)) {
            return failure(
                NbzZipRetailRepackStatus::source_read_failed,
                "Opaque tail bytes after the rewritten descriptor could not be preserved.");
        }
    }

    if (input.cursor() != snapshot.computed_central_start ||
        snapshot.computed_central_start > bound.artifact().size ||
        !input.consume(bound.artifact().size - input.cursor()) ||
        !input.no_extra_byte()) {
        return failure(
            NbzZipRetailRepackStatus::source_read_failed,
            "The source artifact could not be completely observed through the retail-repack I/O boundary.");
    }
    const auto observed_source_sha = input.finalize_sha();
    if (!observed_source_sha.has_value() ||
        !same_hex_digest(*observed_source_sha, bound.artifact().sha256)) {
        return failure(
            NbzZipRetailRepackStatus::source_artifact_mismatch,
            "The source artifact changed after binding; all partially authored output was discarded.");
    }

    const auto new_central_offset = output.size();
    if (!fits_zip32(new_central_offset)) {
        return failure(
            NbzZipRetailRepackStatus::zip32_overflow,
            "The rebuilt central-directory offset reaches the classic ZIP32 sentinel domain.");
    }

    std::vector<NbzZipRetailRepackEntryReceipt> entry_receipts;
    entry_receipts.reserve(indexed_entries.size());
    for (std::size_t index = 0U; index < indexed_entries.size(); ++index) {
        const auto& indexed = indexed_entries[index];
        const auto& preserved = snapshot.entries[index];
        const auto& region = regions[region_by_index[index]];
        if (preserved.central_record_bytes.size() < central_fixed_size ||
            u32_le(preserved.central_record_bytes, 0U) != central_signature) {
            return failure(
                NbzZipRetailRepackStatus::invalid_binding,
                "A preserved central record cannot be safely rebuilt.");
        }
        auto central = preserved.central_record_bytes;
        put_u32_le(central, 42U, region.new_offset);
        if (region.changed) {
            put_u32_le(central, 16U, region.new_crc);
            put_u32_le(central, 20U, region.new_compressed_size);
            put_u32_le(central, 24U, region.new_uncompressed_size);
        }
        if (!output.write(central)) {
            return failure(
                NbzZipRetailRepackStatus::destination_write_failed,
                "A rebuilt raw central-directory record could not be emitted.");
        }
        entry_receipts.push_back(NbzZipRetailRepackEntryReceipt{
            .central_index = static_cast<std::uint32_t>(index),
            .changed = region.changed,
            .compression_method = indexed.compression_method,
            .crc32 = region.new_crc,
            .compressed_size = region.new_compressed_size,
            .uncompressed_size = region.new_uncompressed_size,
            .local_header_offset = region.new_offset,
        });
    }

    const auto new_central_size = output.size() - new_central_offset;
    if (!fits_zip32(new_central_size) ||
        snapshot.eocd_bytes.size() < eocd_fixed_size ||
        u32_le(snapshot.eocd_bytes, 0U) != eocd_signature) {
        return failure(
            NbzZipRetailRepackStatus::zip32_overflow,
            "Rebuilt central-directory or EOCD metadata is outside the supported classic ZIP domain.");
    }
    auto eocd = snapshot.eocd_bytes;
    put_u32_le(
        eocd, 12U, static_cast<std::uint32_t>(new_central_size));
    put_u32_le(
        eocd, 16U, static_cast<std::uint32_t>(new_central_offset));
    if (!output.write(eocd) || !fits_zip32(output.size())) {
        return failure(
            NbzZipRetailRepackStatus::destination_write_failed,
            "The rebuilt terminal EOCD/comment region could not be emitted within ZIP32 bounds.");
    }

    const auto output_sha = output.finalize_sha();
    const auto output_size = output.size();
    if (!output_sha.has_value() || !output.close()) {
        return failure(
            NbzZipRetailRepackStatus::destination_write_failed,
            "The rebuilt retail NBZ could not be finalized.");
    }

    NbzZipSource reopened(
        std::string{source.id()} + "::repack-validation", temp_path);
    if (!reopened.valid() || reopened.entries().size() != indexed_entries.size()) {
        return failure(
            NbzZipRetailRepackStatus::canonical_reopen_failed,
            "The rebuilt temporary retail NBZ did not reopen through canonical NbzZipSource.");
    }
    for (std::size_t index = 0U; index < reopened.entries().size(); ++index) {
        const auto& actual = reopened.entries()[index];
        const auto& expected = entry_receipts[index];
        if (actual.central_index != expected.central_index ||
            actual.logical_path != indexed_entries[index].logical_path ||
            actual.flags != indexed_entries[index].flags ||
            actual.compression_method != expected.compression_method ||
            actual.crc32 != expected.crc32 ||
            actual.compressed_size != expected.compressed_size ||
            actual.uncompressed_size != expected.uncompressed_size ||
            actual.local_header_offset != expected.local_header_offset) {
            return failure(
                NbzZipRetailRepackStatus::canonical_validation_failed,
                "Canonical reopen metadata differs from the writer receipt.");
        }
        const auto* replacement = replacement_by_index[index];
        if (replacement == nullptr) {
            continue;
        }
        const ResourceId id{
            .source_id = std::string{reopened.id()},
            .logical_path = actual.logical_path,
            .container_chain = chain_for(actual.central_index),
            .offset = 0U,
            .size = actual.uncompressed_size,
        };
        const auto payload = reopened.read(id);
        if (!payload.has_value() || has_error(payload->diagnostics) ||
            payload->bytes != replacement->materialized_bytes) {
            return failure(
                NbzZipRetailRepackStatus::canonical_validation_failed,
                "A changed member did not materialize back to the exact requested replacement bytes.");
        }
    }

    NbzZipRetailRepackReceipt receipt{
        .source_sha256 = bound.artifact().sha256,
        .output_sha256 = *output_sha,
        .source_size = bound.artifact().size,
        .output_size = output_size,
        .central_offset = static_cast<std::uint32_t>(new_central_offset),
        .central_size = static_cast<std::uint32_t>(new_central_size),
        .entries = std::move(entry_receipts),
    };
    if (!receipt.valid()) {
        return failure(
            NbzZipRetailRepackStatus::canonical_validation_failed,
            "The completed retail NBZ writer receipt violates its own bounds.");
    }

    path_error.clear();
    std::filesystem::rename(temp_path, destination, path_error);
    if (path_error) {
        return failure(
            NbzZipRetailRepackStatus::destination_commit_failed,
            "The validated temporary retail NBZ could not be committed to its destination.");
    }
    temp_guard.committed = true;
    return NbzZipRetailRepackResult{
        .status = NbzZipRetailRepackStatus::ok,
        .receipt = std::move(receipt),
        .detail = {},
    };
}

} // namespace dmc::rengine::gdspaces
