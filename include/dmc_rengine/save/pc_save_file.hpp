#pragma once

#include "dmc_rengine/formats/diagnostic.hpp"

#include <array>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <span>
#include <vector>

namespace dmc::rengine::save::pc {

inline constexpr std::size_t file_size = 0x4A30U;
inline constexpr std::size_t integrity_trailer_size = 0x04U;

inline constexpr std::size_t header_data_offset = 0x0000U;
inline constexpr std::size_t header_data_size = 0x0134U;
inline constexpr std::size_t header_trailer_offset = 0x0134U;
inline constexpr std::size_t header_block_size = 0x0138U;

inline constexpr std::size_t summary_count = 10U;
inline constexpr std::size_t summary_blocks_offset = 0x0138U;
inline constexpr std::size_t summary_data_size = 0x003CU;
inline constexpr std::size_t summary_block_size = 0x0040U;

inline constexpr std::size_t payload_count = 10U;
inline constexpr std::size_t payload_blocks_offset = 0x03B8U;
inline constexpr std::size_t payload_data_size = 0x0708U;
inline constexpr std::size_t payload_block_size = 0x070CU;

inline constexpr std::size_t header_marker_offset = 0x0108U;
inline constexpr std::uint16_t header_marker = 0xDEC0U;
inline constexpr std::uint16_t valid_integrity_fold = 0xFFFFU;

static_assert(
    header_block_size + summary_count * summary_block_size +
            payload_count * payload_block_size ==
        file_size);
static_assert(
    summary_blocks_offset + summary_count * summary_block_size ==
        payload_blocks_offset);

[[nodiscard]] std::optional<std::uint8_t> decode_packed_bcd(
    std::uint8_t value) noexcept;

[[nodiscard]] std::uint16_t ones_complement_folded_sum(
    std::span<const std::byte> bytes) noexcept;

[[nodiscard]] std::uint16_t generate_integrity_word(
    std::span<const std::byte> data,
    std::uint16_t status_or_presence) noexcept;

enum class BlockKind {
    header,
    slot_summary,
    slot_payload,
};

struct BlockDescriptor final {
    BlockKind kind{BlockKind::header};
    std::size_t index{};
    std::size_t data_offset{};
    std::size_t data_size{};
    std::size_t trailer_offset{};
    std::size_t block_size{};

    [[nodiscard]] bool valid() const noexcept;
};

struct IntegrityTrailer final {
    std::uint16_t status_or_presence{};
    std::uint16_t integrity_word{};
};

struct BlockIntegrity final {
    BlockDescriptor descriptor;
    IntegrityTrailer trailer;
    std::uint16_t folded_sum{};
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

struct AnalysisResult final {
    bool recognized{false};
    std::uint16_t observed_header_marker{};
    std::vector<BlockIntegrity> blocks;
    std::array<SummaryRecord, summary_count> summaries{};
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
