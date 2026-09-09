#pragma once

#include <cstdint>

namespace dmc::rengine::spider {

// Spider is one architecture with three deliberately separated roles.
// The family tag is descriptive metadata; it must not be used to hide
// format logic, duplicate parsers, or introduce a scripting runtime.
enum class Family : std::uint8_t {
    black_widow,
    tarantula,
    crusader,
};

[[nodiscard]] constexpr const char* to_string(Family family) noexcept {
    switch (family) {
    case Family::black_widow: return "black-widow";
    case Family::tarantula: return "tarantula";
    case Family::crusader: return "crusader";
    }
    return "unknown";
}

} // namespace dmc::rengine::spider
