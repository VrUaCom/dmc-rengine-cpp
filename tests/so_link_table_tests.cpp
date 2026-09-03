#include "dmc_rengine/formats/so/link_table.hpp"

#include <array>
#include <cassert>
#include <cstddef>

namespace links = dmc::rengine::formats::so::link_table;

int main() {
    const std::array<std::byte, 8> bytes{
        std::byte{0x06}, std::byte{0x00}, std::byte{0x00}, std::byte{0x00},
        std::byte{0x01}, std::byte{0x09}, std::byte{0x01}, std::byte{0x00},
    };
    const auto result = links::parse(bytes);
    assert(result.ok());
    assert(result.records.size() == 2U);
    assert(result.records[1].field1 == 9U);

    const std::array<std::byte, 3> invalid{std::byte{0}, std::byte{0}, std::byte{0}};
    assert(!links::parse(invalid).recognized);
    return 0;
}
