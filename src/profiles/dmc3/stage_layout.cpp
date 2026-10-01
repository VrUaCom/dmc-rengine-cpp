#include "dmc_rengine/profiles/dmc3/stage_layout.hpp"

#include <cctype>
#include <cmath>
#include <cstddef>
#include <string>

namespace dmc::rengine::profiles::dmc3::stage_layout {

std::vector<float> numbers(std::string_view line) {
    std::vector<float> out;
    std::string token;
    const auto flush = [&] {
        if (token.empty()) return;
        try {
            std::size_t used = 0U;
            const float v = std::stof(token, &used);
            if (used == token.size()) out.push_back(v);
        } catch (...) {
        }
        token.clear();
    };
    for (const char c : line) {
        if (c == ';') break;
        if (c == ',' || c == ' ' || c == '\t' || c == '\r') {
            flush();
        } else {
            token.push_back(c);
        }
    }
    flush();
    return out;
}

namespace {

bool blank(char c) noexcept { return c == ' ' || c == '\t'; }

}  // namespace

GameLayout parse_game(std::string_view text) {
    GameLayout out;
    GameSet* current = nullptr;
    bool after_beff = false;  // the parser's r14b: `epos` fills the eff or beff slot
    std::size_t start = 0U;
    while (start < text.size()) {
        auto end = text.find('\n', start);
        if (end == std::string_view::npos) end = text.size();
        auto line = text.substr(start, end - start);
        start = end + 1U;
        while (!line.empty() && blank(line.front())) line.remove_prefix(1U);
        if (line.starts_with("$") || line.starts_with("# GAME_END")) break;
        if (line.starts_with("# SET")) {
            const auto rest = line.substr(5U);
            std::string kind;
            std::size_t i = 0U;
            while (i < rest.size() && blank(rest[i])) ++i;
            while (i < rest.size() && !blank(rest[i])) ++i;  // set number
            while (i < rest.size() && blank(rest[i])) ++i;
            while (i < rest.size() && rest[i] > ' ' && rest[i] != ';') kind.push_back(rest[i++]);
            out.sets.push_back(GameSet{});
            out.sets.back().kind = std::move(kind);
            current = &out.sets.back();
            after_beff = false;
            continue;
        }
        if (current == nullptr) continue;
        const auto key_end = line.find_first_of(" \t");
        if (key_end == std::string_view::npos) continue;
        const auto key = line.substr(0U, key_end);
        const auto values = numbers(line.substr(key_end));
        if (key == "model" && !values.empty()) {
            current->model = static_cast<int>(values[0]);
        } else if (key == "pos" && values.size() >= 3U) {
            current->pos = {values[0], values[1], values[2]};
        } else if (key == "rot" && values.size() >= 3U) {
            current->rot = {values[0], values[1], values[2]};
        } else if (key == "scale" && values.size() >= 3U) {
            current->scale = {values[0], values[1], values[2]};
        } else if (key == "eff" || key == "beff") {
            // "V 98": a kind letter and a decimal id.
            const auto rest = line.substr(key_end);
            std::size_t i = 0U;
            while (i < rest.size() && blank(rest[i])) ++i;
            if (i < rest.size() && std::isalpha(static_cast<unsigned char>(rest[i])) != 0) {
                after_beff = key == "beff";
                (after_beff ? current->broken_effect_kind : current->effect_kind) = rest[i];
                const auto id = numbers(rest.substr(i + 1U));
                if (!id.empty()) (after_beff ? current->broken_effect_id : current->effect_id) = static_cast<int>(id[0]);
            }
        } else if (key == "epos" && values.size() >= 3U) {
            (after_beff ? current->broken_effect_pos : current->effect_pos) = Vec3{values[0], values[1], values[2]};
        } else if (key == "bmodel" && !values.empty()) {
            current->broken_model = static_cast<int>(values[0]);
        } else if (key == "remain") {
            auto rest = line.substr(key_end);
            rest = rest.substr(0U, rest.find(';'));
            current->remain = rest.find("on2") != std::string_view::npos ? 2U
                              : rest.find("on") != std::string_view::npos ? 1U : 0U;
        } else if (key == "uv" && values.size() >= 4U) {
            current->uv.push_back({values[0], values[1], values[2], values[3]});
        } else if (key == "cam_init" && values.size() >= 3U) {
            out.has_camera = true;
            out.camera = {values[0], values[1], values[2]};
        }
    }
    return out;
}

Vec3 place_normal(const GameSet& set, const Vec3& normal) noexcept {
    constexpr float kRad = 3.14159265358979F / 180.0F;
    const float cx = std::cos(set.rot.x * kRad), sx = std::sin(set.rot.x * kRad);
    const float cy = std::cos(set.rot.y * kRad), sy = std::sin(set.rot.y * kRad);
    const float cz = std::cos(set.rot.z * kRad), sz = std::sin(set.rot.z * kRad);
    Vec3 v = normal;
    v = {v.x, cx * v.y - sx * v.z, sx * v.y + cx * v.z};
    v = {cy * v.x + sy * v.z, v.y, -sy * v.x + cy * v.z};
    return {cz * v.x - sz * v.y, sz * v.x + cz * v.y, v.z};
}

ObjectState object_state(const GameSet& set, bool broken) noexcept {
    ObjectState out;
    if (broken && set.kind == "BREAK") {
        out.model = set.broken_model;
        out.effect_kind = set.broken_effect_kind;
        out.effect_id = set.broken_effect_id;
        out.effect_pos = set.broken_effect_pos;
        out.once = true;
        return out;
    }
    out.model = set.model;
    out.effect_kind = set.effect_kind;
    out.effect_id = set.effect_id;
    out.effect_pos = set.effect_pos;
    return out;
}

Vec3 place_point(const GameSet& set, const Vec3& point) noexcept {
    const Vec3 r = place_normal(set, {point.x * set.scale.x, point.y * set.scale.y, point.z * set.scale.z});
    return {r.x + set.pos.x, r.y + set.pos.y, r.z + set.pos.z};
}

}  // namespace dmc::rengine::profiles::dmc3::stage_layout
