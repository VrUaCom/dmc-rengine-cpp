#pragma once

#include <cstddef>
#include <cstdint>
#include <optional>
#include <span>
#include <string>
#include <vector>

namespace dmc::rengine::formats {

enum class MotParseError : std::uint8_t {
    none,
    truncated_header,
    invalid_magic,
    data_offset_out_of_bounds,
    track_count_limit,
    truncated_track,
    track_size_mismatch,
    stamp_not_increasing,
    chain_does_not_close,
    invalid_document,
};

// One animation track: a run of time-stamped keys over the document's timeline.
struct MotTrack final {
    std::uint32_t track_index{};
    std::uint64_t track_offset{};
    std::uint32_t key_count{};
    std::uint32_t kind{};
    std::uint64_t key_offset{};
    std::int16_t first_stamp{};
    std::int16_t last_stamp{};

    [[nodiscard]] std::int32_t span() const noexcept {
        return static_cast<std::int32_t>(last_stamp) -
            static_cast<std::int32_t>(first_stamp);
    }
};

struct MotDocument final {
    std::uint64_t document_size{};
    std::uint64_t data_offset{};
    float duration{};
    std::uint32_t track_count{};
    std::uint64_t total_key_count{};
    // True when the header's duration equals every track's stamp span. Two
    // independent places in the file agreeing is what raises this layout above
    // a plausible reading of one sample.
    bool duration_matches_stamps{false};
    std::vector<MotTrack> tracks;

    [[nodiscard]] bool valid() const noexcept;
};

struct MotParseResult final {
    std::optional<MotDocument> document;
    MotParseError error{MotParseError::none};
    std::string message;

    [[nodiscard]] bool ok() const noexcept {
        return document.has_value() && error == MotParseError::none;
    }
};

// Structural reader for the DMC3 motion payload.
//
// The format carries no usable magic — the `MOT` tag at `+4` is compared
// nowhere in the executable — so what identifies a motion is that its own
// arithmetic closes: every track's declared size must equal its key count's,
// the chain of sizes must land exactly on the terminator, and every track's
// stamps must increase. A file that fails any of those is refused rather than
// read part-way.
class MotParser final {
public:
    // Product-side bound. The one real payload declares 69 tracks.
    static constexpr std::uint32_t k_max_track_count = 4096U;

    [[nodiscard]] static MotParseResult parse(std::span<const std::byte> bytes);
    [[nodiscard]] static bool structurally_valid(
        std::span<const std::byte> bytes) noexcept;
};

} // namespace dmc::rengine::formats
