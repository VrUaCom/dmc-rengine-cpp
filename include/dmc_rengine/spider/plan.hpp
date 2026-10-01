#pragma once

#include <cstddef>
#include <cstdint>
#include <vector>

namespace dmc::rengine::spider {

enum class ExecutionDomain : std::uint8_t {
    cpu,
    io,
    gpu_graphics,
    gpu_compute,
    gpu_transfer,
};

enum class OpCode : std::uint8_t {
    validate_plan,
    acquire_window,
    validate_window,
    publish_packet,
};

// Compact runtime instruction. Spider frontends describe work; native C++20
// modules remain the authority that performs it.
struct Instruction final {
    OpCode op{OpCode::validate_plan};
    ExecutionDomain domain{ExecutionDomain::cpu};
    std::uint16_t dependency_count{};
    std::uint32_t operand{};
};

static_assert(sizeof(Instruction) == 8U,
              "Spider instructions are intentionally compact");

struct Plan final {
    std::vector<Instruction> instructions;

    [[nodiscard]] bool empty() const noexcept {
        return instructions.empty();
    }

    [[nodiscard]] std::size_t size() const noexcept {
        return instructions.size();
    }
};

} // namespace dmc::rengine::spider
