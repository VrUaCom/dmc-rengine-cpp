#include "dmc_rengine/runtime/runtime_info.hpp"

#include "dmc_rengine/core/version.hpp"
#include "dmc_rengine/runtime/application.hpp"
#include "dmc_rengine/runtime/status.hpp"

#include <string>

namespace dmc::rengine::runtime {
namespace {

[[nodiscard]] constexpr PlatformKind detect_host_platform() noexcept {
#if defined(__ANDROID__)
    return PlatformKind::android;
#elif defined(_WIN32)
    return PlatformKind::windows_desktop;
#elif defined(__APPLE__)
    return PlatformKind::macos_desktop;
#elif defined(__linux__)
    return PlatformKind::linux_desktop;
#else
    return PlatformKind::headless;
#endif
}

} // namespace

RuntimeBuildInfo build_info() {
    RuntimeBuildInfo info{};
    info.runtime_version = std::string{dmc::rengine::version()};
    info.cplusplus = static_cast<std::uint64_t>(__cplusplus);
    info.native_expected = DMC_RENGINE_HAS_STD_EXPECTED != 0;
    info.native_move_only_function = DMC_RENGINE_HAS_MOVE_ONLY_FUNCTION != 0;
    info.host_platform = detect_host_platform();
    return info;
}

std::string describe_build(const RuntimeBuildInfo& info, const RenderBackendRegistry& registry) {
    std::string text;
    text += "runtime-version: " + info.runtime_version + "\n";
    text += "host-platform: " + std::string{to_string(info.host_platform)} + "\n";
    text += "cplusplus: " + std::to_string(info.cplusplus) + "\n";
    text += std::string{"std-expected: "} + (info.native_expected ? "native" : "fallback") + "\n";
    text += std::string{"std-move-only-function: "} +
            (info.native_move_only_function ? "native" : "fallback") + "\n";

    for (const auto& entry : registry.entries()) {
        text += "backend: " + std::string{to_string(entry.kind)} + " " +
                (entry.availability == RenderBackendAvailability::implemented ? "implemented"
                                                                              : "declared") +
                "\n";
    }
    return text;
}

} // namespace dmc::rengine::runtime
