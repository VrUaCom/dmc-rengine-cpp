#pragma once

#include "dmc_rengine/spider/family.hpp"
#include "dmc_rengine/spider/native_executor.hpp"

#include <span>

namespace dmc::rengine::spider::crusader {

// Crusader is the C++ module-orchestration face of Spider. It is intentionally
// a zero-overhead facade over the existing generic native executor: no plan,
// dependency graph, scheduler state, or operation binding is duplicated.
inline constexpr Family family = Family::crusader;

using OperationId = NativeOperationId;
using OperationFn = NativeOperationFn;
using Instruction = NativeInstruction;
using Plan = NativePlan;
using OperationBinding = NativeOperationBinding;
using ExecutionStatus = NativeExecutionStatus;
using ExecutionReport = NativeExecutionReport;
using Domain = ExecutionDomain;

[[nodiscard]] inline ExecutionReport execute(
    const Plan& plan,
    std::span<const OperationBinding> bindings,
    void* state) noexcept {
    return execute_native_plan(plan, bindings, state);
}

[[nodiscard]] constexpr const char* to_string(
    ExecutionStatus status) noexcept {
    return dmc::rengine::spider::to_string(status);
}

} // namespace dmc::rengine::spider::crusader
