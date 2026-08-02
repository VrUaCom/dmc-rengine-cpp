#pragma once

#include <cassert>
#include <cstdlib>
#include <iostream>
#include <string_view>

namespace dmc::rengine::test {

inline void require(
    bool condition,
    std::string_view expression,
    std::string_view file,
    int line) {
    if (condition) {
        return;
    }

    std::cerr << file << ':' << line
              << ": requirement failed: " << expression << '\n';
    std::abort();
}

} // namespace dmc::rengine::test

#define DMC_REQUIRE(expression)                                                   \
    ::dmc::rengine::test::require(                                               \
        static_cast<bool>(expression), #expression, __FILE__, __LINE__)

#ifdef assert
#undef assert
#endif
#define assert(expression) DMC_REQUIRE(expression)
