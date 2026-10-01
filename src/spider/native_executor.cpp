#include "dmc_rengine/spider/native_executor.hpp"

#include <algorithm>

namespace dmc::rengine::spider {
namespace {

[[nodiscard]] const NativeOperationBinding* find_binding(
    std::span<const NativeOperationBinding> bindings,
    NativeOperationId operation) noexcept {
    const auto it = std::find_if(
        bindings.begin(), bindings.end(),
        [operation](const NativeOperationBinding& binding) {
            return binding.operation == operation;
        });
    return it == bindings.end() ? nullptr : &*it;
}

[[nodiscard]] bool bindings_valid(
    std::span<const NativeOperationBinding> bindings) noexcept {
    for (std::size_t index = 0U; index < bindings.size(); ++index) {
        if (bindings[index].execute == nullptr) {
            return false;
        }
        for (std::size_t other = index + 1U; other < bindings.size(); ++other) {
            if (bindings[index].operation == bindings[other].operation) {
                return false;
            }
        }
    }
    return true;
}

} // namespace

bool NativePlan::structurally_valid() const noexcept {
    for (std::size_t index = 0U; index < instructions.size(); ++index) {
        const auto& instruction = instructions[index];
        const auto begin = static_cast<std::size_t>(instruction.dependency_begin);
        const auto count = static_cast<std::size_t>(instruction.dependency_count);
        if (begin > dependencies.size() || count > dependencies.size() - begin) {
            return false;
        }
        for (std::size_t offset = 0U; offset < count; ++offset) {
            const auto dependency = static_cast<std::size_t>(
                dependencies[begin + offset]);
            if (dependency >= index) {
                return false;
            }
        }
    }
    return true;
}

NativeExecutionReport execute_native_plan(
    const NativePlan& plan,
    std::span<const NativeOperationBinding> bindings,
    void* state) noexcept {
    if (!plan.structurally_valid()) {
        return {
            .status = NativeExecutionStatus::invalid_plan,
            .completed_instructions = 0U,
            .failed_instruction = NativeExecutionReport::npos,
        };
    }
    if (!bindings_valid(bindings)) {
        return {
            .status = NativeExecutionStatus::invalid_bindings,
            .completed_instructions = 0U,
            .failed_instruction = NativeExecutionReport::npos,
        };
    }

    for (std::size_t index = 0U; index < plan.instructions.size(); ++index) {
        const auto& instruction = plan.instructions[index];
        const auto* binding = find_binding(bindings, instruction.operation);
        if (binding == nullptr) {
            return {
                .status = NativeExecutionStatus::missing_operation,
                .completed_instructions = index,
                .failed_instruction = index,
            };
        }
        if (!binding->execute(state, instruction.operand)) {
            return {
                .status = NativeExecutionStatus::operation_failed,
                .completed_instructions = index,
                .failed_instruction = index,
            };
        }
    }

    return {
        .status = NativeExecutionStatus::ok,
        .completed_instructions = plan.instructions.size(),
        .failed_instruction = NativeExecutionReport::npos,
    };
}

} // namespace dmc::rengine::spider
