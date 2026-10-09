#include "dmc_rengine/spider/python_json.hpp"

#include <array>
#include <charconv>
#include <cmath>
#include <cstdio>
#include <optional>
#include <type_traits>

namespace dmc::rengine::spider {
namespace {

/// The next code point of UTF-8 `text` at `index`; an invalid byte stands for itself.
[[nodiscard]] std::uint32_t next_code_point(std::string_view text, std::size_t& index) noexcept {
    const auto lead = static_cast<unsigned char>(text[index]);
    const auto continuation = [&text](std::size_t at) {
        return at < text.size() && (static_cast<unsigned char>(text[at]) & 0xC0U) == 0x80U;
    };
    const auto bits = [&text](std::size_t at) { return static_cast<std::uint32_t>(text[at]) & 0x3FU; };
    if (lead >= 0xF0U && lead < 0xF8U && continuation(index + 1U) && continuation(index + 2U) &&
        continuation(index + 3U)) {
        const auto value = ((lead & 0x07U) << 18U) | (bits(index + 1U) << 12U) | (bits(index + 2U) << 6U) |
            bits(index + 3U);
        index += 4U;
        return value;
    }
    if (lead >= 0xE0U && lead < 0xF0U && continuation(index + 1U) && continuation(index + 2U)) {
        const auto value = ((lead & 0x0FU) << 12U) | (bits(index + 1U) << 6U) | bits(index + 2U);
        index += 3U;
        return value;
    }
    if (lead >= 0xC0U && lead < 0xE0U && continuation(index + 1U)) {
        const auto value = ((lead & 0x1FU) << 6U) | bits(index + 1U);
        index += 2U;
        return value;
    }
    ++index;
    return lead;
}

void append_u_escape(std::string& output, std::uint32_t unit) {
    std::array<char, 8> buffer{};
    std::snprintf(buffer.data(), buffer.size(), "\\u%04x", static_cast<unsigned>(unit));
    output += buffer.data();
}

void dump_string(std::string& output, std::string_view text) {
    output.push_back('"');
    for (std::size_t index = 0U; index < text.size();) {
        const auto code_point = next_code_point(text, index);
        switch (code_point) {
        case '"': output += "\\\""; continue;
        case '\\': output += "\\\\"; continue;
        case '\n': output += "\\n"; continue;
        case '\r': output += "\\r"; continue;
        case '\t': output += "\\t"; continue;
        case '\b': output += "\\b"; continue;
        case '\f': output += "\\f"; continue;
        default: break;
        }
        // ensure_ascii writes only space..tilde as itself.
        if (code_point < 0x20U || code_point > 0x7EU) {
            if (code_point >= 0x10000U) {
                const auto value = code_point - 0x10000U;
                append_u_escape(output, 0xD800U | (value >> 10U));
                append_u_escape(output, 0xDC00U | (value & 0x3FFU));
            } else {
                append_u_escape(output, code_point);
            }
        } else {
            output.push_back(static_cast<char>(code_point));
        }
    }
    output.push_back('"');
}

void dump_double(std::string& output, double value) {
    if (std::isnan(value)) { output += "NaN"; return; }
    if (std::isinf(value)) { output += value < 0.0 ? "-Infinity" : "Infinity"; return; }
    // repr(): the shortest digits that round-trip, positional between 1e-4 and 1e16.
    std::array<char, 64> buffer{};
    const auto magnitude = std::fabs(value);
    const bool positional = magnitude == 0.0 || (magnitude >= 1e-4 && magnitude < 1e16);
    const auto result = std::to_chars(buffer.data(), buffer.data() + buffer.size(), value,
                                      positional ? std::chars_format::fixed : std::chars_format::scientific);
    std::string text{buffer.data(), result.ptr};
    if (positional) {
        if (text.find('.') == std::string::npos) text += ".0";
    } else {
        // to_chars writes 1e+16 and 1.5e-05; Python writes 1e+16 and 1.5e-05 too.
        const auto e = text.find('e');
        if (e != std::string::npos && text[e + 1U] != '-' && text[e + 1U] != '+') text.insert(e + 1U, "+");
    }
    output += text;
}

void dump(std::string& output, const OrderedJson& value, int indent, int depth) {
    const auto newline = [&output, indent](int level) {
        output.push_back('\n');
        output.append(static_cast<std::size_t>(indent * level), ' ');
    };
    std::visit(
        [&](const auto& held) {
            using Held = std::decay_t<decltype(held)>;
            if constexpr (std::is_same_v<Held, std::nullptr_t>) {
                output += "null";
            } else if constexpr (std::is_same_v<Held, bool>) {
                output += held ? "true" : "false";
            } else if constexpr (std::is_same_v<Held, std::int64_t> || std::is_same_v<Held, std::uint64_t>) {
                output += std::to_string(held);
            } else if constexpr (std::is_same_v<Held, double>) {
                dump_double(output, held);
            } else if constexpr (std::is_same_v<Held, std::string>) {
                dump_string(output, held);
            } else if constexpr (std::is_same_v<Held, OrderedJson::Array>) {
                if (held.empty()) { output += "[]"; return; }
                output.push_back('[');
                for (std::size_t index = 0U; index < held.size(); ++index) {
                    if (index != 0U) output.push_back(',');
                    newline(depth + 1);
                    dump(output, held[index], indent, depth + 1);
                }
                newline(depth);
                output.push_back(']');
            } else {
                if (held.empty()) { output += "{}"; return; }
                output.push_back('{');
                for (std::size_t index = 0U; index < held.size(); ++index) {
                    if (index != 0U) output.push_back(',');
                    newline(depth + 1);
                    dump_string(output, held[index].first);
                    output += ": ";
                    dump(output, held[index].second, indent, depth + 1);
                }
                newline(depth);
                output.push_back('}');
            }
        },
        value.data);
}

/// A number as a comparable pair: exact integers first, doubles otherwise.
struct Number final {
    bool integral{};
    bool negative{};
    std::uint64_t magnitude{};
    double real{};
};

[[nodiscard]] std::optional<Number> number_of(const core::json::Value& value) noexcept {
    if (const auto* held = value.as_i64()) {
        return Number{.integral = true, .negative = *held < 0,
                      .magnitude = *held < 0 ? 0U - static_cast<std::uint64_t>(*held) : static_cast<std::uint64_t>(*held),
                      .real = static_cast<double>(*held)};
    }
    if (const auto* held = value.as_u64()) {
        return Number{.integral = true, .negative = false, .magnitude = *held, .real = static_cast<double>(*held)};
    }
    if (const auto* held = value.as_double()) return Number{.integral = false, .real = *held};
    return std::nullopt;
}

} // namespace

OrderedJson& OrderedJson::set(std::string key, OrderedJson value) {
    if (!std::holds_alternative<Members>(data)) data = Members{};
    std::get<Members>(data).emplace_back(std::move(key), std::move(value));
    return *this;
}

OrderedJson& OrderedJson::push(OrderedJson value) {
    if (!std::holds_alternative<Array>(data)) data = Array{};
    std::get<Array>(data).push_back(std::move(value));
    return *this;
}

OrderedJson to_ordered(const core::json::Value& value) {
    if (value.is_null()) return OrderedJson{nullptr};
    if (const auto* held = value.as_bool()) return OrderedJson{*held};
    if (const auto* held = value.as_i64()) return OrderedJson{*held};
    if (const auto* held = value.as_u64()) return OrderedJson{*held};
    if (const auto* held = value.as_double()) return OrderedJson{*held};
    if (const auto* held = value.as_string()) return OrderedJson{*held};
    if (const auto* held = value.as_array()) {
        auto result = OrderedJson::array();
        for (const auto& item : *held) result.push(to_ordered(item));
        return result;
    }
    auto result = OrderedJson::object();
    if (const auto* held = value.as_object()) {
        for (const auto& [key, item] : *held) result.set(key, to_ordered(item));
    }
    return result;
}

std::string dump_python(const OrderedJson& value, int indent) {
    std::string output;
    dump(output, value, indent, 0);
    return output;
}

bool json_equal(const core::json::Value& left, const core::json::Value& right) {
    const auto left_number = number_of(left);
    const auto right_number = number_of(right);
    if (left_number.has_value() || right_number.has_value()) {
        if (!left_number.has_value() || !right_number.has_value()) return false;
        if (left_number->integral && right_number->integral) {
            return left_number->negative == right_number->negative &&
                left_number->magnitude == right_number->magnitude;
        }
        return left_number->real == right_number->real;
    }
    if (left.is_null() || right.is_null()) return left.is_null() && right.is_null();
    if (const auto* held = left.as_bool()) return right.as_bool() != nullptr && *held == *right.as_bool();
    if (const auto* held = left.as_string()) return right.as_string() != nullptr && *held == *right.as_string();
    if (const auto* held = left.as_array()) {
        const auto* other = right.as_array();
        if (other == nullptr || other->size() != held->size()) return false;
        for (std::size_t index = 0U; index < held->size(); ++index) {
            if (!json_equal((*held)[index], (*other)[index])) return false;
        }
        return true;
    }
    const auto* held = left.as_object();
    const auto* other = right.as_object();
    if (held == nullptr || other == nullptr || held->size() != other->size()) return false;
    for (const auto& [key, item] : *held) {
        const auto found = other->find(key);
        if (found == other->end() || !json_equal(item, found->second)) return false;
    }
    return true;
}

bool contains_key(const core::json::Value& value, std::string_view key) {
    if (const auto* object = value.as_object()) {
        if (object->find(key) != object->end()) return true;
        for (const auto& [name, item] : *object) {
            if (contains_key(item, key)) return true;
        }
        return false;
    }
    if (const auto* array = value.as_array()) {
        for (const auto& item : *array) {
            if (contains_key(item, key)) return true;
        }
    }
    return false;
}

bool is_integer(const core::json::Value& value) noexcept {
    return value.as_i64() != nullptr || value.as_u64() != nullptr;
}

std::size_t python_length(std::string_view text) noexcept {
    std::size_t count = 0U;
    for (std::size_t index = 0U; index < text.size(); ++count) next_code_point(text, index);
    return count;
}

} // namespace dmc::rengine::spider
