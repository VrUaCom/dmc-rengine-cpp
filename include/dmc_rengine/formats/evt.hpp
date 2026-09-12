#pragma once

#include "dmc_rengine/formats/diagnostic.hpp"

#include <cstddef>
#include <cstdint>
#include <span>
#include <vector>

namespace dmc::rengine::formats::evt {

inline constexpr std::size_t header_size = 0x20U;
inline constexpr std::uint32_t corpus_version = 0x00010001U;
inline constexpr std::uint8_t terminal_opcode = 0x20U;
inline constexpr std::uint8_t pre_terminal_opcode = 0x01U;

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
    std::uint32_t version{};
    std::uint32_t terminal_command_offset{};
    std::vector<std::byte> preserved_reserved_bytes;
};

struct Document final {
    Header header;
    std::vector<Command> commands;
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
