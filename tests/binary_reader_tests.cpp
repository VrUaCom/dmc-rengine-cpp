#include "dmc_rengine/binary/reader.hpp"

#include <cassert>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <span>
#include <vector>

int main() {
    const std::vector<std::byte> bytes{
        std::byte{0x34},
        std::byte{0x12},
        std::byte{0x78},
        std::byte{0x56},
        std::byte{0x34},
        std::byte{0x12},
        std::byte{'A'},
        std::byte{'B'},
        std::byte{0x00},
        std::byte{'C'},
        std::byte{0x00},
        std::byte{0x00},
        std::byte{0x80},
        std::byte{0x3F},
    };

    const dmc::rengine::binary::Reader reader(
        std::span<const std::byte>{bytes});

    assert(reader.size() == bytes.size());
    assert(reader.contains(0U, bytes.size()));
    assert(!reader.contains(bytes.size(), 1U));
    assert(reader.u8(0U) == 0x34U);
    assert(reader.u16_le(0U) == 0x1234U);
    assert(reader.u32_le(2U) == 0x12345678U);
    assert(!reader.u64_le(8U).has_value());
    assert(reader.fixed_string(6U, 4U).value() == "AB");
    assert(reader.fixed_string(6U, 4U, false).value().size() == 4U);
    assert(reader.matches(6U, "AB"));
    assert(!reader.matches(7U, "AB"));
    assert(std::fabs(reader.f32_le(10U).value() - 1.0F) < 0.0001F);

    return 0;
}
