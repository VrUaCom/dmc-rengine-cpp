#include "dmc_rengine/reverse/ptx_payload_state.hpp"
#include <algorithm>
#include <bit>
#include <cstring>

using namespace dmc::rengine::reverse;
namespace {
struct Context {
    const std::uint64_t* config;
    std::uint64_t* log;
    unsigned index = 0;
    PtxPayloadState* payload;
    PtxPoolImage* pool;
    PtxLoadWorkspace* workspace;
    std::span<const std::byte> source;
    std::uint64_t source_address;
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
int parse(void* v, std::uint64_t address, PtxTextureInputImage& desc) {
    auto& c = *static_cast<Context*>(v);
    event(c, 1, address, digest(desc), c.payload->texture_count, c.payload->records_per_texture);
    std::memcpy(desc.data(), &address, 8);
    desc[0x38] = std::byte(c.index);
    return c.config[3] == c.index ? 0 : 1;
}
int build(void* v, PtxTextureInputImage& desc, std::int32_t selector,
          std::uint64_t argument, PtxPoolImage& pool) {
    auto& c = *static_cast<Context*>(v);
    event(c, 2, static_cast<std::uint32_t>(selector), argument, digest(desc), c.index);
    auto width = static_cast<std::uint32_t>(c.config[8 + c.index]);
    std::memcpy(pool.data() + ptx_scratch_count_offset, &width, 4);
    std::memset(pool.data() + ptx_scratch_records_offset, 0, 32);
    const auto count = std::clamp(std::bit_cast<std::int32_t>(width), 0, 4);
    for (int j = 0; j < count; ++j) {
        const auto index = c.index * 4 + static_cast<unsigned>(j);
        const auto offset = index * 0x50;
        for (unsigned k = 0; k < 0x50; ++k) pool[offset + k] = std::byte((index * 13 + k * 7 + 3) & 255);
        const std::uint16_t one = 1, id = static_cast<std::uint16_t>(index), zero = 0;
        std::memcpy(pool.data() + offset, &one, 2);
        std::memcpy(pool.data() + offset + 2, &id, 2);
        std::memcpy(pool.data() + offset + 0x46, &zero, 2);
        const auto address = ptx_pool_address + offset;
        std::memcpy(pool.data() + ptx_scratch_records_offset + static_cast<unsigned>(j) * 8, &address, 8);
    }
    const bool fail = c.config[4] == c.index;
    ++c.index;
    return fail ? 0 : 1;
}
int finalize(void* v, PtxPayloadState& payload, PtxPoolImage&) {
    auto& c = *static_cast<Context*>(v);
    PtxPayload bytes;
    encode_ptx_payload_state(payload, bytes);
    event(c, 3, payload.texture_count, payload.records_per_texture, digest(bytes), 0);
    return std::bit_cast<std::int32_t>(static_cast<std::uint32_t>(c.config[5]));
}
PtxBundleServices services(Context& c) { return {&c, parse, build, finalize}; }
int cache_load(void* v, std::uint64_t key, PtxPayload& bytes) noexcept {
    auto& c = *static_cast<Context*>(v);
    auto payload = decode_ptx_payload_state(bytes); c.payload = &payload;
    const int result = load_ptx_bundle(c.source, key, payload, *c.pool, *c.workspace, services(c));
    encode_ptx_payload_state(payload, bytes);
    return result;
}
int cache_variant(void* v, std::uint64_t key, PtxPayload& bytes, std::uint64_t argument) noexcept {
    auto& c = *static_cast<Context*>(v);
    auto payload = decode_ptx_payload_state(bytes); c.payload = &payload;
    const int result = load_ptx_bundle_variant(c.source, key, payload, *c.pool, *c.workspace, argument, services(c));
    encode_ptx_payload_state(payload, bytes);
    return result;
}
void cache_release(void* v, PtxPayload& bytes) noexcept {
    auto& c = *static_cast<Context*>(v);
    auto payload = decode_ptx_payload_state(bytes);
    release_ptx_payload(payload, *c.pool); encode_ptx_payload_state(payload, bytes);
}
} // namespace

extern "C" std::int64_t ptx_payload_step(void* raw_payload, void* raw_pool, void* raw_workspace,
    const void* raw_source, std::size_t source_size, const std::uint64_t* config,
    std::uint64_t* log, void* raw_manager) {
    auto& payload = *static_cast<PtxPayloadState*>(raw_payload);
    auto& pool = *static_cast<PtxPoolImage*>(raw_pool);
    auto& workspace = *static_cast<PtxLoadWorkspace*>(raw_workspace);
    Context c{config, log, 0, &payload, &pool, &workspace,
              {static_cast<const std::byte*>(raw_source), source_size}, config[6]};
    log[0] = 0;
    try {
        switch (config[0]) {
        case 0: return static_cast<std::int64_t>(allocate_ptx_record(pool));
        case 1: release_ptx_record_spans(pool, config[1]); return 0;
        case 2: release_ptx_record(pool, config[1]); return 0;
        case 3: release_ptx_payload(payload, pool); return 0;
        case 4: return load_ptx_bundle(c.source, config[6], payload, pool, workspace, services(c));
        case 5: return load_ptx_bundle_variant(c.source, config[6], payload, pool, workspace, config[2], services(c));
        default: {
            auto& manager = *static_cast<PtxManagerImage*>(raw_manager);
            const PtxServices s{&c, cache_load, cache_variant, cache_release, nullptr};
            if (config[0] == 8) return release_ptx(manager, config[6], s);
            const auto* p = config[0] == 6 ? acquire_ptx(manager, config[6], s)
                : acquire_ptx_variant(manager, config[6], config[2], s);
            return p == nullptr ? -1 : reinterpret_cast<const std::byte*>(p) - reinterpret_cast<const std::byte*>(&manager);
        }
        }
    } catch (...) { return -0x70000000; }
}
