#include "dmc_rengine/profiles/dmc3/nbz_retail_repacker.hpp"

#include "dmc_rengine/core/sha256.hpp"

#include <algorithm>
#include <cctype>
#include <cstddef>
#include <cstdint>
#include <fstream>
#include <limits>
#include <map>
#include <span>
#include <string>
#include <system_error>
#include <utility>
#include <vector>

namespace dmc::rengine::profiles::dmc3 {
namespace {

constexpr std::size_t local_fixed_size = 30U;
constexpr std::size_t central_fixed_size = 46U;
constexpr std::size_t eocd_fixed_size = 22U;
constexpr std::uint16_t store_method = 0U;

[[nodiscard]] bool hex_equal(std::string_view left, std::string_view right) noexcept {
    if (left.size() != right.size()) {
        return false;
    }
    for (std::size_t index = 0U; index < left.size(); ++index) {
        const auto lhs = static_cast<unsigned char>(left[index]);
        const auto rhs = static_cast<unsigned char>(right[index]);
        if (std::tolower(lhs) != std::tolower(rhs)) {
            return false;
        }
    }
    return true;
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

void put_u16(
    std::vector<std::byte>& bytes,
    std::size_t offset,
    std::uint16_t value) noexcept {
    bytes[offset] = static_cast<std::byte>(value & 0xFFU);
    bytes[offset + 1U] = static_cast<std::byte>((value >> 8U) & 0xFFU);
}

void put_u32(
    std::vector<std::byte>& bytes,
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
    return value < static_cast<std::uint64_t>(
        NbzRetailRepacker::zip32_u32_sentinel);
}

[[nodiscard]] NbzRetailRepackResult failure(
    NbzRetailRepackStatus status,
    std::string detail) {
    return NbzRetailRepackResult{
        .status = status,
        .receipt = std::nullopt,
        .detail = std::move(detail),
    };
}

struct LocalRegion final {
    const gdspaces::NbzZipSerializationEntry* serialization{};
    std::vector<std::size_t> central_indices;
    const NbzRetailReplacement* replacement{};
    std::uint64_t output_offset{};
    std::uint64_t output_size{};
    std::uint64_t tail_start{};
};

} // namespace

bool NbzRetailRepackEntryReceipt::valid() const noexcept {
    if (logical_path.empty() ||
        original_local_header_offset == zip32_u32_sentinel ||
        output_local_header_offset == zip32_u32_sentinel ||
        output_compressed_size == zip32_u32_sentinel ||
        output_uncompressed_size == zip32_u32_sentinel) {
        return false;
    }
    if (changed) {
        return output_method == store_method &&
            output_compressed_size == output_uncompressed_size;
    }
    return output_method == original_method;
}

bool NbzRetailRepackReceipt::valid() const noexcept {
    if (!source_artifact.valid() || !output_artifact.valid() ||
        output_central_offset == zip32_u32_sentinel ||
        output_central_size == zip32_u32_sentinel) {
        return false;
    }

    std::uint32_t counted_changed = 0U;
    for (std::size_t index = 0U; index < entries.size(); ++index) {
        const auto& entry = entries[index];
        if (entry.central_index != static_cast<std::uint32_t>(index) ||
            !entry.valid()) {
            return false;
        }
        if (entry.changed) {
            ++counted_changed;
        }
    }
    if (counted_changed != changed_entry_count) {
        return false;
    }

    if (byte_identical &&
        (source_artifact.size != output_artifact.size ||
         !hex_equal(source_artifact.sha256, output_artifact.sha256))) {
        return false;
    }
    return true;
}

NbzRetailRepackResult NbzRetailRepacker::write(
    const gdspaces::NbzZipSource& source,
    const gdspaces::ArtifactBoundNbzZipSerializationSnapshot& bound,
    const std::filesystem::path& output_path,
    std::span<const NbzRetailReplacement> replacements,
    NbzRetailRepackLimits limits) {
    if (!source.valid() || !source.index_receipt().has_value() ||
        !source.index_receipt()->valid()) {
        return failure(
            NbzRetailRepackStatus::invalid_source,
            "Retail repack requires a valid indexed NBZ source.");
    }
    if (!bound.valid() || bound.serialization().source_id != source.id() ||
        bound.artifact().size != bound.serialization().archive_size ||
        bound.artifact().size != source.index_receipt()->archive_size) {
        return failure(
            NbzRetailRepackStatus::invalid_bound_snapshot,
            "Retail repack requires an artifact-bound snapshot for the exact source.");
    }
    if (limits.io_chunk_bytes == 0U ||
        limits.io_chunk_bytes > max_io_chunk_bytes ||
        limits.max_replacement_bytes >= zip32_u32_sentinel ||
        limits.max_metadata_bytes == 0U ||
        limits.max_metadata_bytes > max_metadata_bytes) {
        return failure(
            NbzRetailRepackStatus::invalid_limits,
            "Retail repack limits are outside the bounded product domain.");
    }
    if (output_path.empty() || output_path.filename().empty()) {
        return failure(
            NbzRetailRepackStatus::invalid_output_path,
            "Retail repack output path must name a new archive file.");
    }

    std::error_code filesystem_error;
    if (std::filesystem::exists(output_path, filesystem_error)) {
        return failure(
            NbzRetailRepackStatus::output_exists,
            "Retail repack refuses to overwrite an existing output path.");
    }
    if (filesystem_error) {
        return failure(
            NbzRetailRepackStatus::invalid_output_path,
            "Retail repack could not inspect the output path.");
    }

    const auto& serialization = bound.serialization();
    const auto& source_entries = source.entries();
    if (!serialization.valid() ||
        serialization.entries.size() != source_entries.size()) {
        return failure(
            NbzRetailRepackStatus::invalid_bound_snapshot,
            "Serialization/index entry counts are inconsistent.");
    }

    std::vector<const NbzRetailReplacement*> replacement_by_index(
        source_entries.size(), nullptr);
    for (const auto& replacement : replacements) {
        if (replacement.central_index >= source_entries.size()) {
            return failure(
                NbzRetailRepackStatus::replacement_not_found,
                "A replacement central index is outside the source archive.");
        }
        auto*& destination = replacement_by_index[replacement.central_index];
        if (destination != nullptr) {
            return failure(
                NbzRetailRepackStatus::duplicate_replacement,
                "A central entry was supplied more than once for replacement.");
        }

        const auto& entry = source_entries[replacement.central_index];
        if (replacement.expected_logical_path != entry.logical_path) {
            return failure(
                NbzRetailRepackStatus::replacement_path_mismatch,
                "Replacement logical path does not match the exact central entry.");
        }
        if (entry.directory) {
            return failure(
                NbzRetailRepackStatus::directory_replacement,
                "Directory central entries are not writable resource members.");
        }
        if (replacement.bytes.size() > limits.max_replacement_bytes ||
            !fits_zip32(replacement.bytes.size())) {
            return failure(
                NbzRetailRepackStatus::replacement_too_large,
                "Replacement payload exceeds the bounded classic-ZIP domain.");
        }
        destination = &replacement;
    }

    std::map<std::uint64_t, LocalRegion> region_map;
    for (std::size_t index = 0U; index < serialization.entries.size(); ++index) {
        const auto& serialized = serialization.entries[index];
        const auto& indexed = source_entries[index];
        if (serialized.central_index != index || indexed.central_index != index ||
            serialized.local_record_offset != indexed.local_header_offset ||
            serialized.data_offset != indexed.data_offset ||
            serialized.stored_data_size != indexed.compressed_size) {
            return failure(
                NbzRetailRepackStatus::invalid_bound_snapshot,
                "Bound serialization geometry does not match the indexed source entry.");
        }

        auto [iterator, inserted] = region_map.try_emplace(
            serialized.local_record_offset,
            LocalRegion{
                .serialization = &serialized,
                .central_indices = {},
                .replacement = nullptr,
                .output_offset = 0U,
                .output_size = 0U,
                .tail_start = 0U,
            });
        auto& region = iterator->second;
        if (!inserted) {
            const auto& canonical = *region.serialization;
            if (canonical.local_region_size != serialized.local_region_size ||
                canonical.local_prefix_bytes != serialized.local_prefix_bytes ||
                canonical.data_offset != serialized.data_offset ||
                canonical.stored_data_size != serialized.stored_data_size ||
                canonical.uses_data_descriptor != serialized.uses_data_descriptor) {
                return failure(
                    NbzRetailRepackStatus::invalid_bound_snapshot,
                    "Aliased local records disagree on preserved physical framing.");
            }
        }
        region.central_indices.push_back(index);
    }

    for (auto& [offset, region] : region_map) {
        (void)offset;
        for (const auto central_index : region.central_indices) {
            if (replacement_by_index[central_index] != nullptr) {
                if (region.replacement != nullptr) {
                    return failure(
                        NbzRetailRepackStatus::alias_local_region_unsupported,
                        "Multiple replacement identities target one shared local region.");
                }
                region.replacement = replacement_by_index[central_index];
            }
        }
        if (region.replacement != nullptr && region.central_indices.size() != 1U) {
            return failure(
                NbzRetailRepackStatus::alias_local_region_unsupported,
                "Changed duplicate-offset local regions are not supported in this tier.");
        }
        if (region.replacement != nullptr &&
            region.serialization->uses_data_descriptor) {
            return failure(
                NbzRetailRepackStatus::data_descriptor_replacement_unsupported,
                "Changed bit-3/data-descriptor entries remain fail-closed; unchanged descriptor regions are copied opaque.");
        }
    }

    std::vector<LocalRegion*> regions;
    regions.reserve(region_map.size());
    for (auto& [offset, region] : region_map) {
        (void)offset;
        regions.push_back(&region);
    }

    if (!regions.empty() &&
        regions.front()->serialization->local_record_offset != serialization.prefix_size) {
        return failure(
            NbzRetailRepackStatus::invalid_bound_snapshot,
            "The first local region does not begin at the preserved prefix boundary.");
    }

    std::uint64_t source_cursor = serialization.prefix_size;
    std::uint64_t output_cursor = serialization.prefix_size;
    if (!fits_zip32(output_cursor)) {
        return failure(
            NbzRetailRepackStatus::zip32_overflow,
            "Preserved prefix exceeds the classic-ZIP offset domain.");
    }

    std::vector<std::uint32_t> output_local_offsets(source_entries.size(), 0U);
    for (auto* region : regions) {
        const auto& serialized = *region->serialization;
        if (serialized.local_record_offset != source_cursor) {
            return failure(
                NbzRetailRepackStatus::invalid_bound_snapshot,
                "Local-region spans do not cover the source archive contiguously.");
        }
        if (!fits_zip32(output_cursor)) {
            return failure(
                NbzRetailRepackStatus::zip32_overflow,
                "A rebuilt local-header offset exceeds the classic-ZIP domain.");
        }
        region->output_offset = output_cursor;
        for (const auto central_index : region->central_indices) {
            output_local_offsets[central_index] =
                static_cast<std::uint32_t>(output_cursor);
        }

        const auto prefix_bytes =
            static_cast<std::uint64_t>(serialized.local_prefix_bytes.size());
        if (!checked_add(prefix_bytes, serialized.stored_data_size, region->tail_start) ||
            region->tail_start > serialized.local_region_size) {
            return failure(
                NbzRetailRepackStatus::invalid_bound_snapshot,
                "Stored-data span escapes its preserved local region.");
        }
        const auto tail_size = serialized.local_region_size - region->tail_start;

        if (region->replacement == nullptr) {
            region->output_size = serialized.local_region_size;
        } else {
            std::uint64_t new_size{};
            if (!checked_add(
                    prefix_bytes,
                    static_cast<std::uint64_t>(region->replacement->bytes.size()),
                    new_size) ||
                !checked_add(new_size, tail_size, new_size)) {
                return failure(
                    NbzRetailRepackStatus::zip32_overflow,
                    "Changed local region size overflowed the product domain.");
            }
            region->output_size = new_size;
        }

        if (!checked_add(source_cursor, serialized.local_region_size, source_cursor) ||
            !checked_add(output_cursor, region->output_size, output_cursor)) {
            return failure(
                NbzRetailRepackStatus::zip32_overflow,
                "Archive geometry overflowed while rebuilding local regions.");
        }
    }

    if (source_cursor != serialization.computed_central_start ||
        !fits_zip32(output_cursor)) {
        return failure(
            NbzRetailRepackStatus::invalid_bound_snapshot,
            "Local regions do not terminate at the recovered central-directory boundary.");
    }
    const auto output_central_offset = output_cursor;

    std::uint64_t central_size = 0U;
    for (const auto& entry : serialization.entries) {
        if (entry.central_record_bytes.size() < central_fixed_size ||
            !checked_add(
                central_size,
                static_cast<std::uint64_t>(entry.central_record_bytes.size()),
                central_size)) {
            return failure(
                NbzRetailRepackStatus::zip32_overflow,
                "Central-directory serialization size overflowed.");
        }
    }
    if (central_size != serialization.eocd_offset - serialization.computed_central_start ||
        !fits_zip32(central_size)) {
        return failure(
            NbzRetailRepackStatus::invalid_bound_snapshot,
            "Preserved central-directory size is inconsistent.");
    }

    std::uint64_t output_size{};
    if (!checked_add(output_central_offset, central_size, output_size) ||
        !checked_add(
            output_size,
            static_cast<std::uint64_t>(serialization.eocd_bytes.size()),
            output_size) ||
        !fits_zip32(output_size)) {
        return failure(
            NbzRetailRepackStatus::zip32_overflow,
            "Rebuilt archive exceeds the bounded classic-ZIP archive domain.");
    }

    std::ifstream input(source.archive_path(), std::ios::binary);
    if (!input) {
        return failure(
            NbzRetailRepackStatus::source_read_failure,
            "Could not open the artifact-bound source archive for repack.");
    }

    const auto initial_source_size =
        std::filesystem::file_size(source.archive_path(), filesystem_error);
    if (filesystem_error || initial_source_size != bound.artifact().size) {
        return failure(
            NbzRetailRepackStatus::source_artifact_mismatch,
            "Source archive size changed before retail repack I/O began.");
    }

    std::ofstream output(output_path, std::ios::binary | std::ios::trunc);
    if (!output) {
        return failure(
            NbzRetailRepackStatus::output_write_failure,
            "Could not create the retail repack output archive.");
    }
    bool output_created = true;

    auto cleanup_failure = [&](NbzRetailRepackStatus status, std::string detail) {
        output.close();
        input.close();
        if (output_created) {
            std::error_code remove_error;
            std::filesystem::remove(output_path, remove_error);
        }
        return failure(status, std::move(detail));
    };

    core::Sha256Accumulator source_hash;
    core::Sha256Accumulator output_hash;
    std::vector<std::byte> buffer(
        static_cast<std::size_t>(limits.io_chunk_bytes), std::byte{0});
    std::uint64_t read_cursor = 0U;
    std::uint64_t written_bytes = 0U;

    auto write_bytes = [&](std::span<const std::byte> bytes) -> bool {
        std::size_t cursor = 0U;
        while (cursor < bytes.size()) {
            const auto chunk = std::min<std::size_t>(
                static_cast<std::size_t>(limits.io_chunk_bytes),
                bytes.size() - cursor);
            output.write(
                reinterpret_cast<const char*>(bytes.data() + cursor),
                static_cast<std::streamsize>(chunk));
            if (!output) {
                return false;
            }
            if (!output_hash.update(bytes.subspan(cursor, chunk))) {
                return false;
            }
            cursor += chunk;
            written_bytes += chunk;
        }
        return true;
    };

    auto consume_source = [&](std::uint64_t count, const auto& on_chunk) -> bool {
        std::uint64_t remaining = count;
        while (remaining != 0U) {
            const auto chunk = static_cast<std::size_t>(std::min<std::uint64_t>(
                remaining, limits.io_chunk_bytes));
            input.read(
                reinterpret_cast<char*>(buffer.data()),
                static_cast<std::streamsize>(chunk));
            if (input.gcount() != static_cast<std::streamsize>(chunk)) {
                return false;
            }
            const auto bytes = std::span<const std::byte>{buffer.data(), chunk};
            if (!source_hash.update(bytes) || !on_chunk(bytes)) {
                return false;
            }
            read_cursor += chunk;
            remaining -= chunk;
        }
        return true;
    };

    const auto copy_chunk = [&](std::span<const std::byte> bytes) {
        return write_bytes(bytes);
    };
    const auto discard_chunk = [](std::span<const std::byte>) { return true; };

    if (!consume_source(serialization.prefix_size, copy_chunk)) {
        return cleanup_failure(
            NbzRetailRepackStatus::source_read_failure,
            "Failed while copying/hash-verifying the preserved archive prefix.");
    }

    for (const auto* region : regions) {
        const auto& serialized = *region->serialization;
        if (read_cursor != serialized.local_record_offset ||
            written_bytes != region->output_offset) {
            return cleanup_failure(
                NbzRetailRepackStatus::invalid_bound_snapshot,
                "Runtime I/O cursor disagrees with prevalidated archive geometry.");
        }

        if (region->replacement == nullptr) {
            if (!consume_source(serialized.local_region_size, copy_chunk)) {
                return cleanup_failure(
                    NbzRetailRepackStatus::source_read_failure,
                    "Failed while copying an unchanged local region.");
            }
            continue;
        }

        const auto central_index = region->central_indices.front();
        const auto& indexed = source_entries[central_index];
        const auto& replacement = *region->replacement;
        auto local_prefix = serialized.local_prefix_bytes;
        if (local_prefix.size() < local_fixed_size) {
            return cleanup_failure(
                NbzRetailRepackStatus::invalid_bound_snapshot,
                "Changed local prefix is shorter than a classic ZIP local header.");
        }
        const auto replacement_size = static_cast<std::uint32_t>(replacement.bytes.size());
        const auto replacement_crc = crc32_of(replacement.bytes);
        put_u16(local_prefix, 8U, store_method);
        put_u32(local_prefix, 14U, replacement_crc);
        put_u32(local_prefix, 18U, replacement_size);
        put_u32(local_prefix, 22U, replacement_size);

        if (!write_bytes(local_prefix) || !write_bytes(replacement.bytes)) {
            return cleanup_failure(
                NbzRetailRepackStatus::output_write_failure,
                "Failed while writing a changed STORE local member.");
        }

        std::uint64_t region_relative = 0U;
        const auto write_tail = [&](std::span<const std::byte> bytes) {
            const auto chunk_start = region_relative;
            const auto chunk_end = region_relative + bytes.size();
            bool ok = true;
            if (chunk_end > region->tail_start) {
                const auto tail_begin = static_cast<std::size_t>(
                    region->tail_start > chunk_start
                        ? region->tail_start - chunk_start
                        : 0U);
                ok = write_bytes(bytes.subspan(tail_begin));
            }
            region_relative = chunk_end;
            return ok;
        };
        if (!consume_source(serialized.local_region_size, write_tail)) {
            return cleanup_failure(
                NbzRetailRepackStatus::source_read_failure,
                "Failed while verifying the original changed local region and preserving its opaque tail.");
        }

        if (written_bytes != region->output_offset + region->output_size ||
            indexed.central_index != central_index) {
            return cleanup_failure(
                NbzRetailRepackStatus::invalid_receipt,
                "Changed local-region output size disagrees with prevalidated geometry.");
        }
    }

    const auto source_tail_size = bound.artifact().size - read_cursor;
    if (!consume_source(source_tail_size, discard_chunk) ||
        read_cursor != bound.artifact().size) {
        return cleanup_failure(
            NbzRetailRepackStatus::source_read_failure,
            "Failed while hash-verifying the source central directory and EOCD tail.");
    }

    const auto source_digest = source_hash.finalize();
    const auto final_source_size =
        std::filesystem::file_size(source.archive_path(), filesystem_error);
    if (!source_digest.has_value() || filesystem_error ||
        final_source_size != bound.artifact().size ||
        source_hash.byte_count() != bound.artifact().size ||
        !hex_equal(source_digest->hex(), bound.artifact().sha256)) {
        return cleanup_failure(
            NbzRetailRepackStatus::source_artifact_mismatch,
            "Source artifact changed or failed SHA-256 verification during retail repack.");
    }

    if (written_bytes != output_central_offset) {
        return cleanup_failure(
            NbzRetailRepackStatus::invalid_receipt,
            "Local-region output did not end at the rebuilt central offset.");
    }

    std::vector<NbzRetailRepackEntryReceipt> entry_receipts;
    entry_receipts.reserve(source_entries.size());
    for (std::size_t index = 0U; index < serialization.entries.size(); ++index) {
        const auto& serialized = serialization.entries[index];
        const auto& indexed = source_entries[index];
        auto central = serialized.central_record_bytes;
        if (central.size() < central_fixed_size) {
            return cleanup_failure(
                NbzRetailRepackStatus::invalid_bound_snapshot,
                "Preserved central record is shorter than the classic ZIP header.");
        }

        const auto* replacement = replacement_by_index[index];
        const auto changed = replacement != nullptr;
        const auto output_method = changed ? store_method : indexed.compression_method;
        const auto output_crc = changed ? crc32_of(replacement->bytes) : indexed.crc32;
        const auto output_compressed_size = changed
            ? static_cast<std::uint32_t>(replacement->bytes.size())
            : indexed.compressed_size;
        const auto output_uncompressed_size = changed
            ? static_cast<std::uint32_t>(replacement->bytes.size())
            : indexed.uncompressed_size;

        put_u32(central, 42U, output_local_offsets[index]);
        if (changed) {
            put_u16(central, 10U, output_method);
            put_u32(central, 16U, output_crc);
            put_u32(central, 20U, output_compressed_size);
            put_u32(central, 24U, output_uncompressed_size);
        }
        if (!write_bytes(central)) {
            return cleanup_failure(
                NbzRetailRepackStatus::output_write_failure,
                "Failed while writing the rebuilt central directory.");
        }

        entry_receipts.push_back(NbzRetailRepackEntryReceipt{
            .central_index = static_cast<std::uint32_t>(index),
            .logical_path = indexed.logical_path,
            .changed = changed,
            .original_method = indexed.compression_method,
            .output_method = output_method,
            .original_local_header_offset = indexed.local_header_offset,
            .output_local_header_offset = output_local_offsets[index],
            .output_crc32 = output_crc,
            .output_compressed_size = output_compressed_size,
            .output_uncompressed_size = output_uncompressed_size,
        });
    }

    auto eocd = serialization.eocd_bytes;
    if (eocd.size() < eocd_fixed_size) {
        return cleanup_failure(
            NbzRetailRepackStatus::invalid_bound_snapshot,
            "Preserved EOCD bytes are shorter than the classic ZIP EOCD.");
    }
    put_u32(eocd, 12U, static_cast<std::uint32_t>(central_size));
    put_u32(eocd, 16U, static_cast<std::uint32_t>(output_central_offset));
    if (!write_bytes(eocd) || written_bytes != output_size) {
        return cleanup_failure(
            NbzRetailRepackStatus::output_write_failure,
            "Failed while writing the rebuilt EOCD/archive comment.");
    }

    output.flush();
    if (!output) {
        return cleanup_failure(
            NbzRetailRepackStatus::output_write_failure,
            "Retail repack output could not be flushed completely.");
    }
    output.close();
    input.close();

    const auto output_digest = output_hash.finalize();
    const auto persisted_output_size =
        std::filesystem::file_size(output_path, filesystem_error);
    if (!output_digest.has_value() || filesystem_error ||
        output_hash.byte_count() != output_size ||
        persisted_output_size != output_size) {
        std::error_code remove_error;
        std::filesystem::remove(output_path, remove_error);
        return failure(
            NbzRetailRepackStatus::output_validation_failure,
            "Persisted output size/hash receipt could not be established.");
    }

    evidence::ArtifactIdentity output_artifact{
        .id = output_path.filename().string(),
        .role = "gdspaces-nbz-retail-repack",
        .sha256 = output_digest->hex(),
        .size = output_size,
    };
    if (!output_artifact.valid()) {
        std::error_code remove_error;
        std::filesystem::remove(output_path, remove_error);
        return failure(
            NbzRetailRepackStatus::invalid_receipt,
            "Generated output artifact identity is invalid.");
    }

    gdspaces::NbzZipSource reopened("gdspaces-retail-repack-validation", output_path);
    if (!reopened.valid() || reopened.entries().size() != source_entries.size()) {
        std::error_code remove_error;
        std::filesystem::remove(output_path, remove_error);
        return failure(
            NbzRetailRepackStatus::output_validation_failure,
            "Rebuilt retail NBZ did not reopen through canonical NbzZipSource.");
    }

    for (std::size_t index = 0U; index < reopened.entries().size(); ++index) {
        const auto& actual = reopened.entries()[index];
        const auto& expected = entry_receipts[index];
        if (actual.central_index != expected.central_index ||
            actual.logical_path != expected.logical_path ||
            actual.compression_method != expected.output_method ||
            actual.crc32 != expected.output_crc32 ||
            actual.compressed_size != expected.output_compressed_size ||
            actual.uncompressed_size != expected.output_uncompressed_size ||
            actual.local_header_offset != expected.output_local_header_offset) {
            std::error_code remove_error;
            std::filesystem::remove(output_path, remove_error);
            return failure(
                NbzRetailRepackStatus::output_validation_failure,
                "Canonical reopen disagrees with the retail repack entry receipt.");
        }
    }

    const auto rebound = gdspaces::NbzZipArtifactSerializationBinder::bind(
        reopened,
        output_artifact,
        gdspaces::NbzZipSerializationLimits{
            .max_metadata_bytes = limits.max_metadata_bytes,
        },
        gdspaces::NbzZipArtifactBindingLimits{
            .hash_chunk_bytes = limits.io_chunk_bytes,
        });
    if (!rebound.ok() ||
        rebound.snapshot->serialization().computed_central_start !=
            output_central_offset) {
        std::error_code remove_error;
        std::filesystem::remove(output_path, remove_error);
        return failure(
            NbzRetailRepackStatus::output_validation_failure,
            "Rebuilt retail NBZ failed artifact-bound serialization reopen validation.");
    }

    const auto byte_identical =
        bound.artifact().size == output_artifact.size &&
        hex_equal(bound.artifact().sha256, output_artifact.sha256);
    if (replacements.empty() && !byte_identical) {
        std::error_code remove_error;
        std::filesystem::remove(output_path, remove_error);
        return failure(
            NbzRetailRepackStatus::output_validation_failure,
            "No-edit retail repack was not byte-identical to the artifact-bound source.");
    }

    auto receipt = NbzRetailRepackReceipt{
        .source_artifact = bound.artifact(),
        .output_artifact = std::move(output_artifact),
        .output_central_offset = static_cast<std::uint32_t>(output_central_offset),
        .output_central_size = static_cast<std::uint32_t>(central_size),
        .changed_entry_count = static_cast<std::uint32_t>(replacements.size()),
        .byte_identical = byte_identical,
        .entries = std::move(entry_receipts),
    };
    if (!receipt.valid()) {
        std::error_code remove_error;
        std::filesystem::remove(output_path, remove_error);
        return failure(
            NbzRetailRepackStatus::invalid_receipt,
            "Retail repack receipt failed internal validation.");
    }

    output_created = false;
    return NbzRetailRepackResult{
        .status = NbzRetailRepackStatus::ok,
        .receipt = std::move(receipt),
        .detail = {},
    };
}

} // namespace dmc::rengine::profiles::dmc3
