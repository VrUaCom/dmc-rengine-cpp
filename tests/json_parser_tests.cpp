#include "dmc_rengine/core/json.hpp"

#include <cassert>
#include <cstdint>
#include <string>

int main() {
    using dmc::rengine::core::json::ParseLimits;
    using dmc::rengine::core::json::Parser;

    const auto parsed = Parser::parse(
        R"({"text":"line\n\u0410\uD83D\uDE80","u":18446744073709551615,"i":-9223372036854775808,"f":1.25e2,"a":[true,false,null]})");
    assert(parsed.ok());

    const auto* object = parsed.value->as_object();
    assert(object != nullptr);
    assert(object->at("u").as_u64() != nullptr);
    assert(*object->at("u").as_u64() ==
        static_cast<std::uint64_t>(18446744073709551615ULL));
    assert(object->at("i").as_i64() != nullptr);
    assert(*object->at("i").as_i64() ==
        static_cast<std::int64_t>(-9223372036854775807LL - 1LL));
    assert(object->at("f").as_double() != nullptr);
    assert(*object->at("f").as_double() == 125.0);
    assert(object->at("a").as_array()->size() == 3U);
    assert(object->at("a").as_array()->at(2).is_null());
    assert(*object->at("text").as_string() ==
        std::string{"line\n"} + "\xD0\x90\xF0\x9F\x9A\x80");

    assert(!Parser::parse(R"({"x":1,"x":2})").ok());
    assert(!Parser::parse("[1,]").ok());
    assert(!Parser::parse("01").ok());
    assert(!Parser::parse("1 trailing").ok());
    assert(!Parser::parse(R"("\uD800")").ok());
    assert(!Parser::parse("18446744073709551616").ok());
    assert(!Parser::parse("-9223372036854775809").ok());

    ParseLimits tiny;
    tiny.max_input_bytes = 2U;
    assert(!Parser::parse("null", tiny).ok());

    ParseLimits shallow;
    shallow.max_depth = 1U;
    assert(!Parser::parse("[[0]]", shallow).ok());

    ParseLimits one_item;
    one_item.max_array_items = 1U;
    assert(!Parser::parse("[1,2]", one_item).ok());

    ParseLimits short_string;
    short_string.max_string_bytes = 3U;
    assert(!Parser::parse(R"("four")", short_string).ok());

    return 0;
}
