#include "dmc_rengine/formats/so.hpp"

#include <array>
#include <bit>
#include <cassert>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <vector>

using namespace dmc::rengine::formats::so;

namespace {
void put_u16(std::vector<std::byte>& bytes, std::size_t offset, std::uint16_t value) {
    bytes[offset] = static_cast<std::byte>(value & 0xFFU);
    bytes[offset + 1U] = static_cast<std::byte>((value >> 8U) & 0xFFU);
}

void put_u32(std::vector<std::byte>& bytes, std::size_t offset, std::uint32_t value) {
    for (std::size_t i = 0U; i < 4U; ++i) {
        bytes[offset + i] = static_cast<std::byte>((value >> (8U * i)) & 0xFFU);
    }
}

void put_u64(std::vector<std::byte>& bytes, std::size_t offset, std::uint64_t value) {
    put_u32(bytes, offset, static_cast<std::uint32_t>(value & 0xFFFFFFFFULL));
    put_u32(bytes, offset + 4U, static_cast<std::uint32_t>(value >> 32U));
}

void put_f32(std::vector<std::byte>& bytes, std::size_t offset, float value) {
    put_u32(bytes, offset, std::bit_cast<std::uint32_t>(value));
}
} // namespace

int main() {
    std::vector<std::byte> graph(0x80U);
    put_u16(graph, 0x00U, 6U);
    put_u16(graph, 0x02U, 0x40U);
    put_u16(graph, 0x04U, 0xFFFFU);
    put_u16(graph, 0x06U, 8U);
    put_u16(graph, 0x08U, 0x30U);
    put_u16(graph, 0x0AU, 0x38U);
    put_u16(graph, 0x0CU, 0xFFFFU);
    put_u16(graph, 0x0EU, 0x12U);
    put_u16(graph, 0x10U, 0x20U);

    put_u16(graph, 0x40U, 8U);
    put_u16(graph, 0x42U, 0x20U);
    put_u16(graph, 0x44U, 0x30U);
    put_u16(graph, 0x46U, 0xFFFFU);
    put_u16(graph, 0x48U, 0x0CU);
    put_u16(graph, 0x4AU, 0x18U);

    const auto graph_result = parse_graph(graph);
    assert(graph_result.ok());
    assert(graph_result.blocks.size() == 2U);
    assert(graph_result.blocks[0].type == 6U);
    assert(graph_result.blocks[0].entry_offsets.size() == 2U);
    assert(graph_result.blocks[1].type == 8U);
    assert(graph_result.blocks[1].entry_offsets.size() == 2U);

    const std::array<std::byte, 8> links_bytes{
        std::byte{0x06}, std::byte{0x00}, std::byte{0x00}, std::byte{0x00},
        std::byte{0x01}, std::byte{0x09}, std::byte{0x01}, std::byte{0x00},
    };
    const auto links = parse_links(links_bytes);
    assert(links.ok());
    assert(links.records.size() == 2U);
    assert(links.records[1].field1 == 9U);

    std::vector<std::byte> volume_bytes(volume_record_size);
    put_u32(volume_bytes, 0x00U, 2U);
    put_f32(volume_bytes, 0x10U, 1.0F);
    put_f32(volume_bytes, 0x14U, 2.0F);
    put_f32(volume_bytes, 0x18U, 3.0F);
    put_f32(volume_bytes, 0x1CU, 1.0F);
    put_f32(volume_bytes, 0x20U, 50.0F);

    const auto volumes = parse_volumes(volume_bytes);
    assert(volumes.ok());
    assert(volumes.records.size() == 1U);
    assert(volumes.records[0].type == 2U);
    assert(std::fabs(volumes.records[0].vector0.y - 2.0F) < 0.0001F);
    assert(std::fabs(volumes.records[0].vector1.x - 50.0F) < 0.0001F);

    const auto correlation = correlate_companions(links, volumes);
    assert(correlation.one_header_plus_one_link_per_volume);

    std::vector<std::byte> mod(0x90U);
    mod[0U] = std::byte{'M'};
    mod[1U] = std::byte{'O'};
    mod[2U] = std::byte{'D'};
    mod[3U] = std::byte{' '};
    mod[0x11U] = std::byte{3U};
    put_u64(mod, 0x20U, 0x40U);
    put_u32(mod, 0x40U, 0x20U);
    put_u32(mod, 0x44U, 0x24U);
    put_u32(mod, 0x48U, 0x28U);
    put_u32(mod, 0x4CU, 0x30U);
    mod[0x60U] = std::byte{0xFFU};
    mod[0x61U] = std::byte{0U};
    mod[0x62U] = std::byte{1U};
    mod[0x64U] = std::byte{0U};
    mod[0x65U] = std::byte{1U};
    mod[0x66U] = std::byte{2U};

    const auto mod_domain = parse_mod_transform_domain(mod);
    assert(mod_domain.ok());
    assert(mod_domain.raw_domain_count == 3U);
    assert(mod_domain.permutation_is_complete);
    assert(mod_domain.hierarchy_candidate_is_acyclic);
    assert(mod_domain.derived_hierarchy_candidate.size() == 3U);
    assert(mod_domain.derived_hierarchy_candidate[0] == -1);
    assert(mod_domain.derived_hierarchy_candidate[1] == 0);
    assert(mod_domain.derived_hierarchy_candidate[2] == 1);

    const std::array<std::byte, 16> binding_links_bytes{
        std::byte{0x06}, std::byte{0x00}, std::byte{0x00}, std::byte{0x00},
        std::byte{0x01}, std::byte{0x00}, std::byte{0x01}, std::byte{0x00},
        std::byte{0x01}, std::byte{0x01}, std::byte{0x02}, std::byte{0x00},
        std::byte{0x00}, std::byte{0x00}, std::byte{0x00}, std::byte{0x00},
    };
    const auto binding_links = parse_links(binding_links_bytes);
    std::vector<std::byte> binding_volume_bytes(3U * volume_record_size);
    const auto binding_volumes = parse_volumes(binding_volume_bytes);
    const auto mod_correlation = correlate_mod_companions(mod_domain, binding_links, binding_volumes);
    assert(mod_correlation.link_middle_fields_fit_domain);
    assert(mod_correlation.post_prefix_link_count_equals_domain);
    assert(mod_correlation.volume_count_equals_domain);
    assert(mod_correlation.complete_cardinality_alignment);
    return 0;
}
