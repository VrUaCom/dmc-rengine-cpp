#include "dmc_rengine/runtime/headless_platform.hpp"

#include <cassert>
#include <chrono>

namespace {

using dmc::rengine::runtime::HeadlessPlatform;
using dmc::rengine::runtime::PlatformEvent;
using dmc::rengine::runtime::PlatformEventKind;
using dmc::rengine::runtime::PlatformKind;
using dmc::rengine::runtime::SurfaceGeometry;

void reports_itself_as_a_non_presenting_host() {
    const HeadlessPlatform platform;
    assert(platform.kind() == PlatformKind::headless);
    assert(platform.id() == "headless");
    assert(!platform.capabilities().presents_surface);
    assert(platform.capabilities().recreates_surface);
    assert(!platform.surface().renderable());
}

void surface_transitions_update_reported_geometry() {
    HeadlessPlatform platform;

    platform.post_surface(PlatformEventKind::surface_created, SurfaceGeometry{800U, 600U, 1.0F});
    assert(platform.surface() == SurfaceGeometry(800U, 600U, 1.0F));

    platform.post_surface(PlatformEventKind::surface_resized, SurfaceGeometry{1280U, 720U, 2.0F});
    assert(platform.surface().width == 1280U);
    assert(platform.surface().scale > 1.9F);

    platform.post_surface(PlatformEventKind::surface_destroyed, SurfaceGeometry{});
    assert(!platform.surface().renderable());

    const auto events = platform.drain_events();
    assert(events.size() == 3U);
    assert(events[0].kind == PlatformEventKind::surface_created);
    assert(events[1].kind == PlatformEventKind::surface_resized);
    assert(events[2].kind == PlatformEventKind::surface_destroyed);
}

void draining_empties_the_queue() {
    HeadlessPlatform platform;
    platform.post(PlatformEvent{PlatformEventKind::focus_lost, SurfaceGeometry{}});

    assert(platform.drain_events().size() == 1U);
    assert(platform.drain_events().empty());
}

void time_only_moves_when_advanced() {
    HeadlessPlatform platform;
    const auto start = platform.now();
    assert(platform.now() == start);

    platform.advance(std::chrono::milliseconds{16});
    assert(platform.now() - start == std::chrono::milliseconds{16});

    platform.advance(std::chrono::milliseconds{-5});
    assert(platform.now() - start == std::chrono::milliseconds{16});
}

void exit_request_is_observable_and_queued() {
    HeadlessPlatform platform;
    assert(!platform.exit_requested());

    const auto requested = platform.request_exit();
    assert(requested.has_value());
    assert(platform.exit_requested());

    const auto events = platform.drain_events();
    assert(events.size() == 1U);
    assert(events[0].kind == PlatformEventKind::quit_requested);
}

void initial_surface_can_be_supplied() {
    const HeadlessPlatform platform{SurfaceGeometry{640U, 480U, 1.0F}};
    assert(platform.surface().renderable());
    assert(platform.surface().height == 480U);
}

} // namespace

int main() {
    reports_itself_as_a_non_presenting_host();
    surface_transitions_update_reported_geometry();
    draining_empties_the_queue();
    time_only_moves_when_advanced();
    exit_request_is_observable_and_queued();
    initial_surface_can_be_supplied();
    return 0;
}
