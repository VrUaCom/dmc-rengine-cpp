#include "dmc_rengine/formats/mod/world_transform.hpp"
#include "dmc_rengine/analysis/mot/animated_local.hpp"

#include <array>
#include <cassert>
#include <cmath>

// Reproduces 0x140330450 step by step: dest = source x R for X, then Y, then
// Z, with the D3D row-vector rotations built by 0x140030F10/FC0/1080.
namespace {

using M3 = std::array<std::array<float, 3>, 3>;

M3 mul(const M3& a, const M3& b) {
    M3 out{};
    for (int i = 0; i < 3; ++i)
        for (int j = 0; j < 3; ++j)
            for (int k = 0; k < 3; ++k) out[i][j] += a[i][k] * b[k][j];
    return out;
}
M3 rx(float a) { const float c = std::cos(a), s = std::sin(a); return {{{1, 0, 0}, {0, c, s}, {0, -s, c}}}; }
M3 ry(float a) { const float c = std::cos(a), s = std::sin(a); return {{{c, 0, -s}, {0, 1, 0}, {s, 0, c}}}; }
M3 rz(float a) { const float c = std::cos(a), s = std::sin(a); return {{{c, s, 0}, {-s, c, 0}, {0, 0, 1}}}; }

bool near(float a, float b) { return std::fabs(a - b) < 1e-5F; }

} // namespace

int main() {
    namespace world = dmc::rengine::formats::mod::world_transform;
    const std::array<std::array<float, 3>, 4> cases{{
        {0.3F, 0.5F, 0.7F}, {-1.6580626964569092F, 0.0F, 3.4033920764923096F},
        {1.1F, -0.4F, 2.9F}, {0.0F, 3.839724063873291F, 0.0F}}};
    for (const auto& angles : cases) {
        const M3 identity{{{1, 0, 0}, {0, 1, 0}, {0, 0, 1}}};
        const auto expected = mul(mul(mul(identity, rx(angles[0])), ry(angles[1])), rz(angles[2]));
        dmc::rengine::formats::mod::transform_domain::LocalTransformRecord record{};
        record.rotation_xyz_radians = {angles[0], angles[1], angles[2]};
        record.translation = {1.0F, 2.0F, 3.0F};
        const auto local = world::build_local_matrix(record);
        for (int r = 0; r < 3; ++r)
            for (int c = 0; c < 3; ++c) assert(near(local.values[r * 4 + c], expected[r][c]));
        assert(local.values[12] == 1.0F && local.values[13] == 2.0F && local.values[14] == 3.0F);
    }

    // Rebellion's sheathed record hangs the blade (+Z) downwards.
    dmc::rengine::analysis::mot::JointChannelValues rebellion;
    rebellion.rotation = {-1.6580626964569092F, 0.0F, 3.4033920764923096F};
    const auto sheathed = dmc::rengine::analysis::mot::build_animated_local_matrix(rebellion);
    assert(sheathed.values[9] < -0.95F);
    return 0;
}
