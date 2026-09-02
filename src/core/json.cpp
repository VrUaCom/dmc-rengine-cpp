#include "dmc_rengine/core/json.hpp"

#include <charconv>
#include <cmath>
#include <limits>
#include <locale>
#include <sstream>
#include <string>
#include <utility>

namespace dmc::rengine::core::json {
namespace {

// Converts an already grammar-validated JSON number token to a double.
//
// libc++ ships the integral std::from_chars overloads long before the
// floating-point ones, so the Android NDK has no from_chars(double). The
// fallback deliberately uses a classic-locale stream rather than strtod:
// strtod honours the global C locale, and one whose decimal separator is not
// '.' would silently truncate a JSON fraction at the separator.
//
// The two paths agree because parse_number has already rejected everything
// they would disagree on — leading whitespace, a '+' sign, and "inf"/"nan" —
// and full consumption is required either way.
[[nodiscard]] bool token_to_double(std::string_view token, double& value) {
#if defined(__cpp_lib_to_chars)
    const auto* const first = token.data();
    const auto* const last = token.data() + token.size();
    const auto conversion =
        std::from_chars(first, last, value, std::chars_format::general);
    return conversion.ec == std::errc{} && conversion.ptr == last;
#else
    std::istringstream stream{std::string{token}};
    stream.imbue(std::locale::classic());
    stream >> value;
    return !stream.fail() && stream.eof();
#endif
}

class ParserState final {
public:
    ParserState(std::string_view input, ParseLimits limits)
        : input_(input), limits_(limits) {}

    [[nodiscard]] ParseResult run() {
        ParseResult result;
        if (input_.size() > limits_.max_input_bytes) {
            result.errors.push_back(ParseError{
                .offset = 0,
                .message = "JSON input exceeds the configured byte limit.",
            });
            return result;
        }

        skip_whitespace();
        auto value = parse_value(0U);
        if (!value.has_value()) {
            result.errors = std::move(errors_);
            return result;
        }

        skip_whitespace();
        if (position_ != input_.size()) {
            fail("Unexpected trailing content after the JSON value.");
            result.errors = std::move(errors_);
            return result;
        }

        result.value = std::move(*value);
        return result;
    }

private:
    [[nodiscard]] std::optional<Value> parse_value(std::size_t depth) {
        if (depth > limits_.max_depth) {
            fail("JSON nesting exceeds the configured depth limit.");
            return std::nullopt;
        }
        if (value_count_ >= limits_.max_values) {
            fail("JSON value count exceeds the configured limit.");
            return std::nullopt;
        }
        ++value_count_;

        skip_whitespace();
        if (position_ >= input_.size()) {
            fail("Unexpected end of JSON input.");
            return std::nullopt;
        }

        const auto character = input_[position_];
        if (character == 'n') {
            return parse_literal("null", Value{.data = nullptr});
        }
        if (character == 't') {
            return parse_literal("true", Value{.data = true});
        }
        if (character == 'f') {
            return parse_literal("false", Value{.data = false});
        }
        if (character == '"') {
            auto text = parse_string();
            if (!text.has_value()) {
                return std::nullopt;
            }
            return Value{.data = std::move(*text)};
        }
        if (character == '[') {
            return parse_array(depth);
        }
        if (character == '{') {
            return parse_object(depth);
        }
        if (character == '-' || is_digit(character)) {
            return parse_number();
        }

        fail("Unexpected character while parsing a JSON value.");
        return std::nullopt;
    }

    [[nodiscard]] std::optional<Value> parse_literal(
        std::string_view literal,
        Value value) {
        if (input_.substr(position_, literal.size()) != literal) {
            fail("Invalid JSON literal.");
            return std::nullopt;
        }
        position_ += literal.size();
        return value;
    }

    [[nodiscard]] std::optional<Value> parse_array(std::size_t depth) {
        ++position_;
        Value::Array values;
        skip_whitespace();
        if (consume(']')) {
            return Value{.data = std::move(values)};
        }

        while (true) {
            if (values.size() >= limits_.max_array_items) {
                fail("JSON array item count exceeds the configured limit.");
                return std::nullopt;
            }

            auto value = parse_value(depth + 1U);
            if (!value.has_value()) {
                return std::nullopt;
            }
            values.push_back(std::move(*value));

            skip_whitespace();
            if (consume(']')) {
                return Value{.data = std::move(values)};
            }
            if (!consume(',')) {
                fail("Expected ',' or ']' after a JSON array item.");
                return std::nullopt;
            }
            skip_whitespace();
        }
    }

    [[nodiscard]] std::optional<Value> parse_object(std::size_t depth) {
        ++position_;
        Value::Object object;
        skip_whitespace();
        if (consume('}')) {
            return Value{.data = std::move(object)};
        }

        while (true) {
            if (object.size() >= limits_.max_object_members) {
                fail("JSON object member count exceeds the configured limit.");
                return std::nullopt;
            }
            if (position_ >= input_.size() || input_[position_] != '"') {
                fail("Expected a JSON object member name.");
                return std::nullopt;
            }

            auto key = parse_string();
            if (!key.has_value()) {
                return std::nullopt;
            }

            skip_whitespace();
            if (!consume(':')) {
                fail("Expected ':' after a JSON object member name.");
                return std::nullopt;
            }

            auto value = parse_value(depth + 1U);
            if (!value.has_value()) {
                return std::nullopt;
            }

            const auto inserted = object.emplace(
                std::move(*key), std::move(*value));
            if (!inserted.second) {
                fail("Duplicate JSON object member name.");
                return std::nullopt;
            }

            skip_whitespace();
            if (consume('}')) {
                return Value{.data = std::move(object)};
            }
            if (!consume(',')) {
                fail("Expected ',' or '}' after a JSON object member.");
                return std::nullopt;
            }
            skip_whitespace();
        }
    }

    [[nodiscard]] std::optional<std::string> parse_string() {
        if (!consume('"')) {
            fail("Expected a JSON string.");
            return std::nullopt;
        }

        std::string output;
        while (position_ < input_.size()) {
            const auto character = static_cast<unsigned char>(input_[position_++]);
            if (character == '"') {
                return output;
            }
            if (character < 0x20U) {
                fail("Unescaped control character in JSON string.");
                return std::nullopt;
            }
            if (character != '\\') {
                if (!append_byte(output, static_cast<char>(character))) {
                    return std::nullopt;
                }
                continue;
            }

            if (position_ >= input_.size()) {
                fail("Truncated JSON string escape.");
                return std::nullopt;
            }

            const auto escape = input_[position_++];
            switch (escape) {
            case '"':
            case '\\':
            case '/':
                if (!append_byte(output, escape)) {
                    return std::nullopt;
                }
                break;
            case 'b':
                if (!append_byte(output, '\b')) {
                    return std::nullopt;
                }
                break;
            case 'f':
                if (!append_byte(output, '\f')) {
                    return std::nullopt;
                }
                break;
            case 'n':
                if (!append_byte(output, '\n')) {
                    return std::nullopt;
                }
                break;
            case 'r':
                if (!append_byte(output, '\r')) {
                    return std::nullopt;
                }
                break;
            case 't':
                if (!append_byte(output, '\t')) {
                    return std::nullopt;
                }
                break;
            case 'u': {
                auto code_point = parse_unicode_escape();
                if (!code_point.has_value() || !append_utf8(output, *code_point)) {
                    return std::nullopt;
                }
                break;
            }
            default:
                fail("Invalid JSON string escape.");
                return std::nullopt;
            }
        }

        fail("Unterminated JSON string.");
        return std::nullopt;
    }

    [[nodiscard]] std::optional<std::uint32_t> parse_unicode_escape() {
        auto first = parse_hex_quad();
        if (!first.has_value()) {
            return std::nullopt;
        }

        if (*first >= 0xD800U && *first <= 0xDBFFU) {
            if (position_ + 2U > input_.size() ||
                input_[position_] != '\\' || input_[position_ + 1U] != 'u') {
                fail("A high surrogate must be followed by a low surrogate.");
                return std::nullopt;
            }
            position_ += 2U;
            auto second = parse_hex_quad();
            if (!second.has_value() || *second < 0xDC00U || *second > 0xDFFFU) {
                fail("Invalid low surrogate in JSON string.");
                return std::nullopt;
            }

            return 0x10000U + ((*first - 0xD800U) << 10U) +
                (*second - 0xDC00U);
        }

        if (*first >= 0xDC00U && *first <= 0xDFFFU) {
            fail("Unexpected low surrogate in JSON string.");
            return std::nullopt;
        }

        return *first;
    }

    [[nodiscard]] std::optional<std::uint32_t> parse_hex_quad() {
        if (position_ > input_.size() || input_.size() - position_ < 4U) {
            fail("Truncated Unicode escape in JSON string.");
            return std::nullopt;
        }

        std::uint32_t value = 0U;
        for (std::size_t index = 0; index < 4U; ++index) {
            const auto digit = hex_value(input_[position_ + index]);
            if (!digit.has_value()) {
                fail("Invalid hexadecimal digit in JSON Unicode escape.");
                return std::nullopt;
            }
            value = (value << 4U) | *digit;
        }
        position_ += 4U;
        return value;
    }

    [[nodiscard]] std::optional<Value> parse_number() {
        const auto begin = position_;
        const auto negative = consume('-');

        if (position_ >= input_.size()) {
            fail("Truncated JSON number.");
            return std::nullopt;
        }

        if (input_[position_] == '0') {
            ++position_;
            if (position_ < input_.size() && is_digit(input_[position_])) {
                fail("Leading zero in JSON number.");
                return std::nullopt;
            }
        } else if (input_[position_] >= '1' && input_[position_] <= '9') {
            while (position_ < input_.size() && is_digit(input_[position_])) {
                ++position_;
            }
        } else {
            fail("Invalid JSON number integer part.");
            return std::nullopt;
        }

        auto floating = false;
        if (consume('.')) {
            floating = true;
            if (position_ >= input_.size() || !is_digit(input_[position_])) {
                fail("JSON number fraction requires at least one digit.");
                return std::nullopt;
            }
            while (position_ < input_.size() && is_digit(input_[position_])) {
                ++position_;
            }
        }

        if (position_ < input_.size() &&
            (input_[position_] == 'e' || input_[position_] == 'E')) {
            floating = true;
            ++position_;
            if (position_ < input_.size() &&
                (input_[position_] == '+' || input_[position_] == '-')) {
                ++position_;
            }
            if (position_ >= input_.size() || !is_digit(input_[position_])) {
                fail("JSON number exponent requires at least one digit.");
                return std::nullopt;
            }
            while (position_ < input_.size() && is_digit(input_[position_])) {
                ++position_;
            }
        }

        const auto token = input_.substr(begin, position_ - begin);
        if (floating) {
            double value = 0.0;
            if (!token_to_double(token, value) || !std::isfinite(value)) {
                fail("JSON floating-point number is outside the supported range.");
                return std::nullopt;
            }
            return Value{.data = value};
        }

        std::uint64_t magnitude = 0U;
        const auto digit_begin = negative ? 1U : 0U;
        for (std::size_t index = digit_begin; index < token.size(); ++index) {
            const auto digit = static_cast<std::uint64_t>(token[index] - '0');
            if (magnitude >
                (std::numeric_limits<std::uint64_t>::max() - digit) / 10U) {
                fail("JSON integer is outside the supported 64-bit range.");
                return std::nullopt;
            }
            magnitude = magnitude * 10U + digit;
        }

        if (!negative) {
            return Value{.data = magnitude};
        }

        constexpr auto negative_limit =
            static_cast<std::uint64_t>(std::numeric_limits<std::int64_t>::max()) + 1U;
        if (magnitude > negative_limit) {
            fail("Negative JSON integer is outside the supported 64-bit range.");
            return std::nullopt;
        }
        if (magnitude == negative_limit) {
            return Value{.data = std::numeric_limits<std::int64_t>::min()};
        }
        return Value{.data = -static_cast<std::int64_t>(magnitude)};
    }

    [[nodiscard]] bool append_byte(std::string& output, char byte) {
        if (output.size() >= limits_.max_string_bytes) {
            fail("JSON string exceeds the configured byte limit.");
            return false;
        }
        output.push_back(byte);
        return true;
    }

    [[nodiscard]] bool append_utf8(
        std::string& output,
        std::uint32_t code_point) {
        if (code_point > 0x10FFFFU) {
            fail("JSON Unicode code point is outside the valid range.");
            return false;
        }

        char encoded[4]{};
        std::size_t count = 0U;
        if (code_point <= 0x7FU) {
            encoded[0] = static_cast<char>(code_point);
            count = 1U;
        } else if (code_point <= 0x7FFU) {
            encoded[0] = static_cast<char>(0xC0U | (code_point >> 6U));
            encoded[1] = static_cast<char>(0x80U | (code_point & 0x3FU));
            count = 2U;
        } else if (code_point <= 0xFFFFU) {
            encoded[0] = static_cast<char>(0xE0U | (code_point >> 12U));
            encoded[1] = static_cast<char>(
                0x80U | ((code_point >> 6U) & 0x3FU));
            encoded[2] = static_cast<char>(0x80U | (code_point & 0x3FU));
            count = 3U;
        } else {
            encoded[0] = static_cast<char>(0xF0U | (code_point >> 18U));
            encoded[1] = static_cast<char>(
                0x80U | ((code_point >> 12U) & 0x3FU));
            encoded[2] = static_cast<char>(
                0x80U | ((code_point >> 6U) & 0x3FU));
            encoded[3] = static_cast<char>(0x80U | (code_point & 0x3FU));
            count = 4U;
        }

        if (count > limits_.max_string_bytes - output.size()) {
            fail("JSON string exceeds the configured byte limit.");
            return false;
        }
        output.append(encoded, count);
        return true;
    }

    void skip_whitespace() noexcept {
        while (position_ < input_.size()) {
            const auto character = input_[position_];
            if (character != ' ' && character != '\t' &&
                character != '\r' && character != '\n') {
                break;
            }
            ++position_;
        }
    }

    [[nodiscard]] bool consume(char expected) noexcept {
        if (position_ < input_.size() && input_[position_] == expected) {
            ++position_;
            return true;
        }
        return false;
    }

    void fail(std::string message) {
        if (errors_.empty()) {
            errors_.push_back(ParseError{
                .offset = position_,
                .message = std::move(message),
            });
        }
    }

    [[nodiscard]] static bool is_digit(char character) noexcept {
        return character >= '0' && character <= '9';
    }

    [[nodiscard]] static std::optional<std::uint32_t> hex_value(
        char character) noexcept {
        if (character >= '0' && character <= '9') {
            return static_cast<std::uint32_t>(character - '0');
        }
        if (character >= 'a' && character <= 'f') {
            return static_cast<std::uint32_t>(character - 'a' + 10);
        }
        if (character >= 'A' && character <= 'F') {
            return static_cast<std::uint32_t>(character - 'A' + 10);
        }
        return std::nullopt;
    }

    std::string_view input_;
    ParseLimits limits_;
    std::size_t position_{};
    std::size_t value_count_{};
    std::vector<ParseError> errors_;
};

} // namespace

bool Value::is_null() const noexcept {
    return std::holds_alternative<std::nullptr_t>(data);
}

const bool* Value::as_bool() const noexcept {
    return std::get_if<bool>(&data);
}

const std::int64_t* Value::as_i64() const noexcept {
    return std::get_if<std::int64_t>(&data);
}

const std::uint64_t* Value::as_u64() const noexcept {
    return std::get_if<std::uint64_t>(&data);
}

const double* Value::as_double() const noexcept {
    return std::get_if<double>(&data);
}

const std::string* Value::as_string() const noexcept {
    return std::get_if<std::string>(&data);
}

const Value::Array* Value::as_array() const noexcept {
    return std::get_if<Array>(&data);
}

const Value::Object* Value::as_object() const noexcept {
    return std::get_if<Object>(&data);
}

ParseResult Parser::parse(std::string_view input, ParseLimits limits) {
    return ParserState(input, limits).run();
}

} // namespace dmc::rengine::core::json
