#pragma once

#include "dmc_rengine/spider/plan.hpp"

#include <cstddef>
#include <cstdint>
#include <limits>
#include <span>
#include <vector>

namespace dmc::rengine::spider {

using NativeOperationId = std::uint32_t;
using NativeOperationFn = bool (*)(void* state, std::uint32_t operand) noexcept;

struct NativeInstruction final {
    NativeOperationId operation{};
    std::uint32_t operand{};
    std::uint32_t dependency_begin{};
    std::uint16_t dependency_count{};
    ExecutionDomain domain{ExecutionDomain::cpu};
    std::uint8_t reserved{};
};

static_assert(sizeof(NativeInstruction) == 16U,
              "Spider native instructions stay compact and ABI-simple");

struct NativePlan final {
    std::vector<NativeInstruction> instructions;
    std::vector<std::uint32_t> dependencies;

    [[nodiscard]] bool structurally_valid() const noexcept;
};

struct NativeOperationBinding final {
    NativeOperationId operation{};
    NativeOperationFn execute{};
    // The operation may run at the same time as other concurrent operations
    // of the same plan (it touches only state its operand owns). Only the
    // parallel executor reads this; the serial one ignores it.
    bool concurrent{false};
};

enum class NativeExecutionStatus : std::uint8_t {
    ok,
    invalid_plan,
    invalid_bindings,
    missing_operation,
    operation_failed,
};

[[nodiscard]] constexpr const char* to_string(
    NativeExecutionStatus status) noexcept {
    switch (status) {
    case NativeExecutionStatus::ok: return "ok";
    case NativeExecutionStatus::invalid_plan: return "invalid-plan";
    case NativeExecutionStatus::invalid_bindings: return "invalid-bindings";
    case NativeExecutionStatus::missing_operation: return "missing-operation";
    case NativeExecutionStatus::operation_failed: return "operation-failed";
    }
    return "invalid-plan";
}

struct NativeExecutionReport final {
    static constexpr std::size_t npos = std::numeric_limits<std::size_t>::max();

    NativeExecutionStatus status{NativeExecutionStatus::invalid_plan};
    std::size_t completed_instructions{};
    std::size_t failed_instruction{npos};

    [[nodiscard]] bool ok() const noexcept {
        return status == NativeExecutionStatus::ok;
    }
};

// Low-level generic native Spider execution kernel. The product-facing Spider
// family name for this role is Crusader; `spider/crusader.hpp` exposes a
// zero-overhead facade over these exact types and this exact function.
//
// Executes a topologically ordered native plan serially. ExecutionDomain is
// preserved as scheduling metadata but this kernel deliberately does not create
// threads, GPU queues, scripting runtimes, UI policy, or format logic. Native
// operation bindings remain thin calls into authoritative C++20 modules.
[[nodiscard]] NativeExecutionReport execute_native_plan(
    const NativePlan& plan,
    std::span<const NativeOperationBinding> bindings,
    void* state) noexcept;

// The same plan and the same report, with independent work run at once:
// consecutive cpu-domain instructions whose binding is `concurrent` and whose
// dependencies all completed before them run as one wave on up to
// `max_threads` threads (0 = one per core). Everything else runs alone, in
// plan order. A failing wave reports its lowest failing instruction, exactly
// as the serial kernel would have stopped there.
[[nodiscard]] NativeExecutionReport execute_native_plan_parallel(
    const NativePlan& plan,
    std::span<const NativeOperationBinding> bindings,
    void* state,
    std::size_t max_threads = 0U) noexcept;

} // namespace dmc::rengine::spider
