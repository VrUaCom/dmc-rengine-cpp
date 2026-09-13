#pragma once

#include "dmc_rengine/runtime/platform.hpp"
#include "dmc_rengine/runtime/render_backend_registry.hpp"

#include <cstdint>
#include <string>
#include <vector>

namespace dmc::rengine::runtime {

/// What this particular build of the runtime layer actually is.
///
/// Reported rather than assumed: the C++ standard and library features vary by
/// toolchain, and the Android build in particular must be able to state which
/// facilities it got.
struct RuntimeBuildInfo final {
    std::string runtime_version;
    std::uint64_t cplusplus{};
    bool native_expected{};
    bool native_move_only_function{};
    PlatformKind host_platform{PlatformKind::headless};

    friend bool operator==(const RuntimeBuildInfo&, const RuntimeBuildInfo&) = default;
};

[[nodiscard]] RuntimeBuildInfo build_info();

/// One-line-per-entry human summary used by the CLI and the Android bridge.
[[nodiscard]] std::string describe_build(const RuntimeBuildInfo& info,
                                         const RenderBackendRegistry& registry);

} // namespace dmc::rengine::runtime
