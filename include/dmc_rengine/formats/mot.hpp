#pragma once

#include <cstddef>
#include <cstdint>
#include <optional>
#include <span>
#include <string>
#include <vector>

namespace dmc::rengine::formats::mot {

enum class ParseError : std::uint8_t {
    none,
    truncated_header,
    invalid_magic,
    unsupported_observed_header_size,
    unreasonable_record_count,
    truncated_record,
    invalid_record_span,
    nonzero_tail,
};

struct Record final {
    std::size_t offset{};
    std::uint16_t span{};
};

struct Document final {
    std::size_t physical_size{};
    std::uint32_t header_size{};
    float raw_time_a{};
    float raw_time_b{};
    float raw_time_c{};
    std::vector<std::byte> raw_header;
    std::uint32_t record_count{};
    std::vector<Record> records;
    std::size_t zero_padding_size{};

    [[nodiscard]] bool valid() const noexcept;
};

struct ParseResult final {
    std::optional<Document> document;
    ParseError error{ParseError::none};
    std::string message;

    [[nodiscard]] bool ok() const noexcept {
        return document.has_value() && error == ParseError::none;
    }
};

// Read-only structural parser for the MOT envelope observed across all 82
// em000 motion-container children. Header float semantics and record payload
// semantics intentionally remain raw until bound to CMotion consumers.
class Reader final {
public:
    static constexpr std::uint32_t k_observed_header_small = 0x30U;
    static constexpr std::uint32_t k_observed_header_large = 0x50U;
    static constexpr std::uint32_t k_safety_max_records = 65536U;

    [[nodiscard]] static ParseResult parse(std::span<const std::byte> bytes);
    [[nodiscard]] static bool structurally_valid(
        std::span<const std::byte> bytes) noexcept;
};

} // namespace dmc::rengine::formats::mot
