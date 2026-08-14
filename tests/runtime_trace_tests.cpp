#include "dmc_rengine/evidence/runtime_trace.hpp"

#include <cassert>
#include <cstdint>
#include <vector>

using dmc::rengine::evidence::RuntimeMemorySnapshot;
using dmc::rengine::evidence::RuntimeTraceEvent;
using dmc::rengine::evidence::RuntimeTracePhase;
using dmc::rengine::evidence::RuntimeTracePlan;
using dmc::rengine::evidence::RuntimeTraceTarget;

int main() {
    constexpr auto canonical_sha =
        "e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082";

    RuntimeTracePlan plan{
        canonical_sha,
        {
            RuntimeTraceTarget{0x14005E7A0ULL, {0x40, 0x53, 0x48, 0x83}, 64},
            RuntimeTraceTarget{0x14005B460ULL, {0x48, 0x89}, 64},
        },
    };
    assert(plan.valid());

    auto bad_sha = plan;
    bad_sha.target_sha256 = "not-a-sha";
    assert(!bad_sha.valid());

    auto missing_expected_bytes = plan;
    missing_expected_bytes.targets[0].expected_bytes.clear();
    assert(!missing_expected_bytes.valid());

    RuntimeTraceEvent event{};
    event.sequence = 7;
    event.phase = RuntimeTracePhase::after_call;
    event.function_va = 0x14005E7A0ULL;
    event.caller_va = 0x140123456ULL;
    event.selected_source = 1;
    event.reject_mask = 0x0008;
    event.dynamic_category = 0x0E;
    event.result_code = 1;
    event.snapshots.push_back(RuntimeMemorySnapshot{
        0x0000000012345000ULL,
        std::vector<std::uint8_t>{0x00, 0x01, 0x02, 0x03},
    });
    assert(event.valid());

    auto bad_event = event;
    bad_event.snapshots[0].address = 0;
    assert(!bad_event.valid());

    // Tooling capture-window size is deliberately not tied to any claimed
    // original-game ABI size. Pass 10 may enlarge it without changing the
    // evidence meaning of the trace contract.
    plan.targets[0].capture_window_bytes = 128;
    assert(plan.valid());

    return 0;
}
