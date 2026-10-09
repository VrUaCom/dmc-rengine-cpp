#pragma once

#include "dmc_rengine/core/json.hpp"

#include <cstdint>
#include <string>
#include <string_view>
#include <utility>
#include <variant>
#include <vector>

/**
 * Tarantula's JSON: the output Python scripts produced, byte for byte.
 *
 * Every script on the Tarantula queue ends in `json.dumps(value, indent=2)`,
 * and what it wrote is hashed, bound and compared downstream. A port is only
 * a replacement if it writes the same bytes, so this keeps object members in
 * the order they were set (Python dicts keep insertion order) and dumps them
 * the way CPython does: two-space indent, ", " / ": " separators, non-ASCII
 * as \uXXXX escapes (surrogate pairs past the BMP), and floats in their
 * shortest round-trip form.
 */
namespace dmc::rengine::spider {

struct OrderedJson final {
    using Array = std::vector<OrderedJson>;
    using Members = std::vector<std::pair<std::string, OrderedJson>>;
    using Storage = std::variant<
        std::nullptr_t, bool, std::int64_t, std::uint64_t, double, std::string, Array, Members>;

    Storage data{nullptr};

    OrderedJson() = default;
    OrderedJson(std::nullptr_t) {}
    OrderedJson(bool value) : data{value} {}
    OrderedJson(int value) : data{static_cast<std::int64_t>(value)} {}
    OrderedJson(std::int64_t value) : data{value} {}
    OrderedJson(std::uint64_t value) : data{value} {}
    OrderedJson(double value) : data{value} {}
    OrderedJson(const char* value) : data{std::string{value}} {}
    OrderedJson(std::string_view value) : data{std::string{value}} {}
    OrderedJson(std::string value) : data{std::move(value)} {}
    OrderedJson(Array value) : data{std::move(value)} {}

    [[nodiscard]] static OrderedJson object() { OrderedJson value; value.data = Members{}; return value; }
    [[nodiscard]] static OrderedJson array() { OrderedJson value; value.data = Array{}; return value; }

    /// Appends a member to an object; the order members are set is the order they are written.
    OrderedJson& set(std::string key, OrderedJson value);
    /// Appends an item to an array.
    OrderedJson& push(OrderedJson value);
};

/// A parsed value as ordered JSON. core::json keeps object members sorted, so they come out sorted.
[[nodiscard]] OrderedJson to_ordered(const core::json::Value& value);

/// `json.dumps(value, indent=indent)` as CPython writes it (ensure_ascii on).
[[nodiscard]] std::string dump_python(const OrderedJson& value, int indent = 2);

/// Structural equality, numbers by value and bools apart from numbers.
[[nodiscard]] bool json_equal(const core::json::Value& left, const core::json::Value& right);

/// Whether `key` names a member of any object anywhere inside `value`.
[[nodiscard]] bool contains_key(const core::json::Value& value, std::string_view key);

/// A JSON integer that is not a bool, as Python's `isinstance(x, int)` without bool.
[[nodiscard]] bool is_integer(const core::json::Value& value) noexcept;

/// How many characters Python's `len()` counts in UTF-8 `text`.
[[nodiscard]] std::size_t python_length(std::string_view text) noexcept;

} // namespace dmc::rengine::spider
