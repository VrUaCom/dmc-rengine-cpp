#include "dmc_rengine/save/pc_save_file.hpp"

#include <algorithm>
#include <array>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <span>
#include <string_view>
#include <vector>

namespace {

void write_u8(
    std::vector<std::byte>& bytes,
    std::size_t offset,
    std::uint8_t value) {
    bytes[offset] = static_cast<std::byte>(value);
}

void write_u16_le(
    std::vector<std::byte>& bytes,
    std::size_t offset,
    std::uint16_t value) {
    bytes[offset] = static_cast<std::byte>(value & 0xFFU);
    bytes[offset + 1U] = static_cast<std::byte>((value >> 8U) & 0xFFU);
}

void write_u32_le(
    std::vector<std::byte>& bytes,
    std::size_t offset,
    std::uint32_t value) {
    for (std::size_t index = 0U; index < 4U; ++index) {
        bytes[offset + index] = static_cast<std::byte>(
            (value >> static_cast<unsigned>(index * 8U)) & 0xFFU);
    }
}

void finalize_record(
    std::vector<std::byte>& bytes,
    std::size_t body_offset,
    std::size_t body_size,
    std::uint16_t record_state) {
    using namespace dmc::rengine::save::pc;
    write_u16_le(bytes, body_offset + body_size, record_state);
    write_u16_le(bytes, body_offset + body_size + record_state_size, 0U);
    const auto record = std::span<const std::byte>{bytes}.subspan(
        body_offset,
        body_size + integrity_trailer_size);
    const auto checksum = calculate_record_checksum(record);
    assert(checksum.has_value());
    write_u16_le(
        bytes,
        body_offset + body_size + record_state_size,
        *checksum);
}

[[nodiscard]] std::vector<std::byte> valid_save() {
    using namespace dmc::rengine::save::pc;
    std::vector<std::byte> bytes(file_size, std::byte{0});

    write_u32_le(bytes, 0x004U, 0x12345678U);
    write_u16_le(bytes, 0x018U, 0x2468U);
    write_u8(bytes, 0x01AU, 0x7FU);
    write_u8(bytes, 0x107U, 1U);
    write_u16_le(bytes, compatibility_tag_offset, compatibility_tag);
    write_u8(bytes, 0x10AU, 2U);
    write_u8(bytes, 0x10BU, 3U);

    const auto summary_0 = summary_records_offset;
    write_u8(bytes, summary_0 + 0x00U, 1U);
    write_u8(bytes, summary_0 + 0x01U, 0x24U);
    write_u8(bytes, summary_0 + 0x02U, 0x02U);
    write_u8(bytes, summary_0 + 0x03U, 0x04U);
    write_u8(bytes, summary_0 + 0x04U, 13U);
    write_u8(bytes, summary_0 + 0x05U, 39U);
    write_u8(bytes, summary_0 + 0x06U, 24U);
    write_u8(bytes, summary_0 + 0x07U, 1U);
    write_u32_le(bytes, summary_0 + 0x08U, 7U);
    write_u32_le(bytes, summary_0 + 0x0CU, 0x11223344U);
    write_u32_le(bytes, summary_0 + 0x10U, 0x55667788U);
    write_u32_le(bytes, summary_0 + 0x14U, 0x99AABBCCU);
    write_u8(bytes, summary_0 + 0x18U, 2U);
    write_u8(bytes, summary_0 + 0x19U, 3U);
    write_u8(bytes, summary_0 + 0x1AU, 0xFEU);
    write_u32_le(bytes, summary_0 + 0x1CU, 12U);
    write_u32_le(bytes, summary_0 + 0x20U, 34U);
    write_u8(bytes, summary_0 + 0x24U, 0x13U);
    write_u8(bytes, summary_0 + 0x25U, 0x39U);
    write_u8(bytes, summary_0 + 0x26U, 0x24U);
    write_u8(bytes, summary_0 + 0x27U, 4U);
    write_u8(bytes, summary_0 + 0x28U, 5U);
    write_u16_le(bytes, summary_0 + 0x2AU, 0x1234U);

    finalize_record(bytes, global_body_offset, global_body_size, 1U);
    constexpr std::array<std::uint16_t, summary_count> summary_states{
        1U, 1U, 0U, 0U, 1U, 0U, 0U, 0U, 1U, 1U,
    };
    for (std::size_t index = 0U; index < summary_count; ++index) {
        finalize_record(
            bytes,
            summary_records_offset + index * summary_record_size,
            summary_body_size,
            summary_states[index]);
    }
    for (std::size_t index = 0U; index < payload_count; ++index) {
        const auto offset = payload_records_offset + index * payload_record_size;
        write_u8(bytes, offset, static_cast<std::uint8_t>(index + 1U));
        finalize_record(bytes, offset, payload_body_size, 1U);
    }
    return bytes;
}

[[nodiscard]] bool has_diagnostic(
    const dmc::rengine::save::pc::AnalysisResult& result,
    std::string_view code) {
    return std::any_of(
        result.diagnostics.begin(), result.diagnostics.end(),
        [code](const auto& diagnostic) {
            return diagnostic.code == code;
        });
}

} // namespace

int main() {
    using namespace dmc::rengine::save::pc;

    static_assert(file_size == 0x4A30U);
    static_assert(record_count == 21U);
    static_assert(summary_records_offset == 0x0138U);
    static_assert(payload_records_offset == 0x03B8U);
    static_assert(summary_record_size == 0x0040U);
    static_assert(payload_record_size == 0x070CU);
    static_assert(sizeof(GlobalRecordEnvelopeLayout) == 0x0138U);
    static_assert(sizeof(SummaryRecordEnvelopeLayout) == 0x0040U);
    static_assert(sizeof(PayloadRecordEnvelopeLayout) == 0x070CU);
    static_assert(sizeof(GlobalRecordLayout) == 0x0134U);
    static_assert(sizeof(DetailedPayloadLayout) == 0x0708U);

    const auto bytes = valid_save();
    const auto result = Analyzer::analyze(bytes);
    assert(result.recognized);
    assert(result.ok());
    assert(result.observed_compatibility_tag == compatibility_tag);
    assert(result.observed_header_marker == header_marker);
    assert(result.blocks.size() == record_count);
    assert(result.valid_block_count() == record_count);

    const auto* global = result.block(BlockKind::global_record);
    const auto* legacy_global = result.block(BlockKind::header);
    const auto* summary_0 = result.block(BlockKind::slot_summary, 0U);
    const auto* summary_2 = result.block(BlockKind::slot_summary, 2U);
    const auto* summary_9 = result.block(BlockKind::slot_summary, 9U);
    const auto* payload_0 = result.block(BlockKind::slot_payload, 0U);
    const auto* payload_9 = result.block(BlockKind::slot_payload, 9U);
    assert(global != nullptr && global->valid);
    assert(global == legacy_global);
    assert(global->descriptor.trailer_offset == 0x0134U);
    assert(global->trailer.record_state == 1U);
    assert(global->trailer.status_or_presence == 1U);
    assert(global->trailer.checksum == global->trailer.integrity_word);
    assert(global->checksum_matches && global->valid_fold);
    assert(summary_0 != nullptr && summary_0->valid);
    assert(summary_0->descriptor.data_offset == 0x0138U);
    assert(summary_2 != nullptr && summary_2->valid);
    assert(summary_2->trailer.record_state == 0U);
    assert(summary_2->trailer.checksum == 0xFFFFU);
    assert(summary_9 != nullptr && summary_9->valid);
    assert(summary_9->descriptor.data_offset == 0x0378U);
    assert(payload_0 != nullptr && payload_0->valid);
    assert(payload_0->descriptor.data_offset == 0x03B8U);
    assert(payload_9 != nullptr && payload_9->valid);
    assert(payload_9->descriptor.data_offset == 0x4324U);
    assert(payload_9->descriptor.trailer_offset == 0x4A2CU);
    for (std::size_t index = 0U; index < payload_count; ++index) {
        const auto* payload = result.block(BlockKind::slot_payload, index);
        assert(payload != nullptr);
        assert(payload->trailer.record_state == 1U);
        assert(result.payload_boundaries[index].cleared_reserved_is_zero);
        assert(result.payload_boundaries[index].observed_tail_is_zero);
    }

    assert(result.global_record.runtime_value_004 == 0x12345678U);
    assert(result.global_record.field_018 == 0x2468U);
    assert(result.global_record.field_01a == 0x7FU);
    assert(result.global_record.runtime_flag_107 == 1U);
    assert(result.global_record.compatibility_tag_108 == compatibility_tag);
    assert(result.global_record.legacy_fallback_10a == 2U);
    assert(result.global_record.legacy_fallback_10b == 3U);
    assert(result.global_record.cleared_region_is_zero);
    assert(result.global_record.reserved_tail_is_zero);

    const auto& summary = result.summaries[0];
    assert(summary.valid == 1U);
    assert(summary.packed_bcd_fields_valid());
    assert(decode_packed_bcd(summary.year_bcd) == 24U);
    assert(decode_packed_bcd(summary.month_bcd) == 2U);
    assert(decode_packed_bcd(summary.day_bcd) == 4U);
    assert(summary.play_hours == 13U);
    assert(summary.play_minutes == 39U);
    assert(summary.play_seconds == 24U);
    assert(summary.mission_or_event_index == 7U);
    assert(summary.field_0c == 0x11223344U);
    assert(summary.field_10 == 0x55667788U);
    assert(summary.field_14 == 0x99AABBCCU);
    assert(summary.progress_category == static_cast<std::int8_t>(-2));
    assert(summary.progress_index == 12U);
    assert(summary.progress_lookup_value == 34U);
    assert(summary.option_word == 0x1234U);

    assert(classify_record_state(0U) == RecordStateClass::zero);
    assert(classify_record_state(1U) == RecordStateClass::one);
    assert(classify_record_state(2U) == RecordStateClass::other);
    assert(!decode_packed_bcd(0xFAU).has_value());

    const auto global_record = std::span<const std::byte>{bytes}.first(
        global_record_size);
    assert(calculate_record_checksum(global_record) == global->trailer.checksum);
    assert(validate_record_checksum(global_record));
    assert(!calculate_record_checksum(
        std::span<const std::byte>{bytes}.first(3U)).has_value());

    auto corrupted = bytes;
    corrupted[payload_records_offset + payload_record_size + 0x10U] ^=
        std::byte{0x01U};
    const auto corrupted_result = Analyzer::analyze(corrupted);
    assert(corrupted_result.recognized);
    assert(!corrupted_result.ok());
    assert(corrupted_result.valid_block_count() == record_count - 1U);
    assert(has_diagnostic(corrupted_result, "pc-save.checksum-mismatch"));
    const auto* corrupted_payload = corrupted_result.block(
        BlockKind::slot_payload,
        1U);
    assert(corrupted_payload != nullptr && !corrupted_payload->valid);

    auto nonzero_reserved = bytes;
    const auto payload_3 = payload_records_offset + 3U * payload_record_size;
    write_u8(
        nonzero_reserved,
        payload_3 + payload_cleared_region_offset,
        1U);
    finalize_record(nonzero_reserved, payload_3, payload_body_size, 1U);
    const auto reserved_result = Analyzer::analyze(nonzero_reserved);
    assert(reserved_result.ok());
    assert(has_diagnostic(
        reserved_result,
        "pc-save.payload-cleared-region-nonzero"));

    auto bad_tag = bytes;
    write_u16_le(bad_tag, compatibility_tag_offset, 0U);
    const auto bad_tag_result = Analyzer::analyze(bad_tag);
    assert(!bad_tag_result.recognized);
    assert(has_diagnostic(
        bad_tag_result,
        "pc-save.compatibility-tag-mismatch"));

    const std::vector<std::byte> wrong_size(file_size - 1U, std::byte{0});
    const auto wrong_size_result = Analyzer::analyze(wrong_size);
    assert(!wrong_size_result.recognized);
    assert(has_diagnostic(wrong_size_result, "pc-save.invalid-size"));

    const std::array<std::byte, 2U> sample{
        std::byte{0x34U},
        std::byte{0x12U},
    };
    assert(ones_complement_folded_sum(sample) == 0x1234U);
    const auto generated = generate_integrity_word(sample, 1U);
    std::array<std::byte, 6U> complete_record{
        sample[0], sample[1],
        std::byte{0x01U}, std::byte{0x00U},
        static_cast<std::byte>(generated & 0xFFU),
        static_cast<std::byte>((generated >> 8U) & 0xFFU),
    };
    assert(validate_record_checksum(complete_record));
    assert(ones_complement_folded_sum(complete_record) == 0xFFFFU);
    return 0;
}
