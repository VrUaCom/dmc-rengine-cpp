#include "dmc_rengine/gdspaces/resource_path_normalizer.hpp"

#include <string>
#include <string_view>

namespace dmc::rengine::gdspaces {
namespace {

[[nodiscard]] constexpr bool separator(char value) noexcept {
    return value == '/' || value == '\\';
}

[[nodiscard]] constexpr bool has_flag(
    std::uint32_t flags,
    ResourcePathNormalizationFlag flag) noexcept {
    return (flags & flag_value(flag)) != 0U;
}

} // namespace

std::string ResourcePathNormalizer::normalize(
    std::string_view path,
    std::uint32_t flags) {
    std::size_t begin = 0U;
    std::size_t end = path.size();

    if (has_flag(flags, ResourcePathNormalizationFlag::strip_leading_separators)) {
        while (begin < end && separator(path[begin])) {
            ++begin;
        }
    }

    if (has_flag(flags, ResourcePathNormalizationFlag::strip_trailing_separators)) {
        while (end > begin && separator(path[end - 1U])) {
            --end;
        }
    }

    std::string result;
    result.reserve(end - begin);

    const auto uppercase = has_flag(
        flags, ResourcePathNormalizationFlag::uppercase_ascii);
    const auto lowercase = !uppercase && has_flag(
        flags, ResourcePathNormalizationFlag::lowercase_ascii);

    bool previous_separator = false;
    for (std::size_t index = begin; index < end; ++index) {
        auto value = path[index];
        if (uppercase && value >= 'a' && value <= 'z') {
            value = static_cast<char>(value - ('a' - 'A'));
        } else if (lowercase && value >= 'A' && value <= 'Z') {
            value = static_cast<char>(value + ('a' - 'A'));
        }

        if (separator(value)) {
            if (!previous_separator) {
                result.push_back('\\');
                previous_separator = true;
            }
        } else {
            result.push_back(value);
            previous_separator = false;
        }
    }

    return result;
}

} // namespace dmc::rengine::gdspaces
