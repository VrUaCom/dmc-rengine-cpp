#pragma once

#include "dmc_rengine/formats/diagnostic.hpp"

#include <array>
#include <cstddef>
#include <cstdint>
#include <optional>
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
    /// Index into CamDocument::records of the zone record holding it; -1 when
    /// it was found by shape.
    int record{-1};
};

/// A zone record (type 0x400): a 0x40-byte head, then `zone_count` zones of
/// 0x50 bytes. The count is the byte at +0x14: st002 keeps more bytes after
/// it (`02 01 78`), so a 32-bit read is wrong.
struct CamRecord final {
    std::uint64_t offset{};
    std::uint32_t flags{};
    std::uint32_t zone_count{};
    std::uint64_t size{};
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
    /// The zone records from the first record on, in file order; the rest of
    /// the file is read by shape.
    std::vector<CamRecord> records;
    std::vector<CamArea> areas;
    std::vector<CamPath> paths;
    std::vector<CamRun> coefficient_runs;
    /// Bytes no shape above accounts for that are not zero.
    std::uint64_t unexplained_bytes{};
};


// ---- CAM: the game's cameras -------------------------------------------------
//
// Every CAM record is one camera and the zones where it is used, in a row to
// the end of the file (st006cfg: five records, 0x10 -> 0x1240 exactly). The
// first word is the camera's type, which fixes the record's head; the zone
// count is the byte at +0x14, the flags are at +0x40, and the zones (four
// (x, y, z, 1) floor corners, then the height they reach and 12 zero bytes,
// 0x50 each) close the record:
// - type 0, a fixed camera, head 0xB0: the eye at +0x90, the field of view in
//   radians at +0xA0 (1.05), the target at +0x50. With flag 0x200 the target
//   follows the player and +0x50 is its offset from the feet (st006: 0 105 0);
//   without it the target is that point.
// - type 0x100, a rail camera, head 0x490: up to 32 eye points at +0x80, 32
//   target points at +0x200, 32 fields of view at +0x380, the point count at
//   +0x480. The player's place along the target rail picks the eye at the same
//   place on the eye rail.
// - type 0x400, zones only, head 0x40.
// Read from st000, st001, st002 and st006 configurations (status: candidate;
// the runtime consumer is not traced).

inline constexpr std::uint32_t cam_fixed = 0x000U;
inline constexpr std::uint32_t cam_rail = 0x100U;
inline constexpr std::uint32_t cam_zones_only = 0x400U;
inline constexpr std::uint32_t cam_follows_player = 0x200U;
inline constexpr std::size_t cam_zones_head = 0x40U;
inline constexpr std::size_t cam_fixed_head = 0xB0U;
inline constexpr std::size_t cam_rail_head = 0x490U;
inline constexpr std::size_t cam_rail_points = 32U;
inline constexpr std::size_t cam_zone_size = 0x50U;

struct CamShot final {
    std::uint64_t offset{};
    std::uint32_t type{};
    std::uint32_t flags{};
    std::vector<Vec3> eye;     ///< fixed: one point; rail: the eye rail
    std::vector<Vec3> target;  ///< fixed: one point (absolute or the offset); rail: the target rail
    std::vector<float> fov;    ///< radians, one per eye point
    bool follows_player{};     ///< fixed: target = player + target[0]
    std::vector<CamArea> zones;
};

/// Empty unless the records walk the whole file as described.
[[nodiscard]] std::vector<CamShot> read_cam_shots(std::span<const std::byte> bytes);

/// The camera for a player standing at `player`: the shot one of whose zones
/// holds the player (the smallest such zone when several do).
struct CamView final {
    Vec3 eye;
    Vec3 target;
    float fov_radians{};
    int shot{-1};
};
[[nodiscard]] bool zone_holds(const CamArea& zone, const Vec3& point) noexcept;
[[nodiscard]] std::optional<CamView> cam_view_at(const std::vector<CamShot>& shots, const Vec3& player);

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
    /// +0x00: the effect bank's dispatch kind (ste_effect_kind); +0x02: the
    /// effect id in the stage's bank (stNNN_effect.pac).
    std::uint16_t kind{};
    std::uint16_t number{};
    Vec3 position;
    /// Degrees about x, y and z.
    Vec3 rotation_degrees;
    Vec3 scale;
};

/// The FXBANK kind an STE record starts: 0 P, 1 E, 2 G, 3 V. st006cfg's ten
/// records are all kind 3, id 98 at scale 0.5: the bank's one V (fire E 7
/// with glow E 154 above it), the wall torches at y 510..580 and the braziers
/// at y 150. Status: candidate (from the files and the drawn result in the
/// Native Reader; the runtime reader of STE is not traced).
[[nodiscard]] constexpr std::optional<char> ste_effect_kind(std::uint16_t kind) noexcept {
    constexpr std::array<char, 4> kinds{'P', 'E', 'G', 'V'};
    if (kind >= kinds.size()) return std::nullopt;
    return kinds[kind];
}

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

/// SEF section value 2, laid out as a cutscene's GFG fog (st006cfg: f32 1000,
/// f32 2360, then bytes R G B A, A 0x80 = 1.0): clear up to the nearer
/// distance, full at the farther. st001 writes them far first (2100, 1680).
/// Status: candidate (inferred from the files and the cutscene clips).
inline constexpr std::uint32_t sef_fog_section = 2U;
struct SefFog final {
    float start{};
    float end{};
    std::array<std::uint8_t, 3> colour{};
    float alpha{};
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
/// The fog of a read SEF file, if its value-2 section holds one.
[[nodiscard]] std::optional<SefFog> sef_fog(std::span<const std::byte> bytes, const SefDocument& document);

} // namespace dmc::rengine::formats::stage_cfg
