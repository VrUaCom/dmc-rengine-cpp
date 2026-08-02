#pragma once

#include <string_view>

namespace dmc::rengine {

[[nodiscard]] constexpr std::string_view version() noexcept {
    return "0.2.0";
}

[[nodiscard]] constexpr std::string_view architecture_name() noexcept {
    return "Evidence-backed GDSpaces-first C++ foundation";
}

} // namespace dmc::rengine
