#include "dmc_rengine/formats/so/graph.hpp"

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <vector>

namespace graph = dmc::rengine::formats::so::graph;

namespace {
void put_u16(std::vector<std::byte>& bytes, const std::size_t offset, const std::uint16_t value) {
    bytes[offset] = static_cast<std::byte>(value & 0xFFU);
    bytes[offset + 1U] = static_cast<std::byte>((value >> 8U) & 0xFFU);
}
} // namespace

int main() {
    std::vector<std::byte> bytes(0x80U);
    put_u16(bytes, 0x00U, 6U);
    put_u16(bytes, 0x02U, 0x40U);
    put_u16(bytes, 0x04U, 0xFFFFU);
    put_u16(bytes, 0x06U, 8U);
    put_u16(bytes, 0x08U, 0x30U);
    put_u16(bytes, 0x0AU, 0x38U);
    put_u16(bytes, 0x0CU, 0xFFFFU);
    put_u16(bytes, 0x0EU, 0x12U);
    put_u16(bytes, 0x10U, 0x20U);

    put_u16(bytes, 0x40U, 8U);
    put_u16(bytes, 0x42U, 0x20U);
    put_u16(bytes, 0x44U, 0x30U);
    put_u16(bytes, 0x46U, 0xFFFFU);
    put_u16(bytes, 0x48U, 0x0CU);
    put_u16(bytes, 0x4AU, 0x18U);

    const auto result = graph::parse(bytes);
    assert(result.ok());
    assert(result.blocks.size() == 2U);
    assert(result.blocks[0].type == 6U);
    assert(result.blocks[0].entry_offsets.size() == 2U);
    assert(result.blocks[1].type == 8U);
    assert(result.blocks[1].entry_offsets.size() == 2U);

    bytes[0U] = std::byte{0U};
    const auto not_so = graph::parse(bytes);
    assert(!not_so.recognized);
    return 0;
}
