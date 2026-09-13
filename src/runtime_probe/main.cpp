/// Reports what this build of the runtime host layer is and proves the loop
/// runs end to end on a machine with no display.
///
/// It is deliberately a separate executable from `dmc-rengine`: the CLI stays
/// on the core library's C++20 baseline, while this probe follows the runtime
/// target's standard.

#include "dmc_rengine/runtime/runtime.hpp"

#include <chrono>
#include <cstdint>
#include <iostream>
#include <string>

namespace {

using namespace dmc::rengine::runtime;

[[nodiscard]] int run_headless_frames(std::uint32_t frames) {
    HeadlessPlatform platform{SurfaceGeometry{1280U, 720U, 1.0F}};
    NullRenderDevice device;
    RuntimeApplication application{platform, device};

    if (auto started = application.start(); !started.has_value()) {
        std::cerr << "start failed: " << started.error().describe() << '\n';
        return 1;
    }

    for (std::uint32_t frame = 0U; frame < frames; ++frame) {
        platform.advance(std::chrono::milliseconds{16});
        auto outcome = application.tick();
        if (!outcome.has_value()) {
            std::cerr << "tick failed: " << outcome.error().describe() << '\n';
            return 1;
        }
        if (*outcome != FrameOutcome::rendered) {
            std::cerr << "unexpected frame outcome: " << to_string(*outcome) << '\n';
            return 1;
        }
    }

    application.stop();
    std::cout << "headless-frames: " << application.frame_index() << '\n';
    return 0;
}

} // namespace

int main(int argc, char** argv) {
    const auto registry = RenderBackendRegistry::with_defaults();
    std::cout << describe_build(build_info(), registry);

    std::uint32_t frames = 0U;
    for (int index = 1; index < argc; ++index) {
        const std::string argument{argv[index]};
        if (argument == "--frames" && index + 1 < argc) {
            frames = static_cast<std::uint32_t>(std::stoul(argv[++index]));
        } else if (argument == "--help") {
            std::cout << "usage: dmc-rengine-runtime-probe [--frames N]\n";
            return 0;
        } else {
            std::cerr << "unknown argument: " << argument << '\n';
            return 2;
        }
    }

    if (frames == 0U) {
        return 0;
    }
    return run_headless_frames(frames);
}
