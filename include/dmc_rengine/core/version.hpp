#pragma once

#include <string_view>

namespace dmc::rengine {

[[nodiscard]] constexpr std::string_view version() noexcept {
    return "0.1.0";
}

[[nodiscard]] constexpr std::string_view architecture_name() noexcept {
    return "GDSpaces-first C++ foundation";
}

} // namespace dmc::rengine
