#pragma once

#include <cstdint>

namespace dmc::rengine::spider {

enum class ExecutionDomain : std::uint8_t {
    cpu,
    io,
    gpu_graphics,
    gpu_compute,
    gpu_transfer,
};

} // namespace dmc::rengine::spider
