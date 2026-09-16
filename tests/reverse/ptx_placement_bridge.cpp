#include "dmc_rengine/reverse/ptx_record_placement.hpp"
#include "dmc_rengine/reverse/ptx_record_materializer.hpp"
#include <bit>
#include <cstring>

using namespace dmc::rengine::reverse;
namespace {
struct Context {
    const std::uint64_t* config;
    std::uint64_t* log;
    PtxPoolImage* pool;
    const PtxTextureInputImage* input;
    std::span<const std::byte> source;
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
    auto* p = c.log + 1 + c.log[0] * 5;
    p[0] = kind; p[1] = a; p[2] = b; p[3] = d; p[4] = e; ++c.log[0];
}
std::uint8_t place(void* v, PtxPoolImage& pool, std::uint64_t address,
                    std::uint32_t index, std::uint32_t mode) {
    auto& c = *static_cast<Context*>(v);
    event(c, 1, address, index, mode, digest({pool.data() + (address - ptx_pool_address), 0x50}));
    return place_ptx_record(pool, address, std::bit_cast<std::int32_t>(index),
                            std::bit_cast<std::int32_t>(mode),
                            {static_cast<std::uint16_t>(c.config[5]), static_cast<std::uint16_t>(c.config[8])});
}
std::uint8_t palette(void* v, PtxPoolImage& pool, std::uint64_t address, std::uint64_t argument) {
    auto& c = *static_cast<Context*>(v); auto* p = pool.data() + (address - ptx_pool_address);
    event(c, 2, address, argument, digest({p, 0x50}), 0);
    const std::uint16_t selector = 7, base = 0x3000;
    std::memcpy(p + 0x1a, &selector, 2); std::memcpy(p + 0x1e, &base, 2);
    return static_cast<std::uint8_t>(c.config[6]);
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

extern "C" std::int64_t ptx_placement_step(void* raw_pool, const void* raw_input, void* raw_payload,
    void* raw_manager, const void* source, std::size_t source_size,
    const std::uint64_t* config, std::uint64_t* log) {
    auto& storage = *static_cast<PtxPoolStorageImage*>(raw_pool);
    auto& pool = storage.prefix;
    const auto& input = *static_cast<const PtxTextureInputImage*>(raw_input);
    auto& payload = *static_cast<PtxPayloadState*>(raw_payload);
    auto& manager = *static_cast<PtxManagerImage*>(raw_manager);
    Context c{config, log, &pool, &input, {static_cast<const std::byte*>(source), source_size}, {}, 0};
    log[0] = 0;
    try {
        const PtxPlacementConfig profile{static_cast<std::uint16_t>(config[5]), static_cast<std::uint16_t>(config[8])};
        if (config[0] == 7) { initialize_ptx_pool(storage, profile); return 0; }
        if (config[0] == 8) return configure_ptx_pool_reservation(pool, manager, static_cast<std::uint32_t>(config[9]), profile);
        if (config[0] == 0) return place(&c, pool, config[4],
            static_cast<std::uint32_t>(config[1]), static_cast<std::uint32_t>(config[2]));
        if (config[0] == 1) return materialize_ptx_records(input, -1, config[3], pool, records(c));
        if (config[0] == 2) return load_ptx_bundle(c.source, config[7], payload, pool, c.workspace, bundles(c));
        if (config[0] == 3) return load_ptx_bundle_variant(c.source, config[7], payload, pool, c.workspace, config[3], bundles(c));
        const PtxServices s{&c, basic, variant, release, nullptr};
        if (config[0] == 6) return release_ptx(manager, config[7], s);
        const auto* p = config[0] == 4 ? acquire_ptx(manager, config[7], s)
            : acquire_ptx_variant(manager, config[7], config[3], s);
        return p == nullptr ? -1 : reinterpret_cast<const std::byte*>(p) - reinterpret_cast<const std::byte*>(&manager);
    } catch (...) { return -0x70000000; }
}
