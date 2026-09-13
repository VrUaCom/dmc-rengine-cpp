#include "dmc_rengine/runtime/application.hpp"
#include "dmc_rengine/runtime/headless_platform.hpp"
#include "dmc_rengine/runtime/null_render_device.hpp"

#include <cassert>
#include <chrono>
#include <cstdint>

namespace {

using dmc::rengine::runtime::ApplicationConfig;
using dmc::rengine::runtime::DrawCall;
using dmc::rengine::runtime::DrawList;
using dmc::rengine::runtime::FrameClock;
using dmc::rengine::runtime::FrameContext;
using dmc::rengine::runtime::FrameOutcome;
using dmc::rengine::runtime::HeadlessPlatform;
using dmc::rengine::runtime::NullRenderDevice;
using dmc::rengine::runtime::PlatformEvent;
using dmc::rengine::runtime::PlatformEventKind;
using dmc::rengine::runtime::RuntimeApplication;
using dmc::rengine::runtime::RuntimeErrorCode;
using dmc::rengine::runtime::SurfaceGeometry;

const SurfaceGeometry kSurface{1280U, 720U, 1.0F};

[[nodiscard]] ApplicationConfig config_with_step(std::chrono::nanoseconds step) {
    ApplicationConfig config;
    config.clock = FrameClock{step, 8U};
    return config;
}

void ticking_before_start_is_refused() {
    HeadlessPlatform platform;
    NullRenderDevice device;
    RuntimeApplication app{platform, device};

    const auto ticked = app.tick();
    assert(!ticked.has_value());
    assert(ticked.error().code == RuntimeErrorCode::not_initialized);
}

void an_existing_surface_is_adopted_at_start() {
    HeadlessPlatform platform{kSurface};
    NullRenderDevice device;
    RuntimeApplication app{platform, device};

    assert(app.start().has_value());
    assert(app.running());
    assert(app.surface_available());
    assert(device.initialized());

    const auto again = app.start();
    assert(!again.has_value());
    assert(again.error().code == RuntimeErrorCode::already_initialized);
}

void a_late_surface_arrives_as_an_event() {
    HeadlessPlatform platform;
    NullRenderDevice device;
    RuntimeApplication app{platform, device};

    assert(app.start().has_value());
    assert(!app.surface_available());

    // No surface yet: the loop keeps running without rendering.
    platform.advance(std::chrono::milliseconds{16});
    const auto starved = app.tick();
    assert(starved.has_value());
    assert(*starved == FrameOutcome::no_surface);
    assert(app.frame_index() == 0U);

    platform.post_surface(PlatformEventKind::surface_created, kSurface);
    platform.advance(std::chrono::milliseconds{16});
    const auto rendered = app.tick();
    assert(rendered.has_value());
    assert(*rendered == FrameOutcome::rendered);
    assert(app.frame_index() == 1U);
    assert(device.initialize_count() == 1U);
}

void surface_loss_and_recreation_survive_the_loop() {
    HeadlessPlatform platform{kSurface};
    NullRenderDevice device;
    RuntimeApplication app{platform, device};
    assert(app.start().has_value());

    platform.post_surface(PlatformEventKind::surface_destroyed, SurfaceGeometry{});
    platform.advance(std::chrono::milliseconds{16});
    const auto lost = app.tick();
    assert(lost.has_value());
    assert(*lost == FrameOutcome::no_surface);
    assert(!app.surface_available());
    assert(!device.initialized());

    platform.post_surface(PlatformEventKind::surface_created, kSurface);
    platform.advance(std::chrono::milliseconds{16});
    const auto revived = app.tick();
    assert(revived.has_value());
    assert(*revived == FrameOutcome::rendered);
    assert(device.initialize_count() == 2U);
}

void a_resize_reaches_the_device_without_reinitializing() {
    HeadlessPlatform platform{kSurface};
    NullRenderDevice device;
    RuntimeApplication app{platform, device};
    assert(app.start().has_value());

    platform.post_surface(PlatformEventKind::surface_resized, SurfaceGeometry{640U, 360U, 1.0F});
    platform.advance(std::chrono::milliseconds{16});
    const auto ticked = app.tick();
    assert(ticked.has_value());
    assert(*ticked == FrameOutcome::rendered);
    assert(device.initialize_count() == 1U);
    assert(device.current_surface().width == 640U);
}

void suspension_stops_frames_and_resume_drops_the_backlog() {
    HeadlessPlatform platform{kSurface};
    NullRenderDevice device;
    RuntimeApplication app{platform, device, config_with_step(std::chrono::milliseconds{10})};

    std::uint32_t simulated = 0U;
    app.set_simulation_step([&simulated](FrameClock::Duration) { ++simulated; });
    assert(app.start().has_value());

    platform.post(PlatformEvent{PlatformEventKind::suspend, kSurface});
    platform.advance(std::chrono::seconds{30});
    const auto suspended = app.tick();
    assert(suspended.has_value());
    assert(*suspended == FrameOutcome::suspended);
    assert(app.suspended());
    assert(simulated == 0U);

    platform.post(PlatformEvent{PlatformEventKind::resume, kSurface});
    platform.advance(std::chrono::milliseconds{10});
    const auto resumed = app.tick();
    assert(resumed.has_value());
    assert(*resumed == FrameOutcome::rendered);

    // Resume restarts the clock at the current instant, so the resume frame
    // itself is a zero-delta frame: none of the thirty suspended seconds is
    // replayed as simulation.
    assert(simulated == 0U);

    // Stepping picks up normally from the next frame onwards.
    platform.advance(std::chrono::milliseconds{10});
    const auto after = app.tick();
    assert(after.has_value());
    assert(*after == FrameOutcome::rendered);
    assert(simulated == 1U);
}

void an_unfocused_host_can_be_told_not_to_present() {
    HeadlessPlatform platform{kSurface};
    NullRenderDevice device;
    ApplicationConfig config = config_with_step(std::chrono::milliseconds{10});
    config.present_while_unfocused = false;
    RuntimeApplication app{platform, device, config};

    std::uint32_t simulated = 0U;
    app.set_simulation_step([&simulated](FrameClock::Duration) { ++simulated; });
    assert(app.start().has_value());

    platform.post(PlatformEvent{PlatformEventKind::focus_lost, kSurface});
    platform.advance(std::chrono::milliseconds{10});
    const auto throttled = app.tick();
    assert(throttled.has_value());
    assert(*throttled == FrameOutcome::suspended);
    assert(!app.focused());

    // Throttled, not stopped: simulation still advanced, the frame did not.
    assert(simulated == 1U);
    assert(device.recorded_frames().empty());

    platform.post(PlatformEvent{PlatformEventKind::focus_gained, kSurface});
    platform.advance(std::chrono::milliseconds{10});
    const auto rendered = app.tick();
    assert(rendered.has_value());
    assert(*rendered == FrameOutcome::rendered);
    assert(device.recorded_frames().size() == 1U);
}

void the_draw_source_feeds_the_device_each_frame() {
    HeadlessPlatform platform{kSurface};
    NullRenderDevice device;
    RuntimeApplication app{platform, device, config_with_step(std::chrono::milliseconds{10})};

    std::uint64_t seen_frame = 0U;
    app.set_draw_source([&seen_frame](const FrameContext& context) {
        seen_frame = context.frame_index;
        DrawList draws;
        draws.push_back(DrawCall{1U, 0U, 12U, "quad"});
        return draws;
    });
    assert(app.start().has_value());

    platform.advance(std::chrono::milliseconds{10});
    assert(app.tick().has_value());
    platform.advance(std::chrono::milliseconds{10});
    assert(app.tick().has_value());

    assert(seen_frame == 1U);
    assert(device.recorded_frames().size() == 2U);
    assert(device.recorded_frames()[0].submitted_draw_calls == 1U);
    assert(device.recorded_frames()[1].frame_index == 1U);
}

void a_quit_request_ends_the_loop() {
    HeadlessPlatform platform{kSurface};
    NullRenderDevice device;
    RuntimeApplication app{platform, device};
    assert(app.start().has_value());

    assert(platform.request_exit().has_value());
    const auto ticked = app.tick();
    assert(ticked.has_value());
    assert(*ticked == FrameOutcome::exit_requested);
    assert(!app.running());

    app.stop();
    assert(!app.running());
}

void stopping_shuts_the_device_down() {
    HeadlessPlatform platform{kSurface};
    NullRenderDevice device;
    RuntimeApplication app{platform, device};
    assert(app.start().has_value());
    assert(device.initialized());

    app.stop();
    assert(!app.running());
    assert(!device.initialized());
    assert(!app.surface_available());
}

} // namespace

int main() {
    ticking_before_start_is_refused();
    an_existing_surface_is_adopted_at_start();
    a_late_surface_arrives_as_an_event();
    surface_loss_and_recreation_survive_the_loop();
    a_resize_reaches_the_device_without_reinitializing();
    suspension_stops_frames_and_resume_drops_the_backlog();
    an_unfocused_host_can_be_told_not_to_present();
    the_draw_source_feeds_the_device_each_frame();
    a_quit_request_ends_the_loop();
    stopping_shuts_the_device_down();
    return 0;
}
