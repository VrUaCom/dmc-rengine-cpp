#include "dmc_rengine/so.hpp"

#include <array>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <vector>

namespace analysis = dmc::rengine::analysis::so;
namespace graph = dmc::rengine::formats::so::graph;
namespace links = dmc::rengine::formats::so::link_table;
namespace volumes = dmc::rengine::formats::so::volume_table;
namespace domain = dmc::rengine::formats::mod::transform_domain;

namespace {
void put_u16(std::vector<std::byte>& bytes, const std::size_t offset, const std::uint16_t value) {
    bytes[offset] = static_cast<std::byte>(value & 0xFFU);
    bytes[offset + 1U] = static_cast<std::byte>((value >> 8U) & 0xFFU);
}

void put_u32(std::vector<std::byte>& bytes, const std::size_t offset, const std::uint32_t value) {
    for (std::size_t i = 0U; i < 4U; ++i) {
        bytes[offset + i] = static_cast<std::byte>((value >> (8U * i)) & 0xFFU);
    }
}

void put_u64(std::vector<std::byte>& bytes, const std::size_t offset, const std::uint64_t value) {
    put_u32(bytes, offset, static_cast<std::uint32_t>(value & 0xFFFFFFFFULL));
    put_u32(bytes, offset + 4U, static_cast<std::uint32_t>(value >> 32U));
}
} // namespace

int main() {
    std::vector<std::byte> graph_bytes(0x80U);
    put_u16(graph_bytes, 0x00U, 6U);
    put_u16(graph_bytes, 0x02U, 0x40U);
    put_u16(graph_bytes, 0x0EU, 0x12U);
    put_u16(graph_bytes, 0x10U, 0x20U);
    put_u16(graph_bytes, 0x40U, 8U);
    put_u16(graph_bytes, 0x48U, 0x0CU);
    put_u16(graph_bytes, 0x4AU, 0x18U);
    assert(graph::parse(graph_bytes).ok());

    const std::array<std::byte, 16> link_bytes{
        std::byte{0x06}, std::byte{0x00}, std::byte{0x00}, std::byte{0x00},
        std::byte{0x01}, std::byte{0x00}, std::byte{0x01}, std::byte{0x00},
        std::byte{0x01}, std::byte{0x01}, std::byte{0x02}, std::byte{0x00},
        std::byte{0x00}, std::byte{0x00}, std::byte{0x00}, std::byte{0x00},
    };
    const auto link_result = links::parse(link_bytes);
    assert(link_result.ok());

    std::vector<std::byte> volume_bytes(3U * volumes::record_size);
    put_u32(volume_bytes, 0U, 2U);
    const auto volume_result = volumes::parse(volume_bytes);
    assert(volume_result.ok());
    assert(analysis::correlate_companions(link_result, volume_result).one_header_plus_one_link_per_volume);

    std::vector<std::byte> mod_bytes(0x90U);
    mod_bytes[0U] = std::byte{'M'};
    mod_bytes[1U] = std::byte{'O'};
    mod_bytes[2U] = std::byte{'D'};
    mod_bytes[3U] = std::byte{' '};
    mod_bytes[0x11U] = std::byte{3U};
    put_u64(mod_bytes, 0x20U, 0x40U);
    put_u32(mod_bytes, 0x40U, 0x20U);
    put_u32(mod_bytes, 0x44U, 0x24U);
    put_u32(mod_bytes, 0x48U, 0x28U);
    mod_bytes[0x60U] = std::byte{0xFFU};
    mod_bytes[0x61U] = std::byte{0U};
    mod_bytes[0x62U] = std::byte{1U};
    mod_bytes[0x64U] = std::byte{0U};
    mod_bytes[0x65U] = std::byte{1U};
    mod_bytes[0x66U] = std::byte{2U};

    const auto mod_result = domain::parse(mod_bytes);
    assert(mod_result.ok());
    assert(analysis::analyze_mod_binding(mod_result, link_result, volume_result).complete_cardinality_alignment);
    return 0;
}
