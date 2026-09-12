#pragma once

#include "dmc_rengine/formats/diagnostic.hpp"

#include <cstddef>
#include <cstdint>
#include <span>
#include <vector>

namespace dmc::rengine::formats::evt {

inline constexpr std::size_t header_size = 0x20U;
inline constexpr std::uint16_t corpus_revision = 1U;
inline constexpr std::uint8_t terminal_opcode = 0x20U;
inline constexpr std::uint8_t single_stream_pre_terminal_opcode = 0x01U;
inline constexpr std::uint8_t multi_stream_pre_terminal_opcode = 0x0EU;
inline constexpr std::uint8_t observed_stream_entry_opcode = 0x57U;
inline constexpr std::uint8_t observed_stream_boundary_opcode = 0x00U;

struct Command final {
    std::uint64_t offset{};
    std::uint32_t raw_header{};
    std::uint8_t opcode{};
    std::uint8_t argument_count{};
    std::vector<std::uint32_t> arguments;

    [[nodiscard]] std::uint64_t serialized_size() const noexcept {
        return 4U + static_cast<std::uint64_t>(argument_count) * 4U;
    }
};

struct Header final {
    // Exact packed little-endian word at +0x04. Kept as `version` for source
    // compatibility with the first bounded reader. Full GData corpus evidence
    // shows low16=revision and high16=stream_count.
    std::uint32_t version{};
    std::uint16_t revision{};
    std::uint16_t stream_count{};
    std::uint32_t terminal_command_offset{};
    std::vector<std::byte> preserved_reserved_bytes;
};

struct Document final {
    Header header;
    std::vector<Command> commands;
    // First stream is implicit at +0x20. Remaining stream starts are serialized
    // as u32 offsets immediately after the terminal 0x20 command.
    std::vector<std::uint32_t> stream_offsets;
    std::vector<std::byte> trailing_padding;
    std::vector<std::byte> source_bytes;
};

struct ParseResult final {
    bool recognized{false};
    Document document;
    std::vector<ParseDiagnostic> diagnostics;

    [[nodiscard]] bool ok() const noexcept;
};

class Parser final {
public:
    [[nodiscard]] static ParseResult parse(std::span<const std::byte> bytes);
};

} // namespace dmc::rengine::formats::evt
