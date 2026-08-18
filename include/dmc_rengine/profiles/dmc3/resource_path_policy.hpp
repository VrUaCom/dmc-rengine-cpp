#pragma once

#include <cstdint>
#include <string>
#include <string_view>

namespace dmc::rengine::profiles::dmc3 {

class ResourcePathPolicy final {
public:
    // Recovered DMC3 provider normalizer profiles.
    static constexpr std::uint32_t physical_flags = 0x0CU;
    static constexpr std::uint32_t archive_flags = 0x0EU;

    [[nodiscard]] static bool valid_input(std::string_view path) noexcept;
    [[nodiscard]] static std::string physical(std::string_view path);
    [[nodiscard]] static std::string archive(std::string_view path);
};

} // namespace dmc::rengine::profiles::dmc3
