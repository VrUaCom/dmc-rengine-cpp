#include "dmc_rengine/reverse/ptx_palette_lifecycle.hpp"
#include <bit>
#include <cstring>
#include <stdexcept>

using namespace dmc::rengine::reverse;
namespace {
constexpr std::array<std::uint64_t, 11> addresses{
    ptx_pool_address, 0x55000040, runtime_block_arenas_address,
    0x57000040, 0x58000040, 0x59000040, 0x5a000040, 0x5b000040, 0x5c000040,
    0x54000040, 0x56000040};
struct Context {
    void* const* buffers;
    const std::size_t* sizes;
    std::uint64_t* log;
};
std::uint64_t digest(std::span<const std::byte> bytes) {
    std::uint64_t value = 14695981039346656037ULL;
    for (auto b : bytes) { value ^= std::to_integer<unsigned char>(b); value *= 1099511628211ULL; }
    return value;
}
std::span<std::byte> region(Context& c, std::size_t i) {
    return {static_cast<std::byte*>(c.buffers[i]), c.sizes[i]};
}
void event(Context& c, std::uint64_t kind, std::uint64_t a, std::uint64_t b,
           std::uint64_t count, std::uint64_t extra = 0) {
    if (c.log[0] >= 200) throw std::out_of_range("lifecycle events");
    auto* p = c.log + 1 + 7 * c.log[0]++;
    p[0] = kind; p[1] = a; p[2] = b; p[3] = count;
    p[4] = digest(region(c, 1)); p[5] = digest(region(c, 2));
    p[6] = kind == 3 ? extra : digest(region(c, 0));
}
std::span<std::byte> view(Context& c, std::uint64_t address, std::uint64_t count) {
    for (std::size_t i = 0; i < addresses.size(); ++i) {
        if (address >= addresses[i] && address - addresses[i] <= c.sizes[i] &&
            count <= c.sizes[i] - (address - addresses[i]))
            return region(c, i).subspan(static_cast<std::size_t>(address - addresses[i]), static_cast<std::size_t>(count));
    }
    throw std::out_of_range("lifecycle memory view");
}
void fill(void* v, std::uint64_t address, std::uint8_t value, std::uint64_t count) {
    auto& c = *static_cast<Context*>(v); auto bytes = view(c, address, count);
    event(c, 1, address, value, count);
    std::memset(bytes.data(), value, bytes.size());
}
void drain(void* v, BlockAllocationState& node) {
    auto& c = *static_cast<Context*>(v);
    const auto offset = reinterpret_cast<std::byte*>(&node) - static_cast<std::byte*>(c.buffers[1]);
    event(c, 2, 0x55000040 + static_cast<std::uint64_t>(offset), node.callback_head,
          static_cast<std::uint32_t>(node.first));
    node.callback_head = 0; // Explicit synthetic boundary, not a recovered queue implementation.
}
void move(void* v, std::uint64_t dest, std::uint64_t source, std::uint32_t count) {
    auto& c = *static_cast<Context*>(v); auto from = view(c, source, count); auto to = view(c, dest, count);
    event(c, 3, dest, source, count, digest(from));
    std::memmove(to.data(), from.data(), count);
}
} // namespace

// buffers: pool, context, three arena states, map0/1/2, data0/1/2, profile, destination.
// config: operation, count13/bytes, count14/route mode, arena index,
// fallback flag, force-arena2 flag, allocation index, palette record address.
extern "C" std::int64_t ptx_palette_lifecycle_step(void* const* buffers, const std::size_t* sizes,
    const std::uint64_t* config, std::uint64_t* log) {
    Context c{buffers, sizes, log}; log[0] = 0;
    auto& context = *static_cast<PtxPaletteContextImage*>(buffers[1]);
    auto& pool = static_cast<PtxPoolStorageImage*>(buffers[0])->prefix;
    auto* states = static_cast<BlockArenaState*>(buffers[2]);
    std::array<BlockArenaView, 3> arenas;
    for (std::size_t i = 0; i < 3; ++i) {
        std::span<const std::byte> map;
        if (states[i].occupancy >= addresses[3 + i] && states[i].occupancy - addresses[3 + i] <= sizes[3 + i])
            map = region(c, 3 + i).subspan(static_cast<std::size_t>(states[i].occupancy - addresses[3 + i]));
        arenas[i] = {runtime_block_arenas_address + i * 0x28, states + i, map};
    }
    BlockAllocatorView memory{arenas, {&c, fill, drain}};
    const auto signed32 = [](std::uint64_t value) { return std::bit_cast<std::int32_t>(static_cast<std::uint32_t>(value)); };
    const BlockRoutingFlags flags{static_cast<std::uint8_t>(config[4]), static_cast<std::uint8_t>(config[5])};
    try {
        auto& node = context.allocations.at(static_cast<std::size_t>(config[6]));
        if (config[0] == 0) return initialize_ptx_palette_context(context, pool,
            signed32(config[1]), signed32(config[2]), {0x1000, 0x1800}, flags, memory);
        if (config[0] == 1) { destroy_ptx_palette_context(context, pool, memory); return 0; }
        if (config[0] == 2) return static_cast<std::int64_t>(allocate_routed_blocks(node,
            static_cast<std::uint32_t>(config[1]), signed32(config[2]), flags, memory));
        if (config[0] == 3) return static_cast<std::int64_t>(allocate_runtime_blocks(node,
            config[3] == UINT64_MAX ? 0 : runtime_block_arenas_address + config[3] * 0x28,
            static_cast<std::uint32_t>(config[1]), memory));
        if (config[0] == 4) return release_runtime_blocks(node, memory);
        if (config[0] == 5) return find_runtime_block_run(arenas.at(static_cast<std::size_t>(config[3])),
            static_cast<std::uint32_t>(config[1]));
        return prepare_ptx_palette(pool, config[7], context.prefix, 0, {&c, move});
    } catch (...) { return -0x70000000; }
}
