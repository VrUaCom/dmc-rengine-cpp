#include "dmc_rengine/gdspaces/nbz_zip_serialization.hpp"

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

constexpr std::uint32_t central_signature = 0x02014B50U;
constexpr std::uint32_t local_signature = 0x04034B50U;
constexpr std::uint32_t eocd_signature = 0x06054B50U;
constexpr std::size_t central_fixed_size = 46U;
constexpr std::size_t local_fixed_size = 30U;
constexpr std::size_t eocd_fixed_size = 22U;
constexpr std::uint16_t data_descriptor_flag = 0x0008U;

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

void add_error(
    NbzZipSerializationScanResult& result,
    std::string code,
    std::string message) {
    result.diagnostics.push_back(Diagnostic{
        .severity = DiagnosticSeverity::error,
        .code = std::move(code),
        .message = std::move(message),
        .resource = std::nullopt,
    });
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

[[nodiscard]] bool same_local_alias(
    const NbzZipSerializationEntry& left,
    const NbzZipSerializationEntry& right) noexcept {
    return left.local_record_offset == right.local_record_offset &&
        left.local_prefix_bytes == right.local_prefix_bytes &&
        left.data_offset == right.data_offset &&
        left.stored_data_size == right.stored_data_size &&
        left.local_region_size == right.local_region_size &&
        left.uses_data_descriptor == right.uses_data_descriptor;
}

} // namespace

bool NbzZipSerializationEntry::valid(
    std::uint64_t archive_size,
    std::uint64_t central_start) const noexcept {
    if (central_record_bytes.size() < central_fixed_size ||
        local_prefix_bytes.size() < local_fixed_size ||
        central_record_offset >= central_start ||
        local_record_offset >= central_start ||
        local_region_size == 0U ||
        local_region_size > central_start - local_record_offset) {
        return false;
    }

    const auto central_span = std::span<const std::byte>{central_record_bytes};
    const auto local_span = std::span<const std::byte>{local_prefix_bytes};
    if (u32_le(central_span, 0U) != central_signature ||
        u32_le(local_span, 0U) != local_signature) {
        return false;
    }

    std::uint64_t expected_data_offset{};
    if (!checked_add(
            local_record_offset,
            static_cast<std::uint64_t>(local_prefix_bytes.size()),
            expected_data_offset) ||
        expected_data_offset != data_offset ||
        data_offset > archive_size ||
        stored_data_size > archive_size - data_offset) {
        return false;
    }

    const auto consumed_before_tail =
        static_cast<std::uint64_t>(local_prefix_bytes.size()) + stored_data_size;
    return consumed_before_tail <= local_region_size;
}

bool NbzZipSerializationSnapshot::valid() const noexcept {
    if (source_id.empty() || archive_size < eocd_fixed_size ||
        prefix_size > computed_central_start ||
        computed_central_start > eocd_offset ||
        eocd_offset > archive_size ||
        eocd_bytes.size() != archive_size - eocd_offset ||
        eocd_bytes.size() < eocd_fixed_size ||
        u32_le(std::span<const std::byte>{eocd_bytes}, 0U) != eocd_signature) {
        return false;
    }

    std::uint64_t central_cursor = computed_central_start;
    for (std::size_t index = 0U; index < entries.size(); ++index) {
        const auto& entry = entries[index];
        if (entry.central_index != index ||
            entry.central_record_offset != central_cursor ||
            !entry.valid(archive_size, computed_central_start)) {
            return false;
        }
        if (!checked_add(
                central_cursor,
                static_cast<std::uint64_t>(entry.central_record_bytes.size()),
                central_cursor) ||
            central_cursor > eocd_offset) {
            return false;
        }
    }
    return central_cursor == eocd_offset;
}

bool NbzZipSerializationScanResult::ok() const noexcept {
    if (!snapshot.has_value() || !snapshot->valid()) {
        return false;
    }
    return std::none_of(
        diagnostics.begin(), diagnostics.end(),
        [](const Diagnostic& diagnostic) {
            return diagnostic.severity == DiagnosticSeverity::error;
        });
}

NbzZipSerializationScanResult NbzZipSerializationScanner::scan(
    const NbzZipSource& source,
    NbzZipSerializationLimits limits) {
    NbzZipSerializationScanResult result;
    if (!source.valid() || !source.index_receipt().has_value() ||
        !source.index_receipt()->valid()) {
        add_error(
            result,
            "gdspaces.nbz.serialization.source-invalid",
            "Retail serialization scanning requires a valid indexed NBZ source.");
        return result;
    }
    if (limits.max_metadata_bytes < eocd_fixed_size) {
        add_error(
            result,
            "gdspaces.nbz.serialization.metadata-budget",
            "The serialization metadata budget is smaller than a ZIP EOCD record.");
        return result;
    }

    const auto& receipt = *source.index_receipt();
    const auto& indexed_entries = source.entries();
    if (indexed_entries.size() != receipt.walked_entry_count) {
        add_error(
            result,
            "gdspaces.nbz.serialization.index-count-mismatch",
            "Indexed entry count does not match the recovered central-directory walk receipt.");
        return result;
    }

    std::ifstream stream(source.archive_path(), std::ios::binary);
    if (!stream) {
        add_error(
            result,
            "gdspaces.nbz.serialization.open-failed",
            "Unable to reopen the NBZ source for serialization scanning.");
        return result;
    }

    NbzZipSerializationSnapshot snapshot{
        .source_id = std::string{source.id()},
        .archive_size = receipt.archive_size,
        .prefix_size = receipt.computed_central_start,
        .computed_central_start = receipt.computed_central_start,
        .eocd_offset = receipt.eocd_offset,
        .eocd_bytes = {},
        .entries = {},
    };
    snapshot.entries.reserve(indexed_entries.size());

    std::uint64_t metadata_used = 0U;
    const auto eocd_size = snapshot.archive_size - snapshot.eocd_offset;
    if (!consume_metadata_budget(
            eocd_size, limits.max_metadata_bytes, metadata_used) ||
        eocd_size > static_cast<std::uint64_t>(
            std::numeric_limits<std::size_t>::max())) {
        add_error(
            result,
            "gdspaces.nbz.serialization.metadata-budget",
            "EOCD/comment bytes exceed the configured serialization metadata budget.");
        return result;
    }
    snapshot.eocd_bytes.resize(static_cast<std::size_t>(eocd_size));
    if (!read_exact(stream, snapshot.eocd_offset, snapshot.eocd_bytes)) {
        add_error(
            result,
            "gdspaces.nbz.serialization.eocd-read",
            "Unable to preserve the complete EOCD/comment byte region.");
        return result;
    }

    std::uint64_t central_cursor = snapshot.computed_central_start;
    for (std::size_t index = 0U; index < indexed_entries.size(); ++index) {
        const auto& indexed = indexed_entries[index];
        if (indexed.central_index != index ||
            snapshot.eocd_offset - central_cursor < central_fixed_size) {
            add_error(
                result,
                "gdspaces.nbz.serialization.central-order",
                "The indexed central-directory identity is not contiguous and ordered.");
            return result;
        }

        std::array<std::byte, central_fixed_size> central{};
        if (!read_exact(stream, central_cursor, central) ||
            u32_le(central, 0U) != central_signature) {
            add_error(
                result,
                "gdspaces.nbz.serialization.central-read",
                "Unable to preserve an expected central-directory record.");
            return result;
        }

        const auto name_length = u16_le(central, 28U);
        const auto extra_length = u16_le(central, 30U);
        const auto comment_length = u16_le(central, 32U);
        const auto central_local_offset = u32_le(central, 42U);
        std::uint64_t record_size = central_fixed_size;
        if (!checked_add(record_size, name_length, record_size) ||
            !checked_add(record_size, extra_length, record_size) ||
            !checked_add(record_size, comment_length, record_size) ||
            record_size > snapshot.eocd_offset - central_cursor ||
            record_size > static_cast<std::uint64_t>(
                std::numeric_limits<std::size_t>::max()) ||
            central_local_offset != indexed.local_header_offset) {
            add_error(
                result,
                "gdspaces.nbz.serialization.central-range",
                "A central-directory record cannot be preserved within the recovered boundaries.");
            return result;
        }
        if (!consume_metadata_budget(
                record_size, limits.max_metadata_bytes, metadata_used)) {
            add_error(
                result,
                "gdspaces.nbz.serialization.metadata-budget",
                "Central-directory metadata exceeds the configured serialization budget.");
            return result;
        }

        NbzZipSerializationEntry preserved{
            .central_index = static_cast<std::uint32_t>(index),
            .central_record_offset = central_cursor,
            .central_record_bytes = std::vector<std::byte>(
                static_cast<std::size_t>(record_size)),
            .local_record_offset = indexed.local_header_offset,
            .local_prefix_bytes = {},
            .data_offset = indexed.data_offset,
            .stored_data_size = indexed.compressed_size,
            .local_region_size = 0U,
            .uses_data_descriptor = (indexed.flags & data_descriptor_flag) != 0U,
        };
        if (!read_exact(
                stream, preserved.central_record_offset,
                preserved.central_record_bytes)) {
            add_error(
                result,
                "gdspaces.nbz.serialization.central-read",
                "Unable to read the complete raw central-directory record.");
            return result;
        }

        if (preserved.data_offset < preserved.local_record_offset) {
            add_error(
                result,
                "gdspaces.nbz.serialization.local-prefix-range",
                "Indexed data offset precedes its local-header offset.");
            return result;
        }
        const auto local_prefix_size =
            preserved.data_offset - preserved.local_record_offset;
        if (local_prefix_size < local_fixed_size ||
            local_prefix_size > static_cast<std::uint64_t>(
                std::numeric_limits<std::size_t>::max()) ||
            !consume_metadata_budget(
                local_prefix_size, limits.max_metadata_bytes, metadata_used)) {
            add_error(
                result,
                "gdspaces.nbz.serialization.metadata-budget",
                "Local-header/name/extra metadata exceeds the configured serialization budget.");
            return result;
        }
        preserved.local_prefix_bytes.resize(
            static_cast<std::size_t>(local_prefix_size));
        if (!read_exact(
                stream, preserved.local_record_offset,
                preserved.local_prefix_bytes) ||
            u32_le(
                std::span<const std::byte>{preserved.local_prefix_bytes}, 0U) !=
                local_signature) {
            add_error(
                result,
                "gdspaces.nbz.serialization.local-read",
                "Unable to preserve the complete local-header/name/extra byte prefix.");
            return result;
        }

        snapshot.prefix_size = std::min(
            snapshot.prefix_size, preserved.local_record_offset);
        snapshot.entries.push_back(std::move(preserved));
        central_cursor += record_size;
    }

    if (central_cursor != snapshot.eocd_offset) {
        add_error(
            result,
            "gdspaces.nbz.serialization.central-end",
            "Preserved central-directory records do not end exactly at EOCD.");
        return result;
    }

    std::vector<std::uint64_t> local_offsets;
    local_offsets.reserve(snapshot.entries.size());
    for (const auto& entry : snapshot.entries) {
        local_offsets.push_back(entry.local_record_offset);
    }
    std::sort(local_offsets.begin(), local_offsets.end());
    local_offsets.erase(
        std::unique(local_offsets.begin(), local_offsets.end()),
        local_offsets.end());

    for (auto& entry : snapshot.entries) {
        const auto next = std::upper_bound(
            local_offsets.begin(), local_offsets.end(), entry.local_record_offset);
        const auto region_end = next == local_offsets.end()
            ? snapshot.computed_central_start
            : *next;
        if (region_end <= entry.local_record_offset) {
            add_error(
                result,
                "gdspaces.nbz.serialization.local-region-order",
                "Local record offsets do not form bounded increasing source regions.");
            return result;
        }
        entry.local_region_size = region_end - entry.local_record_offset;
        if (!entry.valid(snapshot.archive_size, snapshot.computed_central_start)) {
            add_error(
                result,
                "gdspaces.nbz.serialization.local-region-range",
                "A preserved local region does not contain its complete prefix and stored data span.");
            return result;
        }
    }

    for (std::size_t left = 0U; left < snapshot.entries.size(); ++left) {
        for (std::size_t right = left + 1U; right < snapshot.entries.size(); ++right) {
            if (snapshot.entries[left].local_record_offset ==
                    snapshot.entries[right].local_record_offset &&
                !same_local_alias(snapshot.entries[left], snapshot.entries[right])) {
                add_error(
                    result,
                    "gdspaces.nbz.serialization.local-alias-conflict",
                    "Two central identities alias one local-header offset but disagree on preserved local serialization framing.");
                return result;
            }
        }
    }

    if (!snapshot.valid()) {
        add_error(
            result,
            "gdspaces.nbz.serialization.snapshot-invalid",
            "The completed NBZ serialization snapshot violates its internal preservation invariants.");
        return result;
    }

    result.snapshot = std::move(snapshot);
    return result;
}

} // namespace dmc::rengine::gdspaces
