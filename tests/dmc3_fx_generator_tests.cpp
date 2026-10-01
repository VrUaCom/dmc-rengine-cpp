// G records (CGenerator) against emulated runs of dmc3.exe on synthetic
// records: the spawn events (tick, child kind / id, follow flag, 16-float
// matrix) of 100 ticks under a rotated, translated world, with an injected
// linear congruential generator in place of the shared 0x140059390 draw; one
// scenario follows a C clip (B-spline), and the spline is also sampled.
#include "dmc_rengine/profiles/dmc3/fx/generator.hpp"

#include <array>
#include <cassert>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <string>
#include <vector>

#include "dmc3_fx_generator_truth.inc"

namespace {

using namespace dmc::rengine::profiles::dmc3::fx;

std::vector<std::uint8_t> from_hex(const char* hex) {
    std::vector<std::uint8_t> out;
    for (const char* p = hex; p[0] != '\0' && p[1] != '\0'; p += 2) {
        out.push_back(static_cast<std::uint8_t>(std::stoi(std::string(p, 2U), nullptr, 16)));
    }
    return out;
}

bool near(float a, float b, float tol) { return std::fabs(a - b) <= tol * (1.0F + std::fabs(b)); }

#define EXPECT(cond, ...)                         \
    do {                                          \
        if (!(cond)) {                            \
            std::printf(__VA_ARGS__);             \
            std::printf("\n");                    \
            assert(false);                        \
        }                                         \
    } while (false)

template <class Truth>
void check(const char* label) {
    const auto def = generator::parse(from_hex(Truth::kRecordHex));
    EXPECT(def.has_value(), "%s: parse", label);
    std::optional<generator::Clip> clip;
    if (Truth::kClipHex[0] != '\0') {
        clip = generator::parse_clip(from_hex(Truth::kClipHex));
        EXPECT(clip.has_value(), "%s: clip", label);
    }
    std::uint32_t state = Truth::kSeed;
    generator::Simulation sim(
        *def, [&state]() { state = state * 1664525U + 1013904223U; return state; },
        clip.has_value() ? &*clip : nullptr);
    Matrix4 world;
    for (int i = 0; i < 16; ++i) world.values[static_cast<std::size_t>(i)] = Truth::kWorld[i];
    std::vector<generator::Spawn> spawns;
    for (int t = 0; t < Truth::kTicks; ++t) sim.step(world, &spawns);
    EXPECT(static_cast<int>(spawns.size()) == Truth::kEvents, "%s: %zu spawns, expected %d", label,
           spawns.size(), Truth::kEvents);
    for (int i = 0; i < Truth::kEvents; ++i) {
        const auto& want = Truth::kEvent[i];
        const auto& got = spawns[static_cast<std::size_t>(i)];
        EXPECT(static_cast<int>(got.tick) == want.tick && got.kind == want.kind && got.id == want.id &&
                   static_cast<int>(got.follow) == want.follow,
               "%s: event %d header (tick %u kind %u id %u)", label, i, got.tick, got.kind, got.id);
        for (int k = 0; k < 16; ++k) {
            EXPECT(near(got.matrix[static_cast<std::size_t>(k)], want.m[k], 2e-4F),
                   "%s: event %d matrix[%d] %g vs %g", label, i, k, got.matrix[static_cast<std::size_t>(k)],
                   want.m[k]);
        }
    }
}

}  // namespace

int main() {
    check<truth_g1>("drift + scale ramp + jitter");
    check<truth_g2>("still, follow parent, endless");
    check<truth_g3>("clip motion");
    const auto clip = generator::parse_clip(from_hex(truth_g3::kClipHex));
    assert(clip.has_value());
    for (const auto& s : truth_spline::kPoint) {
        const auto p = generator::evaluate(*clip, s[0]);
        for (int a = 0; a < 3; ++a) {
            EXPECT(near(p[static_cast<std::size_t>(a)], s[1 + a], 1e-4F), "spline t=%g axis %d: %g vs %g", s[0], a,
                   p[static_cast<std::size_t>(a)], s[1 + a]);
        }
    }
    std::printf("generator ok\n");
    return 0;
}
