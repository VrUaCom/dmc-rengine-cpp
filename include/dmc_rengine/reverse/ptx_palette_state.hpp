#pragma once

#include "dmc_rengine/reverse/ptx_payload_state.hpp"

namespace dmc::rengine::reverse {

// Proven prefix of the argument to 0x140331BD0. The complete object also has
// allocator subobjects starting at +0x18/+0x38; their ownership is still open.
struct PtxPaletteContextState {
    std::array<std::uint64_t, 2> bank_records; // 0: format 0x13, 1: format 0x14
    std::array<std::uint16_t, 2> next;
    std::array<std::uint16_t, 2> capacity;
};
static_assert(sizeof(PtxPaletteContextState) == 0x18);
static_assert(offsetof(PtxPaletteContextState, next) == 0x10);
static_assert(offsetof(PtxPaletteContextState, capacity) == 0x14);

struct PtxPaletteServices {
    void* context;
    // VCRUNTIME140.dll!memmove via 0x14032D3D0. This receives BYTE counts,
    // after the original wrapper's uint32 count << 4. Each move is ordered.
    void (*move_bytes)(void*, std::uint64_t destination, std::uint64_t source,
                       std::uint32_t byte_count);
};

// 0x140331BD0. Copies FROM the selected bank TO record +0x28, then writes
// record +0x1E. Negative record selector reserves a counter slot, with no
// rollback in this function. Nonnegative selectors use incoming_home_word
// for the COPY offset, but record +0x1A for the final base calculation.
// incoming_home_word is the dword at ENTRY_RSP +8, untouched by the prologue;
// it is explicit machine state here, not an invented third original argument.
// Other formats copy nothing and store its low word in record +0x1E.
// Pool/context/copy buffers must not alias. Bank/record accesses outside the
// represented pool throw host guards, not an original EXE failure result.
std::uint8_t prepare_ptx_palette(PtxPoolImage&, std::uint64_t record,
    PtxPaletteContextState&, std::uint32_t incoming_home_word, const PtxPaletteServices&);

} // namespace dmc::rengine::reverse
