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
 * - ITM: a 16-byte header (`ITM\0`, u16 version, u16 count) and `count`
 *   20-byte records — item id, position, rotation about Y — padded with zeros
 *   to a multiple of 16 bytes (the static item placements).
 * - STE: a 16-byte header (`STE\0`, u16 version, u16 count) and `count`
 *   40-byte records: two u16 fields, then position, rotation in degrees and
 *   scale, each three floats.
 * - EST: a 16-byte header (`EST\0`, u16 version, u16 count, u32 offset of
 *   a table) and, at that offset, `count` rows of five u32 offsets, one per
 *   difficulty mode. Each offset starts a program of commands: a u32 whose
 *   low byte is the command and whose next byte is how many i32 arguments
 *   follow, the upper half zero; a zero word ends the program.
 * - SEF: a 16-byte-aligned header (`SEF\0`, u16 section count, u16) and a
 *   table from +0x08 of (u32 value, u32 offset) pairs, one per section, that
 *   runs up to the first section; sections follow in offset order. Inside a
 *   section the layout is open.
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


// ---- ITM -------------------------------------------------------------------

inline constexpr std::size_t itm_header_size = 0x10U;
inline constexpr std::size_t itm_record_size = 0x14U;

struct ItmRecord final {
    std::uint64_t offset{};
    std::uint32_t item_id{};
    Vec3 position;
    float rotation_y{};
};

struct ItmDocument final {
    Header header;
    std::vector<ItmRecord> records;
};

// ---- STE -------------------------------------------------------------------

inline constexpr std::size_t ste_header_size = 0x10U;
inline constexpr std::size_t ste_record_size = 0x28U;

struct SteRecord final {
    std::uint64_t offset{};
    /// +0x00 and +0x02: what they mean is open; +0x00 was 2 in every record seen.
    std::uint16_t kind{};
    std::uint16_t number{};
    Vec3 position;
    /// Degrees about x, y and z.
    Vec3 rotation_degrees;
    Vec3 scale;
};

struct SteDocument final {
    Header header;
    std::vector<SteRecord> records;
};

// ---- EST -------------------------------------------------------------------

inline constexpr std::size_t est_header_size = 0x10U;
/// Offsets in each row of the table: one per difficulty mode (Easy, Normal,
/// Hard, Very Hard, Dante Must Die).
inline constexpr std::size_t est_modes = 5U;
/// More arguments than this is not a command but bytes misread as one.
inline constexpr std::uint32_t est_max_arguments = 8U;

struct EstCommand final {
    std::uint64_t offset{};
    std::uint8_t code{};
    std::vector<std::int32_t> arguments;
};

struct EstProgram final {
    std::uint64_t offset{};
    std::vector<EstCommand> commands;
    /// The program ended on a zero word, not at the end of the bytes.
    bool terminated{false};
};

struct EstDocument final {
    Header header;
    std::uint32_t table_offset{};
    /// `count` rows of `est_modes` indices into `programs`; -1 for an offset of 0.
    std::vector<std::array<std::int32_t, est_modes>> rows;
    /// Each distinct program once, in offset order.
    std::vector<EstProgram> programs;
    /// Non-zero bytes between the header and the table no program reads.
    std::uint64_t unexplained_bytes{};
};

// ---- SEF -------------------------------------------------------------------

struct SefSection final {
    std::uint32_t value{};
    std::uint64_t offset{};
    std::uint64_t size{};
};

struct SefDocument final {
    /// u16 at +0x04: the number of sections, as the table confirms.
    std::uint16_t sections_declared{};
    /// u16 at +0x06: open.
    std::uint16_t field6{};
    std::vector<SefSection> sections;
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
[[nodiscard]] Result<ItmDocument> read_itm(std::span<const std::byte> bytes);
[[nodiscard]] Result<SteDocument> read_ste(std::span<const std::byte> bytes);
[[nodiscard]] Result<EstDocument> read_est(std::span<const std::byte> bytes);
[[nodiscard]] Result<SefDocument> read_sef(std::span<const std::byte> bytes);

} // namespace dmc::rengine::formats::stage_cfg
