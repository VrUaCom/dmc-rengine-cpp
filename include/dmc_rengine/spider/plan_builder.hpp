#pragma once

#include "dmc_rengine/spider/native_executor.hpp"

#include <cstddef>
#include <cstdint>
#include <initializer_list>
#include <limits>
#include <span>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace dmc::rengine::spider {

// A node of a plan under construction (instruction index once built).
struct PlanNode final {
    std::uint32_t index{std::numeric_limits<std::uint32_t>::max()};
};

// Builds a NativePlan from named dependencies instead of hand-maintained
// dependency_begin / dependency_count offsets. The executor's rules hold by
// construction: a node can only depend on nodes added before it, so the plan
// is topologically ordered; a dependency on an unknown node marks the builder
// failed and build() yields a plan the executor rejects (fail-closed), never a
// silently different graph.
//
// Labels are optional, cost nothing at execution time and turn an
// ExecutionReport into a readable step name ("transform[3]").
class PlanBuilder final {
public:
    PlanBuilder() = default;

    // Appends an instruction that runs after `after`; returns its node.
    PlanNode add(NativeOperationId operation,
                 std::uint32_t operand = 0U,
                 ExecutionDomain domain = ExecutionDomain::cpu,
                 std::initializer_list<PlanNode> after = {},
                 std::string_view label = {}) {
        return add_span(operation, operand, domain,
                        std::span<const PlanNode>{after.begin(), after.size()}, label);
    }

    // Same, with a runtime list of predecessors (fan-in after a fan-out).
    PlanNode add_span(NativeOperationId operation,
                      std::uint32_t operand,
                      ExecutionDomain domain,
                      std::span<const PlanNode> after,
                      std::string_view label = {}) {
        const auto index = plan_.instructions.size();
        if (index >= std::numeric_limits<std::uint32_t>::max() ||
            after.size() > std::numeric_limits<std::uint16_t>::max() ||
            plan_.dependencies.size() > std::numeric_limits<std::uint32_t>::max() - after.size()) {
            ok_ = false;
            return {};
        }
        NativeInstruction instruction{};
        instruction.operation = operation;
        instruction.operand = operand;
        instruction.domain = domain;
        instruction.dependency_begin = static_cast<std::uint32_t>(plan_.dependencies.size());
        instruction.dependency_count = static_cast<std::uint16_t>(after.size());
        for (const auto node : after) {
            if (node.index >= index) ok_ = false;  // unknown or not earlier
            plan_.dependencies.push_back(node.index);
        }
        plan_.instructions.push_back(instruction);
        labels_.emplace_back(label);
        return {static_cast<std::uint32_t>(index)};
    }

    [[nodiscard]] bool ok() const noexcept { return ok_; }
    [[nodiscard]] std::size_t size() const noexcept { return plan_.instructions.size(); }

    // The plan; when the builder failed, an always-invalid plan so execution
    // stops with invalid_plan before any operation runs.
    [[nodiscard]] NativePlan build() const {
        if (ok_) return plan_;
        NativePlan invalid;
        invalid.instructions.push_back(NativeInstruction{.dependency_begin = 1U, .dependency_count = 1U});
        return invalid;
    }

    [[nodiscard]] std::string_view label(std::size_t index) const noexcept {
        return index < labels_.size() ? std::string_view{labels_[index]} : std::string_view{};
    }

    // Label of the instruction a report stopped at, or empty.
    [[nodiscard]] std::string_view failed_label(const NativeExecutionReport& report) const noexcept {
        return report.failed_instruction == NativeExecutionReport::npos ? std::string_view{}
                                                                         : label(report.failed_instruction);
    }

private:
    NativePlan plan_;
    std::vector<std::string> labels_;
    bool ok_{true};
};

} // namespace dmc::rengine::spider
