#pragma once

#include <cstdint>
#include <string>
#include <string_view>

namespace dmc::rengine::gdspaces {

enum class ResourcePathNormalizationFlag : std::uint32_t {
    uppercase_ascii = 0x01U,
    lowercase_ascii = 0x02U,
    strip_leading_separators = 0x04U,
    strip_trailing_separators = 0x08U,
};

[[nodiscard]] constexpr std::uint32_t flag_value(
    ResourcePathNormalizationFlag flag) noexcept {
    return static_cast<std::uint32_t>(flag);
}

class ResourcePathNormalizer final {
public:
    // Reproduces the confirmed in-place runtime primitive at 0x140327160.
    // The transformation never grows the input:
    // 1) optional leading separator stripping;
    // 2) optional trailing separator stripping;
    // 3) uppercase when bit 0 is set, otherwise lowercase when bit 1 is set;
    // 4) '/' -> '\\';
    // 5) collapse repeated '\\'.
    [[nodiscard]] static std::string normalize(
        std::string_view path,
        std::uint32_t flags);
};

} // namespace dmc::rengine::gdspaces
