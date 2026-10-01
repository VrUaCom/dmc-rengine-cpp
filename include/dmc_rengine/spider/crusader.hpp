#pragma once

#include "dmc_rengine/spider/family.hpp"
#include "dmc_rengine/spider/native_executor.hpp"
#include "dmc_rengine/spider/plan_builder.hpp"

#include <cstdint>
#include <span>
#include <type_traits>

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
using Node = PlanNode;
using Builder = PlanBuilder;

// Typed workflow state: a concrete object, never a pointer.
template <class State>
concept StateObject = std::is_object_v<State> && !std::is_pointer_v<State>;

// An operation written against its real state type.
template <class State>
using TypedOperationFn = bool (*)(State&, std::uint32_t) noexcept;

[[nodiscard]] inline ExecutionReport execute(
    const Plan& plan,
    std::span<const OperationBinding> bindings,
    void* state) noexcept {
    return execute_native_plan(plan, bindings, state);
}

// Typed call site: the executor keeps its tiny void* ABI, callers never cast.
template <StateObject State>
[[nodiscard]] inline ExecutionReport execute(
    const Plan& plan,
    std::span<const OperationBinding> bindings,
    State& state) noexcept {
    return execute_native_plan(plan, bindings, &state);
}

// Adapts `bool fn(State&, operand) noexcept` to the executor ABI. The
// adapter is a distinct function per (State, fn), so bindings stay plain
// function pointers: no std::function, no allocation, no type erasure object.
template <StateObject State, TypedOperationFn<State> Fn>
[[nodiscard]] bool typed_operation(void* raw, std::uint32_t operand) noexcept {
    return raw != nullptr && Fn(*static_cast<State*>(raw), operand);
}

template <StateObject State, TypedOperationFn<State> Fn>
[[nodiscard]] constexpr OperationBinding bind(OperationId operation) noexcept {
    return {.operation = operation, .execute = &typed_operation<State, Fn>};
}

[[nodiscard]] constexpr const char* to_string(
    ExecutionStatus status) noexcept {
    return dmc::rengine::spider::to_string(status);
}

} // namespace dmc::rengine::spider::crusader
