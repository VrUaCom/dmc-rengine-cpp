#pragma once

#include "dmc_rengine/runtime/frame_clock.hpp"
#include "dmc_rengine/runtime/platform.hpp"
#include "dmc_rengine/runtime/render_device.hpp"
#include "dmc_rengine/runtime/status.hpp"

#include <cstdint>
#include <string_view>
#include <version>

#if defined(__cpp_lib_move_only_function) && __cpp_lib_move_only_function >= 202110L
#include <functional>
#define DMC_RENGINE_HAS_MOVE_ONLY_FUNCTION 1
#else
#include <functional>
#define DMC_RENGINE_HAS_MOVE_ONLY_FUNCTION 0
#endif

namespace dmc::rengine::runtime {

#if DMC_RENGINE_HAS_MOVE_ONLY_FUNCTION
using DrawSource = std::move_only_function<DrawList(const FrameContext&)>;
using SimulationStep = std::move_only_function<void(FrameClock::Duration)>;
#else
using DrawSource = std::function<DrawList(const FrameContext&)>;
using SimulationStep = std::function<void(FrameClock::Duration)>;
#endif

enum class FrameOutcome {
    rendered,
    no_surface,
    suspended,
    exit_requested,
};

[[nodiscard]] constexpr std::string_view to_string(FrameOutcome outcome) noexcept {
    switch (outcome) {
    case FrameOutcome::rendered: return "rendered";
    case FrameOutcome::no_surface: return "no-surface";
    case FrameOutcome::suspended: return "suspended";
    case FrameOutcome::exit_requested: return "exit-requested";
    }
    return "exit-requested";
}

struct ApplicationConfig final {
    FrameClock clock{};

    /// When false, an unfocused host is throttled rather than stopped: the
    /// simulation still steps, but no frame is submitted and `tick` reports
    /// `FrameOutcome::suspended`.
    bool present_while_unfocused{true};
};

/// Host-independent run loop.
///
/// The application owns event ordering, surface lifetime handling and the
/// fixed-step clock. It deliberately owns no resources and no scene state: the
/// draw list arrives from a caller-supplied source each frame.
///
/// Suspension is dropped, not replayed. A `resume` event restarts the clock at
/// the current instant, so the resume frame is a zero-delta frame and normal
/// stepping picks up from the frame after it. Without that, returning from a
/// backgrounded Android process would hand the simulation an arbitrarily large
/// backlog to chew through.
class RuntimeApplication final {
public:
    RuntimeApplication(IPlatform& platform, IRenderDevice& device, ApplicationConfig config = {});

    void set_draw_source(DrawSource source);
    void set_simulation_step(SimulationStep step);

    [[nodiscard]] Status start();
    [[nodiscard]] Expected<FrameOutcome> tick();
    void stop() noexcept;

    [[nodiscard]] bool running() const noexcept;
    [[nodiscard]] bool suspended() const noexcept;
    [[nodiscard]] bool focused() const noexcept;
    [[nodiscard]] bool surface_available() const noexcept;
    [[nodiscard]] std::uint64_t frame_index() const noexcept;
    [[nodiscard]] const FrameClock& clock() const noexcept;

private:
    [[nodiscard]] Status apply(const PlatformEvent& event);

    IPlatform* platform_{};
    IRenderDevice* device_{};
    ApplicationConfig config_{};
    DrawSource draw_source_{};
    SimulationStep simulation_step_{};
    IPlatform::Clock::time_point last_tick_{};
    std::uint64_t frame_index_{};
    bool running_{false};
    bool suspended_{false};
    bool focused_{true};
    bool surface_available_{false};
    bool exit_requested_{false};
};

} // namespace dmc::rengine::runtime
