#include "dmc_rengine/runtime/render_backend_registry.hpp"

#include "dmc_rengine/runtime/null_render_device.hpp"

#include <memory>
#include <string>
#include <utility>

namespace dmc::rengine::runtime {

RenderBackendRegistry RenderBackendRegistry::with_defaults() {
    RenderBackendRegistry registry;
    registry.implement(RenderBackendKind::null, [] { return std::make_unique<NullRenderDevice>(); });

    // Declared, not implemented. Creating one fails closed until a backend
    // lands with its own specification and tests.
    registry.declare(RenderBackendKind::vulkan);
    registry.declare(RenderBackendKind::gles);
#if defined(_WIN32)
    registry.declare(RenderBackendKind::d3d11);
#endif
#if defined(__APPLE__)
    registry.declare(RenderBackendKind::metal);
#endif
    return registry;
}

void RenderBackendRegistry::declare(RenderBackendKind kind) {
    auto& slot = slots_[kind];
    if (slot.availability == RenderBackendAvailability::implemented) {
        return;
    }
    slot.availability = RenderBackendAvailability::declared;
    slot.factory = Factory{};
}

void RenderBackendRegistry::implement(RenderBackendKind kind, Factory factory) {
    auto& slot = slots_[kind];

    // An empty factory demotes the backend. `declare` cannot be reused here:
    // it deliberately refuses to downgrade an implemented slot, which would
    // leave the previous factory installed behind a "declared" label.
    if (!factory) {
        slot.availability = RenderBackendAvailability::declared;
        slot.factory = Factory{};
        return;
    }

    slot.availability = RenderBackendAvailability::implemented;
    slot.factory = std::move(factory);
}

bool RenderBackendRegistry::known(RenderBackendKind kind) const noexcept {
    return slots_.find(kind) != slots_.end();
}

RenderBackendAvailability RenderBackendRegistry::availability(
    RenderBackendKind kind) const noexcept {
    const auto entry = slots_.find(kind);
    if (entry == slots_.end()) {
        return RenderBackendAvailability::declared;
    }
    return entry->second.availability;
}

std::vector<RenderBackendEntry> RenderBackendRegistry::entries() const {
    std::vector<RenderBackendEntry> listed;
    listed.reserve(slots_.size());
    for (const auto& [kind, slot] : slots_) {
        listed.push_back(RenderBackendEntry{kind, slot.availability});
    }
    return listed;
}

std::size_t RenderBackendRegistry::size() const noexcept {
    return slots_.size();
}

Expected<std::unique_ptr<IRenderDevice>> RenderBackendRegistry::create(
    RenderBackendKind kind) const {
    const auto entry = slots_.find(kind);
    if (entry == slots_.end()) {
        return fail(make_error(RuntimeErrorCode::invalid_argument, "render-backend-registry",
                               "unknown backend '" + std::string{to_string(kind)} + "'"));
    }
    if (entry->second.availability != RenderBackendAvailability::implemented ||
        !entry->second.factory) {
        return fail(make_error(RuntimeErrorCode::unsupported, "render-backend-registry",
                               "backend '" + std::string{to_string(kind)} +
                                   "' is declared but not implemented"));
    }

    auto device = entry->second.factory();
    if (!device) {
        return fail(make_error(RuntimeErrorCode::internal, "render-backend-registry",
                               "factory for '" + std::string{to_string(kind)} +
                                   "' produced no device"));
    }
    return device;
}

} // namespace dmc::rengine::runtime
