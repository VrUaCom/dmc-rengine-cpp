#include "dmc_rengine/runtime/application.hpp"

#include <chrono>
#include <utility>

namespace dmc::rengine::runtime {

RuntimeApplication::RuntimeApplication(IPlatform& platform, IRenderDevice& device,
                                       ApplicationConfig config)
    : platform_(&platform), device_(&device), config_(std::move(config)) {}

void RuntimeApplication::set_draw_source(DrawSource source) {
    draw_source_ = std::move(source);
}

void RuntimeApplication::set_simulation_step(SimulationStep step) {
    simulation_step_ = std::move(step);
}

Status RuntimeApplication::start() {
    if (running_) {
        return fail(make_error(RuntimeErrorCode::already_initialized, "runtime-application",
                               "application is already running"));
    }

    frame_index_ = 0U;
    suspended_ = false;
    focused_ = true;
    exit_requested_ = false;
    surface_available_ = false;
    config_.clock.reset();

    // A surface may already exist at start (desktop) or may arrive later as an
    // event (Android). Both paths converge on the same device state.
    const SurfaceGeometry geometry = platform_->surface();
    if (geometry.renderable()) {
        if (auto initialized = device_->initialize(geometry); !initialized.has_value()) {
            return fail(initialized.error());
        }
        surface_available_ = true;
    }

    last_tick_ = platform_->now();
    running_ = true;
    return ok();
}

Status RuntimeApplication::apply(const PlatformEvent& event) {
    switch (event.kind) {
    case PlatformEventKind::surface_created:
    case PlatformEventKind::surface_resized: {
        if (!event.geometry.renderable()) {
            surface_available_ = false;
            return ok();
        }

        auto result = device_->initialized() ? device_->resize(event.geometry)
                                             : device_->initialize(event.geometry);
        if (!result.has_value()) {
            surface_available_ = false;
            return fail(result.error());
        }
        surface_available_ = true;
        return ok();
    }
    case PlatformEventKind::surface_destroyed: {
        surface_available_ = false;
        return device_->notify_surface_lost();
    }
    case PlatformEventKind::focus_gained:
        focused_ = true;
        return ok();
    case PlatformEventKind::focus_lost:
        focused_ = false;
        return ok();
    case PlatformEventKind::suspend:
        suspended_ = true;
        return ok();
    case PlatformEventKind::resume:
        suspended_ = false;
        // Drop the time spent suspended instead of simulating it as a backlog.
        config_.clock.reset();
        last_tick_ = platform_->now();
        return ok();
    case PlatformEventKind::low_memory:
        // The application owns no cache; residency belongs to the resource
        // bridge and the stage host, which observe this event themselves.
        return ok();
    case PlatformEventKind::quit_requested:
        exit_requested_ = true;
        return ok();
    }

    return ok();
}

Expected<FrameOutcome> RuntimeApplication::tick() {
    if (!running_) {
        return fail(make_error(RuntimeErrorCode::not_initialized, "runtime-application",
                               "tick before start"));
    }

    for (const auto& event : platform_->drain_events()) {
        if (auto applied = apply(event); !applied.has_value()) {
            return fail(applied.error());
        }
    }

    if (exit_requested_) {
        running_ = false;
        return FrameOutcome::exit_requested;
    }

    const auto now = platform_->now();
    const auto elapsed = now - last_tick_;
    last_tick_ = now;

    if (suspended_) {
        return FrameOutcome::suspended;
    }

    const auto step = config_.clock.advance(std::chrono::duration_cast<FrameClock::Duration>(elapsed));
    if (simulation_step_) {
        for (std::uint32_t index = 0U; index < step.simulation_steps; ++index) {
            simulation_step_(config_.clock.fixed_step());
        }
    }

    // An unfocused host that must not present is throttled, not stopped: the
    // simulation above still ran, only the frame is withheld.
    if (!focused_ && !config_.present_while_unfocused) {
        return FrameOutcome::suspended;
    }

    if (!surface_available_ || !device_->initialized()) {
        return FrameOutcome::no_surface;
    }

    const SurfaceGeometry geometry = platform_->surface();
    if (!geometry.renderable()) {
        surface_available_ = false;
        return FrameOutcome::no_surface;
    }

    FrameContext context{geometry, frame_index_, step.interpolation};
    DrawList draws = draw_source_ ? draw_source_(context) : DrawList{};

    auto stats = device_->render(context, draws);
    if (!stats.has_value()) {
        if (stats.error().code == RuntimeErrorCode::surface_lost ||
            stats.error().code == RuntimeErrorCode::device_lost) {
            surface_available_ = false;
            if (auto lost = device_->notify_surface_lost(); !lost.has_value()) {
                return fail(lost.error());
            }
            return FrameOutcome::no_surface;
        }
        return fail(stats.error());
    }

    ++frame_index_;
    return FrameOutcome::rendered;
}

void RuntimeApplication::stop() noexcept {
    if (!running_) {
        return;
    }
    device_->shutdown();
    surface_available_ = false;
    running_ = false;
}

bool RuntimeApplication::running() const noexcept {
    return running_;
}

bool RuntimeApplication::suspended() const noexcept {
    return suspended_;
}

bool RuntimeApplication::focused() const noexcept {
    return focused_;
}

bool RuntimeApplication::surface_available() const noexcept {
    return surface_available_;
}

std::uint64_t RuntimeApplication::frame_index() const noexcept {
    return frame_index_;
}

const FrameClock& RuntimeApplication::clock() const noexcept {
    return config_.clock;
}

} // namespace dmc::rengine::runtime
