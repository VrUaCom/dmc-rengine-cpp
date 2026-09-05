#include "dmc_rengine/formats/mod_skin.hpp"

#include <array>
#include <cstddef>

namespace dmc::rengine::formats::mod {

SkinDecodeResult decode_vertex_skin(
    const std::array<std::uint8_t, 4>& blend_indices,
    std::uint16_t packed_weights_and_topology,
    std::uint8_t node_count) noexcept {
    SkinDecodeResult out;
    out.skin.topology_break =
        (packed_weights_and_topology & topology_break_mask) != 0U;

    const auto payload = static_cast<std::uint16_t>(
        packed_weights_and_topology & packed_weight_mask);
    const std::array<std::uint8_t, 3> quantized{
        static_cast<std::uint8_t>((payload >> 0U) & 0x1FU),
        static_cast<std::uint8_t>((payload >> 5U) & 0x1FU),
        static_cast<std::uint8_t>((payload >> 10U) & 0x1FU),
    };

    const auto sum = static_cast<unsigned>(quantized[0]) +
                     static_cast<unsigned>(quantized[1]) +
                     static_cast<unsigned>(quantized[2]);
    if (sum != quantized_weight_sum) {
        out.status = SkinDecodeStatus::quantized_sum_mismatch;
        return out;
    }

    std::array<bool, 256> seen{};
    for (std::size_t component = 0U;
         component < quantized.size();
         ++component) {
        const auto q = quantized[component];
        if (q == 0U) continue;

        const auto raw_index = blend_indices[component + 1U];
        if ((raw_index % matrix_row_stride) != 0U) {
            out.status = SkinDecodeStatus::active_index_not_matrix_aligned;
            return out;
        }

        const auto bone_index =
            static_cast<std::uint8_t>(raw_index / matrix_row_stride);
        if (bone_index >= node_count) {
            out.status = SkinDecodeStatus::bone_index_out_of_range;
            return out;
        }
        if (seen[bone_index]) {
            out.status = SkinDecodeStatus::duplicate_active_bone;
            return out;
        }
        seen[bone_index] = true;

        auto& influence =
            out.skin.influences[out.skin.influence_count++];
        influence.bone_index = bone_index;
        influence.quantized_weight = q;
        influence.weight =
            static_cast<float>(q) /
            static_cast<float>(quantized_weight_sum);
    }

    out.status = SkinDecodeStatus::ok;
    return out;
}

} // namespace dmc::rengine::formats::mod
