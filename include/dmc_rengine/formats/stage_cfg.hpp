#pragma once

#include "dmc_rengine/formats/diagnostic.hpp"

#include <array>
#include <cstddef>
#include <cstdint>
#include <span>
#include <vector>

/**
 * Three members of a stage configuration archive (`stNNNcfg.pac`): POS, EVE
 * and CAM.
 *
 * Their purpose is settled — POS places, EVE marks event volumes, CAM drives
 * the stage camera (rengine format-purpose closure, StageCfg families) — and
 * their schemas are not: the runtime compares none of these tags, so the tag
 * is an authoring convention and these readers are readers of what the files
 * hold, checked against every byte, not of how the game consumes them.
 *
 * - POS: a 16-byte header (`POS\0`, u16 version, u16 count) and 48-byte
 *   records to the end of the file: a u32, a position, a heading in degrees,
 *   and zeros. Records past `count` are all zero.
 * - EVE: a 16-byte header (`EVE\0`, u16 version, u16 count) and exactly
 *   `count` 128-byte records: three u32 fields, then at +0x20 four corners
 *   (x, y, z, 1) of a floor quadrilateral, then at +0x60 a vertical extent.
 * - CAM: a 16-byte header (`CAM\0`, u16 version, u16 count) and `count`
 *   records of varying length whose layout is open. What can be found by
 *   shape is found: areas laid out as EVE's (four corners, w = 1, one height,
 *   then an extent), paths of points, and runs of one repeated coefficient.
 *   Everything else is left as bytes.
 */
namespace dmc::rengine::formats::stage_cfg {

struct Vec3 final {
    float x{};
    float y{};
    float z{};
};

struct Header final {
    std::uint16_t version{};
    std::uint16_t count{};
};

// ---- POS -------------------------------------------------------------------

inline constexpr std::size_t pos_header_size = 0x10U;
inline constexpr std::size_t pos_record_size = 0x30U;

struct PosRecord final {
    std::uint64_t offset{};
    std::uint32_t tag{};
    Vec3 position;
    float heading_degrees{};
    /// The 28 bytes after the heading are zero, as in every record seen.
    bool tail_zero{true};
};

struct PosDocument final {
    Header header;
    /// Records the file has room for: (size - 16) / 48.
    std::uint32_t capacity{};
    std::vector<PosRecord> records;
};

// ---- EVE -------------------------------------------------------------------

inline constexpr std::size_t eve_header_size = 0x10U;
inline constexpr std::size_t eve_record_size = 0x80U;

struct Quad final {
    std::array<Vec3, 4> corners{};
    /// Every corner's fourth component is 1.0.
    bool homogeneous{true};
    /// The four corners share one height: a floor area.
    bool level{true};
};

struct EveRecord final {
    std::uint64_t offset{};
    /// +0x00, +0x04, +0x08: what they mean is open; +0x04 groups the records.
    std::uint32_t field0{};
    std::uint32_t kind{};
    std::uint32_t argument{};
    Quad area;
    /// +0x60: how far the volume reaches above its floor area.
    float extent{};
    /// Everything not named above is zero.
    bool rest_zero{true};
};

struct EveDocument final {
    Header header;
    std::vector<EveRecord> records;
};

// ---- CAM -------------------------------------------------------------------

inline constexpr std::size_t cam_header_size = 0x10U;

struct CamArea final {
    std::uint64_t offset{};
    Quad quad;
    float extent{};
};

struct CamPath final {
    std::uint64_t offset{};
    std::vector<Vec3> points;
};

struct CamRun final {
    std::uint64_t offset{};
    std::uint32_t count{};
    float value{};
};

struct CamDocument final {
    Header header;
    std::vector<CamArea> areas;
    std::vector<CamPath> paths;
    std::vector<CamRun> coefficient_runs;
    /// Bytes no shape above accounts for that are not zero.
    std::uint64_t unexplained_bytes{};
};

template <typename Document>
struct Result final {
    bool recognized{false};
    Document document;
    std::vector<ParseDiagnostic> diagnostics;

    [[nodiscard]] bool ok() const noexcept {
        if (!recognized) return false;
        for (const auto& diagnostic : diagnostics) {
            if (diagnostic.severity == ParseSeverity::error) return false;
        }
        return true;
    }
};

[[nodiscard]] Result<PosDocument> read_pos(std::span<const std::byte> bytes);
[[nodiscard]] Result<EveDocument> read_eve(std::span<const std::byte> bytes);
[[nodiscard]] Result<CamDocument> read_cam(std::span<const std::byte> bytes);

} // namespace dmc::rengine::formats::stage_cfg
