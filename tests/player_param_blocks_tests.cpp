// A player PAC's parameter blocks: typed by the slot they sit in and only
// when their bytes read as that slot's block; the structure view lists them.

#include "dmc_rengine/integration/resource_structure.hpp"
#include "dmc_rengine/profiles/dmc3/player_param_blocks.hpp"

#include <cassert>
#include <cstring>
#include <iostream>
#include <limits>
#include <string>
#include <vector>

namespace pp = dmc::rengine::profiles::dmc3::player_params;

namespace {

[[nodiscard]] std::vector<std::byte> floats(std::size_t count, float first) {
    std::vector<std::byte> bytes(count * 4U);
    for (std::size_t index = 0U; index < count; ++index) {
        const float value = first + static_cast<float>(index);
        std::memcpy(bytes.data() + index * 4U, &value, 4U);
    }
    return bytes;
}

[[nodiscard]] std::vector<std::byte> pairs_then_floats(std::size_t pairs, std::size_t count) {
    std::vector<std::byte> bytes;
    for (std::size_t index = 0U; index < pairs; ++index) {
        const std::uint16_t first = 92U, second = static_cast<std::uint16_t>(132U + 20U * index);
        const auto* a = reinterpret_cast<const std::byte*>(&first);
        const auto* b = reinterpret_cast<const std::byte*>(&second);
        bytes.insert(bytes.end(), a, a + 2);
        bytes.insert(bytes.end(), b, b + 2);
    }
    const auto tail = floats(count, 0.3F);
    bytes.insert(bytes.end(), tail.begin(), tail.end());
    return bytes;
}

} // namespace

int main() {
    assert(pp::is_player_pac("pl000") && pp::is_player_pac("pl011"));
    assert(!pp::is_player_pac("plwp_sword") && !pp::is_player_pac("em000") && !pp::is_player_pac("pl00"));

    const auto params = floats(224U, 10.0F);
    assert(pp::slot_format("pl000", 9U, params) == "player-params");
    assert(pp::slot_format("pl001", 11U, floats(448U, 1.0F)) == "player-params");
    // Another slot, another PAC, or bytes that are not floats: nothing.
    assert(!pp::slot_format("pl000", 8U, params));
    assert(!pp::slot_format("em028", 9U, params));
    auto broken = params;
    const float nan = std::numeric_limits<float>::quiet_NaN();
    std::memcpy(broken.data() + 8, &nan, 4U);
    assert(!pp::slot_format("pl000", 9U, broken));

    const auto pairs = pairs_then_floats(24U, 96U);
    assert(pp::leading_pairs(pairs) == 24U);
    assert(pp::slot_format("pl011", 10U, pairs) == "player-pairs");
    assert(!pp::slot_format("pl011", 10U, floats(120U, 1.0F)));

    assert(pp::known_reads(896U).size() == 1U && pp::known_reads(896U)[0].offset == 0x12CU);
    assert(pp::known_reads(1792U).size() == 3U);
    assert(pp::known_reads(480U).empty());

    std::string detail;
    const auto view = dmc::rengine::integration::read_structure("player-params", params, "pl000_009", detail);
    assert(view && view->format == "player-params" && view->sections.size() >= 3U);
    const auto pair_view = dmc::rengine::integration::read_structure("player-pairs", pairs, "pl000_010", detail);
    assert(pair_view && pair_view->summary.find("24 u16 pairs") == 0U);
    assert(!dmc::rengine::integration::read_structure("player-pairs", floats(8U, 1.0F), "", detail));
    std::cout << "player_param_blocks_tests: blocks typed by slot and bytes, and listed\n";
    return 0;
}
