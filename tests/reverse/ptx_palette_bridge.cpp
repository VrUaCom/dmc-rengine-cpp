#include "dmc_rengine/reverse/ptx_palette_state.hpp"
#include "dmc_rengine/reverse/ptx_record_materializer.hpp"
#include "dmc_rengine/reverse/ptx_record_placement.hpp"
#include <bit>
#include <cstring>
#include <stdexcept>

using namespace dmc::rengine::reverse;
namespace {
struct Region { std::uint64_t address; std::span<std::byte> bytes; };
struct Context {
    const std::uint64_t* config;
    std::uint64_t* log;
    PtxPoolImage* pool;
    PtxPaletteContextState* palette;
    const PtxTextureInputImage* input;
    std::span<const std::byte> source;
    std::array<Region, 3> buffers;
    PtxLoadWorkspace workspace{};
    unsigned textures = 0;
};
std::uint64_t digest(std::span<const std::byte> bytes) {
    std::uint64_t value = 14695981039346656037ULL;
    for (auto b : bytes) { value ^= std::to_integer<unsigned char>(b); value *= 1099511628211ULL; }
    return value;
}
void event(Context& c, std::uint64_t kind, std::uint64_t a, std::uint64_t b,
           std::uint64_t d, std::uint64_t e) {
    if (c.log[0] >= 400) throw std::out_of_range("PTX palette event capacity");
    auto* p = c.log + 1 + c.log[0] * 5;
    p[0] = kind; p[1] = a; p[2] = b; p[3] = d; p[4] = e; ++c.log[0];
}
std::span<std::byte> view(Context& c, std::uint64_t address, std::uint32_t length) {
    for (auto& r : c.buffers) {
        if (address >= r.address && address - r.address <= r.bytes.size() &&
            length <= r.bytes.size() - (address - r.address))
            return r.bytes.subspan(static_cast<std::size_t>(address - r.address), length);
    }
    throw std::out_of_range("PTX palette memmove view");
}
void move_bytes(void* v, std::uint64_t dest, std::uint64_t source, std::uint32_t length) {
    auto& c = *static_cast<Context*>(v);
    auto from = view(c, source, length); auto to = view(c, dest, length);
    event(c, 5, dest, source, length, digest(from));
    std::memmove(to.data(), from.data(), length);
}
std::uint8_t palette(void* v, PtxPoolImage& pool, std::uint64_t address, std::uint64_t argument) {
    auto& c = *static_cast<Context*>(v);
    if (argument != 0x55000040) throw std::out_of_range("PTX palette context view");
    event(c, 2, address, argument, digest({pool.data() + (address - ptx_pool_address), 0x50}), 0);
    return prepare_ptx_palette(pool, address, *c.palette, static_cast<std::uint32_t>(c.config[2]),
                                {&c, move_bytes});
}
std::uint8_t place(void* v, PtxPoolImage& pool, std::uint64_t address,
                    std::uint32_t index, std::uint32_t mode) {
    auto& c = *static_cast<Context*>(v);
    event(c, 1, address, index, mode, digest({pool.data() + (address - ptx_pool_address), 0x50}));
    return place_ptx_record(pool, address, std::bit_cast<std::int32_t>(index),
        std::bit_cast<std::int32_t>(mode), {static_cast<std::uint16_t>(c.config[5]),
                                          static_cast<std::uint16_t>(c.config[6])});
}
PtxRecordServices records(Context& c) { return {&c, palette, place}; }
int parse(void* v, std::uint64_t address, PtxTextureInputImage& input) {
    auto& c = *static_cast<Context*>(v);
    event(c, 3, address, c.textures++, 0, 0); input = *c.input; return 1;
}
int materialize(void* v, PtxTextureInputImage& input, std::int32_t selector,
                std::uint64_t argument, PtxPoolImage& pool) {
    auto& c = *static_cast<Context*>(v);
    return materialize_ptx_records(input, selector, argument, pool, records(c));
}
int finalize(void* v, PtxPayloadState& payload, PtxPoolImage&) {
    auto& c = *static_cast<Context*>(v); PtxPayload bytes; encode_ptx_payload_state(payload, bytes);
    event(c, 4, payload.texture_count, payload.records_per_texture, digest(bytes), 0); return 1;
}
PtxBundleServices bundles(Context& c) { return {&c, parse, materialize, finalize}; }
int basic(void* v, std::uint64_t key, PtxPayload& bytes) noexcept {
    auto& c = *static_cast<Context*>(v); auto payload = decode_ptx_payload_state(bytes);
    const int result = load_ptx_bundle(c.source, key, payload, *c.pool, c.workspace, bundles(c));
    encode_ptx_payload_state(payload, bytes); return result;
}
int variant(void* v, std::uint64_t key, PtxPayload& bytes, std::uint64_t argument) noexcept {
    auto& c = *static_cast<Context*>(v); auto payload = decode_ptx_payload_state(bytes);
    const int result = load_ptx_bundle_variant(c.source, key, payload, *c.pool, c.workspace, argument, bundles(c));
    encode_ptx_payload_state(payload, bytes); return result;
}
void release(void* v, PtxPayload& bytes) noexcept {
    auto& c = *static_cast<Context*>(v); auto payload = decode_ptx_payload_state(bytes);
    release_ptx_payload(payload, *c.pool); encode_ptx_payload_state(payload, bytes);
}
} // namespace

// blocks: pool, input, payload, manager, source, palette context, destination,
// bank13 bytes, bank14 bytes, profile. Sizes exclude the two 64-byte guards.
// config: op, selector, incoming home dword, context address, record address,
// profile start, profile end, cache/source key.
extern "C" std::int64_t ptx_palette_step(void* const* blocks, const std::size_t* sizes,
                                        const std::uint64_t* config, std::uint64_t* log) {
    auto& pool = static_cast<PtxPoolStorageImage*>(blocks[0])->prefix;
    const auto& input = *static_cast<const PtxTextureInputImage*>(blocks[1]);
    auto& payload = *static_cast<PtxPayloadState*>(blocks[2]);
    auto& manager = *static_cast<PtxManagerImage*>(blocks[3]);
    Context c{config, log, &pool, static_cast<PtxPaletteContextState*>(blocks[5]), &input,
        {static_cast<const std::byte*>(blocks[4]), sizes[4]}, {}, {}, 0};
    for (std::size_t i = 0; i < 3; ++i)
        c.buffers[i] = {0x56000040 + i * 0x1000000, {static_cast<std::byte*>(blocks[6 + i]), sizes[6 + i]}};
    log[0] = 0;
    try {
        if (config[0] == 0) return palette(&c, pool, config[4], config[3]);
        if (config[0] == 1) return materialize_ptx_records(input,
            std::bit_cast<std::int32_t>(static_cast<std::uint32_t>(config[1])), config[3], pool, records(c));
        if (config[0] == 2) return load_ptx_bundle(c.source, config[7], payload, pool, c.workspace, bundles(c));
        if (config[0] == 3) return load_ptx_bundle_variant(c.source, config[7], payload, pool, c.workspace, config[3], bundles(c));
        const PtxServices services{&c, basic, variant, release, nullptr};
        if (config[0] == 6) return release_ptx(manager, config[7], services);
        const auto* p = config[0] == 4 ? acquire_ptx(manager, config[7], services)
            : acquire_ptx_variant(manager, config[7], config[3], services);
        return p == nullptr ? -1 : reinterpret_cast<const std::byte*>(p) - reinterpret_cast<const std::byte*>(&manager);
    } catch (...) { return -0x70000000; }
}
