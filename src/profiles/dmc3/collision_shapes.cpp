#include "dmc_rengine/profiles/dmc3/collision_shapes.hpp"

#include <cmath>
#include <cstring>

namespace dmc::rengine::profiles::dmc3::collision {
namespace {

[[nodiscard]] float f32(std::span<const std::uint8_t> bytes, std::size_t offset) noexcept {
    float value = 0.0F;
    std::memcpy(&value, bytes.data() + offset, sizeof(value));
    return value;
}

}  // namespace

bool looks_like_shape_table(std::span<const std::uint8_t> bytes) noexcept {
    if (bytes.size() < kShapeRecordSize || bytes.size() % kShapeRecordSize != 0U) return false;
    std::size_t known = 0U;
    for (std::size_t o = 0U; o < bytes.size(); o += kShapeRecordSize) {
        if (bytes[o] > 6U) return false;
        for (std::size_t k = 1U; k < 0x10U; ++k) {
            if (bytes[o + k] != 0U) return false;
        }
        for (std::size_t k = 0x10U; k < kShapeRecordSize; k += 4U) {
            const float v = f32(bytes, o + k);
            if (!std::isfinite(v) || std::fabs(v) > 1.0e6F) return false;
        }
        known += bytes[o] >= 2U && bytes[o] <= 4U ? 1U : 0U;
    }
    return known != 0U;
}

std::vector<Shape> parse_shapes(std::span<const std::uint8_t> bytes) {
    std::vector<Shape> out;
    if (bytes.size() % kShapeRecordSize != 0U) return out;
    out.reserve(bytes.size() / kShapeRecordSize);
    for (std::size_t o = 0U; o + kShapeRecordSize <= bytes.size(); o += kShapeRecordSize) {
        Shape s;
        s.type = bytes[o];
        for (std::size_t k = 0U; k < s.raw.size(); ++k) s.raw[k] = f32(bytes, o + 0x10U + k * 4U);
        s.a = {s.raw[0], s.raw[1], s.raw[2]};
        switch (static_cast<ShapeType>(s.type)) {
        case ShapeType::Sphere:
            s.radius = s.raw[4];
            break;
        case ShapeType::Box:
            s.b = {s.raw[3], s.raw[4], s.raw[5]};
            s.size = {s.raw[6], s.raw[7], s.raw[8]};
            break;
        case ShapeType::Capsule:
            s.b = {s.raw[4], s.raw[5], s.raw[6]};
            s.radius = s.raw[8];
            break;
        default:
            break;
        }
        out.push_back(s);
    }
    return out;
}

bool looks_like_attack_index(std::span<const std::uint8_t> bytes,
                             std::optional<std::size_t> shape_count) noexcept {
    if (bytes.size() < 8U || bytes.size() % 4U != 0U) return false;
    std::size_t used = 0U;
    for (std::size_t o = 0U; o < bytes.size(); o += 4U) {
        const std::size_t shape = bytes[o + 2U] | (bytes[o + 3U] << 8U);
        if (bytes[o] > 7U || bytes[o + 1U] > 200U) return false;
        if (bytes[o] == 0U) continue;
        ++used;
        if (shape_count && shape >= *shape_count) return false;
    }
    return used != 0U;
}

std::vector<AttackEntry> parse_attack_index(std::span<const std::uint8_t> bytes) {
    std::vector<AttackEntry> out;
    for (std::size_t o = 0U; o + 4U <= bytes.size(); o += 4U) {
        out.push_back({bytes[o], bytes[o + 1U],
                       static_cast<std::uint16_t>(bytes[o + 2U] | (bytes[o + 3U] << 8U))});
    }
    return out;
}

}  // namespace dmc::rengine::profiles::dmc3::collision
