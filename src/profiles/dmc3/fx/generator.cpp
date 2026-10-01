#include "dmc_rengine/profiles/dmc3/fx/generator.hpp"

#include <algorithm>
#include <cmath>
#include <cstring>



namespace dmc::rengine::profiles::dmc3::fx::generator {
namespace {

constexpr float kTwoPi = 6.28318501F;  // 0x140371920
constexpr float kTick = 1.0F;          // dt: one 60 Hz update

using Mat = std::array<float, 16>;

[[nodiscard]] float f32(std::span<const std::uint8_t> b, std::size_t at) noexcept {
    std::uint32_t v = 0U;
    for (std::size_t i = 0U; i < 4U; ++i) v |= static_cast<std::uint32_t>(b[at + i]) << (8U * i);
    float out = 0.0F;
    std::memcpy(&out, &v, sizeof out);
    return out;
}
[[nodiscard]] std::int32_t i32(std::span<const std::uint8_t> b, std::size_t at) noexcept {
    std::uint32_t v = 0U;
    for (std::size_t i = 0U; i < 4U; ++i) v |= static_cast<std::uint32_t>(b[at + i]) << (8U * i);
    return static_cast<std::int32_t>(v);
}
[[nodiscard]] std::uint16_t u16(std::span<const std::uint8_t> b, std::size_t at) noexcept {
    return static_cast<std::uint16_t>(b[at] | (b[at + 1U] << 8U));
}

[[nodiscard]] Mat mul(const Mat& a, const Mat& b) {  // v * a * b
    Mat r{};
    for (int i = 0; i < 4; ++i) {
        for (int j = 0; j < 4; ++j) {
            float s = 0.0F;
            for (int k = 0; k < 4; ++k) s += a[static_cast<std::size_t>(i * 4 + k)] * b[static_cast<std::size_t>(k * 4 + j)];
            r[static_cast<std::size_t>(i * 4 + j)] = s;
        }
    }
    return r;
}

[[nodiscard]] Mat rotation(float rx, float ry, float rz) {
    return local_matrix_zyx({0.0F, 0.0F, 0.0F}, {rx, ry, rz}).values;
}

}  // namespace

std::optional<Def> parse(std::span<const std::uint8_t> b) {
    if (b.size() < 0x5AU || b[1] > 2U) return std::nullopt;
    Def d;
    d.follow_parent = b[0] == 1U;
    d.motion = b[1];
    d.clip = u16(b, 2U);
    d.speed = f32(b, 0x04U);
    d.deceleration = f32(b, 0x08U);
    d.yaw = f32(b, 0x10U);
    d.roll = f32(b, 0x14U);
    d.first_delay = i32(b, 0x18U);
    d.interval = i32(b, 0x1CU);
    d.life = i32(b, 0x20U);
    d.child_kind = b[0x24U];
    d.child_id = u16(b, 0x26U);
    d.angle = f32(b, 0x28U);
    d.angle_roll = f32(b, 0x2CU);
    d.endless = b[0x30U] != 0U;
    d.interval_mask = static_cast<std::uint32_t>(i32(b, 0x34U));
    d.scale_start = f32(b, 0x38U);
    d.scale_end = f32(b, 0x3CU);
    d.scale_steps = u16(b, 0x40U);
    d.jitter = {f32(b, 0x44U), f32(b, 0x48U), f32(b, 0x4CU)};
    d.scale_low = f32(b, 0x50U);
    d.scale_high = f32(b, 0x54U);
    d.angle_jitter = b[0x59U];
    return d;
}

std::optional<Clip> parse_clip(std::span<const std::uint8_t> b) {
    if (b.size() < 20U) return std::nullopt;
    const std::uint32_t count = static_cast<std::uint32_t>(i32(b, 0U));
    if (count < 2U || count > 64U || 8U + count * 12U > b.size()) return std::nullopt;
    Clip clip;
    for (std::uint32_t i = 0U; i < count; ++i) {
        clip.points.push_back({f32(b, 8U + i * 12U), f32(b, 12U + i * 12U), f32(b, 16U + i * 12U)});
    }
    return clip;
}

std::array<float, 3> evaluate(const Clip& clip, float t) {
    const int n = static_cast<int>(clip.points.size()) - 1;
    const float s = std::min(1.0F, std::max(0.0F, t)) * static_cast<float>(n + 2) - 1.0F;
    std::array<float, 3> out{};
    for (int j = -3; j <= n + 3; ++j) {
        const float u = std::fabs(s - static_cast<float>(j));
        float w = 0.0F;
        if (u < 1.0F) w = (3.0F * u * u * u - 6.0F * u * u + 4.0F) / 6.0F;
        else if (u < 2.0F) w = (2.0F - u) * (2.0F - u) * (2.0F - u) / 6.0F;
        if (w == 0.0F) continue;
        const auto& p = clip.points[static_cast<std::size_t>(std::min(std::max(j, 0), n))];
        for (std::size_t a = 0U; a < 3U; ++a) out[a] += w * p[a];
    }
    return out;
}

Simulation::Simulation(const Def& def, Random random, const Clip* clip)
    : def_(def), random_(std::move(random)) {
    if (clip != nullptr) {
        clip_ = *clip;
        has_clip_ = true;
    }
    // Init 0x1402EBC10: timers, drift state, and (with a scale ramp) one random scale factor.
    life_ = static_cast<float>(def_.life);
    timer_ = static_cast<float>(def_.first_delay);
    speed_ = def_.motion == 0U ? def_.speed : 0.0F;
    clip_time_ = 0.0F;
    if (def_.motion == 1U && has_clip_) position_ = evaluate(clip_, 0.0F);
    parent_ = {1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1};
}

bool Simulation::step(const Matrix4& world, std::vector<Spawn>* out) {
    if (ended_) return false;
    ++tick_;
    // Random scale factor: init when there is a ramp, every update without one.
    const auto random_scale = [this]() {
        const float range = def_.scale_high - def_.scale_low;
        const int whole = static_cast<int>(range);
        if (whole == 0) return 1.0F;
        const int tenths = static_cast<int>(range * 10.0F);
        const std::uint32_t n = static_cast<std::uint32_t>(tenths < 0 ? -tenths : tenths);
        if (n == 0U) return 1.0F;
        return static_cast<float>(random_() % n) * (whole > 0 ? 0.1F : -0.1F);
    };
    if (tick_ == 1U) {
        if (def_.scale_steps != 0U) scale_random_ = random_scale();
        return true;  // the state-0 tick only initialises
    }

    // 0x1402EBDC0: life (unless endless).
    if (!def_.endless) {
        life_ -= kTick;
        if (0.0F > life_) {
            ended_ = true;
            return false;
        }
    }
    // Scale: a ramp from start to end over `steps` ticks, or a fresh random factor.
    float scale = def_.scale_start;
    if (def_.scale_steps != 0U) {
        const float range = def_.scale_end - scale;
        const float ramp = range / static_cast<float>(def_.scale_steps) * counter_;
        float next = scale + ramp;
        next = range > 0.0F ? std::min(def_.scale_end, next) : std::max(def_.scale_end, next);
        counter_ += kTick;
        scale = next;
    } else {
        scale_random_ = random_scale();
    }
    // Yaw with a random spread, wrapped to +-180 degrees.
    float angle = def_.angle;
    if (def_.angle_jitter != 0U) {
        const std::uint32_t b = def_.angle_jitter;
        const float r = static_cast<float>(random_() % b) - static_cast<float>(b >> 1U);
        angle += r;
        if (angle > 180.0F) angle += -360.0F;
        if (-180.0F > angle) angle += 360.0F;
    }
    scale *= scale_random_;
    const float yaw = angle / 360.0F * kTwoPi;
    const float roll = def_.angle_roll / 360.0F * kTwoPi;

    // Local matrix: Rz*Ry*Rx(0, yaw, roll), translation = drift position, uniform scale rows.
    Mat local = rotation(0.0F, yaw, roll);
    local[12] = position_[0];
    local[13] = position_[1];
    local[14] = position_[2];
    for (int row = 0; row < 3; ++row) {
        for (int col = 0; col < 4; ++col) local[static_cast<std::size_t>(row * 4 + col)] *= scale;
    }
    parent_ = mul(local, world.values);

    timer_ -= kTick;
    if (!(0.0F < timer_)) {
        const float extra = static_cast<float>((random_() & def_.interval_mask) +
                                               static_cast<std::uint32_t>(def_.interval));
        timer_ = extra;
        std::array<float, 3> jitter{};
        for (std::size_t a = 0U; a < 3U; ++a) {
            const int n = static_cast<int>(def_.jitter[a]);
            if (n != 0) {
                const std::uint32_t un = static_cast<std::uint32_t>(n);
                jitter[a] = static_cast<float>(random_() % un) - static_cast<float>(n >> 1);
            }
        }
        Spawn spawn;
        spawn.tick = tick_;
        spawn.kind = def_.child_kind;
        spawn.id = def_.child_id;
        spawn.follow = def_.follow_parent;
        // 0x1402EC256 resets the local scratch matrix to identity before the
        // jitter is added, so a following child only carries the jitter.
        spawn.matrix = def_.follow_parent ? Mat{1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1} : parent_;
        for (std::size_t a = 0U; a < 3U; ++a) spawn.matrix[12U + a] += jitter[a];
        if (out != nullptr) out->push_back(spawn);
    }

    // Drift (motion 0): along row 0 of Rz*Ry*Rx(0, yaw, roll), the speed decays.
    if (def_.motion == 0U) {
        const Mat dir = rotation(0.0F, def_.yaw / 360.0F * kTwoPi, def_.roll / 360.0F * kTwoPi);
        const float distance = kTick * speed_;
        for (std::size_t a = 0U; a < 3U; ++a) position_[a] += distance * dir[a];
        speed_ -= def_.deceleration * kTick;
    } else if (def_.motion == 1U && has_clip_) {
        // Motion 1: the position is the clip at t, then t advances by +0x04 per tick.
        position_ = evaluate(clip_, clip_time_);
        clip_time_ = def_.speed * kTick + clip_time_;
    }
    return true;
}

}  // namespace dmc::rengine::profiles::dmc3::fx::generator
