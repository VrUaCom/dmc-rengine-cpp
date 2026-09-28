#include "dmc_rengine/reverse/ptx_record_materializer.hpp"
#include <bit>
#include <cstring>

using namespace dmc::rengine::reverse;
namespace {
struct Context {
    const std::uint64_t* config;
    std::uint64_t* log;
    unsigned placements = 0;
    unsigned textures = 0;
    PtxPoolImage* pool;
    PtxTextureInputImage* input;
    std::span<const std::byte> source;
    PtxLoadWorkspace workspace{};
};
std::uint64_t digest(std::span<const std::byte> bytes) {
    std::uint64_t value = 14695981039346656037ULL;
    for (auto b : bytes) { value ^= std::to_integer<unsigned char>(b); value *= 1099511628211ULL; }
    return value;
}
template<class T> T get(const PtxPoolImage& p, std::size_t off) {
    T value; std::memcpy(&value, p.data() + off, sizeof value); return value;
}
template<class T> void put(PtxPoolImage& p, std::size_t off, T value) {
    std::memcpy(p.data() + off, &value, sizeof value);
}
void event(Context& c, std::uint64_t kind, std::uint64_t a, std::uint64_t b,
           std::uint64_t d, std::uint64_t e) {
    auto* p = c.log + 1 + c.log[0] * 5;
    p[0] = kind; p[1] = a; p[2] = b; p[3] = d; p[4] = e; ++c.log[0];
}
std::uint8_t palette(void* v, PtxPoolImage& pool, std::uint64_t address, std::uint64_t argument) {
    auto& c = *static_cast<Context*>(v); const auto off = static_cast<std::size_t>(address - ptx_pool_address);
    event(c, 1, address, argument, digest({pool.data() + off, 0x50}), 0);
    put(pool, off + 0x1a, std::uint16_t{7});
    put(pool, off + 0x1e, std::uint16_t{0x3000});
    return static_cast<std::uint8_t>(c.config[3]);
}
std::uint8_t place(void* v, PtxPoolImage& pool, std::uint64_t address,
                    std::uint32_t flag8, std::uint32_t flag9) {
    auto& c = *static_cast<Context*>(v); const auto off = static_cast<std::size_t>(address - ptx_pool_address);
    event(c, 2, address, flag8 | (static_cast<std::uint64_t>(flag9) << 32),
          digest({pool.data() + off, 0x50}), c.placements);
    put(pool, off + 6, static_cast<std::uint16_t>(0x1000 + (off / 0x50) * 0x80));
    put(pool, off + 0xe, std::uint16_t{2});
    put(pool, off + 0x26, std::uint16_t{2});
    if (get<std::uint16_t>(pool, off + 0x44) == 0 && get<std::uint16_t>(pool, off + 0x18) == 0)
        put(pool, off + 0x1e, std::uint16_t{0x2000});
    const auto result = c.config[4] == c.placements ? c.config[5] : c.config[8];
    ++c.placements;
    return static_cast<std::uint8_t>(result);
}
PtxRecordServices record_services(Context& c) { return {&c, palette, place}; }
int parse(void* v, std::uint64_t address, PtxTextureInputImage& input) {
    auto& c = *static_cast<Context*>(v);
    event(c, 3, address, c.textures, 0, 0);
    input = *c.input;
    if (c.config[9] != 0) {
        const auto count = static_cast<std::uint16_t>(c.config[12 + c.textures]);
        std::memcpy(input.data() + 0x3c, &count, 2);
    }
    ++c.textures; return 1;
}
int build(void* v, PtxTextureInputImage& input, std::int32_t selector,
          std::uint64_t argument, PtxPoolImage& pool) {
    auto& c = *static_cast<Context*>(v);
    return materialize_ptx_records(input, selector, argument, pool, record_services(c));
}
int finalize(void* v, PtxPayloadState& p, PtxPoolImage&) {
    auto& c = *static_cast<Context*>(v); PtxPayload bytes; encode_ptx_payload_state(p, bytes);
    event(c, 4, p.texture_count, p.records_per_texture, digest(bytes), 0); return 1;
}
PtxBundleServices bundle_services(Context& c) { return {&c, parse, build, finalize}; }
int cache_basic(void* v, std::uint64_t key, PtxPayload& bytes) noexcept {
    auto& c = *static_cast<Context*>(v); auto payload = decode_ptx_payload_state(bytes);
    const int result = load_ptx_bundle(c.source, key, payload, *c.pool, c.workspace, bundle_services(c));
    encode_ptx_payload_state(payload, bytes); return result;
}
int cache_variant(void* v, std::uint64_t key, PtxPayload& bytes, std::uint64_t arg) noexcept {
    auto& c = *static_cast<Context*>(v); auto payload = decode_ptx_payload_state(bytes);
    const int result = load_ptx_bundle_variant(c.source, key, payload, *c.pool, c.workspace, arg, bundle_services(c));
    encode_ptx_payload_state(payload, bytes); return result;
}
void cache_release(void* v, PtxPayload& bytes) noexcept {
    auto& c = *static_cast<Context*>(v); auto payload = decode_ptx_payload_state(bytes);
    release_ptx_payload(payload, *c.pool); encode_ptx_payload_state(payload, bytes);
}
} // namespace

extern "C" std::int64_t ptx_materializer_step(void* raw_pool, void* raw_input, void* raw_payload,
    void* raw_manager, const void* source, std::size_t source_size,
    const std::uint64_t* config, std::uint64_t* log) {
    auto& pool = *static_cast<PtxPoolImage*>(raw_pool);
    auto& input = *static_cast<PtxTextureInputImage*>(raw_input);
    auto& payload = *static_cast<PtxPayloadState*>(raw_payload);
    auto& manager = *static_cast<PtxManagerImage*>(raw_manager);
    Context c{config, log, 0, 0, &pool, &input, {static_cast<const std::byte*>(source), source_size}, {}};
    log[0] = 0;
    try {
        if (config[0] == 0) return materialize_ptx_records(input,
            std::bit_cast<std::int32_t>(static_cast<std::uint32_t>(config[1])), config[2], pool, record_services(c));
        if (config[0] == 1) { mark_ptx_record_spans(pool, config[7]); return 0; }
        if (config[0] == 2) return load_ptx_bundle(c.source, config[6], payload, pool, c.workspace, bundle_services(c));
        if (config[0] == 3) return load_ptx_bundle_variant(c.source, config[6], payload, pool, c.workspace, config[2], bundle_services(c));
        const PtxServices s{&c, cache_basic, cache_variant, cache_release, nullptr};
        if (config[0] == 6) return release_ptx(manager, config[6], s);
        const auto* p = config[0] == 4 ? acquire_ptx(manager, config[6], s)
            : acquire_ptx_variant(manager, config[6], config[2], s);
        return p == nullptr ? -1 : reinterpret_cast<const std::byte*>(p) - reinterpret_cast<const std::byte*>(&manager);
    } catch (...) { return -0x70000000; }
}
