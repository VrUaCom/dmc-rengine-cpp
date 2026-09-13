#include "dmc_rengine/runtime/frame_clock.hpp"

#include <algorithm>
#include <cstdint>
#include <limits>

namespace dmc::rengine::runtime {

FrameClock::FrameClock(Duration fixed_step, std::uint32_t max_steps_per_advance)
    : fixed_step_(fixed_step.count() > 0 ? fixed_step : Duration{1}),
      max_steps_per_advance_(std::max<std::uint32_t>(max_steps_per_advance, 1U)) {}

FrameClock::Duration FrameClock::fixed_step() const noexcept {
    return fixed_step_;
}

std::uint32_t FrameClock::max_steps_per_advance() const noexcept {
    return max_steps_per_advance_;
}

FrameClock::Duration FrameClock::accumulated() const noexcept {
    return accumulated_;
}

std::uint64_t FrameClock::total_simulation_steps() const noexcept {
    return total_simulation_steps_;
}

FrameClock::Step FrameClock::advance(Duration elapsed) {
    if (elapsed.count() > 0) {
        accumulated_ += elapsed;
    }

    Step step{};
    if (accumulated_.count() <= 0) {
        return step;
    }

    const std::int64_t whole_signed = accumulated_.count() / fixed_step_.count();
    const auto whole = static_cast<std::uint64_t>(whole_signed);

    // Consume the entire backlog in one go, then report how much of it was
    // actually simulated. Keeping the remainder would let a single stall
    // compound into a permanent catch-up debt.
    accumulated_ -= Duration{whole_signed * fixed_step_.count()};

    const std::uint64_t budget{max_steps_per_advance_};
    const std::uint64_t runnable = std::min(whole, budget);
    const std::uint64_t dropped = whole - runnable;

    step.simulation_steps = static_cast<std::uint32_t>(runnable);
    step.dropped_steps =
        static_cast<std::uint32_t>(std::min<std::uint64_t>(dropped, std::numeric_limits<std::uint32_t>::max()));
    step.interpolation = static_cast<float>(accumulated_.count()) / static_cast<float>(fixed_step_.count());

    total_simulation_steps_ += runnable;
    return step;
}

void FrameClock::reset() {
    accumulated_ = Duration::zero();
    total_simulation_steps_ = 0U;
}

FrameClock make_frame_clock(std::uint32_t simulation_hz, std::uint32_t max_steps_per_advance) {
    const std::uint32_t hz = std::max<std::uint32_t>(simulation_hz, 1U);
    const auto step = std::chrono::nanoseconds{std::chrono::seconds{1}} / hz;
    return FrameClock{std::chrono::duration_cast<FrameClock::Duration>(step), max_steps_per_advance};
}

} // namespace dmc::rengine::runtime
