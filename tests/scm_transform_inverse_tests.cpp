#include "dmc_rengine/formats/scm_transform.hpp"

#include <cassert>
#include <cmath>
#include <cstddef>

namespace {

[[nodiscard]] bool near(float actual, float expected, float epsilon = 0.0002F) {
    return std::fabs(actual - expected) <= epsilon;
}

void assert_identity(const dmc::rengine::formats::scm::Matrix4f& matrix) {
    for (std::size_t row = 0U; row < 4U; ++row) {
        for (std::size_t column = 0U; column < 4U; ++column) {
            const auto expected = row == column ? 1.0F : 0.0F;
            assert(near(matrix(row, column), expected));
        }
    }
}

} // namespace

int main() {
    using namespace dmc::rengine::formats::scm;

    {
        SceneTransform transform{};
        transform.translation = Vec3f{3.0F, -7.0F, 11.0F};
        const auto local = build_local_transform(transform);
        const auto inverse = invert_dmc3_rigid_transform(local);
        assert(near(inverse(3U, 0U), -3.0F));
        assert(near(inverse(3U, 1U), 7.0F));
        assert(near(inverse(3U, 2U), -11.0F));
        assert_identity(multiply_dmc3_matrices(local, inverse));
        assert_identity(multiply_dmc3_matrices(inverse, local));
    }

    {
        SceneTransform transform{};
        transform.translation = Vec3f{12.5F, -2.0F, 4.25F};
        transform.rotation_xyz_radians = Vec3f{0.37F, -0.81F, 1.12F};
        const auto local = build_local_transform(transform);
        const auto inverse = invert_dmc3_rigid_transform(local);
        assert_identity(multiply_dmc3_matrices(local, inverse));
        assert_identity(multiply_dmc3_matrices(inverse, local));
    }

    return 0;
}
