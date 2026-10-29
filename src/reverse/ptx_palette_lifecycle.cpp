#include "dmc_rengine/reverse/ptx_palette_lifecycle.hpp"
#include "dmc_rengine/reverse/ptx_record_materializer.hpp"
#include <bit>
#include <cstring>

namespace dmc::rengine::reverse {
namespace {
template<class T> void put(PtxPoolImage& pool, std::size_t at, T value) {
    std::memcpy(pool.data() + at, &value, sizeof value);
}
} // namespace

std::uint8_t initialize_ptx_palette_context(PtxPaletteContextImage& context, PtxPoolImage& pool,
    std::int32_t count13, std::int32_t count14, PtxPlacementConfig profile,
    BlockRoutingFlags routing, BlockAllocatorView& memory) {
    for (std::size_t bank = 0; bank < 2; ++bank) {
        const auto count = bank == 0 ? count13 : count14;
        if (count <= 0) continue;
        const auto address = allocate_ptx_record(pool);
        if (address == 0) return 0;
        const auto record = static_cast<std::size_t>(address - ptx_pool_address);
        const std::uint32_t rounding = bank == 0 ? 7U : 31U;
        const auto groups = std::bit_cast<std::int32_t>(static_cast<std::uint32_t>(count) + rounding)
                            / static_cast<std::int32_t>(rounding + 1U);
        const auto height = static_cast<std::uint16_t>(static_cast<std::uint32_t>(groups) << 5);
        put(pool, record + 4, std::uint16_t{0}); put(pool, record + 8, std::uint16_t{1});
        put(pool, record + 0xa, std::uint16_t{64}); put(pool, record + 0xc, height);
        put(pool, record + 0x1c, std::uint16_t{0xffff}); put(pool, record + 0x44, std::uint16_t{0});
        put(pool, record + 0x18, std::uint32_t{0});
        if (place_ptx_record(pool, address, 0, 0, profile) == 0) return 0;
        mark_ptx_record_spans(pool, address);
        context.prefix.bank_records[bank] = address;
        context.prefix.next[bank] = 0;
        context.prefix.capacity[bank] = static_cast<std::uint16_t>(count);
        // Format +4 is set to zero above; real placement/mark preserve it.
        const auto bytes = static_cast<std::uint32_t>(static_cast<std::int32_t>(std::bit_cast<std::int16_t>(height))) * 256U;
        const auto data = allocate_routed_blocks(context.allocations[bank], bytes, 1, routing, memory);
        put(pool, record + 0x10, data);
        if (data == 0) return 0;
        memory.services.fill(memory.services.context, data, 0, (bytes >> 2) << 2);
    }
    return 1;
}

void destroy_ptx_palette_context(PtxPaletteContextImage& context, PtxPoolImage& pool,
                                 BlockAllocatorView& memory) {
    for (std::size_t bank = 0; bank < 2; ++bank) {
        release_ptx_record(pool, context.prefix.bank_records[bank]);
        context.prefix.bank_records[bank] = 0;
        context.prefix.capacity[bank] = 0;
        context.prefix.next[bank] = 0;
        release_runtime_blocks(context.allocations[bank], memory);
    }
}
} // namespace dmc::rengine::reverse
