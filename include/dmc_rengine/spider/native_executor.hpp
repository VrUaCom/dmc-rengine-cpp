#pragma once

#include "dmc_rengine/spider/execution_domain.hpp"

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

// Format-neutral Spider kernel. Product code selects a plan and binds native
// operations; authoritative parsers/codecs remain in their owning modules.
[[nodiscard]] NativeExecutionReport execute_native_plan(
    const NativePlan& plan,
    std::span<const NativeOperationBinding> bindings,
    void* state) noexcept;

} // namespace dmc::rengine::spider
