#include "dmc_rengine/formats/stage_cfg.hpp"

#include <algorithm>
#include <cmath>
#include <cstring>
#include <string>
#include <string_view>
#include <utility>

namespace dmc::rengine::formats::stage_cfg {
namespace {

[[nodiscard]] std::uint32_t u32(std::span<const std::byte> bytes, std::size_t at) noexcept {
    std::uint32_t value = 0U;
    std::memcpy(&value, bytes.data() + at, sizeof value);
    return value;
}

[[nodiscard]] std::uint16_t u16(std::span<const std::byte> bytes, std::size_t at) noexcept {
    std::uint16_t value = 0U;
    std::memcpy(&value, bytes.data() + at, sizeof value);
    return value;
}

[[nodiscard]] float f32(std::span<const std::byte> bytes, std::size_t at) noexcept {
    float value = 0.0F;
    std::memcpy(&value, bytes.data() + at, sizeof value);
    return value;
}

[[nodiscard]] Vec3 vec3(std::span<const std::byte> bytes, std::size_t at) noexcept {
    return {f32(bytes, at), f32(bytes, at + 4U), f32(bytes, at + 8U)};
}

[[nodiscard]] bool zero(std::span<const std::byte> bytes, std::size_t from, std::size_t to) noexcept {
    return std::all_of(bytes.begin() + static_cast<std::ptrdiff_t>(from),
                       bytes.begin() + static_cast<std::ptrdiff_t>(to),
                       [](std::byte value) { return value == std::byte{0}; });
}

template <typename Document>
void diagnose(Result<Document>& result, ParseSeverity severity, std::string code, std::string message,
              std::uint64_t offset) {
    result.diagnostics.push_back({.severity = severity, .code = std::move(code), .message = std::move(message),
                                  .offset = offset});
}

/// The 16-byte header all three share: tag, u16 version, u16 count, then eight zero bytes.
template <typename Document>
[[nodiscard]] bool header(std::span<const std::byte> bytes, std::string_view tag, Result<Document>& result) {
    if (bytes.size() < 0x10U || std::memcmp(bytes.data(), tag.data(), 4U) != 0) {
        diagnose(result, ParseSeverity::error, "stage-cfg.tag",
                 "The bytes do not open with the " + std::string{tag.substr(0, 3)} + " tag and a 16-byte header.",
                 0U);
        return false;
    }
    result.recognized = true;
    result.document.header = {.version = u16(bytes, 4U), .count = u16(bytes, 6U)};
    if (!zero(bytes, 8U, 16U)) {
        diagnose(result, ParseSeverity::warning, "stage-cfg.header-tail",
                 "Header bytes 8..15 are not zero, as they are in every file seen.", 8U);
    }
    return true;
}

/// Four (x, y, z, w) corners at `at`.
[[nodiscard]] Quad quad(std::span<const std::byte> bytes, std::size_t at) noexcept {
    Quad result;
    for (std::size_t corner = 0U; corner < 4U; ++corner) {
        const auto base = at + corner * 16U;
        result.corners[corner] = vec3(bytes, base);
        result.homogeneous = result.homogeneous && f32(bytes, base + 12U) == 1.0F;
    }
    const float height = result.corners[0].y;
    for (const auto& corner : result.corners) {
        result.level = result.level && std::fabs(corner.y - height) <= 0.01F + std::fabs(height) * 1e-5F;
    }
    return result;
}

[[nodiscard]] bool finite_point(const Vec3& point) noexcept {
    return std::isfinite(point.x) && std::isfinite(point.y) && std::isfinite(point.z);
}

/// A point of a path: finite, every coordinate a real number and none zero,
/// within the range a stage spans. Bit patterns of small integers, which
/// read as denormals, are excluded by the lower bound.
[[nodiscard]] bool path_point(const Vec3& point) noexcept {
    const auto coordinate = [](float value) {
        const float magnitude = std::fabs(value);
        return std::isfinite(value) && magnitude >= 1e-3F && magnitude < 1e6F;
    };
    return coordinate(point.x) && coordinate(point.y) && coordinate(point.z);
}

} // namespace

Result<PosDocument> read_pos(std::span<const std::byte> bytes) {
    Result<PosDocument> result;
    if (!header(bytes, std::string_view{"POS\0", 4U}, result)) return result;
    auto& document = result.document;
    if ((bytes.size() - pos_header_size) % pos_record_size != 0U) {
        diagnose(result, ParseSeverity::error, "pos.size",
                 "The file is not a 16-byte header and whole 48-byte records.", bytes.size());
        return result;
    }
    document.capacity = static_cast<std::uint32_t>((bytes.size() - pos_header_size) / pos_record_size);
    if (document.header.count > document.capacity) {
        diagnose(result, ParseSeverity::error, "pos.count",
                 "The header counts " + std::to_string(document.header.count) + " records; the file holds " +
                     std::to_string(document.capacity) + ".",
                 6U);
        return result;
    }
    for (std::uint32_t index = 0U; index < document.capacity; ++index) {
        const auto at = pos_header_size + index * pos_record_size;
        if (index >= document.header.count) {
            if (!zero(bytes, at, at + pos_record_size)) {
                diagnose(result, ParseSeverity::warning, "pos.spare-record",
                         "A record past the count is not zero.", at);
            }
            continue;
        }
        PosRecord record{
            .offset = at,
            .tag = u32(bytes, at),
            .position = vec3(bytes, at + 4U),
            .heading_degrees = f32(bytes, at + 16U),
            .tail_zero = zero(bytes, at + 20U, at + pos_record_size),
        };
        if (!finite_point(record.position) || !std::isfinite(record.heading_degrees)) {
            diagnose(result, ParseSeverity::error, "pos.not-finite", "A position or heading is not a number.", at);
        }
        if (!record.tail_zero) {
            diagnose(result, ParseSeverity::info, "pos.tail",
                     "Bytes after the heading are not zero; they are kept and not read.", at + 20U);
        }
        document.records.push_back(record);
    }
    return result;
}

Result<EveDocument> read_eve(std::span<const std::byte> bytes) {
    Result<EveDocument> result;
    if (!header(bytes, std::string_view{"EVE\0", 4U}, result)) return result;
    auto& document = result.document;
    const auto expected = eve_header_size + std::size_t{document.header.count} * eve_record_size;
    if (bytes.size() != expected) {
        diagnose(result, ParseSeverity::error, "eve.size",
                 "The header counts " + std::to_string(document.header.count) + " records, which take " +
                     std::to_string(expected) + " bytes; the file has " + std::to_string(bytes.size()) + ".",
                 6U);
        return result;
    }
    for (std::uint32_t index = 0U; index < document.header.count; ++index) {
        const auto at = eve_header_size + index * eve_record_size;
        EveRecord record{
            .offset = at,
            .field0 = u32(bytes, at),
            .kind = u32(bytes, at + 4U),
            .argument = u32(bytes, at + 8U),
            .area = quad(bytes, at + 0x20U),
            .extent = f32(bytes, at + 0x60U),
            .rest_zero = zero(bytes, at + 0x0CU, at + 0x20U) && zero(bytes, at + 0x64U, at + eve_record_size),
        };
        if (!record.area.homogeneous) {
            diagnose(result, ParseSeverity::warning, "eve.corner-w",
                     "A corner's fourth component is not 1.", at + 0x20U);
        }
        for (const auto& corner : record.area.corners) {
            if (!finite_point(corner)) {
                diagnose(result, ParseSeverity::error, "eve.not-finite", "A corner is not a number.", at + 0x20U);
                break;
            }
        }
        if (!record.rest_zero) {
            diagnose(result, ParseSeverity::info, "eve.unread-bytes",
                     "Bytes this reader does not name are not zero; they are kept.", at);
        }
        document.records.push_back(record);
    }
    return result;
}

Result<CamDocument> read_cam(std::span<const std::byte> bytes) {
    Result<CamDocument> result;
    if (!header(bytes, std::string_view{"CAM\0", 4U}, result)) return result;
    auto& document = result.document;
    if (bytes.size() % 4U != 0U) {
        diagnose(result, ParseSeverity::error, "cam.size", "The file is not made of whole 32-bit words.",
                 bytes.size());
        return result;
    }
    std::size_t at = cam_header_size;
    while (at + 4U <= bytes.size()) {
        if (u32(bytes, at) == 0U) {
            at += 4U;
            continue;
        }
        // An area, as EVE lays one out: four (x, y, z, 1) corners on one
        // level, then a positive extent.
        if (at % 16U == 0U && at + 68U <= bytes.size()) {
            const auto area = quad(bytes, at);
            const float extent = f32(bytes, at + 64U);
            bool finite = true;
            for (const auto& corner : area.corners) finite = finite && path_point(corner);
            if (area.homogeneous && area.level && finite && std::isfinite(extent) && extent > 0.0F) {
                document.areas.push_back({.offset = at, .quad = area, .extent = extent});
                at += 68U;
                continue;
            }
        }
        // A run of one coefficient repeated at least three times, in (0, 16).
        {
            const float value = f32(bytes, at);
            std::size_t count = 0U;
            while (at + (count + 1U) * 4U <= bytes.size() && u32(bytes, at + count * 4U) == u32(bytes, at)) ++count;
            if (count >= 3U && std::isfinite(value) && value > 0.0F && value < 16.0F) {
                document.coefficient_runs.push_back(
                    {.offset = at, .count = static_cast<std::uint32_t>(count), .value = value});
                at += count * 4U;
                continue;
            }
        }
        // A path: two or more points in a row.
        {
            CamPath path{.offset = at, .points = {}};
            std::size_t cursor = at;
            while (cursor + 12U <= bytes.size()) {
                const auto point = vec3(bytes, cursor);
                // A homogeneous 1.0 after three coordinates belongs to an
                // area-shaped record, not to a path of points.
                if (!path_point(point)) break;
                path.points.push_back(point);
                cursor += 12U;
            }
            if (path.points.size() >= 2U) {
                document.paths.push_back(std::move(path));
                at = cursor;
                continue;
            }
        }
        document.unexplained_bytes += 4U;
        at += 4U;
    }
    if (document.unexplained_bytes > 0U) {
        diagnose(result, ParseSeverity::info, "cam.unexplained",
                 std::to_string(document.unexplained_bytes) +
                     " non-zero bytes fit no shape this reader knows; the record layout is open.",
                 cam_header_size);
    }
    return result;
}

Result<ItmDocument> read_itm(std::span<const std::byte> bytes) {
    Result<ItmDocument> result;
    if (!header(bytes, std::string_view{"ITM\0", 4U}, result)) return result;
    auto& document = result.document;
    const auto used = itm_header_size + std::size_t{document.header.count} * itm_record_size;
    const auto padded = (used + 15U) & ~std::size_t{15U};
    if (bytes.size() < used) {
        diagnose(result, ParseSeverity::error, "itm.size",
                 "The header counts " + std::to_string(document.header.count) + " records, which take " +
                     std::to_string(used) + " bytes; the file has " + std::to_string(bytes.size()) + ".",
                 6U);
        return result;
    }
    if (bytes.size() != padded) {
        diagnose(result, ParseSeverity::warning, "itm.padding",
                 "Records are padded to a multiple of 16 bytes in every file seen; this one is " +
                     std::to_string(bytes.size()) + " bytes, not " + std::to_string(padded) + ".",
                 used);
    }
    if (!zero(bytes, used, bytes.size())) {
        diagnose(result, ParseSeverity::info, "itm.tail", "Bytes after the last record are not zero; they are kept.",
                 used);
    }
    for (std::uint32_t index = 0U; index < document.header.count; ++index) {
        const auto at = itm_header_size + index * itm_record_size;
        ItmRecord record{.offset = at, .item_id = u32(bytes, at), .position = vec3(bytes, at + 4U),
                         .rotation_y = f32(bytes, at + 16U)};
        if (!finite_point(record.position) || !std::isfinite(record.rotation_y)) {
            diagnose(result, ParseSeverity::error, "itm.not-finite", "A position or rotation is not a number.", at);
        }
        document.records.push_back(record);
    }
    return result;
}

Result<SteDocument> read_ste(std::span<const std::byte> bytes) {
    Result<SteDocument> result;
    if (!header(bytes, std::string_view{"STE\0", 4U}, result)) return result;
    auto& document = result.document;
    const auto used = ste_header_size + std::size_t{document.header.count} * ste_record_size;
    if (bytes.size() < used) {
        diagnose(result, ParseSeverity::error, "ste.size",
                 "The header counts " + std::to_string(document.header.count) + " records, which take " +
                     std::to_string(used) + " bytes; the file has " + std::to_string(bytes.size()) + ".",
                 6U);
        return result;
    }
    if (!zero(bytes, used, bytes.size())) {
        diagnose(result, ParseSeverity::info, "ste.tail", "Bytes after the last record are not zero; they are kept.",
                 used);
    }
    for (std::uint32_t index = 0U; index < document.header.count; ++index) {
        const auto at = ste_header_size + index * ste_record_size;
        SteRecord record{
            .offset = at,
            .kind = u16(bytes, at),
            .number = u16(bytes, at + 2U),
            .position = vec3(bytes, at + 4U),
            .rotation_degrees = vec3(bytes, at + 16U),
            .scale = vec3(bytes, at + 28U),
        };
        if (!finite_point(record.position) || !finite_point(record.rotation_degrees) || !finite_point(record.scale)) {
            diagnose(result, ParseSeverity::error, "ste.not-finite", "A transform value is not a number.", at);
        }
        document.records.push_back(record);
    }
    return result;
}

Result<EstDocument> read_est(std::span<const std::byte> bytes) {
    Result<EstDocument> result;
    if (bytes.size() < est_header_size || std::memcmp(bytes.data(), "EST\0", 4U) != 0) {
        diagnose(result, ParseSeverity::error, "stage-cfg.tag",
                 "The bytes do not open with the EST tag and a 16-byte header.", 0U);
        return result;
    }
    result.recognized = true;
    auto& document = result.document;
    document.header = {.version = u16(bytes, 4U), .count = u16(bytes, 6U)};
    document.table_offset = u32(bytes, 8U);
    if (!zero(bytes, 12U, 16U)) {
        diagnose(result, ParseSeverity::warning, "stage-cfg.header-tail",
                 "Header bytes 12..15 are not zero, as they are in every file seen.", 12U);
    }
    const auto table = std::size_t{document.table_offset};
    const auto table_end = table + std::size_t{document.header.count} * est_modes * 4U;
    if (table < est_header_size || table % 4U != 0U || table_end > bytes.size()) {
        diagnose(result, ParseSeverity::error, "est.table",
                 "The table of " + std::to_string(document.header.count) + " rows of five offsets at " +
                     std::to_string(table) + " does not fit the file.",
                 8U);
        return result;
    }
    if (!zero(bytes, table_end, bytes.size())) {
        diagnose(result, ParseSeverity::info, "est.tail", "Bytes after the table are not zero; they are kept.",
                 table_end);
    }

    // Each distinct offset is one program; rows index into them.
    std::vector<std::uint32_t> starts;
    for (std::size_t at = table; at < table_end; at += 4U) {
        const auto start = u32(bytes, at);
        if (start != 0U && std::find(starts.begin(), starts.end(), start) == starts.end()) starts.push_back(start);
    }
    std::sort(starts.begin(), starts.end());
    std::vector<bool> read(table, false);
    for (const auto start : starts) {
        EstProgram program{.offset = start, .commands = {}, .terminated = false};
        if (start < est_header_size || start % 4U != 0U || start >= table) {
            diagnose(result, ParseSeverity::error, "est.offset",
                     "A program offset " + std::to_string(start) + " lies outside the bytes between header and table.",
                     table);
            document.programs.push_back(std::move(program));
            continue;
        }
        std::size_t at = start;
        while (at + 4U <= table) {
            const auto word = u32(bytes, at);
            std::fill(read.begin() + static_cast<std::ptrdiff_t>(at), read.begin() + static_cast<std::ptrdiff_t>(at + 4U),
                      true);
            if (word == 0U) {
                program.terminated = true;
                break;
            }
            const auto arguments = (word >> 8U) & 0xFFU;
            if ((word >> 16U) != 0U || arguments > est_max_arguments || at + 4U + arguments * 4U > table) {
                diagnose(result, ParseSeverity::error, "est.command",
                         "A word that is not a command: " + std::to_string(word) + ".", at);
                break;
            }
            EstCommand command{.offset = at, .code = static_cast<std::uint8_t>(word & 0xFFU), .arguments = {}};
            for (std::uint32_t argument = 0U; argument < arguments; ++argument) {
                const auto where = at + 4U + argument * 4U;
                command.arguments.push_back(static_cast<std::int32_t>(u32(bytes, where)));
                std::fill(read.begin() + static_cast<std::ptrdiff_t>(where),
                          read.begin() + static_cast<std::ptrdiff_t>(where + 4U), true);
            }
            at += 4U + arguments * 4U;
            program.commands.push_back(std::move(command));
        }
        if (!program.terminated) {
            diagnose(result, ParseSeverity::warning, "est.unterminated",
                     "A program runs to the table without a zero word to end it.", start);
        }
        document.programs.push_back(std::move(program));
    }
    for (std::uint32_t row = 0U; row < document.header.count; ++row) {
        std::array<std::int32_t, est_modes> indices{};
        for (std::size_t mode = 0U; mode < est_modes; ++mode) {
            const auto start = u32(bytes, table + (row * est_modes + mode) * 4U);
            const auto found = std::find(starts.begin(), starts.end(), start);
            indices[mode] = start == 0U ? -1 : static_cast<std::int32_t>(found - starts.begin());
        }
        document.rows.push_back(indices);
    }
    for (std::size_t at = est_header_size; at < table; ++at) {
        if (!read[at] && bytes[at] != std::byte{0}) ++document.unexplained_bytes;
    }
    if (document.unexplained_bytes > 0U) {
        diagnose(result, ParseSeverity::info, "est.unexplained",
                 std::to_string(document.unexplained_bytes) + " non-zero bytes lie in no program the table names.",
                 est_header_size);
    }
    return result;
}

Result<SefDocument> read_sef(std::span<const std::byte> bytes) {
    Result<SefDocument> result;
    if (bytes.size() < 0x10U || std::memcmp(bytes.data(), "SEF\0", 4U) != 0) {
        diagnose(result, ParseSeverity::error, "stage-cfg.tag", "The bytes do not open with the SEF tag.", 0U);
        return result;
    }
    result.recognized = true;
    auto& document = result.document;
    document.sections_declared = u16(bytes, 4U);
    document.field6 = u16(bytes, 6U);
    // The table runs from +0x08 up to the first section it names.
    const auto first = std::size_t{u32(bytes, 12U)};
    if (first < 0x10U || first > bytes.size() || (first - 8U) % 8U != 0U) {
        diagnose(result, ParseSeverity::error, "sef.table",
                 "The first section offset " + std::to_string(first) + " does not end a table of (value, offset) pairs.",
                 12U);
        return result;
    }
    const auto entries = (first - 8U) / 8U;
    if (entries != document.sections_declared) {
        diagnose(result, ParseSeverity::warning, "sef.count",
                 "The header names " + std::to_string(document.sections_declared) + " sections; the table holds " +
                     std::to_string(entries) + ".",
                 4U);
    }
    std::uint64_t previous = first;
    for (std::size_t index = 0U; index < entries; ++index) {
        const auto at = 8U + index * 8U;
        SefSection section{.value = u32(bytes, at), .offset = u32(bytes, at + 4U), .size = 0U};
        if (section.offset < previous || section.offset > bytes.size()) {
            diagnose(result, ParseSeverity::error, "sef.offset",
                     "Section " + std::to_string(index) + " starts at " + std::to_string(section.offset) +
                         ", before the one ahead of it or past the end.",
                     at + 4U);
            return result;
        }
        previous = section.offset;
        document.sections.push_back(section);
    }
    for (std::size_t index = 0U; index < document.sections.size(); ++index) {
        const auto end = index + 1U < document.sections.size() ? document.sections[index + 1U].offset : bytes.size();
        document.sections[index].size = end - document.sections[index].offset;
    }
    return result;
}

} // namespace dmc::rengine::formats::stage_cfg
