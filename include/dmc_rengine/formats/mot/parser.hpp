#pragma once

#include "dmc_rengine/formats/mot/ir.hpp"

#include <cstdint>
#include <optional>
#include <span>
#include <string>

namespace dmc::rengine::formats::mot {

enum class ParseError : std::uint8_t {
    none,
    truncated_header,
    invalid_magic,
    invalid_header_size,
    unknown_channel_mask_bits,
    unreasonable_record_count,
    record_count_mask_mismatch,
    truncated_track,
    invalid_track_span,
    known_compression_span_mismatch,
    non_monotonic_key_time,
    nonzero_tail,
};

struct ParseResult final {
    std::optional<Document> document;
    ParseError error{ParseError::none};
    std::string message;

    [[nodiscard]] bool ok() const noexcept {
        return document.has_value() && error == ParseError::none;
    }
};

class Parser final {
public:
    static constexpr std::uint32_t k_safety_max_records = 65536U;

    [[nodiscard]] static ParseResult parse(std::span<const std::byte> bytes);
    [[nodiscard]] static bool structurally_valid(
        std::span<const std::byte> bytes) noexcept;
};

} // namespace dmc::rengine::formats::mot
