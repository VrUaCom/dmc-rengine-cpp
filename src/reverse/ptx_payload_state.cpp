#include "dmc_rengine/reverse/ptx_payload_state.hpp"
#include <bit>
#include <cstring>
#include <limits>
#include <stdexcept>

namespace dmc::rengine::reverse {
namespace {
static_assert(std::endian::native == std::endian::little);
template<class T> T read(std::span<const std::byte> memory, std::size_t offset) {
    if (offset > memory.size() || sizeof(T) > memory.size() - offset)
        throw std::out_of_range("PTX image read");
    T value;
    std::memcpy(&value, memory.data() + offset, sizeof value);
    return value;
}
template<class T> void write(PtxPoolImage& memory, std::size_t offset, T value) {
    if (offset > memory.size() || sizeof(T) > memory.size() - offset)
        throw std::out_of_range("PTX image write");
    std::memcpy(memory.data() + offset, &value, sizeof value);
}
std::int32_t signed32(std::uint32_t value) noexcept { return std::bit_cast<std::int32_t>(value); }
std::int32_t product(const PtxPayloadState& state) noexcept {
    return signed32(state.texture_count * state.records_per_texture);
}
std::size_t record_offset(std::uint64_t address) {
    if (address < ptx_pool_address || address - ptx_pool_address > 0x27b0)
        throw std::out_of_range("PTX record address");
    return static_cast<std::size_t>(address - ptx_pool_address);
}
void clear_span(PtxPoolImage& pool, std::int16_t base, std::int16_t length) {
    // The original do-while has no zero/negative-length guard. Such inputs can
    // traverse unrelated memory; they are outside this bounded state model.
    if (length <= 0) throw std::domain_error("PTX span length outside normal domain");
    const auto units = read<std::uint32_t>(pool, ptx_pool_base_units_offset);
    const auto delta = signed32(static_cast<std::uint32_t>(static_cast<std::int32_t>(base)) - units);
    const auto start = static_cast<std::int64_t>(ptx_allocation_map_offset) + delta / 32;
    if (start < 0 || start > static_cast<std::int64_t>(pool.size()) - length)
        throw std::out_of_range("PTX occupancy span");
    std::memset(pool.data() + start, 0, static_cast<std::size_t>(length));
}
int load(std::span<const std::byte> source, std::uint64_t source_address,
         PtxPayloadState& payload, PtxPoolImage& pool, PtxLoadWorkspace& workspace,
         std::uint64_t argument, const PtxBundleServices& services) {
    const auto count = read<std::int32_t>(source, 0);
    std::uint32_t width = 0;
    std::uint64_t cursor = source_address + 0x800;
    for (std::int32_t i = 0; i < count; ++i) {
        if (services.parse_texture(services.context, cursor, workspace.descriptor) == 0 ||
            services.materialize_records(services.context, workspace.descriptor, -1, argument, pool) == 0) {
            release_ptx_payload(payload, pool);
            return 0;
        }
        ++payload.texture_count;
        if (i == 0) {
            width = read<std::uint32_t>(pool, ptx_scratch_count_offset);
            payload.records_per_texture = width;
        }
        // Subsequent materializer counts are deliberately ignored by the EXE.
        for (std::int32_t j = 0; j < signed32(width); ++j) {
            const auto first = signed32(static_cast<std::uint32_t>(i) * width);
            const auto index = static_cast<std::int64_t>(first) + j;
            if (index < 0 || index >= static_cast<std::int64_t>(payload.records.size()))
                throw std::out_of_range("PTX payload record slot");
            payload.records[static_cast<std::size_t>(index)] =
                read<std::uint64_t>(pool, ptx_scratch_records_offset + static_cast<std::size_t>(j) * 8);
        }
        const auto blocks = read<std::uint32_t>(source, 4 + static_cast<std::size_t>(i) * 4);
        cursor += static_cast<std::uint32_t>(blocks << 11); // 32-bit SHL truncation is observable.
    }
    // Even zero/negative source counts finalize the existing payload.
    return services.finalize_payload(services.context, payload, pool);
}
} // namespace

std::uint64_t allocate_ptx_record(PtxPoolImage& pool) noexcept {
    for (std::uint16_t i = 0; i < ptx_record_count; ++i) {
        const std::size_t offset = static_cast<std::size_t>(i) * ptx_record_size;
        if (read<std::uint16_t>(pool, offset) != 0) continue;
        write(pool, offset, std::uint16_t{1});
        write(pool, offset + 2, i);
        return ptx_pool_address + offset;
    }
    return 0;
}

void release_ptx_record_spans(PtxPoolImage& pool, std::uint64_t address) {
    const auto offset = record_offset(address);
    clear_span(pool, read<std::int16_t>(pool, offset + 6), read<std::int16_t>(pool, offset + 0xe));
    if (read<std::int16_t>(pool, offset + 0x1c) >= 0 &&
        read<std::int16_t>(pool, offset + 0x18) == 0 &&
        read<std::int16_t>(pool, offset + 0x44) == 0) {
        clear_span(pool, read<std::int16_t>(pool, offset + 0x1e),
                   read<std::int16_t>(pool, offset + 0x26));
    }
}

void release_ptx_record(PtxPoolImage& pool, std::uint64_t address) {
    if (address < ptx_pool_address || address > ptx_pool_address + 0x27b0) return;
    const auto offset = record_offset(address);
    if (read<std::uint16_t>(pool, offset + 0x46) != 0) release_ptx_record_spans(pool, address);
    std::memset(pool.data() + offset, 0, ptx_record_size);
}

void release_ptx_payload(PtxPayloadState& payload, PtxPoolImage& pool) {
    // Product uses signed low 32 bits and is re-read by the EXE each iteration.
    for (std::int32_t i = 0; i < product(payload); ++i)
        release_ptx_record(pool, payload.records.at(static_cast<std::size_t>(i)));
    payload = {};
}

int load_ptx_bundle(std::span<const std::byte> source, std::uint64_t source_address,
                    PtxPayloadState& payload, PtxPoolImage& pool, PtxLoadWorkspace& workspace,
                    const PtxBundleServices& services) {
    return load(source, source_address, payload, pool, workspace, 0, services);
}
int load_ptx_bundle_variant(std::span<const std::byte> source, std::uint64_t source_address,
                            PtxPayloadState& payload, PtxPoolImage& pool, PtxLoadWorkspace& workspace,
                            std::uint64_t argument, const PtxBundleServices& services) {
    return load(source, source_address, payload, pool, workspace, argument, services);
}
PtxPayloadState decode_ptx_payload_state(const PtxPayload& payload) noexcept {
    return std::bit_cast<PtxPayloadState>(payload);
}
void encode_ptx_payload_state(const PtxPayloadState& state, PtxPayload& payload) noexcept {
    payload = std::bit_cast<PtxPayload>(state);
}
} // namespace dmc::rengine::reverse
