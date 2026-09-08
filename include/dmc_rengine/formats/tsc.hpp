#pragma once

#include <cstddef>
#include <cstdint>
#include <optional>
#include <span>
#include <string>
#include <vector>

namespace dmc::rengine::formats::tsc {

enum class ParseError : std::uint8_t {
    none,
    empty,
    non_text_byte,
    nonzero_tail,
    missing_magic,
    missing_terminator,
};

struct SourceLine final {
    std::size_t source_line{};
    std::string raw;
    std::string key;
    std::string value;
    bool comment{false};
    bool block_marker{false};
};

struct Document final {
    std::size_t physical_size{};
    std::size_t text_size{};
    std::size_t zero_padding_size{};
    std::vector<SourceLine> lines;

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

// Preservation-first reader for the text-serialized DMC3 TSC motion/control
// resource. Field values remain strings until their runtime consumers are
// independently recovered.
class Reader final {
public:
    [[nodiscard]] static ParseResult parse(std::span<const std::byte> bytes);
    [[nodiscard]] static bool structurally_valid(
        std::span<const std::byte> bytes) noexcept;
};

} // namespace dmc::rengine::formats::tsc
