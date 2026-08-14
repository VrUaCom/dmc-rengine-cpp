#pragma once

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <optional>
#include <span>
#include <string>
#include <string_view>

namespace dmc::recovered::dmc3::runtime::media {

inline constexpr std::uint32_t no_explicit_loop =
    std::numeric_limits<std::uint32_t>::max();

inline constexpr std::uint64_t hd_audio_lookup_code_va = 0x140031D80ULL;
inline constexpr std::uint64_t hd_audio_loop_apply_code_va = 0x140032A80ULL;
inline constexpr std::uint64_t hd_video_rewrite_code_va = 0x14032C10DULL;
inline constexpr std::uint32_t hd_audio_descriptor_count = 154U;

// Exact x64 runtime descriptor ABI observed in the 154 x 0x10 HD audio table.
// `filename_pointer` points at the physical OGG name in the original process.
struct HdAudioRuntimeDescriptor final {
    std::uintptr_t filename_pointer{};                 // +0x00
    std::uint32_t loop_start_ms{no_explicit_loop};    // +0x08
    std::uint32_t loop_end_ms{no_explicit_loop};      // +0x0C
};

static_assert(sizeof(std::uintptr_t) == 8U);
static_assert(sizeof(HdAudioRuntimeDescriptor) == 0x10U);
static_assert(offsetof(HdAudioRuntimeDescriptor, filename_pointer) == 0x00U);
static_assert(offsetof(HdAudioRuntimeDescriptor, loop_start_ms) == 0x08U);
static_assert(offsetof(HdAudioRuntimeDescriptor, loop_end_ms) == 0x0CU);

// Host-safe decoded view used by tests/research harnesses after resolving the
// original process pointer. This is not claimed to be the in-game object ABI.
struct HdAudioDescriptorView final {
    std::string_view physical_name;
    std::uint32_t loop_start_ms{no_explicit_loop};
    std::uint32_t loop_end_ms{no_explicit_loop};

    [[nodiscard]] bool has_explicit_loop() const noexcept {
        return loop_start_ms != no_explicit_loop &&
            loop_end_ms != no_explicit_loop;
    }
};

// Compatibility name for host-side decoded descriptors. The exact game ABI is
// HdAudioRuntimeDescriptor above; this alias must not be used as an ABI claim.
using HdAudioDescriptor = HdAudioDescriptorView;

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

[[nodiscard]] inline std::optional<std::string> replace_final_extension(
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

// Recovered at the HD audio lookup boundary around canonical VA 0x140031D80:
// legacy request -> basename -> .ogg -> ASCII lowercase -> descriptor lookup.
[[nodiscard]] inline std::optional<std::string> make_ogg_lookup_key(
    std::string_view legacy_path) {
    const auto rewritten = replace_final_extension(
        basename_of(legacy_path), ".ogg");
    if (!rewritten.has_value()) {
        return std::nullopt;
    }
    return ascii_lower(*rewritten);
}

[[nodiscard]] inline const HdAudioDescriptorView* find_hd_audio_descriptor(
    std::string_view legacy_path,
    std::span<const HdAudioDescriptorView> descriptors) {
    const auto key = make_ogg_lookup_key(legacy_path);
    if (!key.has_value()) {
        return nullptr;
    }

    const auto iterator = std::find_if(
        descriptors.begin(), descriptors.end(),
        [&key](const HdAudioDescriptorView& descriptor) {
            return ascii_lower(std::string{descriptor.physical_name}) == *key;
        });
    return iterator == descriptors.end() ? nullptr : &*iterator;
}

// The loop fields are passed to FMOD_Channel_SetLoopPoints using millisecond
// units at the recovered boundary around canonical VA 0x140032A80. This helper
// only exposes the already-decoded descriptor values; it does not recreate FMOD.
struct HdAudioLoopPoints final {
    std::uint32_t start_ms{};
    std::uint32_t end_ms{};
};

[[nodiscard]] inline std::optional<HdAudioLoopPoints> explicit_loop_points(
    const HdAudioDescriptorView& descriptor) noexcept {
    if (!descriptor.has_explicit_loop()) {
        return std::nullopt;
    }
    return HdAudioLoopPoints{
        .start_ms = descriptor.loop_start_ms,
        .end_ms = descriptor.loop_end_ms,
    };
}

// Recovered video boundary around canonical VA 0x14032C10D: preserve the path
// and replace its final legacy extension with .wmv before media loading.
[[nodiscard]] inline std::optional<std::string> rewrite_legacy_video_to_wmv(
    std::string_view legacy_path) {
    return replace_final_extension(legacy_path, ".wmv");
}

} // namespace dmc::recovered::dmc3::runtime::media
