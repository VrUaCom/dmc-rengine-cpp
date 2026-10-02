#include "dmc_rengine/spider/native_executor.hpp"

#include <array>
#include <atomic>
#include <chrono>
#include <thread>
#include <cassert>
#include <cstdint>

namespace {

struct State final {
    std::array<std::uint32_t, 8U> order{};
    std::size_t count{};
    std::uint32_t fail_operand{0xFFFFFFFFU};
};

bool record(void* raw, std::uint32_t operand) noexcept {
    auto* state = static_cast<State*>(raw);
    if (state == nullptr || state->count >= state->order.size()) {
        return false;
    }
    if (operand == state->fail_operand) {
        return false;
    }
    state->order[state->count++] = operand;
    return true;
}

bool record_offset(void* raw, std::uint32_t operand) noexcept {
    auto* state = static_cast<State*>(raw);
    if (state == nullptr || state->count >= state->order.size()) {
        return false;
    }
    state->order[state->count++] = operand + 100U;
    return true;
}

struct WaveState final {
    std::array<std::atomic<int>, 16U> done{};
    std::atomic<int> running{0};
    std::atomic<int> peak{0};
    std::atomic<int> order{0};
    std::array<int, 16U> sequence{};
    std::uint32_t fail_operand{0xFFFFFFFFU};
};

bool wave_step(void* raw, std::uint32_t operand) noexcept {
    auto* state = static_cast<WaveState*>(raw);
    const int now = ++state->running;
    int seen = state->peak.load();
    while (now > seen && !state->peak.compare_exchange_weak(seen, now)) {
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(20));
    --state->running;
    state->sequence[operand] = state->order++;
    state->done[operand] = 1;
    return operand != state->fail_operand;
}

} // namespace

void parallel_waves() {
    namespace spider = dmc::rengine::spider;
    constexpr spider::NativeOperationId kSerial = 1U;
    constexpr spider::NativeOperationId kWave = 2U;
    // 0 serial -> 1..6 concurrent (each on 0) -> 7 serial (on 1..6)
    spider::NativePlan plan;
    plan.instructions.push_back({.operation = kSerial, .operand = 0U});
    for (std::uint32_t k = 1U; k <= 6U; ++k) {
        plan.instructions.push_back({.operation = kWave, .operand = k,
                                     .dependency_begin = 0U, .dependency_count = 1U});
    }
    plan.dependencies = {0U, 1U, 2U, 3U, 4U, 5U, 6U};
    plan.instructions.push_back({.operation = kSerial, .operand = 7U,
                                 .dependency_begin = 1U, .dependency_count = 6U});
    const std::array bindings{
        spider::NativeOperationBinding{.operation = kSerial, .execute = &wave_step},
        spider::NativeOperationBinding{.operation = kWave, .execute = &wave_step, .concurrent = true},
    };
    {
        WaveState state;
        const auto report = spider::execute_native_plan_parallel(plan, bindings, &state, 4U);
        assert(report.ok() && report.completed_instructions == 8U);
        assert(state.peak.load() >= 2);                 // the wave ran at once
        assert(state.sequence[0] == 0 && state.sequence[7] == 7);  // serial ends stay in order
    }
    {
        WaveState state;
        state.fail_operand = 4U;
        const auto report = spider::execute_native_plan_parallel(plan, bindings, &state, 4U);
        assert(report.status == spider::NativeExecutionStatus::operation_failed);
        assert(report.failed_instruction == 4U && report.completed_instructions == 4U);
        assert(state.done[7] == 0);                      // nothing after the failing wave
    }
    {
        // One thread: the same results in plan order.
        WaveState state;
        const auto report = spider::execute_native_plan_parallel(plan, bindings, &state, 1U);
        assert(report.ok() && state.peak.load() == 1);
        for (int k = 0; k < 8; ++k) assert(state.sequence[static_cast<std::size_t>(k)] == k);
    }
}

int main() {
    parallel_waves();
    namespace spider = dmc::rengine::spider;

    constexpr spider::NativeOperationId kProbe = 1U;
    constexpr spider::NativeOperationId kParse = 2U;
    constexpr spider::NativeOperationId kProject = 3U;

    const std::array bindings{
        spider::NativeOperationBinding{.operation = kProbe, .execute = &record},
        spider::NativeOperationBinding{.operation = kParse, .execute = &record},
        spider::NativeOperationBinding{.operation = kProject, .execute = &record_offset},
    };

    spider::NativePlan plan;
    plan.dependencies = {0U, 1U};
    plan.instructions = {
        spider::NativeInstruction{
            .operation = kProbe,
            .operand = 10U,
            .dependency_begin = 0U,
            .dependency_count = 0U,
            .domain = spider::ExecutionDomain::cpu,
        },
        spider::NativeInstruction{
            .operation = kParse,
            .operand = 20U,
            .dependency_begin = 0U,
            .dependency_count = 1U,
            .domain = spider::ExecutionDomain::cpu,
        },
        spider::NativeInstruction{
            .operation = kProject,
            .operand = 30U,
            .dependency_begin = 1U,
            .dependency_count = 1U,
            .domain = spider::ExecutionDomain::cpu,
        },
    };

    assert(plan.structurally_valid());
    State state;
    const auto ok = spider::execute_native_plan(plan, bindings, &state);
    assert(ok.ok());
    assert(ok.completed_instructions == 3U);
    assert(ok.failed_instruction == spider::NativeExecutionReport::npos);
    assert(state.count == 3U);
    assert(state.order[0U] == 10U);
    assert(state.order[1U] == 20U);
    assert(state.order[2U] == 130U);

    // Missing module binding fails closed before invoking the unknown operation.
    const std::array partial_bindings{
        bindings[0U],
        bindings[1U],
    };
    state = {};
    const auto missing = spider::execute_native_plan(plan, partial_bindings, &state);
    assert(missing.status == spider::NativeExecutionStatus::missing_operation);
    assert(missing.completed_instructions == 2U);
    assert(missing.failed_instruction == 2U);
    assert(state.count == 2U);

    // A module failure stops publication; later operations are never invoked.
    state = {};
    state.fail_operand = 20U;
    const auto failed = spider::execute_native_plan(plan, bindings, &state);
    assert(failed.status == spider::NativeExecutionStatus::operation_failed);
    assert(failed.completed_instructions == 1U);
    assert(failed.failed_instruction == 1U);
    assert(state.count == 1U);

    // Dependencies must point strictly backward in topological order.
    auto invalid = plan;
    invalid.dependencies[0U] = 1U;
    assert(!invalid.structurally_valid());
    const auto invalid_report = spider::execute_native_plan(invalid, bindings, &state);
    assert(invalid_report.status == spider::NativeExecutionStatus::invalid_plan);

    // Duplicate operation bindings are rejected to keep dispatch deterministic.
    const std::array duplicate_bindings{
        bindings[0U],
        bindings[0U],
    };
    const auto duplicate = spider::execute_native_plan(plan, duplicate_bindings, &state);
    assert(duplicate.status == spider::NativeExecutionStatus::invalid_bindings);

    // Empty plans are valid no-op orchestration units.
    spider::NativePlan empty;
    assert(empty.structurally_valid());
    const auto empty_report = spider::execute_native_plan(empty, bindings, &state);
    assert(empty_report.ok());
    assert(empty_report.completed_instructions == 0U);

    return 0;
}
