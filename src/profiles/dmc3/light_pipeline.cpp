// Game light tables (LIG2): reading, candidate rules, top-four selection and
// the light matrices, and the model and stage light rigs. See dmc_rengine/profiles/dmc3/light_pipeline.hpp.
#include "dmc_rengine/profiles/dmc3/light_pipeline.hpp"

#include <algorithm>
#include <bit>
#include <cmath>
#include <cstring>
#include <utility>

namespace dmc::rengine::profiles::dmc3::lights {
namespace {

// Little-endian reads through memcpy (no unaligned pointer casts).
template <typename T>
T read_le(std::span<const std::uint8_t> bytes, std::size_t offset) noexcept {
    T value{};
    for (std::size_t i = 0; i < sizeof value; ++i) {
        value = static_cast<T>(value | static_cast<T>(T{bytes[offset + i]} << (8U * i)));
    }
    return value;
}

std::uint16_t read_u16(std::span<const std::uint8_t> bytes, std::size_t offset) noexcept {
    return read_le<std::uint16_t>(bytes, offset);
}

float read_f32(std::span<const std::uint8_t> bytes, std::size_t offset) noexcept {
    return std::bit_cast<float>(read_le<std::uint32_t>(bytes, offset));
}

std::array<float, 3> delta_of(const LightRecord& record, std::array<float, 3> query) noexcept {
    return {record.position[0] - query[0], record.position[1] - query[1], record.position[2] - query[2]};
}

// The stage rig's colour scale: the float with bits 0x3B808081 (about 1/255).
constexpr float kStageColourScale = std::bit_cast<float>(std::uint32_t{0x3B808081u});

}  // namespace

std::optional<LightTable> read_light_table(std::span<const std::uint8_t> bytes) noexcept {
    if (bytes.size() < kTableSize) return std::nullopt;
    if (bytes[0] != 'L' || bytes[1] != 'I' || bytes[2] != 'G' || bytes[3] != '2') return std::nullopt;

    LightTable table{};
    table.version = read_u16(bytes, 0x04);
    for (std::size_t i = 0; i < table.header_words.size(); ++i) {
        table.header_words[i] = read_u16(bytes, 0x06 + i * 2);
    }
    table.static_light_count = static_cast<std::int8_t>(bytes[0x04]);
    for (std::size_t c = 0; c < 3; ++c) {
        table.ambient[c] = static_cast<float>(read_u16(bytes, 0x0C + c * 2));
        table.ambient_mask2[c] = static_cast<float>(read_u16(bytes, 0x06 + c * 2));
    }

    for (std::size_t i = 0; i < kRecordCount; ++i) {
        const auto rec = bytes.subspan(kHeaderSize + i * kRecordSize, kRecordSize);
        LightRecord& r = table.records[i];
        r.index = static_cast<std::uint8_t>(i);
        r.active = std::any_of(rec.begin(), rec.end(), [](std::uint8_t b) { return b != 0; });
        r.flags = rec[0x00];
        r.colour_modifier = rec[0x01];
        r.type = rec[0x02];
        r.position = {read_f32(rec, 0x10), read_f32(rec, 0x14), read_f32(rec, 0x18)};
        r.aux_w = read_f32(rec, 0x1C);
        r.falloff = read_f32(rec, 0x20);
        r.rgb = {read_u16(rec, 0x24), read_u16(rec, 0x26), read_u16(rec, 0x28)};

        if (r.active) {
            const bool finite = std::isfinite(r.position[0]) && std::isfinite(r.position[1]) &&
                                std::isfinite(r.position[2]) && std::isfinite(r.aux_w) &&
                                std::isfinite(r.falloff);
            if (!finite) return std::nullopt;
        }
    }
    return table;
}

std::optional<LightTable> read_light_table(std::span<const std::byte> bytes) noexcept {
    return read_light_table(std::span<const std::uint8_t>{reinterpret_cast<const std::uint8_t*>(bytes.data()), bytes.size()});
}

std::optional<LightTable> find_light_table(std::span<const std::byte> bytes) noexcept {
    return find_light_table(std::span<const std::uint8_t>{reinterpret_cast<const std::uint8_t*>(bytes.data()), bytes.size()});
}

std::optional<LightTable> find_light_table(std::span<const std::uint8_t> bytes) noexcept {
    for (std::size_t offset = 0; offset + kTableSize <= bytes.size(); offset += 4) {
        if (auto table = read_light_table(bytes.subspan(offset))) return table;
    }
    return std::nullopt;
}

bool is_candidate(const LightRecord& record, unsigned mask, ListKind list) noexcept {
    if (!record.active) return false;
    if (record.colour_modifier != 0) return false;
    if (record.type > 1) return false;
    const unsigned list_bit = list == ListKind::main ? 4u : 8u;
    if ((mask & list_bit) == 0) return false;

    unsigned f = record.flags;
    if (list == ListKind::secondary) f &= 0xF7u;
    if ((f & 4u) != 0 && (mask & 0x10u) != 0) return true;
    const unsigned low = f & ~4u;
    switch (low) {
        case 1: return (mask & 3u) != 0;
        case 2: return (mask & 2u) != 0;
        case 3: return (mask & 1u) != 0;
        default: return false;
    }
}

float score(const LightRecord& record, std::array<float, 3> query) noexcept {
    if (record.type == 1) return 1.0f;
    const auto d = delta_of(record, query);
    const float dot = d[0] * d[0] + d[1] * d[1] + d[2] * d[2];
    return std::max(0.0f, 1.0f - dot * record.falloff);
}

SelectedLights select_lights(const LightTable& table, std::array<float, 3> query, unsigned mask,
                             ListKind list, const LightRecord* single) noexcept {
    // Four slots, best first; an empty slot scores 0 and holds no record.
    std::array<const LightRecord*, 4> slot_record{};
    std::array<float, 4> slot_score{};

    for (const LightRecord& r : table.records) {
        if (!is_candidate(r, mask, list)) continue;
        const float s = score(r, query);
        if (!(s > slot_score[3])) continue;
        slot_record[3] = &r;
        slot_score[3] = s;
        for (std::size_t j = 3; j > 0 && slot_score[j - 1] <= slot_score[j]; --j) {
            std::swap(slot_record[j - 1], slot_record[j]);
            std::swap(slot_score[j - 1], slot_score[j]);
        }
    }

    if (single != nullptr && (mask & 2u) != 0) {
        for (std::size_t j = 3; j > 0; --j) {
            slot_record[j] = slot_record[j - 1];
            slot_score[j] = slot_score[j - 1];
        }
        slot_record[0] = single;
        slot_score[0] = score(*single, query);
    }

    SelectedLights out{};
    for (std::size_t slot = 0; slot < 4; ++slot) {
        const LightRecord* r = slot_record[slot];
        if (r == nullptr) continue;
        ++out.count;
        const auto d = delta_of(*r, query);
        const std::size_t row = slot * 4;
        out.index[slot] = r->index;
        out.score[slot] = slot_score[slot];
        out.lc[row + 0] = static_cast<float>(r->rgb[0]);
        out.lc[row + 1] = static_cast<float>(r->rgb[1]);
        out.lc[row + 2] = static_cast<float>(r->rgb[2]);
        out.lc[row + 3] = r->type == 0 ? r->falloff : 1.0f;
        out.lv[row + 0] = d[0];
        out.lv[row + 1] = d[1];
        out.lv[row + 2] = d[2];
        out.lv[row + 3] = r->type == 0 ? r->aux_w : 0.0f;
    }
    return out;
}

ModelRig model_rig(const LightTable& table, const SelectedLights& lights, unsigned mask,
                   int limit) noexcept {
    ModelRig rig{};
    const int n = std::max(0, std::min({lights.count, 3, std::max(limit, 0)}));
    rig.count = n;
    for (int i = 0; i < n; ++i) {
        const std::size_t row = static_cast<std::size_t>(i) * 4;
        const float x = lights.lv[row + 0], y = lights.lv[row + 1], z = lights.lv[row + 2];
        const float w = lights.lv[row + 3];
        const float len2 = x * x + y * y + z * z;
        const float a = w != 0.0f ? std::max(0.0f, 1.0f - len2 * w) : 1.0f;
        auto& colour = rig.colour[static_cast<std::size_t>(i)];
        colour = {lights.lc[row + 0] * a, lights.lc[row + 1] * a, lights.lc[row + 2] * a};
        auto& direction = rig.direction[static_cast<std::size_t>(i)];
        const float len = std::sqrt(len2);
        if (len > 0.0f) direction = {x / len, y / len, z / len};
    }
    rig.ambient = mask == 2u ? table.ambient_mask2 : table.ambient;
    return rig;
}

StageRig stage_rig(const LightTable& table, const SelectedLights& lights, std::array<float, 3> query,
                   unsigned mask, int limit) noexcept {
    StageRig rig{};
    const int n = std::max(0, std::min({lights.count, 4, std::max(limit, 0)}));
    rig.count = n;
    for (int i = 0; i < n; ++i) {
        const auto slot = static_cast<std::size_t>(i);
        const std::size_t row = slot * 4;
        rig.position[slot] = {query[0] + lights.lv[row + 0], query[1] + lights.lv[row + 1],
                              query[2] + lights.lv[row + 2]};
        rig.falloff[slot] = lights.lv[row + 3];
        rig.colour[slot] = {lights.lc[row + 0] * kStageColourScale, lights.lc[row + 1] * kStageColourScale,
                            lights.lc[row + 2] * kStageColourScale};
    }
    rig.ambient = mask == 1u ? table.ambient : table.ambient_mask2;
    return rig;
}

std::size_t candidate_count(const LightTable& table, unsigned mask, ListKind list) noexcept {
    return static_cast<std::size_t>(std::count_if(table.records.begin(), table.records.end(),
                                                  [&](const LightRecord& r) { return is_candidate(r, mask, list); }));
}

std::string describe(const LightTable& table) noexcept {
    std::size_t active = 0, type0 = 0, type1 = 0, modified = 0;
    for (const LightRecord& r : table.records) {
        if (!r.active) continue;
        ++active;
        if (r.type == 0) ++type0;
        if (r.type == 1) ++type1;
        if (r.colour_modifier != 0) ++modified;
    }
    try {
        return "LIG2 v" + std::to_string(table.version) + ": " + std::to_string(active) + " of " +
               std::to_string(kRecordCount) + " records active (type 0: " + std::to_string(type0) +
               ", type 1: " + std::to_string(type1) + "; " + std::to_string(modified) +
               " with a colour modifier)";
    } catch (...) {
        return {};
    }
}

} // namespace dmc::rengine::profiles::dmc3::lights
