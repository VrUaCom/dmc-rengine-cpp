#include "dmc_rengine/core/json.hpp"

#include <cassert>
#include <charconv>
#include <cmath>
#include <locale>
#include <sstream>
#include <string>
#include <string_view>
#include <vector>

namespace {

// A locale whose decimal separator is ',' — the shape of locale that silently
// truncates a JSON fraction at the separator when a converter honours the
// global locale (strtod, an un-imbued stream) instead of ignoring it.
class CommaDecimalPoint final : public std::numpunct<char> {
protected:
    char do_decimal_point() const override { return ','; }
    char do_thousands_sep() const override { return '.'; }
    std::string do_grouping() const override { return "\3"; }
};

// The exact conversion the Android build compiles: libc++ ships the integral
// std::from_chars overloads long before the floating-point ones, so
// src/core/json.cpp falls back to a classic-locale stream there.
[[nodiscard]] bool stream_token_to_double(std::string_view token, double& value) {
    std::istringstream stream{std::string{token}};
    stream.imbue(std::locale::classic());
    stream >> value;
    return !stream.fail() && stream.eof();
}

[[nodiscard]] std::vector<std::string> number_tokens() {
    return {
        "0.5",
        "-0.5",
        "1.25e2",
        "1.25E2",
        "1.25e+2",
        "1.25e-2",
        "-1.25e-2",
        "0.0",
        "-0.0",
        "3.141592653589793",
        "2.2250738585072014e-308",
        "1.7976931348623157e308",
        "1234.5678",
        "0.000001",
        "9007199254740993.0",
    };
}

void fallback_matches_from_chars() {
#if defined(__cpp_lib_to_chars)
    // Pins the Android path to the host path: every token must convert to the
    // bit-identical double under both implementations. Without this, a
    // divergence would only ever show up on a device.
    for (const auto& token : number_tokens()) {
        double reference = 0.0;
        const auto* const first = token.data();
        const auto* const last = token.data() + token.size();
        const auto conversion =
            std::from_chars(first, last, reference, std::chars_format::general);
        assert(conversion.ec == std::errc{});
        assert(conversion.ptr == last);

        double fallback = 0.0;
        assert(stream_token_to_double(token, fallback));
        assert(fallback == reference);
    }
#endif
}

void fallback_rejects_what_the_grammar_already_rejects() {
    // parse_number never emits these, but the fallback must not widen the
    // accepted set if it ever saw one: a stream would otherwise take "inf",
    // a '+' sign or trailing text that from_chars refuses.
    double value = 0.0;
    assert(!stream_token_to_double("inf", value));
    assert(!stream_token_to_double("nan", value));
    assert(!stream_token_to_double("1.5x", value));
    assert(!stream_token_to_double("", value));
}

void parser_ignores_a_hostile_global_locale() {
    const auto parsed = dmc::rengine::core::json::Parser::parse(
        R"({"half":0.5,"scaled":1.25e2,"negative":-0.125,"grouped":1234.5678})");
    assert(parsed.ok());

    const auto* object = parsed.value->as_object();
    assert(object != nullptr);
    assert(*object->at("half").as_double() == 0.5);
    assert(*object->at("scaled").as_double() == 125.0);
    assert(*object->at("negative").as_double() == -0.125);
    assert(*object->at("grouped").as_double() == 1234.5678);
}

void parser_still_rejects_out_of_range_numbers() {
    assert(!dmc::rengine::core::json::Parser::parse(R"({"x":1e999})").ok());
}

} // namespace

int main() {
    // Every check runs with the hostile locale installed globally. Running the
    // cross-check under the classic locale instead would pass even for a
    // fallback that forgot to imbue, which is the one mistake it exists to
    // catch.
    const auto previous = std::locale::global(
        std::locale(std::locale::classic(), new CommaDecimalPoint));

    fallback_matches_from_chars();
    fallback_rejects_what_the_grammar_already_rejects();
    parser_ignores_a_hostile_global_locale();
    parser_still_rejects_out_of_range_numbers();

    std::locale::global(previous);
    return 0;
}
