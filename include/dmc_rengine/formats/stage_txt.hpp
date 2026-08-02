#pragma once

#include "dmc_rengine/formats/diagnostic.hpp"

#include <cstddef>
#include <cstdint>
#include <span>
#include <string>
#include <string_view>
#include <vector>

namespace dmc::rengine::formats::stage_txt {

enum class TokenKind {
    directive_set,
    door,
    box_in,
    next_room,
    stage_set_value,
    identifier,
    number,
    string_literal,
    symbol,
};

[[nodiscard]] constexpr std::string_view to_string(
    TokenKind kind) noexcept {
    switch (kind) {
    case TokenKind::directive_set: return "directive-set";
    case TokenKind::door: return "door";
    case TokenKind::box_in: return "box-in";
    case TokenKind::next_room: return "next-room";
    case TokenKind::stage_set_value: return "stage-set-value";
    case TokenKind::identifier: return "identifier";
    case TokenKind::number: return "number";
    case TokenKind::string_literal: return "string-literal";
    case TokenKind::symbol: return "symbol";
    }
    return "identifier";
}

struct Token final {
    TokenKind kind{TokenKind::identifier};
    std::string value;
    std::uint64_t offset{};
    std::uint64_t size{};
    std::uint32_t line{1U};
    std::uint32_t column{1U};

    [[nodiscard]] bool valid() const noexcept {
        return !value.empty() && size != 0U;
    }
};

struct LexResult final {
    bool recognized{false};
    std::vector<Token> tokens;
    std::vector<ParseDiagnostic> diagnostics;

    [[nodiscard]] bool ok() const noexcept;
    [[nodiscard]] std::vector<const Token*> by_kind(TokenKind kind) const;
};

class Lexer final {
public:
    [[nodiscard]] static LexResult scan(std::span<const std::byte> bytes);
};

} // namespace dmc::rengine::formats::stage_txt
