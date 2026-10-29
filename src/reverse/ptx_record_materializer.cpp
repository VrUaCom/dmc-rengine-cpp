#include "dmc_rengine/reverse/ptx_record_materializer.hpp"
#include <bit>
#include <cstring>
#include <stdexcept>

namespace dmc::rengine::reverse {
namespace {
template<class T> T get(std::span<const std::byte> memory, std::size_t offset) {
    if (offset > memory.size() || sizeof(T) > memory.size() - offset)
        throw std::out_of_range("PTX materializer read");
    T value;
    std::memcpy(&value, memory.data() + offset, sizeof value);
    return value;
}
template<class T> void put(PtxPoolImage& memory, std::size_t offset, T value) {
    if (offset > memory.size() || sizeof(T) > memory.size() - offset)
        throw std::out_of_range("PTX materializer write");
    std::memcpy(memory.data() + offset, &value, sizeof value);
}
std::size_t offset_of(std::uint64_t address) {
    if (address < ptx_pool_address || address > ptx_pool_address + 0x27b0)
        throw std::out_of_range("PTX materializer record");
    return static_cast<std::size_t>(address - ptx_pool_address);
}
void mark_span(PtxPoolImage& pool, std::int16_t base, std::int16_t length) {
    if (length <= 0) throw std::domain_error("PTX mark span outside normal domain");
    const auto units = get<std::uint32_t>(pool, ptx_pool_base_units_offset);
    const auto delta = std::bit_cast<std::int32_t>(
        static_cast<std::uint32_t>(static_cast<std::int32_t>(base)) - units);
    const auto start = static_cast<std::int64_t>(ptx_allocation_map_offset) + delta / 32;
    if (start < 0 || start > static_cast<std::int64_t>(pool.size()) - length)
        throw std::out_of_range("PTX mark span");
    std::memset(pool.data() + start, 1, static_cast<std::size_t>(length));
}
std::uint64_t scratch(const PtxPoolImage& pool, std::uint32_t index) {
    return get<std::uint64_t>(pool, ptx_scratch_records_offset + static_cast<std::size_t>(index) * 8);
}
} // namespace

void mark_ptx_record_spans(PtxPoolImage& pool, std::uint64_t address) {
    const auto offset = offset_of(address);
    put(pool, offset + 0x46, std::uint16_t{1});
    mark_span(pool, get<std::int16_t>(pool, offset + 6), get<std::int16_t>(pool, offset + 0xe));
    if (get<std::int16_t>(pool, offset + 0x1c) >= 0 &&
        get<std::int16_t>(pool, offset + 0x18) == 0 &&
        get<std::int16_t>(pool, offset + 0x44) == 0)
        mark_span(pool, get<std::int16_t>(pool, offset + 0x1e), get<std::int16_t>(pool, offset + 0x26));
}

int materialize_ptx_records(const PtxTextureInputImage& input, std::int32_t selector,
                            std::uint64_t argument, PtxPoolImage& pool, const PtxRecordServices& services) {
    put(pool, ptx_scratch_count_offset, std::uint32_t{0});
    std::memset(pool.data() + ptx_scratch_records_offset, 0, 4 * 8);
    std::uint64_t first = 0;
    for (std::uint32_t level = 0;
         static_cast<std::int32_t>(level) < get<std::int16_t>(input, 0x3c); ++level) {
        const auto address = allocate_ptx_record(pool);
        if (address == 0) return 0; // This original failure edge bypasses rollback.
        put(pool, ptx_scratch_count_offset, get<std::uint32_t>(pool, ptx_scratch_count_offset) + 1U);
        put(pool, ptx_scratch_records_offset + static_cast<std::size_t>(level) * 8, address);
        const auto record = offset_of(address);
        put(pool, record + 0x44, static_cast<std::uint16_t>(level));
        put(pool, record + 4, get<std::uint16_t>(input, 0x10));
        if (level == 0) {
            first = address;
            put(pool, record + 8, get<std::uint16_t>(input, 0x12));
            put(pool, record + 0xa, get<std::uint16_t>(input, 0x14));
            put(pool, record + 0xc, get<std::uint16_t>(input, 0x16));
            put(pool, record + 0x10, get<std::uint64_t>(input, 0));
        } else {
            const auto shift = level & 31U; // x86 SAR uses the low five count bits.
            auto width = static_cast<std::int32_t>(get<std::int16_t>(input, 0x12)) >> shift;
            if (width == 0) width = 1;
            put(pool, record + 8, static_cast<std::uint16_t>(width));
            put(pool, record + 0xa, static_cast<std::uint16_t>(get<std::int16_t>(input, 0x14) >> shift));
            put(pool, record + 0xc, static_cast<std::uint16_t>(get<std::int16_t>(input, 0x16) >> shift));
            const auto previous = offset_of(scratch(pool, level - 1));
            put(pool, record + 0x10, get<std::uint64_t>(pool, previous + 0x10) +
                get<std::uint32_t>(input, 0x2c + static_cast<std::size_t>(level - 1) * 4));
        }
        if (get<std::uint64_t>(input, 8) == UINT64_MAX) {
            put(pool, record + 0x1c, std::uint16_t{0xffff});
        } else {
            put(pool, record + 0x1c, get<std::uint16_t>(input, 0x18));
            if (level != 0) {
                put(pool, record + 0x1e, get<std::uint16_t>(pool, offset_of(first) + 0x1e));
            } else {
                put(pool, record + 0x18, static_cast<std::uint16_t>(argument != 0));
                put(pool, record + 0x1a, static_cast<std::uint16_t>(selector));
                put(pool, record + 0x20, get<std::uint16_t>(input, 0x1a));
                put(pool, record + 0x22, get<std::uint16_t>(input, 0x1c));
                put(pool, record + 0x24, get<std::uint16_t>(input, 0x1e));
                put(pool, record + 0x28, get<std::uint64_t>(input, 8));
                if (argument != 0 && services.prepare_palette(services.context, pool, address, argument) == 0)
                    return 0; // Same direct failure exit as pool exhaustion.
            }
        }
        if (services.place_record(services.context, pool, address, 0, 0) == 0) {
            for (std::int32_t i = 0;
                 i < get<std::int32_t>(pool, ptx_scratch_count_offset); ++i)
                release_ptx_record(pool, scratch(pool, static_cast<std::uint32_t>(i)));
            return 0;
        }
        mark_ptx_record_spans(pool, address);
    }
    return 1;
}
} // namespace dmc::rengine::reverse
