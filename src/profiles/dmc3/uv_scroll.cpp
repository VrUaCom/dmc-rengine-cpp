#include "dmc_rengine/profiles/dmc3/uv_scroll.hpp"

#include <algorithm>
#include <cmath>
#include <numbers>
#include <string>


namespace dmc::rengine::profiles::dmc3::motion {
namespace {

// Tokenizer 0x140322AB0: separators are NUL, tab, LF, CR, space and ','; ';'
// skips to the end of the line; '$' (or NUL) ends the text.
class Tokens final {
public:
    explicit Tokens(std::string_view text) : text_(text) {}

    [[nodiscard]] std::optional<std::string_view> next() {
        while (pos_ < text_.size()) {
            const char c = text_[pos_];
            if (c == '$' || c == '\0') {
                pos_ = text_.size();
                return std::nullopt;
            }
            if (c == ';') {
                while (pos_ < text_.size() && text_[pos_] != '\n') ++pos_;
                continue;
            }
            if (separator(c)) {
                ++pos_;
                continue;
            }
            break;
        }
        if (pos_ >= text_.size()) return std::nullopt;
        const auto begin = pos_;
        while (pos_ < text_.size() && !separator(text_[pos_]) && text_[pos_] != '$' &&
               text_[pos_] != ';' && text_[pos_] != '\0') {
            ++pos_;
        }
        return text_.substr(begin, pos_ - begin);
    }

    [[nodiscard]] int next_int() {
        const auto token = next();
        if (!token) return 0;
        try {
            return std::stoi(std::string{*token});
        } catch (...) {
            return 0;
        }
    }

    [[nodiscard]] float next_float() {
        const auto token = next();
        if (!token) return 0.0F;
        try {
            return std::stof(std::string{*token});
        } catch (...) {
            return 0.0F;
        }
    }

private:
    [[nodiscard]] static bool separator(char c) noexcept {
        return c == '\t' || c == '\n' || c == '\r' || c == ' ' || c == ',';
    }

    std::string_view text_;
    std::size_t pos_{};
};

// 0x14030ABE0: "ScrlNo n ScrlType t" then keys until "End>".
std::optional<ScrollRecord> parse_block(Tokens& tokens) {
    if (tokens.next() != std::string_view{"ScrlNo"}) return std::nullopt;
    ScrollRecord record;
    record.number = static_cast<std::uint8_t>(tokens.next_int());
    if (tokens.next() != std::string_view{"ScrlType"}) return std::nullopt;
    record.type = static_cast<std::uint8_t>(tokens.next_int());
    while (const auto token = tokens.next()) {
        const auto key = *token;
        if (key == "End>") break;
        if (key == "TexNo") {
            record.texture = static_cast<std::int16_t>(tokens.next_int());
        } else if (key == "JntNo") {
            record.joint = static_cast<std::int16_t>(tokens.next_int());
        } else if (key == "DirUV") {
            const auto u = tokens.next().value_or("");
            record.direction[0] = u == "left" ? 1 : u == "right" ? -1 : 0;
            const auto v = tokens.next().value_or("");
            record.direction[1] = v == "up" ? 1 : v == "down" ? -1 : 0;
        } else if (key == "RateUV") {
            record.rate[0] = tokens.next_float();
            record.rate[1] = tokens.next_float();
        } else if (key == "TimeUV") {
            record.time[0] = tokens.next_float();
            record.time[1] = tokens.next_float();
        } else if (key == "InterUV") {
            record.interval[0] = tokens.next_float();
            record.interval[1] = tokens.next_float();
        } else if (key == "TurnTimeUV") {
            record.turn_time[0] = tokens.next_float();
            record.turn_time[1] = tokens.next_float();
            record.has_turn_time = true;
        } else if (key == "MinimumUV") {
            record.minimum[0] = tokens.next_float();
            record.minimum[1] = tokens.next_float();
            record.has_minimum = true;
        } else if (key == "RndUV") {
            for (auto& value : record.random) value = tokens.next_float();
            record.has_random = true;
        }
    }
    return record;
}

[[nodiscard]] float wrap(float value) noexcept { return value - std::floor(value); }


}  // namespace

bool looks_like_tsc(std::string_view text) {
    Tokens tokens{text.substr(0U, std::min<std::size_t>(text.size(), 256U))};
    return tokens.next() == std::string_view{".TSC"};
}

std::vector<ScrollRecord> parse_tsc(std::string_view text) {
    std::vector<ScrollRecord> out;
    Tokens tokens{text};
    // 0x14030A9B0: ".TSC", then "#" -> RELATIVE (100) / ABSOLUTE (200, unread);
    // in RELATIVE "<Start" parses a block and "<Finish>" returns to state 0.
    if (tokens.next() != std::string_view{".TSC"}) return out;
    int state = 0;
    while (const auto token = tokens.next()) {
        if (state == 0) {
            if (*token == "#") state = 1;
        } else if (state == 1) {
            if (*token == "RELATIVE") state = 100;
            else if (*token == "ABSOLUTE") state = 200;
        } else if (state == 100) {
            if (*token == "<Finish>") {
                state = 0;
            } else if (*token == "<Start") {
                auto record = parse_block(tokens);
                if (!record) break;
                out.push_back(*record);
            }
        }
    }
    return out;
}

ScrollState start_scroll(const ScrollRecord& record) noexcept {
    ScrollState state;
    state.direction = record.direction;
    // Record init (0x14030ABE0): counters start at 0 unless InterUV /
    // TurnTimeUV primed them.
    state.interval_left = record.interval;
    if (record.has_turn_time) state.turn_left = record.turn_time;
    return state;
}

namespace {

constexpr float kPi = std::numbers::pi_v<float>;

// 0x14030C2D0 / 0x14030C4F0: InterUV countdown, then phase += dir * rate.
bool linear_step(const ScrollRecord& record, ScrollState& state, std::size_t axis, float rate) {
    state.interval_left[axis] -= 1.0F;
    if (0.0F < state.interval_left[axis]) return false;
    state.interval_left[axis] = record.interval[axis];
    state.phase[axis] = wrap(state.phase[axis] +
                             static_cast<float>(state.direction[axis]) * rate);
    state.output[axis] = static_cast<std::int32_t>(state.phase[axis] * 4096.0F);
    return true;
}

// 0x14030C850 / 0x14030CC30: same countdown, cosine ease, MinimumUV drift.
bool eased_step(const ScrollRecord& record, ScrollState& state, std::size_t axis, float rate) {
    const float dir = static_cast<float>(state.direction[axis]);
    if (record.time[axis] == 0.0F || dir == 0.0F) {
        state.output[axis] = 0;
        return false;
    }
    state.interval_left[axis] -= 1.0F;
    if (0.0F < state.interval_left[axis]) return false;
    state.interval_left[axis] = record.interval[axis];
    state.phase[axis] = wrap(state.phase[axis] + dir * rate);
    float value = (std::cos((1.0F - state.phase[axis]) * kPi) + 1.0F) * 0.5F;
    if (record.has_minimum) {
        state.drift[axis] = wrap(state.drift[axis] + dir * record.minimum[axis]);
        value += state.drift[axis];
    }
    state.output[axis] = static_cast<std::int32_t>(value * 4096.0F);
    return true;
}

// Types 4/5 (0x14030B820 / 0x14030B980): after a step, the turn counter
// drops and, at <= 0, reloads with TurnTimeUV and reverses DirUV.
void turn(const ScrollRecord& record, ScrollState& state, std::size_t axis) {
    state.turn_left[axis] -= 1.0F;
    if (0.0F < state.turn_left[axis]) return;
    state.turn_left[axis] = record.turn_time[axis];
    state.direction[axis] = static_cast<std::int16_t>(-state.direction[axis]);
}

}  // namespace

void step_scroll(const ScrollRecord& record, ScrollState& state, float facing) noexcept {
    ++state.frames;
    for (std::size_t axis = 0U; axis < 2U; ++axis) {
        const float dir = static_cast<float>(state.direction[axis]);
        const float time = record.time[axis];
        switch (record.type) {
        case 0U:
            (void)linear_step(record, state, axis, record.rate[axis]);
            break;
        case 1U:
        case 4U:
            // 0x14030C710: TimeUV 0 or stay -> offset 0.
            if (time == 0.0F || dir == 0.0F) {
                state.output[axis] = 0;
                break;
            }
            if (linear_step(record, state, axis, 1.0F / time) && record.type == 4U) {
                turn(record, state, axis);
            }
            break;
        case 2U:
            (void)eased_step(record, state, axis, record.rate[axis]);
            break;
        case 3U:
        case 5U:
            if (eased_step(record, state, axis, time != 0.0F ? 1.0F / time : 0.0F) &&
                record.type == 5U) {
                turn(record, state, axis);
            }
            break;
        case 10U: {
            // 0x14030BB50: s = (facing + 1) / 2, folded to 1 - s, times RateUV.
            float s = (facing + 1.0F) * 0.5F;
            s = s >= 0.0F ? 1.0F - s : 1.0F + s;
            state.output[axis] = (record.rate[axis] == 0.0F || dir == 0.0F)
                ? 0
                : static_cast<std::int32_t>(s * record.rate[axis] * dir * 4096.0F);
            break;
        }
        default:
            break;
        }
    }
}

std::optional<std::array<float, 2>> scroll_offset(const ScrollRecord& record,
                                                  float frames) noexcept {
    if (!std::isfinite(frames)) return std::nullopt;
    if (record.type > 5U && record.type != 10U) return std::nullopt;
    auto state = start_scroll(record);
    const auto count = static_cast<std::uint32_t>(std::max(frames, 0.0F));
    for (std::uint32_t i = 0U; i < count; ++i) step_scroll(record, state);
    // 0x140309570 masks the u16 output with 0xFFF.
    return std::array<float, 2>{static_cast<float>(state.output[0] & 0xFFF) / 4096.0F,
                                static_cast<float>(state.output[1] & 0xFFF) / 4096.0F};
}

}  // namespace dmc::rengine::profiles::dmc3::motion
