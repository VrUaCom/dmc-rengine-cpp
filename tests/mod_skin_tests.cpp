#include "dmc_rengine/formats/mod_skin.hpp"

#include <array>
#include <cassert>
#include <cmath>
#include <cstdint>

int main() {
    namespace mod = dmc::rengine::formats::mod;

    {
        const auto decoded = mod::decode_vertex_skin(
            std::array<std::uint8_t, 4>{0U, 20U, 8U, 12U},
            0x001FU,
            24U);
        assert(decoded.ok());
        assert(decoded.skin.influence_count == 1U);
        assert(decoded.skin.influences[0].bone_index == 5U);
        assert(decoded.skin.influences[0].quantized_weight == 31U);
        assert(decoded.skin.influences[0].weight == 1.0F);
        assert(!decoded.skin.topology_break);
    }

    {
        // q0=22, q1=9, q2=0. The high bit is independent topology state.
        const auto decoded = mod::decode_vertex_skin(
            std::array<std::uint8_t, 4>{0U, 20U, 8U, 12U},
            static_cast<std::uint16_t>(0x8000U | 0x0136U),
            24U);
        assert(decoded.ok());
        assert(decoded.skin.topology_break);
        assert(decoded.skin.influence_count == 2U);
        assert(decoded.skin.influences[0].bone_index == 5U);
        assert(decoded.skin.influences[0].quantized_weight == 22U);
        assert(decoded.skin.influences[1].bone_index == 2U);
        assert(decoded.skin.influences[1].quantized_weight == 9U);
        const auto sum = decoded.skin.influences[0].weight +
                         decoded.skin.influences[1].weight;
        assert(std::fabs(sum - 1.0F) < 0.000001F);
    }

    {
        // 16 + 9 + 6 = 31.
        const auto decoded = mod::decode_vertex_skin(
            std::array<std::uint8_t, 4>{0U, 128U, 64U, 4U},
            0x1930U,
            33U);
        assert(decoded.ok());
        assert(decoded.skin.influence_count == 3U);
        assert(decoded.skin.influences[0].bone_index == 32U);
        assert(decoded.skin.influences[1].bone_index == 16U);
        assert(decoded.skin.influences[2].bone_index == 1U);
    }

    {
        const auto bad_sum = mod::decode_vertex_skin(
            std::array<std::uint8_t, 4>{0U, 4U, 8U, 12U},
            0U,
            24U);
        assert(
            bad_sum.status ==
            mod::SkinDecodeStatus::quantized_sum_mismatch);
    }

    {
        const auto bad_alignment = mod::decode_vertex_skin(
            std::array<std::uint8_t, 4>{0U, 5U, 8U, 12U},
            0x001FU,
            24U);
        assert(
            bad_alignment.status ==
            mod::SkinDecodeStatus::active_index_not_matrix_aligned);
    }

    {
        const auto bad_range = mod::decode_vertex_skin(
            std::array<std::uint8_t, 4>{0U, 96U, 8U, 12U},
            0x001FU,
            24U);
        assert(
            bad_range.status ==
            mod::SkinDecodeStatus::bone_index_out_of_range);
    }

    {
        const auto duplicate = mod::decode_vertex_skin(
            std::array<std::uint8_t, 4>{0U, 20U, 20U, 12U},
            0x0136U,
            24U);
        assert(
            duplicate.status ==
            mod::SkinDecodeStatus::duplicate_active_bone);
    }

    return 0;
}
