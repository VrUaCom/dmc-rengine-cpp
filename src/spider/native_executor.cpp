#include "dmc_rengine/spider/native_executor.hpp"

#include <algorithm>
#include <atomic>
#include <thread>
#include <vector>

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

NativeExecutionReport execute_native_plan_parallel(
    const NativePlan& plan,
    std::span<const NativeOperationBinding> bindings,
    void* state,
    std::size_t max_threads) noexcept {
    if (!plan.structurally_valid()) {
        return {.status = NativeExecutionStatus::invalid_plan,
                .completed_instructions = 0U,
                .failed_instruction = NativeExecutionReport::npos};
    }
    if (!bindings_valid(bindings)) {
        return {.status = NativeExecutionStatus::invalid_bindings,
                .completed_instructions = 0U,
                .failed_instruction = NativeExecutionReport::npos};
    }
    try {
        const auto n = plan.instructions.size();
        std::vector<const NativeOperationBinding*> resolved(n, nullptr);
        for (std::size_t index = 0U; index < n; ++index) {
            resolved[index] = find_binding(bindings, plan.instructions[index].operation);
            if (resolved[index] == nullptr) {
                return {.status = NativeExecutionStatus::missing_operation,
                        .completed_instructions = index,
                        .failed_instruction = index};
            }
        }
        const auto threads_available = max_threads != 0U
            ? max_threads
            : static_cast<std::size_t>(std::max(1U, std::thread::hardware_concurrency()));
        const auto concurrent = [&](std::size_t index) {
            return resolved[index]->concurrent && plan.instructions[index].domain == ExecutionDomain::cpu;
        };
        std::size_t index = 0U;
        while (index < n) {
            // A wave: consecutive concurrent instructions that depend only on
            // instructions before the wave.
            std::size_t end = index;
            while (end < n && concurrent(end)) {
                const auto& instruction = plan.instructions[end];
                bool ready = true;
                for (std::uint16_t k = 0U; k < instruction.dependency_count; ++k) {
                    ready = ready && plan.dependencies[instruction.dependency_begin + k] < index;
                }
                if (!ready) break;
                ++end;
            }
            if (end - index <= 1U || threads_available <= 1U) {
                const auto last = std::max(end, index + 1U);
                for (; index < last; ++index) {
                    if (!resolved[index]->execute(state, plan.instructions[index].operand)) {
                        return {.status = NativeExecutionStatus::operation_failed,
                                .completed_instructions = index,
                                .failed_instruction = index};
                    }
                }
                continue;
            }
            const auto wave = end - index;
            std::vector<unsigned char> succeeded(wave, 0U);
            std::atomic<std::size_t> next{0U};
            const auto run = [&] {
                for (std::size_t k = next++; k < wave; k = next++) {
                    succeeded[k] = resolved[index + k]->execute(state, plan.instructions[index + k].operand) ? 1U : 0U;
                }
            };
            std::vector<std::thread> pool;
            const auto workers = std::min(threads_available, wave) - 1U;
            pool.reserve(workers);
            for (std::size_t t = 0U; t < workers; ++t) pool.emplace_back(run);
            run();
            for (auto& thread : pool) thread.join();
            for (std::size_t k = 0U; k < wave; ++k) {
                if (succeeded[k] == 0U) {
                    return {.status = NativeExecutionStatus::operation_failed,
                            .completed_instructions = index + k,
                            .failed_instruction = index + k};
                }
            }
            index = end;
        }
        return {.status = NativeExecutionStatus::ok,
                .completed_instructions = n,
                .failed_instruction = NativeExecutionReport::npos};
    } catch (...) {
        return {.status = NativeExecutionStatus::operation_failed,
                .completed_instructions = 0U,
                .failed_instruction = 0U};
    }
}

} // namespace dmc::rengine::spider
