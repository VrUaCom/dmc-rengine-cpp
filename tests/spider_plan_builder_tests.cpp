// PlanBuilder and typed Crusader operations: plans built from named
// dependencies execute exactly like hand-written ones, bad dependencies fail
// closed before any operation runs, and labels name the failed step.
#include "dmc_rengine/spider/crusader.hpp"
#include "dmc_rengine/spider/plan_builder.hpp"

#include <array>
#include <cassert>
#include <cstdint>
#include <string_view>
#include <vector>

namespace {

namespace spider = dmc::rengine::spider;
namespace crusader = dmc::rengine::spider::crusader;

struct Trace final {
    std::vector<std::uint32_t> order;
    std::uint32_t fail_operand{0xFFFFFFFFU};
};

bool record(Trace& t, std::uint32_t operand) noexcept {
    if (operand == t.fail_operand) return false;
    try {
        t.order.push_back(operand);
    } catch (...) {
        return false;
    }
    return true;
}

bool record_plus_100(Trace& t, std::uint32_t operand) noexcept {
    return record(t, operand + 100U);
}

constexpr crusader::OperationId k_step = 1U;
constexpr crusader::OperationId k_join = 2U;

constexpr std::array k_bindings{
    crusader::bind<Trace, &record>(k_step),
    crusader::bind<Trace, &record_plus_100>(k_join),
};

// acquire -> {a, b, c} -> join : the fan-out / fan-in shape of workflows
// such as per-slot or per-window transforms.
crusader::Builder fan_plan() {
    crusader::Builder b;
    const auto acquire = b.add(k_step, 0U, crusader::Domain::io, {}, "acquire");
    std::vector<crusader::Node> parts;
    for (std::uint32_t i = 1U; i <= 3U; ++i) {
        parts.push_back(b.add(k_step, i, crusader::Domain::cpu, {acquire}, "part"));
    }
    b.add_span(k_join, 7U, crusader::Domain::cpu, parts, "join");
    return b;
}

void builds_valid_ordered_plans() {
    const auto b = fan_plan();
    assert(b.ok() && b.size() == 5U);
    const auto plan = b.build();
    assert(plan.structurally_valid());
    // Same encoding a hand-written plan would have.
    assert(plan.instructions[4].dependency_count == 3U);
    assert(plan.dependencies.size() == 6U);
    assert(plan.dependencies[plan.instructions[4].dependency_begin] == 1U);
    assert(plan.instructions[0].domain == crusader::Domain::io);

    Trace trace;
    const auto report = crusader::execute(plan, k_bindings, trace);  // typed: no &, no cast
    assert(report.ok() && report.completed_instructions == 5U);
    assert((trace.order == std::vector<std::uint32_t>{0U, 1U, 2U, 3U, 107U}));
}

void failure_names_the_step() {
    const auto b = fan_plan();
    Trace trace;
    trace.fail_operand = 2U;
    const auto report = crusader::execute(b.build(), k_bindings, trace);
    assert(report.status == crusader::ExecutionStatus::operation_failed);
    assert(report.failed_instruction == 2U && report.completed_instructions == 2U);
    assert(b.failed_label(report) == "part");
    assert(b.failed_label(crusader::ExecutionReport{.status = crusader::ExecutionStatus::ok}) .empty());
}

void bad_dependencies_fail_closed() {
    crusader::Builder b;
    const auto first = b.add(k_step, 1U);
    b.add(k_step, 2U, crusader::Domain::cpu, {crusader::Node{5U}});  // unknown node
    assert(!b.ok());
    Trace trace;
    const auto report = crusader::execute(b.build(), k_bindings, trace);
    assert(report.status == crusader::ExecutionStatus::invalid_plan && trace.order.empty());

    crusader::Builder self;
    self.add(k_step, 1U, crusader::Domain::cpu, {crusader::Node{0U}});  // depends on itself
    assert(!self.ok());
    (void)first;
}

void typed_binding_rejects_null_state() {
    assert(!k_bindings[0].execute(nullptr, 1U));
    // A missing operation still reports the step.
    crusader::Builder b;
    b.add(k_step, 1U, crusader::Domain::cpu, {}, "first");
    b.add(99U, 2U, crusader::Domain::cpu, {}, "unbound");
    Trace trace;
    const auto report = crusader::execute(b.build(), k_bindings, trace);
    assert(report.status == crusader::ExecutionStatus::missing_operation);
    assert(b.failed_label(report) == "unbound");
}

}  // namespace

int main() {
    builds_valid_ordered_plans();
    failure_names_the_step();
    bad_dependencies_fail_closed();
    typed_binding_rejects_null_state();
    return 0;
}
