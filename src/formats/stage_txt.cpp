#include "dmc_rengine/formats/stage_txt.hpp"

#include <algorithm>
#include <array>
#include <cctype>
#include <string>
#include <string_view>
#include <utility>

namespace dmc::rengine::formats::stage_txt {
namespace {

inline constexpr std::array stage_set_values{
    std::string_view{"DUMMY"},
    std::string_view{"STAY"},
    std::string_view{"BREAK"},
    std::string_view{"ORBREAK"},
    std::string_view{"SEAL"},
    std::string_view{"SWITCH"},
    std::string_view{"YURE"},
};

[[nodiscard]] char character(std::byte value) noexcept {
    return static_cast<char>(std::to_integer<unsigned char>(value));
}

[[nodiscard]] bool identifier_start(char value) noexcept {
    const auto byte = static_cast<unsigned char>(value);
    return std::isalpha(byte) != 0 || value == '_';
}

[[nodiscard]] bool identifier_continue(char value) noexcept {
    const auto byte = static_cast<unsigned char>(value);
    return std::isalnum(byte) != 0 || value == '_';
}

[[nodiscard]] std::string uppercase(std::string_view value) {
    std::string result(value);
    std::transform(
        result.begin(), result.end(), result.begin(),
        [](unsigned char item) {
            return static_cast<char>(std::toupper(item));
        });
    return result;
}

[[nodiscard]] TokenKind classify(std::string_view value) {
    const auto upper = uppercase(value);
    if (upper == "#SET") {
        return TokenKind::directive_set;
    }
    if (upper == "DOOR") {
        return TokenKind::door;
    }
    if (upper == "BOXIN") {
        return TokenKind::box_in;
    }
    if (upper == "NEXTROOM") {
        return TokenKind::next_room;
    }
    if (std::find(
            stage_set_values.begin(), stage_set_values.end(), upper) !=
        stage_set_values.end()) {
        return TokenKind::stage_set_value;
    }
    return TokenKind::identifier;
}

void add_diagnostic(
    LexResult& result,
    ParseSeverity severity,
    std::string code,
    std::string message,
    std::uint64_t offset) {
    result.diagnostics.push_back(ParseDiagnostic{
        .severity = severity,
        .code = std::move(code),
        .message = std::move(message),
        .offset = offset,
    });
}

void advance_position(char value, std::uint32_t& line, std::uint32_t& column) {
    if (value == '\n') {
        ++line;
        column = 1U;
    } else {
        ++column;
    }
}

} // namespace

bool LexResult::ok() const noexcept {
    return recognized && std::none_of(
        diagnostics.begin(), diagnostics.end(),
        [](const ParseDiagnostic& diagnostic) {
            return diagnostic.severity == ParseSeverity::error;
        });
}

std::vector<const Token*> LexResult::by_kind(TokenKind kind) const {
    std::vector<const Token*> result;
    for (const auto& token : tokens) {
        if (token.kind == kind) {
            result.push_back(&token);
        }
    }
    return result;
}

LexResult Lexer::scan(std::span<const std::byte> bytes) {
    LexResult result;
    if (bytes.empty()) {
        add_diagnostic(
            result,
            ParseSeverity::warning,
            "stage-txt.empty",
            "The candidate stage text resource is empty.",
            0U);
        return result;
    }
    if (std::any_of(bytes.begin(), bytes.end(), [](std::byte value) {
            return value == std::byte{0};
        })) {
        add_diagnostic(
            result,
            ParseSeverity::error,
            "stage-txt.binary-null",
            "The candidate stage text contains NUL bytes and is not treated as text.",
            0U);
        return result;
    }
    result.recognized = true;

    std::size_t offset = 0U;
    std::uint32_t line = 1U;
    std::uint32_t column = 1U;
    while (offset < bytes.size()) {
        const auto current = character(bytes[offset]);
        if (std::isspace(static_cast<unsigned char>(current)) != 0) {
            advance_position(current, line, column);
            ++offset;
            continue;
        }

        if (current == '/' && offset + 1U < bytes.size()) {
            const auto next = character(bytes[offset + 1U]);
            if (next == '/') {
                while (offset < bytes.size()) {
                    const auto value = character(bytes[offset++]);
                    advance_position(value, line, column);
                    if (value == '\n') {
                        break;
                    }
                }
                continue;
            }
            if (next == '*') {
                const auto start = offset;
                advance_position(current, line, column);
                advance_position(next, line, column);
                offset += 2U;
                bool closed = false;
                while (offset < bytes.size()) {
                    const auto value = character(bytes[offset]);
                    if (value == '*' && offset + 1U < bytes.size() &&
                        character(bytes[offset + 1U]) == '/') {
                        advance_position(value, line, column);
                        advance_position('/', line, column);
                        offset += 2U;
                        closed = true;
                        break;
                    }
                    advance_position(value, line, column);
                    ++offset;
                }
                if (!closed) {
                    add_diagnostic(
                        result,
                        ParseSeverity::error,
                        "stage-txt.unterminated-comment",
                        "A block comment reaches the end of the resource without */.",
                        start);
                    break;
                }
                continue;
            }
        }

        const auto token_offset = offset;
        const auto token_line = line;
        const auto token_column = column;

        if (current == '"') {
            std::string value;
            advance_position(current, line, column);
            ++offset;
            bool closed = false;
            while (offset < bytes.size()) {
                const auto item = character(bytes[offset]);
                if (item == '"') {
                    advance_position(item, line, column);
                    ++offset;
                    closed = true;
                    break;
                }
                if (item == '\\' && offset + 1U < bytes.size()) {
                    const auto escaped = character(bytes[offset + 1U]);
                    value.push_back(escaped);
                    advance_position(item, line, column);
                    advance_position(escaped, line, column);
                    offset += 2U;
                    continue;
                }
                if (item == '\n' || item == '\r') {
                    add_diagnostic(
                        result,
                        ParseSeverity::error,
                        "stage-txt.newline-in-string",
                        "A quoted string crosses a line boundary.",
                        token_offset);
                    break;
                }
                value.push_back(item);
                advance_position(item, line, column);
                ++offset;
            }
            if (!closed) {
                add_diagnostic(
                    result,
                    ParseSeverity::error,
                    "stage-txt.unterminated-string",
                    "A quoted string reaches the end of the resource without a closing quote.",
                    token_offset);
                break;
            }
            result.tokens.push_back(Token{
                .kind = TokenKind::string_literal,
                .value = std::move(value),
                .offset = token_offset,
                .size = offset - token_offset,
                .line = token_line,
                .column = token_column,
            });
            continue;
        }

        if (current == '#') {
            advance_position(current, line, column);
            ++offset;
            while (offset < bytes.size() &&
                   identifier_continue(character(bytes[offset]))) {
                advance_position(character(bytes[offset]), line, column);
                ++offset;
            }
            const auto value = std::string(
                reinterpret_cast<const char*>(bytes.data() + token_offset),
                offset - token_offset);
            result.tokens.push_back(Token{
                .kind = classify(value),
                .value = value,
                .offset = token_offset,
                .size = offset - token_offset,
                .line = token_line,
                .column = token_column,
            });
            continue;
        }

        if (identifier_start(current)) {
            advance_position(current, line, column);
            ++offset;
            while (offset < bytes.size() &&
                   identifier_continue(character(bytes[offset]))) {
                advance_position(character(bytes[offset]), line, column);
                ++offset;
            }
            const auto value = std::string(
                reinterpret_cast<const char*>(bytes.data() + token_offset),
                offset - token_offset);
            result.tokens.push_back(Token{
                .kind = classify(value),
                .value = value,
                .offset = token_offset,
                .size = offset - token_offset,
                .line = token_line,
                .column = token_column,
            });
            continue;
        }

        const auto next_is_digit = offset + 1U < bytes.size() &&
            std::isdigit(static_cast<unsigned char>(
                character(bytes[offset + 1U]))) != 0;
        const auto numeric_start =
            std::isdigit(static_cast<unsigned char>(current)) != 0 ||
            ((current == '+' || current == '-' || current == '.') &&
             next_is_digit);
        if (numeric_start) {
            bool exponent_seen = false;
            bool decimal_seen = false;
            advance_position(current, line, column);
            ++offset;
            while (offset < bytes.size()) {
                const auto item = character(bytes[offset]);
                if (std::isdigit(static_cast<unsigned char>(item)) != 0) {
                    advance_position(item, line, column);
                    ++offset;
                    continue;
                }
                if (item == '.' && !decimal_seen && !exponent_seen) {
                    decimal_seen = true;
                    advance_position(item, line, column);
                    ++offset;
                    continue;
                }
                if ((item == 'e' || item == 'E') && !exponent_seen) {
                    exponent_seen = true;
                    advance_position(item, line, column);
                    ++offset;
                    if (offset < bytes.size()) {
                        const auto sign = character(bytes[offset]);
                        if (sign == '+' || sign == '-') {
                            advance_position(sign, line, column);
                            ++offset;
                        }
                    }
                    continue;
                }
                break;
            }
            const auto value = std::string(
                reinterpret_cast<const char*>(bytes.data() + token_offset),
                offset - token_offset);
            result.tokens.push_back(Token{
                .kind = TokenKind::number,
                .value = value,
                .offset = token_offset,
                .size = offset - token_offset,
                .line = token_line,
                .column = token_column,
            });
            continue;
        }

        advance_position(current, line, column);
        ++offset;
        result.tokens.push_back(Token{
            .kind = TokenKind::symbol,
            .value = std::string(1U, current),
            .offset = token_offset,
            .size = 1U,
            .line = token_line,
            .column = token_column,
        });
    }
    return result;
}

} // namespace dmc::rengine::formats::stage_txt
