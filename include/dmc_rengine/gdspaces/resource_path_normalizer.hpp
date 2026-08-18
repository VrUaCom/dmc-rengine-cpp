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
    // The recovered runtime primitive consumes a NUL-terminated path. Product
    // callers using string_view must reject embedded NUL explicitly rather
    // than silently truncating or treating the suffix as another key domain.
    [[nodiscard]] static bool c_string_compatible(
        std::string_view path) noexcept;

    // Reproduces the confirmed transformation order of ResourcePathNormalize
    // for C-string-compatible inputs. The transformation never grows input:
    // 1) optional leading separator stripping;
    // 2) optional trailing separator stripping;
    // 3) uppercase when bit 0 is set, otherwise lowercase when bit 1 is set;
    // 4) '/' -> '\\';
    // 5) collapse repeated '\\'.
    //
    // Only ASCII A-Z/a-z case folding is performed. No locale/Unicode folding
    // is inferred from the recovered executable.
    [[nodiscard]] static std::string normalize(
        std::string_view path,
        std::uint32_t flags);
};

} // namespace dmc::rengine::gdspaces
