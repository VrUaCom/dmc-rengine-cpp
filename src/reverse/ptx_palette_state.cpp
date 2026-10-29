#include "dmc_rengine/reverse/ptx_palette_state.hpp"
#include <bit>
#include <cstring>
#include <stdexcept>

namespace dmc::rengine::reverse {
namespace {
// Signed table reads can reach bytes BEFORE the nominal tables when a uint16
// counter >=0x8000 is sign-extended. Preserve the canonical neighboring data;
// these prefixes are not claimed to be part of the semantic lookup tables.
// 0x1405D16D1..0x1405D170F: indexes -31..31 around table 0x1405D16F0.
constexpr std::array<std::int8_t, 63> table14{
    121,22,63,-67,27,79,-65,0,0,0,0,0,0,-128,63,0,0,0,63,-85,-86,-86,62,
    0,0,-128,62,-51,-52,76,62,
    0,1,4,5,16,17,20,21,2,3,6,7,18,19,22,23,
    8,9,12,13,24,25,28,29,10,11,14,15,26,27,30,31};
// 0x1405D1709..0x1405D1717: indexes -7..7 around table 0x1405D1710.
constexpr std::array<std::int8_t, 15> table13{11,14,15,26,27,30,31,0,4,16,20,8,12,24,28};

std::size_t offset_of(std::uint64_t address) {
    if (address < ptx_pool_address || address > ptx_pool_address + 0x27b0)
        throw std::out_of_range("PTX palette record view");
    return static_cast<std::size_t>(address - ptx_pool_address);
}
template<class T> T get(const PtxPoolImage& pool, std::size_t offset) {
    T value; std::memcpy(&value, pool.data() + offset, sizeof value); return value;
}
void put_word(PtxPoolImage& pool, std::size_t offset, std::uint16_t value) {
    std::memcpy(pool.data() + offset, &value, sizeof value);
}
std::int32_t signed_word(std::uint16_t value) { return std::bit_cast<std::int16_t>(value); }
} // namespace

std::uint8_t prepare_ptx_palette(PtxPoolImage& pool, std::uint64_t address,
    PtxPaletteContextState& context, std::uint32_t incoming_home_word,
    const PtxPaletteServices& services) {
    const auto record = offset_of(address);
    const auto format = get<std::uint16_t>(pool, record + 4);
    auto destination = get<std::uint64_t>(pool, record + 0x28);
    if (format != 0x13 && format != 0x14) {
        put_word(pool, record + 0x1e, static_cast<std::uint16_t>(incoming_home_word));
        return 1;
    }
    const std::size_t bank = format == 0x14 ? 1U : 0U;
    auto selector = get<std::uint16_t>(pool, record + 0x1a);
    std::int32_t copy_index;
    if (signed_word(selector) < 0) {
        selector = context.next[bank];
        if (selector >= context.capacity[bank]) return 0; // Unsigned comparison.
        put_word(pool, record + 0x1a, selector);
        context.next[bank] = static_cast<std::uint16_t>(selector + 1U);
        copy_index = signed_word(selector);
    } else {
        copy_index = signed_word(static_cast<std::uint16_t>(incoming_home_word));
    }
    const auto bank_record = offset_of(context.bank_records[bank]);
    const std::int64_t delta = format == 0x14
        ? static_cast<std::int64_t>(copy_index / 8) * 2048 + (copy_index % 8) * 32
        : static_cast<std::int64_t>(copy_index / 4) * 4096 + (copy_index % 4) * 64;
    auto source = get<std::uint64_t>(pool, bank_record + 0x10) + static_cast<std::uint64_t>(delta);
    const std::uint32_t count = format == 0x14 ? 2U : 16U;
    const std::uint32_t bytes = format == 0x14 ? 32U : 64U;
    for (std::uint32_t row = 0; row < count; ++row) {
        services.move_bytes(services.context, destination, source, bytes);
        destination += bytes;
        source += 256;
    }
    const auto index = signed_word(get<std::uint16_t>(pool, record + 0x1a));
    const auto base = get<std::int16_t>(pool, bank_record + 6);
    const auto result = format == 0x14
        ? base + (index / 32) * 32 + table14[static_cast<std::size_t>(31 + index % 32)]
        : base + (index / 8) * 32 + table13[static_cast<std::size_t>(7 + index % 8)];
    put_word(pool, record + 0x1e, static_cast<std::uint16_t>(result));
    return 1;
}
} // namespace dmc::rengine::reverse
