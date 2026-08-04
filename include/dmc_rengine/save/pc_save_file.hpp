#pragma once

#include "dmc_rengine/formats/diagnostic.hpp"

#include <array>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <span>
#include <string_view>
#include <type_traits>
#include <vector>

namespace dmc::rengine::save::pc {

inline constexpr std::size_t file_size = 0x4A30U;
inline constexpr std::size_t record_state_size = 0x02U;
inline constexpr std::size_t checksum_size = 0x02U;
inline constexpr std::size_t integrity_trailer_size =
    record_state_size + checksum_size;
inline constexpr std::size_t record_count = 21U;

inline constexpr std::size_t global_body_offset = 0x0000U;
inline constexpr std::size_t global_body_size = 0x0134U;
inline constexpr std::size_t global_trailer_offset = 0x0134U;
inline constexpr std::size_t global_record_size = 0x0138U;

// Compatibility aliases retained for the Pass 31 public API.
inline constexpr std::size_t header_data_offset = global_body_offset;
inline constexpr std::size_t header_data_size = global_body_size;
inline constexpr std::size_t header_trailer_offset = global_trailer_offset;
inline constexpr std::size_t header_block_size = global_record_size;

inline constexpr std::size_t summary_count = 10U;
inline constexpr std::size_t summary_records_offset = 0x0138U;
inline constexpr std::size_t summary_body_size = 0x003CU;
inline constexpr std::size_t summary_record_size = 0x0040U;
inline constexpr std::size_t summary_blocks_offset = summary_records_offset;
inline constexpr std::size_t summary_data_size = summary_body_size;
inline constexpr std::size_t summary_block_size = summary_record_size;

inline constexpr std::size_t payload_count = 10U;
inline constexpr std::size_t payload_records_offset = 0x03B8U;
inline constexpr std::size_t payload_body_size = 0x0708U;
inline constexpr std::size_t payload_record_size = 0x070CU;
inline constexpr std::size_t payload_blocks_offset = payload_records_offset;
inline constexpr std::size_t payload_data_size = payload_body_size;
inline constexpr std::size_t payload_block_size = payload_record_size;

inline constexpr std::size_t compatibility_tag_offset = 0x0108U;
inline constexpr std::uint16_t compatibility_tag = 0xDEC0U;
inline constexpr std::size_t header_marker_offset = compatibility_tag_offset;
inline constexpr std::uint16_t header_marker = compatibility_tag;
inline constexpr std::uint16_t valid_integrity_fold = 0xFFFFU;

inline constexpr std::size_t global_runtime_state_offset = 0x001CU;
inline constexpr std::size_t global_runtime_state_size = 0x002AU;
inline constexpr std::size_t global_cleared_region_offset = 0x0046U;
inline constexpr std::size_t global_cleared_region_size = 0x00C1U;
inline constexpr std::size_t global_tail_reserved_offset = 0x010CU;
inline constexpr std::size_t global_tail_reserved_size = 0x0028U;

inline constexpr std::size_t payload_active_region_offset = 0x0000U;
inline constexpr std::size_t payload_active_region_size = 0x06ECU;
inline constexpr std::size_t payload_cleared_region_offset = 0x06ECU;
inline constexpr std::size_t payload_cleared_region_size = 0x001AU;
inline constexpr std::size_t payload_observed_zero_tail_offset = 0x0706U;
inline constexpr std::size_t payload_observed_zero_tail_size = 0x0002U;

static_assert(
    global_record_size + summary_count * summary_record_size +
            payload_count * payload_record_size ==
        file_size);
static_assert(
    summary_records_offset + summary_count * summary_record_size ==
        payload_records_offset);
static_assert(
    payload_active_region_size + payload_cleared_region_size +
            payload_observed_zero_tail_size ==
        payload_body_size);

// Physical ABI model only. Parsing still uses explicit little-endian reads.
template <std::size_t BodySize>
struct RecordEnvelopeLayout final {
    std::array<std::byte, BodySize> body{};
    std::uint16_t record_state{};
    std::uint16_t checksum{};
};

using GlobalRecordEnvelopeLayout = RecordEnvelopeLayout<global_body_size>;
using SummaryRecordEnvelopeLayout = RecordEnvelopeLayout<summary_body_size>;
using PayloadRecordEnvelopeLayout = RecordEnvelopeLayout<payload_body_size>;

static_assert(std::is_standard_layout_v<GlobalRecordEnvelopeLayout>);
static_assert(sizeof(GlobalRecordEnvelopeLayout) == global_record_size);
static_assert(sizeof(SummaryRecordEnvelopeLayout) == summary_record_size);
static_assert(sizeof(PayloadRecordEnvelopeLayout) == payload_record_size);
static_assert(
    offsetof(GlobalRecordEnvelopeLayout, record_state) == global_body_size);
static_assert(
    offsetof(GlobalRecordEnvelopeLayout, checksum) ==
    global_body_size + record_state_size);

struct GlobalRecordLayout final {
    std::uint32_t reserved_000{};
    std::uint32_t runtime_value_004{};
    std::array<std::byte, 0x10U> state_008{};
    std::uint16_t field_018{};
    std::uint8_t field_01a{};
    std::uint8_t padding_01b{};
    std::array<std::byte, global_runtime_state_size> runtime_state_01c{};
    std::array<std::byte, global_cleared_region_size> cleared_region_046{};
    std::uint8_t runtime_flag_107{};
    std::uint16_t compatibility_tag_108{};
    std::uint8_t legacy_fallback_10a{};
    std::uint8_t legacy_fallback_10b{};
    std::array<std::byte, global_tail_reserved_size> reserved_10c{};
};

static_assert(std::is_standard_layout_v<GlobalRecordLayout>);
static_assert(sizeof(GlobalRecordLayout) == global_body_size);
static_assert(offsetof(GlobalRecordLayout, runtime_state_01c) == 0x01CU);
static_assert(offsetof(GlobalRecordLayout, cleared_region_046) == 0x0046U);
static_assert(offsetof(GlobalRecordLayout, runtime_flag_107) == 0x0107U);
static_assert(offsetof(GlobalRecordLayout, compatibility_tag_108) == 0x0108U);
static_assert(offsetof(GlobalRecordLayout, reserved_10c) == 0x010CU);

struct DetailedPayloadLayout final {
    std::array<std::byte, payload_active_region_size> active_data{};
    std::array<std::byte, payload_cleared_region_size> cleared_reserved{};
    std::array<std::byte, payload_observed_zero_tail_size> observed_zero_tail{};
};

static_assert(sizeof(DetailedPayloadLayout) == payload_body_size);
static_assert(
    offsetof(DetailedPayloadLayout, cleared_reserved) ==
    payload_cleared_region_offset);
static_assert(
    offsetof(DetailedPayloadLayout, observed_zero_tail) ==
    payload_observed_zero_tail_offset);

[[nodiscard]] std::optional<std::uint8_t> decode_packed_bcd(
    std::uint8_t value) noexcept;

// Confirmed production records are all even-sized. Odd-tail placement is not
// part of the promoted Pass 32 contract until direct function-byte evidence is
// reconciled.
[[nodiscard]] std::uint16_t ones_complement_folded_sum(
    std::span<const std::byte> bytes) noexcept;

[[nodiscard]] std::uint16_t generate_integrity_word(
    std::span<const std::byte> body,
    std::uint16_t record_state) noexcept;

// Computes the checksum over body + recordState while ignoring the final
// stored checksum word. Returns nullopt for malformed records.
[[nodiscard]] std::optional<std::uint16_t> calculate_record_checksum(
    std::span<const std::byte> complete_record) noexcept;

[[nodiscard]] bool validate_record_checksum(
    std::span<const std::byte> complete_record) noexcept;

enum class BlockKind : std::uint8_t {
    global_record = 0,
    header = global_record,
    slot_summary = 1,
    slot_payload = 2,
};

[[nodiscard]] constexpr std::string_view to_string(BlockKind kind) noexcept {
    switch (kind) {
    case BlockKind::global_record: return "global-record";
    case BlockKind::slot_summary: return "slot-summary";
    case BlockKind::slot_payload: return "slot-payload";
    }
    return "global-record";
}

enum class RecordStateClass : std::uint8_t {
    zero = 0,
    one = 1,
    other = 2,
};

[[nodiscard]] constexpr RecordStateClass classify_record_state(
    std::uint16_t value) noexcept {
    if (value == 0U) {
        return RecordStateClass::zero;
    }
    if (value == 1U) {
        return RecordStateClass::one;
    }
    return RecordStateClass::other;
}

struct BlockDescriptor final {
    BlockKind kind{BlockKind::global_record};
    std::size_t index{};
    std::size_t data_offset{};
    std::size_t data_size{};
    std::size_t trailer_offset{};
    std::size_t block_size{};

    [[nodiscard]] bool valid() const noexcept;
    [[nodiscard]] std::size_t checksum_offset() const noexcept {
        return trailer_offset + record_state_size;
    }
};

struct IntegrityTrailer final {
    union {
        std::uint16_t record_state{};
        std::uint16_t status_or_presence;
    };
    union {
        std::uint16_t checksum{};
        std::uint16_t integrity_word;
    };
};

struct BlockIntegrity final {
    BlockDescriptor descriptor;
    IntegrityTrailer trailer;
    std::uint16_t calculated_checksum{};
    std::uint16_t folded_sum{};
    bool checksum_matches{false};
    bool valid_fold{false};
    bool valid{false};
};

struct SummaryRecord final {
    std::uint8_t valid{};
    std::uint8_t year_bcd{};
    std::uint8_t month_bcd{};
    std::uint8_t day_bcd{};
    std::uint8_t play_hours{};
    std::uint8_t play_minutes{};
    std::uint8_t play_seconds{};
    std::uint8_t availability_or_progress_flag{};
    std::uint32_t mission_or_event_index{};
    std::uint32_t field_0c{};
    std::uint32_t field_10{};
    std::uint32_t field_14{};
    std::uint8_t state_byte{};
    std::uint8_t set_flag_count{};
    std::int8_t progress_category{};
    std::uint32_t progress_index{};
    std::uint32_t progress_lookup_value{};
    std::uint8_t save_hour_bcd{};
    std::uint8_t save_minute_bcd{};
    std::uint8_t save_second_bcd{};
    std::uint8_t option_byte_0{};
    std::uint8_t option_byte_1{};
    std::uint16_t option_word{};

    [[nodiscard]] bool packed_bcd_fields_valid() const noexcept;
};

struct GlobalRecordObservation final {
    std::uint32_t reserved_000{};
    std::uint32_t runtime_value_004{};
    std::array<std::byte, 0x10U> state_008{};
    std::uint16_t field_018{};
    std::uint8_t field_01a{};
    std::array<std::byte, global_runtime_state_size> runtime_state_01c{};
    bool cleared_region_is_zero{false};
    std::uint8_t runtime_flag_107{};
    std::uint16_t compatibility_tag_108{};
    std::uint8_t legacy_fallback_10a{};
    std::uint8_t legacy_fallback_10b{};
    bool reserved_tail_is_zero{false};
};

struct PayloadBoundaryObservation final {
    std::size_t index{};
    bool cleared_reserved_is_zero{false};
    bool observed_tail_is_zero{false};
};

struct AnalysisResult final {
    bool recognized{false};
    std::uint16_t observed_compatibility_tag{};
    std::uint16_t observed_header_marker{};
    std::vector<BlockIntegrity> blocks;
    GlobalRecordObservation global_record;
    std::array<SummaryRecord, summary_count> summaries{};
    std::array<PayloadBoundaryObservation, payload_count> payload_boundaries{};
    std::vector<formats::ParseDiagnostic> diagnostics;

    [[nodiscard]] bool ok() const noexcept;
    [[nodiscard]] std::size_t valid_block_count() const noexcept;
    [[nodiscard]] const BlockIntegrity* block(
        BlockKind kind,
        std::size_t index = 0U) const noexcept;
};

class Analyzer final {
public:
    [[nodiscard]] static AnalysisResult analyze(
        std::span<const std::byte> bytes);
};

} // namespace dmc::rengine::save::pc
