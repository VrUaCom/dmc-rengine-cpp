#pragma once

#include "dmc_rengine/reverse/ptx_payload_state.hpp"

namespace dmc::rengine::reverse {

struct PtxRecordServices {
    void* context;
    // Only the low return byte is tested by the canonical caller.
    std::uint8_t (*prepare_palette)(void*, PtxPoolImage&, std::uint64_t record,
                                    std::uint64_t argument); // 0x140331BD0
    std::uint8_t (*place_record)(void*, PtxPoolImage&, std::uint64_t record,
                                 std::uint32_t flag8, std::uint32_t flag9); // 0x140331520
};

// 0x1403310F0: mark word +0x46 and the same spans cleared by 0x140330F60.
// Nonpositive lengths/out-of-view accesses use host guards, not EXE validation.
void mark_ptx_record_spans(PtxPoolImage&, std::uint64_t record);

// 0x1403366E0. Resets scratch count/4 pointers, then builds source records.
// The low signed 16-bit count at descriptor +0x3C controls iteration.
// Allocation and palette failures return without rollback. Placement failure
// releases the scratch prefix but retains its count and pointer values.
// The descriptor, pool and external service state must not alias each other.
int materialize_ptx_records(const PtxTextureInputImage&, std::int32_t selector,
                            std::uint64_t argument, PtxPoolImage&, const PtxRecordServices&);

} // namespace dmc::rengine::reverse
