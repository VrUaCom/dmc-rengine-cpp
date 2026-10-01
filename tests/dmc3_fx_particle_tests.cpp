// P record classes 3 (CPtclSprt00), 1 (CPtclPoly00) and 4 (CPtclLine01)
// against emulated runs of dmc3.exe on synthetic records: spawn state fed in,
// then 14 updates (transform integrator, colour tracks with every ease mode,
// friction / gravity / vertex advance), then the draw matrices of the emitter
// and both layers applied to the packet vertices.
#include "dmc_rengine/profiles/dmc3/fx/particle.hpp"

#include <array>
#include <cassert>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <string>
#include <vector>

#include "dmc3_fx_particle_truth.inc"

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
void check(const char* label, std::uint8_t cls) {
    const auto record = from_hex(Truth::kRecordHex);
    const auto def = particle::parse(record);
    assert(def.has_value() && def->cls == cls && def->layers.size() == 2U);
    particle::Simulation sim(*def, particle::Animation{}, 1U);
    const int n = Truth::kCount;
    assert(static_cast<std::uint32_t>(n) == def->count);
    std::vector<std::array<float, 3>> pos, vel;
    std::vector<std::array<std::array<float, 3>, 4>> vert;
    for (int i = 0; i < n; ++i) {
        vel.push_back({Truth::kInit.vel[i][0], Truth::kInit.vel[i][1], Truth::kInit.vel[i][2]});
        pos.push_back({Truth::kInit.pos[i][0], Truth::kInit.pos[i][1], Truth::kInit.pos[i][2]});
        std::array<std::array<float, 3>, 4> q{};
        for (int k = 0; k < 4; ++k) for (int a = 0; a < 3; ++a) q[k][a] = Truth::kInit.vert[i][k][a];
        vert.push_back(q);
    }
    if (cls == 3U) sim.set_particles(pos, vel);
    else sim.set_vertices(vert, vel);
    Matrix4 world;
    for (int i = 0; i < 16; ++i) world.values[static_cast<std::size_t>(i)] = Truth::kWorld[i];
    const int verts_per = cls == 1U ? 3 : 4;
    for (int f = 0; f < Truth::kFrames; ++f) {
        assert(sim.update(world));
        const auto& t = Truth::kFrame[f];
        const auto& s = sim.state();
        for (int a = 0; a < 3; ++a) {
            EXPECT(near(s.translation[a], t.tr[a], 2.0e-5F) && near(s.rotation[a], t.rot[a], 2.0e-5F) &&
                       near(s.scale[a], t.sc[a], 2.0e-5F),
                   "%s frame %d transform axis %d", label, f, a);
        }
        for (int c = 0; c < 16; ++c) {
            EXPECT(s.color[static_cast<std::size_t>(c)] == t.col[c], "%s frame %d colour %d: %d vs %d", label, f, c,
                   s.color[static_cast<std::size_t>(c)], t.col[c]);
        }
        EXPECT(near(s.life, t.life, 1.0e-6F), "%s frame %d life", label, f);
        for (int i = 0; i < n; ++i) {
            for (int a = 0; a < 3; ++a) {
                EXPECT(near(s.velocity[static_cast<std::size_t>(i)][static_cast<std::size_t>(a)], t.vel[i][a], 1.0e-4F),
                       "%s frame %d particle %d vel %d", label, f, i, a);
                if (cls == 3U) {
                    EXPECT(near(s.position[static_cast<std::size_t>(i)][static_cast<std::size_t>(a)], t.pos[i][a], 1.0e-4F),
                           "%s frame %d particle %d pos %d", label, f, i, a);
                } else {
                    for (int k = 0; k < verts_per; ++k) {
                        EXPECT(near(s.vertex[static_cast<std::size_t>(i)][static_cast<std::size_t>(k)][static_cast<std::size_t>(a)],
                                    t.vert[i][k][a], 1.0e-4F),
                               "%s frame %d particle %d vertex %d axis %d: %g vs %g", label, f, i, k, a,
                               s.vertex[static_cast<std::size_t>(i)][static_cast<std::size_t>(k)][static_cast<std::size_t>(a)],
                               t.vert[i][k][a]);
                    }
                }
            }
        }
        for (int l = 0; l < 2; ++l) {
            const auto& ls = sim.layer_states()[static_cast<std::size_t>(l)];
            for (int a = 0; a < 3; ++a) {
                EXPECT(near(ls.translation[a], t.layer[l].tr[a], 2.0e-5F) && near(ls.rotation[a], t.layer[l].rot[a], 2.0e-5F) &&
                           near(ls.scale[a], t.layer[l].sc[a], 2.0e-5F),
                       "%s frame %d layer %d transform", label, f, l);
            }
            for (int c = 0; c < 16; ++c) {
                EXPECT(ls.color[static_cast<std::size_t>(c)] == t.layer[l].col[c], "%s frame %d layer %d colour %d: %d vs %d",
                       label, f, l, c, ls.color[static_cast<std::size_t>(c)], t.layer[l].col[c]);
            }
        }
    }

    // Final pose through the draw matrices of the emitter and both layers.
    particle::Camera camera;
    camera.right = {1.0F, 0.0F, 0.0F};
    camera.up = {0.0F, -1.0F, 0.0F};
    camera.forward = {0.0F, 0.0F, 1.0F};
    std::vector<particle::Quad> quads;
    sim.quads(world, camera, &quads);
    const std::size_t per_object = cls == 4U ? static_cast<std::size_t>(n) * 2U : static_cast<std::size_t>(n);
    assert(quads.size() == per_object * 3U);
    const auto& last = Truth::kFrame[Truth::kFrames - 1];
    const auto project = [](const float* m, const float* v, float* out) {
        out[0] = v[0] * m[0] + v[1] * m[4] + v[2] * m[8] + m[12];
        out[1] = v[0] * m[1] + v[1] * m[5] + v[2] * m[9] + m[13];
        out[2] = v[0] * m[2] + v[1] * m[6] + v[2] * m[10] + m[14];
    };
    for (std::size_t q = 0U; q < quads.size(); ++q) {
        const std::size_t object = q / per_object;
        const std::size_t in_object = q % per_object;
        const int* color = object == 0U ? last.col : last.layer[object - 1U].col;
        const float* m = Truth::kFinal[object];
        // The translation helper keeps w = 1 (its mask is set by a static initializer).
        assert(m[3] == 0.0F && m[7] == 0.0F && m[11] == 0.0F && m[15] == 1.0F);
        const auto check_corner = [&](int k, const float* v) {
            float e[3];
            project(m, v, e);
            const auto& c = quads[q].corners[static_cast<std::size_t>(k)];
            EXPECT(near(c.x, e[0], 2.0e-4F) && near(c.y, e[1], 2.0e-4F) && near(c.z, e[2], 2.0e-4F),
                   "%s quad %zu corner %d: %g %g %g vs %g %g %g", label, q, k, c.x, c.y, c.z, e[0], e[1], e[2]);
        };
        const auto check_color = [&](int k, int group) {
            for (int c = 0; c < 4; ++c) {
                EXPECT(quads[q].rgba[static_cast<std::size_t>(k)][static_cast<std::size_t>(c)] == color[group * 4 + c],
                       "%s quad %zu corner %d colour", label, q, k);
            }
        };
        if (cls == 3U) {
            // EXE vertex order BR, BL, TL, TR (w 1, 0, 2, 3) -> Reader BL, BR, TR, TL.
            constexpr int kExe[4] = {2, 3, 0, 1};
            constexpr int kGroup[4] = {2, 3, 1, 0};
            for (int k = 0; k < 4; ++k) {
                float v[3] = {Truth::kBaked[in_object][kExe[k]][0], Truth::kBaked[in_object][kExe[k]][1],
                              Truth::kBaked[in_object][kExe[k]][2]};
                check_corner(k, v);
                check_color(k, kGroup[k]);
            }
        } else if (cls == 1U) {
            assert(quads[q].triangle);
            for (int k = 0; k < 3; ++k) {
                check_corner(k, last.vert[in_object][k]);
                check_color(k, k);
            }
        } else {
            assert(quads[q].line);
            const std::size_t particle_index = in_object / 2U;
            const std::size_t segment = in_object % 2U;
            check_corner(0, last.vert[particle_index][2U * segment]);
            check_corner(1, last.vert[particle_index][2U * segment + 1U]);
            check_color(0, 0);
            check_color(1, 1);
        }
        const bool additive = object == 0U ? false : object == 1U;  // blend 0x44 / layer 0 0x48 / layer 1 0x44
        EXPECT(quads[q].additive == additive, "%s quad %zu blend", label, q);
    }
}

}  // namespace

int main() {
    check<truth_sw1>("sprt world gravity", 3U);
    check<truth_sw0>("sprt local gravity", 3U);
    check<truth_pw1>("poly world gravity", 1U);
    check<truth_pw0>("poly local gravity", 1U);
    check<truth_lw1>("line world gravity", 4U);
    check<truth_lw0>("line local gravity", 4U);

    // Parser guards: unported classes and the Poly00 path +0xFA != 1.
    assert(!particle::parse({}).has_value());
    {
        auto bytes = from_hex(truth_pw1::kRecordHex);
        bytes[0x20U + 0xFAU] = 0U;
        assert(!particle::parse(bytes).has_value());
        auto other = from_hex(truth_sw1::kRecordHex);
        other[0x21U] = 2U;  // an unported class
        assert(!particle::parse(other).has_value());
    }
    std::puts("particle_test OK");
    return 0;
}
