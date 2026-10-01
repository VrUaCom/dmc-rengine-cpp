#pragma once

#include <array>
#include <cstdint>
#include <functional>
#include <optional>
#include <span>
#include <vector>

#include "dmc_rengine/profiles/dmc3/fx/fx_types.hpp"

// G records: CGenerator (vtable 0x1405073F8). A generator is an invisible
// effect that spawns child effects (P, E, G or V, by the same dispatch kind
// the V entries use) at a random interval, with a random jitter, an optional
// scale ramp and an optional drift of its own. Reverse authority: dmc3.exe tick
// 0x1402EC840 (state byte +8: 0 init 0x1402EBC10, 1 update 0x1402EBDC0), child
// creators 0x1403127B0 (P), 0x1402E3B90 (E), 0x1403244E0 (V), the direction
// helper 0x1402EC9E0; the port was compared with emulated runs of those
// functions (docs/research/dmc3-generator-g-records-2026-10-01.md).
namespace dmc::rengine::profiles::dmc3::fx::generator {

struct Def final {
    bool follow_parent{};          // +0x00: children keep following the generator
    std::uint8_t motion{};         // +0x01: 0 drift along yaw / roll, 1 C clip, 2 still
    std::uint16_t clip{};          // +0x02: C record id for motion 1
    float speed{};                 // +0x04: drift speed per tick
    float deceleration{};          // +0x08: speed -= deceleration * dt
    float yaw{};                   // +0x10 degrees
    float roll{};                  // +0x14 degrees
    std::int32_t first_delay{};    // +0x18 ticks before the first spawn
    std::int32_t interval{};       // +0x1C base ticks between spawns
    std::int32_t life{};           // +0x20 ticks
    std::uint8_t child_kind{};     // +0x24: 0 P, 1 E, 2 G, 3 V
    std::uint16_t child_id{};      // +0x26
    float angle{};                 // +0x28 degrees (child yaw base)
    float angle_roll{};            // +0x2C degrees
    bool endless{};                // +0x30 != 0: life never runs out
    std::uint32_t interval_mask{}; // +0x34: random extra ticks = rand & mask
    float scale_start{1.0F};       // +0x38
    float scale_end{1.0F};         // +0x3C
    std::uint16_t scale_steps{};   // +0x40: ticks of the scale ramp (0: random scale)
    std::array<float, 3> jitter{}; // +0x44: spawn offset range per axis
    float scale_low{};             // +0x50
    float scale_high{};            // +0x54
    std::uint8_t angle_jitter{};   // +0x59: random yaw spread (degrees)
};

// 96-byte record, mode byte +1 in 0..2.
[[nodiscard]] std::optional<Def> parse(std::span<const std::uint8_t> record);

// C record (CEffectClip path): u32 point count, float rate, then 12-byte
// points; evaluated as a uniform cubic B-spline (0x1402D3660).
struct Clip final {
    std::vector<std::array<float, 3>> points;
};
[[nodiscard]] std::optional<Clip> parse_clip(std::span<const std::uint8_t> record);
// Position of the clip at t in [0, 1]: s = t * (n + 2) - 1 over n + 1 points
// with clamped indices, weights (3|u|^3 - 6u^2 + 4) / 6 and (2 - |u|)^3 / 6.
[[nodiscard]] std::array<float, 3> evaluate(const Clip& clip, float t);

// One child creation. `matrix` is what the creator receives (row vectors):
// the generator's composed world translated by the jitter, or, with
// follow_parent, an identity matrix translated by the jitter that the child
// composes with `parent_world()` of every later moment.
struct Spawn final {
    std::uint32_t tick{};   // generator tick (1 = init tick, first spawn >= 2)
    std::uint8_t kind{};    // 0 P, 1 E, 2 G, 3 V
    std::uint16_t id{};
    bool follow{};
    std::array<float, 16> matrix{};
};

class Simulation final {
public:
    using Random = std::function<std::uint32_t()>;
    // `clip` is the C record a motion-1 generator follows (null otherwise).
    Simulation(const Def& def, Random random, const Clip* clip = nullptr);

    // One 60 Hz tick under `world` (the generator's own world matrix); spawns
    // appended to `out`. Returns false once the generator ended.
    bool step(const Matrix4& world, std::vector<Spawn>* out);
    [[nodiscard]] bool ended() const noexcept { return ended_; }
    [[nodiscard]] std::uint32_t tick() const noexcept { return tick_; }
    // The generator's composed world (obj +0x120) of the last tick.
    [[nodiscard]] const std::array<float, 16>& parent_world() const noexcept { return parent_; }

private:
    Def def_;
    Random random_;
    std::uint32_t tick_{};
    bool ended_{};
    float life_{};
    float timer_{};
    float counter_{};
    float scale_random_{1.0F};
    float speed_{};
    float clip_time_{};
    std::array<float, 3> position_{};
    std::array<float, 16> parent_{};
    Clip clip_;
    bool has_clip_{};
};

}  // namespace dmc::rengine::profiles::dmc3::fx::generator
