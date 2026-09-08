#include "dmc_rengine/spider/native_executor.hpp"

#include <array>
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
    if (state == nullptr || state->count >= state->order.size()) return false;
    if (operand == state->fail_operand) return false;
    state->order[state->count++] = operand;
    return true;
}

bool record_offset(void* raw, std::uint32_t operand) noexcept {
    auto* state = static_cast<State*>(raw);
    if (state == nullptr || state->count >= state->order.size()) return false;
    state->order[state->count++] = operand + 100U;
    return true;
}

} // namespace

int main() {
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
    assert(state.count == 3U);
    assert(state.order[0U] == 10U);
    assert(state.order[1U] == 20U);
    assert(state.order[2U] == 130U);

    const std::array partial_bindings{bindings[0U], bindings[1U]};
    state = {};
    const auto missing = spider::execute_native_plan(plan, partial_bindings, &state);
    assert(missing.status == spider::NativeExecutionStatus::missing_operation);
    assert(missing.completed_instructions == 2U);

    state = {};
    state.fail_operand = 20U;
    const auto failed = spider::execute_native_plan(plan, bindings, &state);
    assert(failed.status == spider::NativeExecutionStatus::operation_failed);
    assert(state.count == 1U);

    auto invalid = plan;
    invalid.dependencies[0U] = 1U;
    assert(!invalid.structurally_valid());

    const std::array duplicate_bindings{bindings[0U], bindings[0U]};
    const auto duplicate = spider::execute_native_plan(plan, duplicate_bindings, &state);
    assert(duplicate.status == spider::NativeExecutionStatus::invalid_bindings);

    spider::NativePlan empty;
    assert(empty.structurally_valid());
    assert(spider::execute_native_plan(empty, bindings, &state).ok());
    return 0;
}
