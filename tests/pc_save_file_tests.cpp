#include "dmc_rengine/save/pc_save_file.hpp"

#include <algorithm>
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

void finalize_block(
    std::vector<std::byte>& bytes,
    std::size_t data_offset,
    std::size_t data_size,
    std::uint16_t status_or_presence) {
    using namespace dmc::rengine::save::pc;
    const auto data = std::span<const std::byte>{bytes}.subspan(
        data_offset,
        data_size);
    const auto integrity_word = generate_integrity_word(
        data,
        status_or_presence);
    write_u16_le(bytes, data_offset + data_size, status_or_presence);
    write_u16_le(bytes, data_offset + data_size + 2U, integrity_word);
}

[[nodiscard]] std::vector<std::byte> valid_save() {
    using namespace dmc::rengine::save::pc;
    std::vector<std::byte> bytes(file_size, std::byte{0});
    write_u16_le(bytes, header_marker_offset, header_marker);

    const auto summary_0 = summary_blocks_offset;
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

    finalize_block(
        bytes,
        header_data_offset,
        header_data_size,
        1U);
    for (std::size_t index = 0U; index < summary_count; ++index) {
        finalize_block(
            bytes,
            summary_blocks_offset + index * summary_block_size,
            summary_data_size,
            index == 0U ? 1U : 0U);
    }
    for (std::size_t index = 0U; index < payload_count; ++index) {
        const auto offset = payload_blocks_offset + index * payload_block_size;
        write_u8(bytes, offset, static_cast<std::uint8_t>(index + 1U));
        finalize_block(bytes, offset, payload_data_size, 1U);
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
    static_assert(summary_blocks_offset == 0x0138U);
    static_assert(payload_blocks_offset == 0x03B8U);
    static_assert(summary_block_size == 0x0040U);
    static_assert(payload_block_size == 0x070CU);

    const auto bytes = valid_save();
    const auto result = Analyzer::analyze(bytes);
    assert(result.recognized);
    assert(result.ok());
    assert(result.observed_header_marker == header_marker);
    assert(result.blocks.size() == 21U);
    assert(result.valid_block_count() == 21U);

    const auto* header = result.block(BlockKind::header);
    const auto* summary_0 = result.block(BlockKind::slot_summary, 0U);
    const auto* summary_9 = result.block(BlockKind::slot_summary, 9U);
    const auto* payload_0 = result.block(BlockKind::slot_payload, 0U);
    const auto* payload_9 = result.block(BlockKind::slot_payload, 9U);
    assert(header != nullptr && header->valid);
    assert(header->descriptor.trailer_offset == 0x0134U);
    assert(summary_0 != nullptr && summary_0->valid);
    assert(summary_0->descriptor.data_offset == 0x0138U);
    assert(summary_9 != nullptr && summary_9->valid);
    assert(summary_9->descriptor.data_offset == 0x0378U);
    assert(payload_0 != nullptr && payload_0->valid);
    assert(payload_0->descriptor.data_offset == 0x03B8U);
    assert(payload_9 != nullptr && payload_9->valid);
    assert(payload_9->descriptor.data_offset == 0x4324U);
    assert(payload_9->descriptor.trailer_offset == 0x4A2CU);

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

    assert(!decode_packed_bcd(0xFAU).has_value());

    auto corrupted = bytes;
    corrupted[payload_blocks_offset + payload_block_size + 0x10U] ^=
        std::byte{0x01U};
    const auto corrupted_result = Analyzer::analyze(corrupted);
    assert(corrupted_result.recognized);
    assert(!corrupted_result.ok());
    assert(corrupted_result.valid_block_count() == 20U);
    assert(has_diagnostic(
        corrupted_result,
        "pc-save.integrity-invalid"));
    const auto* corrupted_payload = corrupted_result.block(
        BlockKind::slot_payload,
        1U);
    assert(corrupted_payload != nullptr && !corrupted_payload->valid);

    auto bad_marker = bytes;
    write_u16_le(bad_marker, header_marker_offset, 0U);
    const auto bad_marker_result = Analyzer::analyze(bad_marker);
    assert(!bad_marker_result.recognized);
    assert(has_diagnostic(
        bad_marker_result,
        "pc-save.header-marker-mismatch"));

    const std::vector<std::byte> wrong_size(file_size - 1U, std::byte{0});
    const auto wrong_size_result = Analyzer::analyze(wrong_size);
    assert(!wrong_size_result.recognized);
    assert(has_diagnostic(
        wrong_size_result,
        "pc-save.invalid-size"));

    const std::array<std::byte, 2U> sample{
        std::byte{0x34U},
        std::byte{0x12U},
    };
    assert(ones_complement_folded_sum(sample) == 0x1234U);
    const auto generated = generate_integrity_word(sample, 1U);
    std::array<std::byte, 6U> full_block{
        sample[0], sample[1],
        std::byte{0x01U}, std::byte{0x00U},
        static_cast<std::byte>(generated & 0xFFU),
        static_cast<std::byte>((generated >> 8U) & 0xFFU),
    };
    assert(ones_complement_folded_sum(full_block) == 0xFFFFU);
    return 0;
}
