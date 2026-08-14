#pragma once

#include <algorithm>
#include <cctype>
#include <cstdint>
#include <optional>
#include <span>
#include <string>
#include <string_view>

namespace dmc::recovered::dmc3::runtime::media {

inline constexpr std::uint32_t kNoExplicitLoop = 0xFFFFFFFFU;

struct HdAudioDescriptor final {
    std::string_view physical_name;
    std::uint32_t loop_start_ms{kNoExplicitLoop};
    std::uint32_t loop_end_ms{kNoExplicitLoop};

    [[nodiscard]] bool has_explicit_loop() const noexcept {
        return loop_start_ms != kNoExplicitLoop &&
            loop_end_ms != kNoExplicitLoop;
    }
};

[[nodiscard]] inline std::string ascii_lower(std::string value) {
    std::transform(
        value.begin(), value.end(), value.begin(),
        [](unsigned char character) {
            if (character >= static_cast<unsigned char>('A') &&
                character <= static_cast<unsigned char>('Z')) {
                return static_cast<char>(character -
                    static_cast<unsigned char>('A') +
                    static_cast<unsigned char>('a'));
            }
            return static_cast<char>(character);
        });
    return value;
}

[[nodiscard]] inline std::string basename_of(std::string_view path) {
    const auto slash = path.find_last_of("/\\");
    return std::string{
        slash == std::string_view::npos ? path : path.substr(slash + 1U)};
}

[[nodiscard]] inline std::optional<std::string> replace_extension(
    std::string_view path,
    std::string_view replacement) {
    const auto slash = path.find_last_of("/\\");
    const auto dot = path.find_last_of('.');
    if (dot == std::string_view::npos ||
        (slash != std::string_view::npos && dot < slash)) {
        return std::nullopt;
    }

    std::string result{path.substr(0U, dot)};
    result.append(replacement);
    return result;
}

// Recovered HD audio lookup behavior: strip any directory component used by the
// caller, replace the legacy extension with .ogg, lowercase ASCII and use that
// name as the key into the 154-entry HD descriptor table.
[[nodiscard]] inline std::optional<std::string> make_ogg_lookup_key(
    std::string_view legacy_path) {
    const auto base = basename_of(legacy_path);
    const auto rewritten = replace_extension(base, ".ogg");
    if (!rewritten.has_value()) {
        return std::nullopt;
    }
    return ascii_lower(*rewritten);
}

[[nodiscard]] inline const HdAudioDescriptor* find_hd_audio_descriptor(
    std::string_view legacy_path,
    std::span<const HdAudioDescriptor> descriptors) {
    const auto key = make_ogg_lookup_key(legacy_path);
    if (!key.has_value()) {
        return nullptr;
    }

    const auto iterator = std::find_if(
        descriptors.begin(), descriptors.end(),
        [&key](const HdAudioDescriptor& descriptor) {
            return ascii_lower(std::string{descriptor.physical_name}) == *key;
        });
    return iterator == descriptors.end() ? nullptr : &*iterator;
}

// Recovered video path behavior: preserve the logical path and replace its final
// legacy extension with the HD-port physical .wmv extension.
[[nodiscard]] inline std::optional<std::string> rewrite_sfd_to_wmv(
    std::string_view legacy_path) {
    return replace_extension(legacy_path, ".wmv");
}

} // namespace dmc::recovered::dmc3::runtime::media
