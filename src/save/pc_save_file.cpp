#include "dmc_rengine/save/pc_save_file.hpp"

#include <algorithm>
#include <string>

namespace dmc::rengine::save::pc {
namespace {

[[nodiscard]] std::uint8_t read_u8(
    std::span<const std::byte> bytes,
    std::size_t offset) noexcept {
    return std::to_integer<std::uint8_t>(bytes[offset]);
}

[[nodiscard]] std::uint16_t read_u16_le(
    std::span<const std::byte> bytes,
    std::size_t offset) noexcept {
    return static_cast<std::uint16_t>(read_u8(bytes, offset)) |
           static_cast<std::uint16_t>(
               static_cast<std::uint16_t>(read_u8(bytes, offset + 1U)) << 8U);
}

[[nodiscard]] std::uint32_t read_u32_le(
    std::span<const std::byte> bytes,
    std::size_t offset) noexcept {
    return static_cast<std::uint32_t>(read_u8(bytes, offset)) |
           (static_cast<std::uint32_t>(read_u8(bytes, offset + 1U)) << 8U) |
           (static_cast<std::uint32_t>(read_u8(bytes, offset + 2U)) << 16U) |
           (static_cast<std::uint32_t>(read_u8(bytes, offset + 3U)) << 24U);
}

[[nodiscard]] std::uint16_t add_ones_complement_word(
    std::uint16_t current,
    std::uint16_t word) noexcept {
    const auto sum = static_cast<std::uint32_t>(current) +
                     static_cast<std::uint32_t>(word);
    auto folded = static_cast<std::uint32_t>(sum & 0xFFFFU) + (sum >> 16U);
    folded = (folded & 0xFFFFU) + (folded >> 16U);
    return static_cast<std::uint16_t>(folded);
}

[[nodiscard]] BlockDescriptor header_descriptor() noexcept {
    return BlockDescriptor{
        .kind = BlockKind::header,
        .index = 0U,
        .data_offset = header_data_offset,
        .data_size = header_data_size,
        .trailer_offset = header_trailer_offset,
        .block_size = header_block_size,
    };
}

[[nodiscard]] BlockDescriptor summary_descriptor(
    std::size_t index) noexcept {
    const auto block_offset = summary_blocks_offset + index * summary_block_size;
    return BlockDescriptor{
        .kind = BlockKind::slot_summary,
        .index = index,
        .data_offset = block_offset,
        .data_size = summary_data_size,
        .trailer_offset = block_offset + summary_data_size,
        .block_size = summary_block_size,
    };
}

[[nodiscard]] BlockDescriptor payload_descriptor(
    std::size_t index) noexcept {
    const auto block_offset = payload_blocks_offset + index * payload_block_size;
    return BlockDescriptor{
        .kind = BlockKind::slot_payload,
        .index = index,
        .data_offset = block_offset,
        .data_size = payload_data_size,
        .trailer_offset = block_offset + payload_data_size,
        .block_size = payload_block_size,
    };
}

[[nodiscard]] BlockIntegrity analyze_block(
    std::span<const std::byte> bytes,
    const BlockDescriptor& descriptor) noexcept {
    const auto trailer = IntegrityTrailer{
        .status_or_presence = read_u16_le(bytes, descriptor.trailer_offset),
        .integrity_word = read_u16_le(bytes, descriptor.trailer_offset + 2U),
    };
    const auto block_bytes = bytes.subspan(
        descriptor.data_offset,
        descriptor.block_size);
    const auto folded_sum = ones_complement_folded_sum(block_bytes);
    return BlockIntegrity{
        .descriptor = descriptor,
        .trailer = trailer,
        .folded_sum = folded_sum,
        .valid = folded_sum == valid_integrity_fold,
    };
}

[[nodiscard]] SummaryRecord parse_summary(
    std::span<const std::byte> bytes,
    std::size_t index) noexcept {
    const auto base = summary_blocks_offset + index * summary_block_size;
    return SummaryRecord{
        .valid = read_u8(bytes, base + 0x00U),
        .year_bcd = read_u8(bytes, base + 0x01U),
        .month_bcd = read_u8(bytes, base + 0x02U),
        .day_bcd = read_u8(bytes, base + 0x03U),
        .play_hours = read_u8(bytes, base + 0x04U),
        .play_minutes = read_u8(bytes, base + 0x05U),
        .play_seconds = read_u8(bytes, base + 0x06U),
        .availability_or_progress_flag = read_u8(bytes, base + 0x07U),
        .mission_or_event_index = read_u32_le(bytes, base + 0x08U),
        .field_0c = read_u32_le(bytes, base + 0x0CU),
        .field_10 = read_u32_le(bytes, base + 0x10U),
        .field_14 = read_u32_le(bytes, base + 0x14U),
        .state_byte = read_u8(bytes, base + 0x18U),
        .set_flag_count = read_u8(bytes, base + 0x19U),
        .progress_category = static_cast<std::int8_t>(
            read_u8(bytes, base + 0x1AU)),
        .progress_index = read_u32_le(bytes, base + 0x1CU),
        .progress_lookup_value = read_u32_le(bytes, base + 0x20U),
        .save_hour_bcd = read_u8(bytes, base + 0x24U),
        .save_minute_bcd = read_u8(bytes, base + 0x25U),
        .save_second_bcd = read_u8(bytes, base + 0x26U),
        .option_byte_0 = read_u8(bytes, base + 0x27U),
        .option_byte_1 = read_u8(bytes, base + 0x28U),
        .option_word = read_u16_le(bytes, base + 0x2AU),
    };
}

void add_diagnostic(
    AnalysisResult& result,
    formats::ParseSeverity severity,
    std::string code,
    std::string message,
    std::uint64_t offset) {
    result.diagnostics.push_back(formats::ParseDiagnostic{
        .severity = severity,
        .code = std::move(code),
        .message = std::move(message),
        .offset = offset,
    });
}

} // namespace

std::optional<std::uint8_t> decode_packed_bcd(
    std::uint8_t value) noexcept {
    const auto high = static_cast<std::uint8_t>((value >> 4U) & 0x0FU);
    const auto low = static_cast<std::uint8_t>(value & 0x0FU);
    if (high > 9U || low > 9U) {
        return std::nullopt;
    }
    return static_cast<std::uint8_t>(high * 10U + low);
}

std::uint16_t ones_complement_folded_sum(
    std::span<const std::byte> bytes) noexcept {
    std::uint16_t sum = 0U;
    std::size_t offset = 0U;
    while (offset + 1U < bytes.size()) {
        sum = add_ones_complement_word(
            sum,
            read_u16_le(bytes, offset));
        offset += 2U;
    }
    if (offset < bytes.size()) {
        sum = add_ones_complement_word(
            sum,
            static_cast<std::uint16_t>(read_u8(bytes, offset)));
    }
    return sum;
}

std::uint16_t generate_integrity_word(
    std::span<const std::byte> data,
    std::uint16_t status_or_presence) noexcept {
    const auto with_status = add_ones_complement_word(
        ones_complement_folded_sum(data),
        status_or_presence);
    return static_cast<std::uint16_t>(~with_status);
}

bool BlockDescriptor::valid() const noexcept {
    if (data_size == 0U || block_size != data_size + integrity_trailer_size ||
        trailer_offset != data_offset + data_size ||
        data_offset > file_size || block_size > file_size - data_offset) {
        return false;
    }
    switch (kind) {
    case BlockKind::header:
        return index == 0U && data_offset == header_data_offset &&
               data_size == header_data_size;
    case BlockKind::slot_summary:
        return index < summary_count && data_size == summary_data_size;
    case BlockKind::slot_payload:
        return index < payload_count && data_size == payload_data_size;
    }
    return false;
}

bool SummaryRecord::packed_bcd_fields_valid() const noexcept {
    const auto year = decode_packed_bcd(year_bcd);
    const auto month = decode_packed_bcd(month_bcd);
    const auto day = decode_packed_bcd(day_bcd);
    const auto hour = decode_packed_bcd(save_hour_bcd);
    const auto minute = decode_packed_bcd(save_minute_bcd);
    const auto second = decode_packed_bcd(save_second_bcd);
    return year.has_value() && month.has_value() && day.has_value() &&
           hour.has_value() && minute.has_value() && second.has_value() &&
           *month >= 1U && *month <= 12U && *day >= 1U && *day <= 31U &&
           *hour <= 23U && *minute <= 59U && *second <= 59U;
}

bool AnalysisResult::ok() const noexcept {
    return recognized && blocks.size() == 21U &&
           valid_block_count() == blocks.size() &&
           std::none_of(
               diagnostics.begin(), diagnostics.end(),
               [](const formats::ParseDiagnostic& diagnostic) {
                   return diagnostic.severity == formats::ParseSeverity::error;
               });
}

std::size_t AnalysisResult::valid_block_count() const noexcept {
    return static_cast<std::size_t>(std::count_if(
        blocks.begin(), blocks.end(),
        [](const BlockIntegrity& block_integrity) {
            return block_integrity.valid;
        }));
}

const BlockIntegrity* AnalysisResult::block(
    BlockKind kind,
    std::size_t index) const noexcept {
    const auto iterator = std::find_if(
        blocks.begin(), blocks.end(),
        [kind, index](const BlockIntegrity& candidate) {
            return candidate.descriptor.kind == kind &&
                   candidate.descriptor.index == index;
        });
    return iterator == blocks.end() ? nullptr : &*iterator;
}

AnalysisResult Analyzer::analyze(std::span<const std::byte> bytes) {
    AnalysisResult result;
    if (bytes.size() != file_size) {
        add_diagnostic(
            result,
            formats::ParseSeverity::error,
            "pc-save.invalid-size",
            "DMC3 PC save must be exactly 0x4A30 bytes.",
            bytes.size());
        return result;
    }

    result.observed_header_marker = read_u16_le(bytes, header_marker_offset);
    if (result.observed_header_marker != header_marker) {
        add_diagnostic(
            result,
            formats::ParseSeverity::error,
            "pc-save.header-marker-mismatch",
            "Expected the confirmed 0xDEC0 header marker at offset 0x108.",
            header_marker_offset);
        return result;
    }

    result.recognized = true;
    result.blocks.reserve(1U + summary_count + payload_count);
    result.blocks.push_back(analyze_block(bytes, header_descriptor()));
    for (std::size_t index = 0U; index < summary_count; ++index) {
        const auto descriptor = summary_descriptor(index);
        result.blocks.push_back(analyze_block(bytes, descriptor));
        result.summaries[index] = parse_summary(bytes, index);
    }
    for (std::size_t index = 0U; index < payload_count; ++index) {
        result.blocks.push_back(analyze_block(bytes, payload_descriptor(index)));
    }

    for (const auto& block_integrity : result.blocks) {
        if (!block_integrity.descriptor.valid()) {
            add_diagnostic(
                result,
                formats::ParseSeverity::error,
                "pc-save.internal-layout-invalid",
                "An internal block descriptor violates the confirmed Pass 31 layout.",
                block_integrity.descriptor.data_offset);
        } else if (!block_integrity.valid) {
            add_diagnostic(
                result,
                formats::ParseSeverity::error,
                "pc-save.integrity-invalid",
                "The block does not fold to 0xFFFF under the confirmed one's-complement validator.",
                block_integrity.descriptor.data_offset);
        }
    }

    for (std::size_t index = 0U; index < summary_count; ++index) {
        const auto& summary = result.summaries[index];
        if (summary.valid != 0U && !summary.packed_bcd_fields_valid()) {
            add_diagnostic(
                result,
                formats::ParseSeverity::warning,
                "pc-save.summary-bcd-invalid",
                "A populated summary contains an invalid packed-BCD calendar or save-clock field.",
                summary_blocks_offset + index * summary_block_size);
        }
    }
    return result;
}

} // namespace dmc::rengine::save::pc
