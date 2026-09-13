#pragma once

#include "dmc_rengine/runtime/render_device.hpp"
#include "dmc_rengine/runtime/status.hpp"

#include <cstddef>
#include <functional>
#include <map>
#include <memory>
#include <vector>

namespace dmc::rengine::runtime {

/// Declared state of a backend.
///
/// `declared` means the runtime knows the backend exists and refuses to create
/// it until an implementation lands. Nothing is silently substituted: asking
/// for an unimplemented backend is an error, never a quiet fallback to `null`.
enum class RenderBackendAvailability {
    implemented,
    declared,
};

struct RenderBackendEntry final {
    RenderBackendKind kind{RenderBackendKind::null};
    RenderBackendAvailability availability{RenderBackendAvailability::declared};

    friend bool operator==(const RenderBackendEntry&, const RenderBackendEntry&) = default;
};

class RenderBackendRegistry final {
public:
    using Factory = std::function<std::unique_ptr<IRenderDevice>()>;

    /// Builds a registry with the platform-appropriate defaults: the `null`
    /// backend implemented everywhere, plus the declared backends for the
    /// targets this build was configured for.
    [[nodiscard]] static RenderBackendRegistry with_defaults();

    void declare(RenderBackendKind kind);
    void implement(RenderBackendKind kind, Factory factory);

    [[nodiscard]] bool known(RenderBackendKind kind) const noexcept;
    [[nodiscard]] RenderBackendAvailability availability(RenderBackendKind kind) const noexcept;
    [[nodiscard]] std::vector<RenderBackendEntry> entries() const;
    [[nodiscard]] std::size_t size() const noexcept;

    [[nodiscard]] Expected<std::unique_ptr<IRenderDevice>> create(RenderBackendKind kind) const;

private:
    struct Slot final {
        RenderBackendAvailability availability{RenderBackendAvailability::declared};
        Factory factory{};
    };

    std::map<RenderBackendKind, Slot> slots_{};
};

} // namespace dmc::rengine::runtime
