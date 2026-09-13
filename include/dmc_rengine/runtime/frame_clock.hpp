#pragma once

#include <chrono>
#include <cstdint>

namespace dmc::rengine::runtime {

/// Fixed-step accumulator.
///
/// Simulation advances in whole steps so that behavior does not depend on the
/// host frame rate, while presentation gets an interpolation factor. The step
/// budget bounds catch-up work: after a long stall the clock drops the backlog
/// instead of spiralling, and reports how much it dropped.
class FrameClock final {
public:
    using Duration = std::chrono::nanoseconds;

    struct Step final {
        std::uint32_t simulation_steps{};
        std::uint32_t dropped_steps{};
        float interpolation{};

        friend bool operator==(const Step&, const Step&) = default;
    };

    FrameClock() = default;
    FrameClock(Duration fixed_step, std::uint32_t max_steps_per_advance);

    [[nodiscard]] Duration fixed_step() const noexcept;
    [[nodiscard]] std::uint32_t max_steps_per_advance() const noexcept;
    [[nodiscard]] Duration accumulated() const noexcept;
    [[nodiscard]] std::uint64_t total_simulation_steps() const noexcept;

    [[nodiscard]] Step advance(Duration elapsed);
    void reset();

private:
    Duration fixed_step_{std::chrono::nanoseconds{16'666'667}};
    std::uint32_t max_steps_per_advance_{8};
    Duration accumulated_{};
    std::uint64_t total_simulation_steps_{};
};

/// Convenience constructor for a whole-number simulation rate.
[[nodiscard]] FrameClock make_frame_clock(std::uint32_t simulation_hz,
                                          std::uint32_t max_steps_per_advance = 8);

} // namespace dmc::rengine::runtime
