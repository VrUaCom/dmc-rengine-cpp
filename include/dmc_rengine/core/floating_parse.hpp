#pragma once

#include <charconv>
#include <concepts>
#include <string_view>
#include <system_error>
#include <version>

#if !(defined(__cpp_lib_to_chars) && __cpp_lib_to_chars >= 201611L)
#include <cmath>
#include <ios>
#include <locale>
#include <sstream>
#include <string>
#endif

namespace dmc::rengine::core {

// Parses the whole of `text` as a decimal floating-point number in
// std::chars_format::general syntax. Returns false on any syntax error,
// trailing characters or out-of-range value; `value` is then unspecified.
//
// Uses std::from_chars where the standard library provides the floating-point
// overloads (MSVC, libstdc++). Apple's libc++ does not, so there the text is
// parsed through a classic-locale stream after rejecting the inputs that
// from_chars would reject but a stream would accept (leading whitespace,
// a leading '+', hexadecimal prefixes).
template <std::floating_point T>
[[nodiscard]] inline bool parse_floating(std::string_view text, T& value) noexcept {
    if (text.empty()) return false;
#if defined(__cpp_lib_to_chars) && __cpp_lib_to_chars >= 201611L
    const auto* last = text.data() + text.size();
    const auto result =
        std::from_chars(text.data(), last, value, std::chars_format::general);
    return result.ec == std::errc{} && result.ptr == last;
#else
    const char first = text.front();
    if (first == '+' || first == ' ' || first == '\t' || first == '\n' ||
        first == '\r' || first == '\f' || first == '\v') {
        return false;
    }
    if (text.find_first_of("xXpP") != std::string_view::npos) return false;
    try {
        std::istringstream stream{std::string{text}};
        stream.imbue(std::locale::classic());
        T parsed{};
        stream >> std::noskipws >> parsed;
        if (stream.fail() ||
            stream.peek() != std::char_traits<char>::eof()) {
            return false;
        }
        value = parsed;
        return true;
    } catch (...) {
        return false;
    }
#endif
}

}  // namespace dmc::rengine::core
