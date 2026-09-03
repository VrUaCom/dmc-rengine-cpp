#include "dmc_rengine/formats/so/volume_table.hpp"

#include <bit>
#include <cassert>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <vector>

namespace volumes = dmc::rengine::formats::so::volume_table;

namespace {
void put_u32(std::vector<std::byte>& bytes, const std::size_t offset, const std::uint32_t value) {
    for (std::size_t i = 0U; i < 4U; ++i) {
        bytes[offset + i] = static_cast<std::byte>((value >> (8U * i)) & 0xFFU);
    }
}

void put_f32(std::vector<std::byte>& bytes, const std::size_t offset, const float value) {
    put_u32(bytes, offset, std::bit_cast<std::uint32_t>(value));
}
} // namespace

int main() {
    std::vector<std::byte> bytes(volumes::record_size);
    put_u32(bytes, 0x00U, 2U);
    put_f32(bytes, 0x10U, 1.0F);
    put_f32(bytes, 0x14U, 2.0F);
    put_f32(bytes, 0x18U, 3.0F);
    put_f32(bytes, 0x1CU, 1.0F);
    put_f32(bytes, 0x20U, 50.0F);

    const auto result = volumes::parse(bytes);
    assert(result.ok());
    assert(result.records.size() == 1U);
    assert(result.records[0].type == 2U);
    assert(std::fabs(result.records[0].vector0.y - 2.0F) < 0.0001F);
    assert(std::fabs(result.records[0].vector1.x - 50.0F) < 0.0001F);

    bytes.push_back(std::byte{0});
    assert(!volumes::parse(bytes).recognized);
    return 0;
}
