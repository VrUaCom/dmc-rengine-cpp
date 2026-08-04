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

[[nodiscard]] bool all_zero(std::span<const std::byte> bytes) noexcept {
    return std::all_of(bytes.begin(), bytes.end(), [](std::byte value) {
        return value == std::byte{0};
    });
}

[[nodiscard]] BlockDescriptor global_descriptor() noexcept {
    return BlockDescriptor{
        .kind = BlockKind::global_record,
        .index = 0U,
        .data_offset = global_body_offset,
        .data_size = global_body_size,
        .trailer_offset = global_trailer_offset,
        .block_size = global_record_size,
    };
}

[[nodiscard]] BlockDescriptor summary_descriptor(
    std::size_t index) noexcept {
    const auto record_offset =
        summary_records_offset + index * summary_record_size;
    return BlockDescriptor{
        .kind = BlockKind::slot_summary,
        .index = index,
        .data_offset = record_offset,
        .data_size = summary_body_size,
        .trailer_offset = record_offset + summary_body_size,
        .block_size = summary_record_size,
    };
}

[[nodiscard]] BlockDescriptor payload_descriptor(
    std::size_t index) noexcept {
    const auto record_offset =
        payload_records_offset + index * payload_record_size;
    return BlockDescriptor{
        .kind = BlockKind::slot_payload,
        .index = index,
        .data_offset = record_offset,
        .data_size = payload_body_size,
        .trailer_offset = record_offset + payload_body_size,
        .block_size = payload_record_size,
    };
}

[[nodiscard]] BlockIntegrity analyze_block(
    std::span<const std::byte> bytes,
    const BlockDescriptor& descriptor) noexcept {
    const auto trailer = IntegrityTrailer{
        .record_state = read_u16_le(bytes, descriptor.trailer_offset),
        .checksum = read_u16_le(bytes, descriptor.checksum_offset()),
    };
    const auto record = bytes.subspan(
        descriptor.data_offset,
        descriptor.block_size);
    const auto calculated = calculate_record_checksum(record).value_or(0U);
    const auto folded_sum = ones_complement_folded_sum(record);
    const auto checksum_matches = calculated == trailer.checksum;
    const auto valid_fold = folded_sum == valid_integrity_fold;
    return BlockIntegrity{
        .descriptor = descriptor,
        .trailer = trailer,
        .calculated_checksum = calculated,
        .folded_sum = folded_sum,
        .checksum_matches = checksum_matches,
        .valid_fold = valid_fold,
        .valid = checksum_matches && valid_fold,
    };
}

[[nodiscard]] SummaryRecord parse_summary(
    std::span<const std::byte> bytes,
    std::size_t index) noexcept {
    const auto base = summary_records_offset + index * summary_record_size;
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

[[nodiscard]] GlobalRecordObservation parse_global_record(
    std::span<const std::byte> bytes) noexcept {
    GlobalRecordObservation result{
        .reserved_000 = read_u32_le(bytes, 0x000U),
        .runtime_value_004 = read_u32_le(bytes, 0x004U),
        .field_018 = read_u16_le(bytes, 0x018U),
        .field_01a = read_u8(bytes, 0x01AU),
        .runtime_flag_107 = read_u8(bytes, 0x107U),
        .compatibility_tag_108 = read_u16_le(bytes, 0x108U),
        .legacy_fallback_10a = read_u8(bytes, 0x10AU),
        .legacy_fallback_10b = read_u8(bytes, 0x10BU),
    };
    std::copy_n(
        bytes.begin() + 0x008U,
        result.state_008.size(),
        result.state_008.begin());
    std::copy_n(
        bytes.begin() + global_runtime_state_offset,
        result.runtime_state_01c.size(),
        result.runtime_state_01c.begin());
    result.cleared_region_is_zero = all_zero(bytes.subspan(
        global_cleared_region_offset,
        global_cleared_region_size));
    result.reserved_tail_is_zero = all_zero(bytes.subspan(
        global_tail_reserved_offset,
        global_tail_reserved_size));
    return result;
}

[[nodiscard]] PayloadBoundaryObservation observe_payload_boundary(
    std::span<const std::byte> bytes,
    std::size_t index) noexcept {
    const auto base = payload_records_offset + index * payload_record_size;
    return PayloadBoundaryObservation{
        .index = index,
        .cleared_reserved_is_zero = all_zero(bytes.subspan(
            base + payload_cleared_region_offset,
            payload_cleared_region_size)),
        .observed_tail_is_zero = all_zero(bytes.subspan(
            base + payload_observed_zero_tail_offset,
            payload_observed_zero_tail_size)),
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
        sum = add_ones_complement_word(sum, read_u16_le(bytes, offset));
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
    std::span<const std::byte> body,
    std::uint16_t record_state) noexcept {
    const auto with_state = add_ones_complement_word(
        ones_complement_folded_sum(body),
        record_state);
    return static_cast<std::uint16_t>(~with_state);
}

std::optional<std::uint16_t> calculate_record_checksum(
    std::span<const std::byte> complete_record) noexcept {
    if (complete_record.size() < integrity_trailer_size ||
        complete_record.size() % 2U != 0U) {
        return std::nullopt;
    }
    return static_cast<std::uint16_t>(~ones_complement_folded_sum(
        complete_record.first(complete_record.size() - checksum_size)));
}

bool validate_record_checksum(
    std::span<const std::byte> complete_record) noexcept {
    const auto calculated = calculate_record_checksum(complete_record);
    if (!calculated.has_value()) {
        return false;
    }
    const auto stored = read_u16_le(
        complete_record,
        complete_record.size() - checksum_size);
    return *calculated == stored &&
           ones_complement_folded_sum(complete_record) == valid_integrity_fold;
}

bool BlockDescriptor::valid() const noexcept {
    if (data_size == 0U || block_size != data_size + integrity_trailer_size ||
        trailer_offset != data_offset + data_size ||
        checksum_offset() + checksum_size != data_offset + block_size ||
        data_offset > file_size || block_size > file_size - data_offset) {
        return false;
    }
    switch (kind) {
    case BlockKind::global_record:
        return index == 0U && data_offset == global_body_offset &&
               data_size == global_body_size;
    case BlockKind::slot_summary:
        return index < summary_count && data_size == summary_body_size &&
               data_offset ==
                   summary_records_offset + index * summary_record_size;
    case BlockKind::slot_payload:
        return index < payload_count && data_size == payload_body_size &&
               data_offset ==
                   payload_records_offset + index * payload_record_size;
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
    return recognized && blocks.size() == record_count &&
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

    result.observed_compatibility_tag = read_u16_le(
        bytes,
        compatibility_tag_offset);
    result.observed_header_marker = result.observed_compatibility_tag;
    if (result.observed_compatibility_tag != compatibility_tag) {
        add_diagnostic(
            result,
            formats::ParseSeverity::error,
            "pc-save.compatibility-tag-mismatch",
            "Expected the confirmed 0xDEC0 compatibility tag at global body offset 0x108.",
            compatibility_tag_offset);
        return result;
    }

    result.recognized = true;
    result.global_record = parse_global_record(bytes);
    result.blocks.reserve(record_count);
    result.blocks.push_back(analyze_block(bytes, global_descriptor()));
    for (std::size_t index = 0U; index < summary_count; ++index) {
        const auto descriptor = summary_descriptor(index);
        result.blocks.push_back(analyze_block(bytes, descriptor));
        result.summaries[index] = parse_summary(bytes, index);
    }
    for (std::size_t index = 0U; index < payload_count; ++index) {
        result.blocks.push_back(analyze_block(bytes, payload_descriptor(index)));
        result.payload_boundaries[index] = observe_payload_boundary(bytes, index);
    }

    for (const auto& block_integrity : result.blocks) {
        if (!block_integrity.descriptor.valid()) {
            add_diagnostic(
                result,
                formats::ParseSeverity::error,
                "pc-save.internal-layout-invalid",
                "An internal block descriptor violates the confirmed Pass 32 record-envelope layout.",
                block_integrity.descriptor.data_offset);
        } else if (!block_integrity.checksum_matches) {
            add_diagnostic(
                result,
                formats::ParseSeverity::error,
                "pc-save.checksum-mismatch",
                "The stored record checksum differs from the Pass 32 calculation.",
                block_integrity.descriptor.checksum_offset());
        } else if (!block_integrity.valid_fold) {
            add_diagnostic(
                result,
                formats::ParseSeverity::error,
                "pc-save.integrity-invalid",
                "The complete record does not fold to 0xFFFF under the confirmed one's-complement validator.",
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
                summary_records_offset + index * summary_record_size);
        }
    }

    if (!result.global_record.cleared_region_is_zero) {
        add_diagnostic(
            result,
            formats::ParseSeverity::warning,
            "pc-save.global-cleared-region-nonzero",
            "The Pass 32 writer-cleared global region +0x046..+0x106 is nonzero.",
            global_cleared_region_offset);
    }
    if (!result.global_record.reserved_tail_is_zero) {
        add_diagnostic(
            result,
            formats::ParseSeverity::warning,
            "pc-save.global-reserved-tail-nonzero",
            "The observed global reserved tail +0x10C..+0x133 is nonzero.",
            global_tail_reserved_offset);
    }
    for (const auto& payload : result.payload_boundaries) {
        const auto base = payload_records_offset + payload.index * payload_record_size;
        if (!payload.cleared_reserved_is_zero) {
            add_diagnostic(
                result,
                formats::ParseSeverity::warning,
                "pc-save.payload-cleared-region-nonzero",
                "The Pass 32 writer-cleared payload region +0x6EC..+0x705 is nonzero.",
                base + payload_cleared_region_offset);
        }
        if (!payload.observed_tail_is_zero) {
            add_diagnostic(
                result,
                formats::ParseSeverity::info,
                "pc-save.payload-observed-tail-nonzero",
                "The sample-observed payload tail +0x706..+0x707 is nonzero; writer ownership remains research required.",
                base + payload_observed_zero_tail_offset);
        }
    }
    return result;
}

} // namespace dmc::rengine::save::pc
