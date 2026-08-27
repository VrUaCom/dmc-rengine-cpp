#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <string_view>

namespace dmc::rengine::profiles::dmc3 {

// The TM2 image reader.
//
// This was found while asking a different question — which payload tags the
// runtime compares anywhere — and it matters because `at.ptx` in a real volume
// is a texture container this project cannot read. TM2 is the PlayStation 2
// texture format, and the image carries a reader for it.
//
// That is not proof about `at.ptx`; nothing here has seen its bytes. It is a
// recognizer for a format the runtime demonstrably reads, put in place so that
// if such a payload appears it is named rather than left as `unknown`.
struct Tm2Contract final {
    static constexpr std::string_view canonical_target_sha256 =
        "e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082";

    static constexpr std::uint64_t reader_va = 0x1403365B0ULL;
    static constexpr std::uint64_t magic_compare_va = 0x1403365BAULL;

    // Compared as one dword at offset zero, NUL included — unlike `PAC`, whose
    // fourth byte the runtime never reads. A reader that accepted `TM2x` would
    // be more permissive than the game.
    static constexpr std::string_view magic = "TM2";
    static constexpr std::size_t magic_offset = 0U;
    static constexpr std::size_t magic_bytes = 4U;
    static constexpr std::uint32_t magic_dword = 0x00324D54U;

    // `ebp = *(u32*)(p + 8); rbp = p + ebp` — a base-relative offset to the
    // payload the decoder is then handed.
    static constexpr std::size_t data_offset_field = 0x08U;
    static constexpr bool data_offset_is_base_relative = true;

    // The secondary structure the reader passes on the path where the decoded
    // object reports nothing at `+0x28`. Recorded as an extent, not a meaning.
    static constexpr std::size_t secondary_offset = 0x38U;

    // On a magic mismatch the reader does not return an error: it constructs a
    // fallback object and reports its dimensions instead. So "this is not a
    // TM2" is not something the runtime says out loud, and a product that
    // guessed from the reader's return value would learn nothing.
    static constexpr std::uint64_t fallback_path_va = 0x140336679ULL;
    static constexpr bool mismatch_is_reported = false;

    [[nodiscard]] static constexpr bool magic_matches(
        std::uint32_t first_dword) noexcept {
        return first_dword == magic_dword;
    }
};

// Which compressed pixel formats the DDS reader accepts, and what it turns
// them into.
//
// This project's texture work has been deriving its compression set from the
// corpus, which shows two of them. The runtime's own chain lists ten, plus
// four aliases, and maps each to a DXGI format code. That moves the set from
// observed to recovered: a texture using `BC5S` is one the game reads, whether
// or not any supplied file happens to.
struct DdsPixelFormatContract final {
    static constexpr std::string_view canonical_target_sha256 =
        "e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082";

    static constexpr std::uint64_t mapper_va = 0x14004A946ULL;
    // `mov eax, dword ptr [rcx + 8]` — the FourCC field of a DDS pixel format.
    static constexpr std::size_t fourcc_offset_in_pixel_format = 0x08U;

    struct FourCcFormat final {
        std::string_view fourcc;
        std::uint32_t format_code;
        std::uint64_t compare_va;
    };

    // In chain order. The codes are DXGI format values.
    static constexpr std::array<FourCcFormat, 10> mappings{
        FourCcFormat{"DXT1", 71U, 0x14004A949ULL},
        FourCcFormat{"DXT3", 74U, 0x14004A956ULL},
        FourCcFormat{"DXT5", 77U, 0x14004A963ULL},
        FourCcFormat{"ATI1", 80U, 0x14004A97EULL},
        FourCcFormat{"BC4S", 81U, 0x14004A992ULL},
        FourCcFormat{"ATI2", 83U, 0x14004A99FULL},
        FourCcFormat{"BC5S", 84U, 0x14004A9B3ULL},
        FourCcFormat{"RGBG", 68U, 0x14004A9C0ULL},
        FourCcFormat{"GRGB", 69U, 0x14004A9CDULL},
        FourCcFormat{"YUY2", 107U, 0x14004A9DAULL},
    };

    // Four FourCCs jump into another's store rather than carrying their own,
    // so they are aliases and not separate formats.
    struct Alias final {
        std::string_view fourcc;
        std::string_view same_as;
    };
    static constexpr std::array<Alias, 4> aliases{
        Alias{"DXT2", "DXT3"},
        Alias{"DXT4", "DXT5"},
        Alias{"BC4U", "ATI1"},
        Alias{"BC5U", "ATI2"},
    };

    [[nodiscard]] static constexpr std::uint32_t format_for(
        std::string_view fourcc) noexcept {
        for (const auto& alias : aliases) {
            if (alias.fourcc == fourcc) {
                fourcc = alias.same_as;
                break;
            }
        }
        for (const auto& mapping : mappings) {
            if (mapping.fourcc == fourcc) {
                return mapping.format_code;
            }
        }
        return 0U;
    }
};

} // namespace dmc::rengine::profiles::dmc3
