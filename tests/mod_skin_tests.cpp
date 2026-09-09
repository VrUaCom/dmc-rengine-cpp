#include "dmc_rengine/analysis/mod/mesh_serialized.hpp"
#include "dmc_rengine/analysis/mod/object_flags.hpp"
#include "dmc_rengine/formats/mod_skin.hpp"
#include "dmc_rengine/formats/mod/transform_domain.hpp"
#include "dmc_rengine/formats/mod/world_transform.hpp"

#include <array>
#include <bit>
#include <cassert>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <vector>

namespace {
void put_u32(std::vector<std::byte>& bytes,
             const std::size_t offset,
             const std::uint32_t value) {
    for (std::size_t i = 0U; i < 4U; ++i) {
        bytes[offset + i] =
            static_cast<std::byte>((value >> (8U * i)) & 0xFFU);
    }
}

void put_u64(std::vector<std::byte>& bytes,
             const std::size_t offset,
             const std::uint64_t value) {
    put_u32(bytes, offset, static_cast<std::uint32_t>(value));
    put_u32(bytes,
            offset + 4U,
            static_cast<std::uint32_t>(value >> 32U));
}

void put_f32(std::vector<std::byte>& bytes,
             const std::size_t offset,
             const float value) {
    put_u32(bytes, offset, std::bit_cast<std::uint32_t>(value));
}
} // namespace

int main() {
    namespace analysis = dmc::rengine::analysis::mod;
    namespace mod = dmc::rengine::formats::mod;
    namespace domain = dmc::rengine::formats::mod::transform_domain;
    namespace world = dmc::rengine::formats::mod::world_transform;

    {
        // Source bit21 is carried in the complete baseline/effective word. The
        // separately proven external mutator sets bit17 only, while the common
        // material helper interprets only bit14 (0x4000). Neither mask aliases
        // source bit21, so these audited paths preserve but do not consume it.
        constexpr std::uint32_t source =
            analysis::source_flag_00200000 | 0x00004000U | 0x00000001U;
        constexpr auto carried =
            analysis::project_source_flag_00200000_carry(source);
        static_assert(carried.active);
        static_assert((carried.runtime_flags14 &
                       analysis::source_flag_00200000) != 0U);
        static_assert((analysis::effective_flag_mutator_set_mask &
                       analysis::source_flag_00200000) == 0U);
        static_assert((analysis::common_material_interpreted_flag_mask &
                       analysis::source_flag_00200000) == 0U);
        static_assert((carried.runtime_flags14 &
                       analysis::common_material_interpreted_flag_mask) ==
                      0x00004000U);

        const auto mutated =
            carried.runtime_flags14 | analysis::effective_flag_mutator_set_mask;
        assert((mutated & analysis::source_flag_00200000) != 0U);
        assert((mutated & analysis::effective_flag_mutator_set_mask) != 0U);
        assert((mutated & analysis::common_material_interpreted_flag_mask) ==
               0x00004000U);
    }

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
        // BLENDINDICES.x is PRESERVED_UNDECODED, not a required zero. The
        // audited skin semantic consumes y/z/w only, so a synthetic non-zero X
        // must not change decoded influences or trigger validation failure.
        const auto zero_x = mod::decode_vertex_skin(
            std::array<std::uint8_t, 4>{0U, 20U, 8U, 12U},
            0x0136U,
            24U);
        const auto nonzero_x = mod::decode_vertex_skin(
            std::array<std::uint8_t, 4>{0xA5U, 20U, 8U, 12U},
            0x0136U,
            24U);
        assert(zero_x.ok());
        assert(nonzero_x.ok());
        assert(zero_x.skin.influence_count == nonzero_x.skin.influence_count);
        assert(zero_x.skin.topology_break == nonzero_x.skin.topology_break);
        for (std::size_t i = 0U; i < zero_x.skin.influence_count; ++i) {
            assert(zero_x.skin.influences[i].bone_index ==
                   nonzero_x.skin.influences[i].bone_index);
            assert(zero_x.skin.influences[i].quantized_weight ==
                   nonzero_x.skin.influences[i].quantized_weight);
            assert(std::fabs(zero_x.skin.influences[i].weight -
                             nonzero_x.skin.influences[i].weight) < 0.000001F);
        }
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

    {
        // Writer-critical unknown mesh bytes must survive the preservation
        // projection even when synthetic fixtures make them non-zero.
        std::vector<std::byte> mesh_bytes(
            analysis::MeshSerializedPreservationAbi::record_size,
            std::byte{0});
        put_u32(mesh_bytes, 0x0CU, 0xDEADBEEFU);
        put_u64(mesh_bytes, 0x38U, 0x0123456789ABCDEFULL);
        put_u32(mesh_bytes, 0x4CU, 0xA5A55A5AU);

        const auto preserved =
            analysis::decode_mesh_serialized_preservation(mesh_bytes, 0U);
        assert(preserved.has_value());
        assert(preserved->preserved0c_u32 == 0xDEADBEEFU);
        assert(preserved->preserved38_u64 == 0x0123456789ABCDEFULL);
        assert(preserved->preserved4c_u32 == 0xA5A55A5AU);
    }

    {
        // transform +0x1C is also PRESERVED_UNDECODED. A synthetic non-zero
        // scalar must survive parsing while remaining outside the currently
        // proven XYZ local-matrix semantic.
        std::vector<std::byte> bytes(0x90U);
        bytes[0U] = std::byte{'M'};
        bytes[1U] = std::byte{'O'};
        bytes[2U] = std::byte{'D'};
        bytes[3U] = std::byte{' '};
        bytes[0x11U] = std::byte{1U};
        put_u64(bytes, 0x20U, 0x40U);

        put_u32(bytes, 0x40U, 0x20U);
        put_u32(bytes, 0x44U, 0x24U);
        put_u32(bytes, 0x48U, 0x28U);
        put_u32(bytes, 0x4CU, 0x30U);
        bytes[0x60U] = std::byte{0xFFU};
        bytes[0x64U] = std::byte{0U};
        bytes[0x68U] = std::byte{0U};

        put_f32(bytes, 0x70U + 0x00U, 0.0F);
        put_f32(bytes, 0x70U + 0x04U, 0.0F);
        put_f32(bytes, 0x70U + 0x08U, 0.0F);
        put_f32(bytes, 0x70U + 0x0CU, 0.0F);
        put_f32(bytes, 0x70U + 0x10U, 0.1F);
        put_f32(bytes, 0x70U + 0x14U, 0.2F);
        put_f32(bytes, 0x70U + 0x18U, 0.3F);
        put_f32(bytes, 0x70U + 0x1CU, 123.25F);

        const auto parsed = domain::parse(bytes);
        assert(parsed.ok());
        assert(parsed.local_transform_records_by_node_index.size() == 1U);
        const auto& record = parsed.local_transform_records_by_node_index[0];
        assert(record.raw_1c == 123.25F);

        auto changed_unknown = record;
        changed_unknown.raw_1c = -99.5F;
        const auto original_matrix = world::build_local_matrix(record);
        const auto changed_matrix = world::build_local_matrix(changed_unknown);
        for (std::size_t i = 0U; i < original_matrix.values.size(); ++i) {
            assert(std::fabs(original_matrix.values[i] -
                             changed_matrix.values[i]) < 0.000001F);
        }
    }

    return 0;
}
