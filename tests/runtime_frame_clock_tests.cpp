#include "dmc_rengine/runtime/frame_clock.hpp"

#include <cassert>
#include <chrono>
#include <cmath>

namespace {

using dmc::rengine::runtime::FrameClock;
using dmc::rengine::runtime::make_frame_clock;
using Nanoseconds = std::chrono::nanoseconds;

void whole_steps_are_reported_and_remainder_is_kept() {
    FrameClock clock{Nanoseconds{10}, 8};

    const auto first = clock.advance(Nanoseconds{25});
    assert(first.simulation_steps == 2U);
    assert(first.dropped_steps == 0U);
    assert(clock.accumulated() == Nanoseconds{5});
    assert(std::fabs(first.interpolation - 0.5F) < 0.0001F);

    const auto second = clock.advance(Nanoseconds{5});
    assert(second.simulation_steps == 1U);
    assert(clock.accumulated() == Nanoseconds{0});
    assert(clock.total_simulation_steps() == 3U);
}

void a_stall_is_dropped_rather_than_replayed() {
    FrameClock clock{Nanoseconds{10}, 3};

    const auto stalled = clock.advance(Nanoseconds{1000});
    assert(stalled.simulation_steps == 3U);
    assert(stalled.dropped_steps == 97U);

    // The backlog must not survive into the next frame.
    assert(clock.accumulated() == Nanoseconds{0});
    const auto after = clock.advance(Nanoseconds{10});
    assert(after.simulation_steps == 1U);
    assert(after.dropped_steps == 0U);
}

void short_frames_accumulate_without_stepping() {
    FrameClock clock{Nanoseconds{10}, 8};

    const auto step = clock.advance(Nanoseconds{4});
    assert(step.simulation_steps == 0U);
    assert(step.dropped_steps == 0U);
    assert(clock.accumulated() == Nanoseconds{4});
    assert(clock.total_simulation_steps() == 0U);
}

void negative_and_zero_deltas_are_ignored() {
    FrameClock clock{Nanoseconds{10}, 8};
    const auto zero = clock.advance(Nanoseconds{0});
    assert(zero.simulation_steps == 0U);

    const auto negative = clock.advance(Nanoseconds{-50});
    assert(negative.simulation_steps == 0U);
    assert(clock.accumulated() == Nanoseconds{0});
}

void reset_clears_backlog_and_history() {
    FrameClock clock{Nanoseconds{10}, 8};
    (void)clock.advance(Nanoseconds{35});
    assert(clock.total_simulation_steps() == 3U);

    clock.reset();
    assert(clock.accumulated() == Nanoseconds{0});
    assert(clock.total_simulation_steps() == 0U);
}

void simulation_rate_helper_matches_requested_hz() {
    const auto clock = make_frame_clock(60U);
    assert(clock.fixed_step() == Nanoseconds{16'666'666});
    assert(clock.max_steps_per_advance() == 8U);

    const auto guarded = make_frame_clock(0U, 0U);
    assert(guarded.fixed_step().count() > 0);
    assert(guarded.max_steps_per_advance() == 1U);
}

void degenerate_configuration_cannot_divide_by_zero() {
    FrameClock clock{Nanoseconds{0}, 4};
    assert(clock.fixed_step().count() > 0);
    const auto step = clock.advance(Nanoseconds{3});
    assert(step.simulation_steps == 3U);
}

} // namespace

int main() {
    whole_steps_are_reported_and_remainder_is_kept();
    a_stall_is_dropped_rather_than_replayed();
    short_frames_accumulate_without_stepping();
    negative_and_zero_deltas_are_ignored();
    reset_clears_backlog_and_history();
    simulation_rate_helper_matches_requested_hz();
    degenerate_configuration_cannot_divide_by_zero();
    return 0;
}
