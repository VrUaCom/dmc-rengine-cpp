#include "dmc_rengine/reverse/ptx_record_placement.hpp"
#include <bit>
#include <cstring>
#include <stdexcept>

namespace dmc::rengine::reverse {
namespace {
template<class T> T get(const PtxPoolImage& memory, std::size_t offset) {
    if (offset > memory.size() || sizeof(T) > memory.size() - offset)
        throw std::out_of_range("PTX placement read");
    T value;
    std::memcpy(&value, memory.data() + offset, sizeof value);
    return value;
}
template<class T> void put(PtxPoolImage& memory, std::size_t offset, T value) {
    if (offset > memory.size() || sizeof(T) > memory.size() - offset)
        throw std::out_of_range("PTX placement write");
    std::memcpy(memory.data() + offset, &value, sizeof value);
}
std::int32_t add(std::int32_t a, std::int32_t b) noexcept {
    return std::bit_cast<std::int32_t>(static_cast<std::uint32_t>(a) + static_cast<std::uint32_t>(b));
}
std::uint16_t units(std::int32_t index) noexcept {
    return static_cast<std::uint16_t>(static_cast<std::uint32_t>(index) << 5);
}
std::byte at(const PtxPoolImage& pool, std::int64_t offset) {
    if (offset < 0 || offset >= static_cast<std::int64_t>(pool.size()))
        throw std::out_of_range("PTX placement scan");
    return pool[static_cast<std::size_t>(offset)];
}
} // namespace

void initialize_ptx_pool(PtxPoolStorageImage& state, PtxPlacementConfig config) {
    state = {};
    put(state.prefix, ptx_pool_base_units_offset, static_cast<std::uint32_t>(config.word_4c));
    put(state.prefix, 0xcb10, static_cast<std::uint32_t>(config.word_4e));
    put(state.prefix, 0xcb18, static_cast<std::uint32_t>(config.word_4e) - config.word_4c);
}

std::uint8_t configure_ptx_pool_reservation(PtxPoolImage& pool, PtxManagerImage& manager,
                                           std::uint32_t blocks, PtxPlacementConfig config) noexcept {
    const auto reserved = blocks << 5;
    const auto limit = static_cast<std::uint32_t>(config.word_4e) - reserved;
    put(pool, 0xcb14, reserved);
    put(pool, ptx_pool_limit_units_offset, limit);
    put(pool, 0xcb10, limit);
    put(pool, 0xcb18, limit - config.word_4c);
    reset_ptx_keys(manager);
    return 1;
}

std::uint8_t place_ptx_record(PtxPoolImage& pool, std::uint64_t address,
                             std::int32_t requested_index, std::int32_t length_mode,
                             PtxPlacementConfig config) {
    if (address < ptx_pool_address || address > ptx_pool_address + 0x27b0)
        throw std::out_of_range("PTX placement record");
    const auto record = static_cast<std::size_t>(address - ptx_pool_address);
    std::int32_t image_length;
    std::int32_t palette_length = 0;
    const bool palette = get<std::int16_t>(pool, record + 0x1c) >= 0 &&
                         get<std::uint16_t>(pool, record + 0x44) == 0 &&
                         get<std::uint16_t>(pool, record + 0x18) == 0;
    if (length_mode == 0) {
        image_length = 1;
        put(pool, record + 0xe, std::uint16_t{1});
        if (palette) {
            palette_length = 1;
            put(pool, record + 0x26, std::uint16_t{1});
        }
    } else {
        image_length = get<std::int16_t>(pool, record + 0xe);
        if (palette) palette_length = get<std::int16_t>(pool, record + 0x26);
    }
    if (image_length <= 0 || (requested_index == 0 && palette_length < 0))
        throw std::domain_error("PTX placement length outside bounded normal domain");
    const auto base_units = get<std::int32_t>(pool, ptx_pool_base_units_offset);
    auto start = base_units / 32; // Signed division truncates toward zero.
    if (requested_index == 0) {
        const auto delta = add(base_units, -static_cast<std::int32_t>(config.word_4c));
        std::int64_t cursor = static_cast<std::int64_t>(ptx_allocation_map_offset) + delta / 32;
        std::int32_t run = 0;
        for (;;) {
            if (at(pool, cursor) != std::byte{0}) {
                const auto limit = get<std::int32_t>(pool, ptx_pool_limit_units_offset) / 32;
                auto candidate = add(run, start);
                do {
                    ++cursor; run = add(run, 1); candidate = add(candidate, 1);
                    if (candidate >= limit) return 0;
                } while (at(pool, cursor) != std::byte{0});
                start = add(start, run);
                run = 0;
            } else {
                ++cursor; run = add(run, 1);
                if (run == add(image_length, palette_length)) {
                    put(pool, record + 6, units(start));
                    // This gate tests +0x18 only, independently of palette/level.
                    if (get<std::uint16_t>(pool, record + 0x18) == 0)
                        put(pool, record + 0x1e, units(add(start, image_length)));
                    return 1;
                }
            }
        }
    }
    // Preserve the canonical two comparisons, including the >= branch that
    // bypasses the limit read. Do not replace this with a conventional range check.
    if (requested_index < start &&
        requested_index >= get<std::int32_t>(pool, ptx_pool_limit_units_offset) / 32)
        return 0;
    std::int64_t cursor = static_cast<std::int64_t>(ptx_allocation_map_offset) + requested_index;
    for (std::int32_t remaining = image_length; remaining > 0; --remaining, ++cursor)
        if (at(pool, cursor) != std::byte{0}) return 0;
    const auto chosen = add(start, requested_index);
    put(pool, record + 6, units(chosen));
    put(pool, record + 0x1e, units(add(chosen, get<std::uint16_t>(pool, record + 0xe))));
    return 1;
}
} // namespace dmc::rengine::reverse
