#include "dmc_rengine/reverse/ptx_manager_state.hpp"
#include <algorithm>
#include <cstring>

namespace {
using namespace dmc::rengine::reverse;
struct Context {
    PtxManagerImage* state;
    std::uint64_t* log;
    int load_result;
    unsigned pattern;
};
void fill(PtxPayload& payload, unsigned pattern) {
    for (std::size_t i = 0; i < payload.size(); ++i)
        payload[i] = static_cast<std::byte>((pattern + i * 17U) & 255U);
}
int load(void* opaque, std::uint64_t key, PtxPayload& payload, std::uint64_t argument, unsigned kind) noexcept {
    auto& c = *static_cast<Context*>(opaque);
    ++c.log[0]; c.log[1] = kind; c.log[2] = key; c.log[3] = argument;
    c.log[6] = std::all_of(payload.begin(), payload.end(), [](std::byte b) { return b == std::byte{0}; });
    fill(payload, c.pattern);
    return c.load_result;
}
int basic(void* c, std::uint64_t k, PtxPayload& p) noexcept { return load(c, k, p, 0, 1); }
int variant(void* c, std::uint64_t k, PtxPayload& p, std::uint64_t a) noexcept { return load(c, k, p, a, 2); }
void release(void* opaque, PtxPayload& payload) noexcept {
    auto& c = *static_cast<Context*>(opaque);
    const auto offset = reinterpret_cast<std::uintptr_t>(&payload) - reinterpret_cast<std::uintptr_t>(c.state);
    ++c.log[0]; c.log[1] = 3; c.log[4] = offset; c.log[5] = payload.size();
    const auto index = (offset - 0x18) / 0x218;
    c.log[7] = c.state->entries[index].resource_key;
    c.log[8] = c.state->entries[index].references;
    fill(payload, 0x5a);
}
void deallocate(void* opaque, PtxManagerImage* state, std::size_t size) noexcept {
    auto& c = *static_cast<Context*>(opaque);
    ++c.log[0]; c.log[1] = 4; c.log[5] = size; c.log[7] = state->vtable_va;
}
}

extern "C" std::int64_t ptx_step(unsigned char* bytes, int op, std::uint64_t key,
    std::uint64_t argument, std::uint32_t flags, int loaded, unsigned pattern, std::uint64_t* log) {
    PtxManagerImage state;
    std::memcpy(&state, bytes + 64, sizeof(state));
    std::fill(log, log + 9, 0);
    Context context{&state, log, loaded, pattern};
    PtxServices services{&context, basic, variant, release, deallocate};
    std::int64_t result = 0;
    PtxPayload* payload = nullptr;
    if (op == 0) result = &construct_ptx_manager(state) == &state ? 0 : -2;
    if (op == 1) reset_ptx_keys(state);
    if (op == 2) payload = acquire_ptx(state, key, services);
    if (op == 3) payload = acquire_ptx_variant(state, key, argument, services);
    if (op == 4) result = release_ptx(state, key, services) ? 1 : 0;
    if (op == 5) destroy_ptx_manager(state);
    if (op == 6) result = deleting_destroy_ptx_manager(state, flags, services) == &state ? 0 : -2;
    if (op == 2 || op == 3) result = payload
        ? static_cast<std::int64_t>(reinterpret_cast<std::uintptr_t>(payload) - reinterpret_cast<std::uintptr_t>(&state)) : -1;
    std::memcpy(bytes + 64, &state, sizeof(state));
    return result;
}
