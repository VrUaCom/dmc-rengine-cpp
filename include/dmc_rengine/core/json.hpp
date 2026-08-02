#pragma once

#include <cstddef>
#include <cstdint>
#include <map>
#include <optional>
#include <string>
#include <string_view>
#include <variant>
#include <vector>

namespace dmc::rengine::core::json {

struct Value final {
    using Array = std::vector<Value>;
    using Object = std::map<std::string, Value, std::less<>>;
    using Storage = std::variant<
        std::nullptr_t,
        bool,
        std::int64_t,
        std::uint64_t,
        double,
        std::string,
        Array,
        Object>;

    Storage data{nullptr};

    [[nodiscard]] bool is_null() const noexcept;
    [[nodiscard]] const bool* as_bool() const noexcept;
    [[nodiscard]] const std::int64_t* as_i64() const noexcept;
    [[nodiscard]] const std::uint64_t* as_u64() const noexcept;
    [[nodiscard]] const double* as_double() const noexcept;
    [[nodiscard]] const std::string* as_string() const noexcept;
    [[nodiscard]] const Array* as_array() const noexcept;
    [[nodiscard]] const Object* as_object() const noexcept;
};

struct ParseLimits final {
    std::size_t max_input_bytes{8U * 1024U * 1024U};
    std::size_t max_depth{64U};
    std::size_t max_values{250000U};
    std::size_t max_array_items{100000U};
    std::size_t max_object_members{100000U};
    std::size_t max_string_bytes{1024U * 1024U};
};

struct ParseError final {
    std::size_t offset{};
    std::string message;
};

struct ParseResult final {
    std::optional<Value> value;
    std::vector<ParseError> errors;

    [[nodiscard]] bool ok() const noexcept {
        return value.has_value() && errors.empty();
    }
};

class Parser final {
public:
    [[nodiscard]] static ParseResult parse(
        std::string_view input,
        ParseLimits limits = {});
};

} // namespace dmc::rengine::core::json
