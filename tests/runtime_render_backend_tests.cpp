#include "dmc_rengine/runtime/null_render_device.hpp"
#include "dmc_rengine/runtime/render_backend_registry.hpp"

#include <cassert>
#include <memory>

namespace {

using dmc::rengine::runtime::DeviceCapabilities;
using dmc::rengine::runtime::DrawCall;
using dmc::rengine::runtime::DrawList;
using dmc::rengine::runtime::FrameContext;
using dmc::rengine::runtime::IRenderDevice;
using dmc::rengine::runtime::NullRenderDevice;
using dmc::rengine::runtime::RenderBackendAvailability;
using dmc::rengine::runtime::RenderBackendKind;
using dmc::rengine::runtime::RenderBackendRegistry;
using dmc::rengine::runtime::RuntimeErrorCode;
using dmc::rengine::runtime::SurfaceGeometry;

const SurfaceGeometry kSurface{1280U, 720U, 1.0F};

void device_refuses_work_before_initialization() {
    NullRenderDevice device;
    assert(!device.initialized());

    const auto rendered = device.render(FrameContext{kSurface, 0U, 0.0F}, DrawList{});
    assert(!rendered.has_value());
    assert(rendered.error().code == RuntimeErrorCode::not_initialized);

    const auto resized = device.resize(kSurface);
    assert(!resized.has_value());
    assert(resized.error().code == RuntimeErrorCode::not_initialized);
}

void device_rejects_unrenderable_surfaces() {
    NullRenderDevice device;
    const auto initialized = device.initialize(SurfaceGeometry{0U, 720U, 1.0F});
    assert(!initialized.has_value());
    assert(initialized.error().code == RuntimeErrorCode::invalid_argument);
    assert(!device.initialized());
}

void double_initialization_is_an_error() {
    NullRenderDevice device;
    assert(device.initialize(kSurface).has_value());
    const auto again = device.initialize(kSurface);
    assert(!again.has_value());
    assert(again.error().code == RuntimeErrorCode::already_initialized);
}

void surface_loss_returns_to_a_revivable_state() {
    NullRenderDevice device;
    assert(device.initialize(kSurface).has_value());
    assert(device.notify_surface_lost().has_value());
    assert(!device.initialized());
    assert(!device.current_surface().renderable());

    assert(device.initialize(kSurface).has_value());
    assert(device.initialized());
    assert(device.initialize_count() == 2U);
}

void invalid_and_over_budget_draws_are_rejected_not_dropped_silently() {
    NullRenderDevice device{DeviceCapabilities{4096U, 2U, false, true}};
    assert(device.initialize(kSurface).has_value());

    DrawList draws;
    draws.push_back(DrawCall{1U, 10U, 3U, "valid"});
    draws.push_back(DrawCall{0U, 10U, 3U, "no-geometry"});
    draws.push_back(DrawCall{2U, 10U, 0U, "no-indices"});
    draws.push_back(DrawCall{3U, 10U, 3U, "valid"});
    draws.push_back(DrawCall{4U, 10U, 3U, "over-budget"});

    const auto stats = device.render(FrameContext{kSurface, 41U, 0.25F}, draws);
    assert(stats.has_value());
    assert(stats->frame_index == 41U);
    assert(stats->submitted_draw_calls == 2U);
    assert(stats->rejected_draw_calls == 3U);
    assert(device.recorded_frames().size() == 1U);
}

void registry_defaults_implement_only_the_null_backend() {
    const auto registry = RenderBackendRegistry::with_defaults();
    assert(registry.known(RenderBackendKind::null));
    assert(registry.availability(RenderBackendKind::null) ==
           RenderBackendAvailability::implemented);
    assert(registry.availability(RenderBackendKind::vulkan) ==
           RenderBackendAvailability::declared);
    assert(registry.availability(RenderBackendKind::gles) == RenderBackendAvailability::declared);
    assert(registry.size() >= 3U);

    const auto created = registry.create(RenderBackendKind::null);
    assert(created.has_value());
    assert((*created)->kind() == RenderBackendKind::null);
}

void declared_backends_fail_closed_instead_of_substituting() {
    const auto registry = RenderBackendRegistry::with_defaults();

    const auto vulkan = registry.create(RenderBackendKind::vulkan);
    assert(!vulkan.has_value());
    assert(vulkan.error().code == RuntimeErrorCode::unsupported);

    const auto gles = registry.create(RenderBackendKind::gles);
    assert(!gles.has_value());
    assert(gles.error().code == RuntimeErrorCode::unsupported);
}

void unknown_backends_are_reported_separately_from_unimplemented_ones() {
    RenderBackendRegistry registry;
    registry.implement(RenderBackendKind::null, [] { return std::make_unique<NullRenderDevice>(); });

    const auto unknown = registry.create(RenderBackendKind::metal);
    assert(!unknown.has_value());
    assert(unknown.error().code == RuntimeErrorCode::invalid_argument);
}

void an_implemented_backend_can_be_demoted_and_restored() {
    RenderBackendRegistry registry;
    registry.declare(RenderBackendKind::vulkan);
    assert(registry.availability(RenderBackendKind::vulkan) ==
           RenderBackendAvailability::declared);

    registry.implement(RenderBackendKind::vulkan,
                       [] { return std::unique_ptr<IRenderDevice>{std::make_unique<NullRenderDevice>()}; });
    assert(registry.availability(RenderBackendKind::vulkan) ==
           RenderBackendAvailability::implemented);
    assert(registry.create(RenderBackendKind::vulkan).has_value());

    // A null factory means "declared", never a silent no-op device.
    registry.implement(RenderBackendKind::vulkan, RenderBackendRegistry::Factory{});
    assert(registry.availability(RenderBackendKind::vulkan) ==
           RenderBackendAvailability::declared);
}

} // namespace

int main() {
    device_refuses_work_before_initialization();
    device_rejects_unrenderable_surfaces();
    double_initialization_is_an_error();
    surface_loss_returns_to_a_revivable_state();
    invalid_and_over_budget_draws_are_rejected_not_dropped_silently();
    registry_defaults_implement_only_the_null_backend();
    declared_backends_fail_closed_instead_of_substituting();
    unknown_backends_are_reported_separately_from_unimplemented_ones();
    an_implemented_backend_can_be_demoted_and_restored();
    return 0;
}
