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

} // namespace dmc::rengine::formats::stage_cfg
