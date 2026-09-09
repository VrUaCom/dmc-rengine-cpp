#pragma once

#include "dmc_rengine/formats/diagnostic.hpp"

#include <cstddef>
#include <cstdint>
#include <span>
#include <vector>

namespace dmc::rengine::formats::so::link_table {

inline constexpr std::size_t record_size = 0x04U;

// Recovered 2026-09-09 from the complete em000 extraction (source archive
// SHA-256 306130125f09824811289366324f4208c3c1aba880c5a7efa3953a88d566d07b),
// whose slot 39 holds a leading word and 23 records — one for each of the 23
// volume records in slot 40.
//
// The reader used to accept any byte run whose length divided by four, which
// is 305 of that corpus's 306 payloads. Two things separate a link table from
// an arbitrary stream of quads, and neither is sufficient alone: the leading
// word (which leaves one effect record) and the reserved fourth byte (which
// leaves the ten effect-M companions). Together they leave one.

/// The leading word. `06 00 00 00`; the graph payload opens on the same 6.
inline constexpr std::uint8_t leading_word_value = 0x06U;
inline constexpr std::size_t leading_word_size = 0x04U;

/// The fourth byte of every record, reserved and zero in all 24 words.
inline constexpr std::size_t reserved_field_offset = 0x03U;

// `field2` is the node a volume hangs off, not the record's own ordinal: it
// equals the ordinal for most records, repeats for volumes sharing a node,
// and is zero for the four that hang off the root. Recorded because reading
// it as a self-index makes those four look corrupt.
inline constexpr std::size_t node_field_offset = 0x02U;

struct Record final {
    std::uint8_t field0{};
    std::uint8_t field1{};
    std::uint8_t field2{};
    std::uint8_t field3{};
};

struct ParseResult final {
    bool recognized{false};
    std::vector<Record> records;
    std::vector<ParseDiagnostic> diagnostics;

    [[nodiscard]] bool ok() const noexcept;
};

/**
 * Whether these bytes carry this payload, without building its records.
 *
 * Classification runs on every slot of every container a browser opens, so
 * the identity question is separated from the reading: `parse` answers both
 * and allocates, this answers only the first and does not.
 */
[[nodiscard]] bool recognizes(std::span<const std::byte> bytes) noexcept;

[[nodiscard]] ParseResult parse(std::span<const std::byte> bytes);

} // namespace dmc::rengine::formats::so::link_table
