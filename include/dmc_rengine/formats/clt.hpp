#pragma once

#include <cstddef>
#include <cstdint>
#include <optional>
#include <span>
#include <string>
#include <vector>

namespace dmc::rengine::formats::clt {

enum class ParseError : std::uint8_t {
    none,
    empty,
    non_text_byte,
    nonzero_tail,
    missing_clt_identity,
    missing_terminator,
};

struct SourceLine final {
    std::size_t source_line{};
    std::string raw;
    std::string key;
    std::string value;
    bool comment{false};
};

struct Document final {
    std::size_t physical_size{};
    std::size_t text_size{};
    std::size_t zero_padding_size{};
    std::string embedded_name;
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

// Structural reader for the DMC3 CLT text-serialized cloth resource family.
// The initial contract intentionally preserves raw lines and does not assign
// runtime meaning to numeric/token values that are not yet EXE-bound.
class Reader final {
public:
    [[nodiscard]] static ParseResult parse(std::span<const std::byte> bytes);
    [[nodiscard]] static bool structurally_valid(
        std::span<const std::byte> bytes) noexcept;
};

} // namespace dmc::rengine::formats::clt
