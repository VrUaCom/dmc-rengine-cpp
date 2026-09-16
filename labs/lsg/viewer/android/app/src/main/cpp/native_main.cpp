#include "rengine/lsg/genome.hpp"
#include "rengine/lsg/runtime.hpp"
#include "vulkan_renderer.hpp"

#include <android/input.h>
#include <android/log.h>
#include <android_native_app_glue.h>

#include <chrono>
#include <memory>

namespace {

constexpr const char* kTag = "RengineLSG";

struct AppState {
  rengine::lsg::VulkanRenderer renderer;
  std::unique_ptr<rengine::lsg::CharacterRuntime> character;
  std::uint32_t character_index{};
  bool detail_enabled{true};
  bool debug_enabled{true};
  bool has_window{false};
};

void log_info(const char* message) {
  __android_log_write(ANDROID_LOG_INFO, kTag, message);
}

void on_command(android_app* app, std::int32_t command) {
  auto* state = static_cast<AppState*>(app->userData);
  switch (command) {
    case APP_CMD_INIT_WINDOW:
      state->has_window = app->window != nullptr;
      if (state->has_window) {
        if (state->renderer.initialize(app->window)) {
          log_info("Vulkan 1.2 LSG surface initialized");
        } else {
          __android_log_write(ANDROID_LOG_ERROR, kTag, "Vulkan initialization failed");
        }
      }
      break;
    case APP_CMD_TERM_WINDOW:
      state->renderer.shutdown();
      state->has_window = false;
      break;
    default:
      break;
  }
}

std::int32_t on_input(android_app* app, AInputEvent* event) {
  auto* state = static_cast<AppState*>(app->userData);
  if (AInputEvent_getType(event) != AINPUT_EVENT_TYPE_KEY ||
      AKeyEvent_getAction(event) != AKEY_EVENT_ACTION_UP) {
    return 0;
  }

  switch (AKeyEvent_getKeyCode(event)) {
    case AKEYCODE_0:
      state->character_index = 0;
      state->character = std::make_unique<rengine::lsg::CharacterRuntime>(rengine::lsg::builtin_profile(0));
      return 1;
    case AKEYCODE_1:
      state->character_index = 1;
      state->character = std::make_unique<rengine::lsg::CharacterRuntime>(rengine::lsg::builtin_profile(1));
      return 1;
    case AKEYCODE_D:
      state->detail_enabled = !state->detail_enabled;
      return 1;
    default:
      return 0;
  }
}

}  // namespace

void android_main(android_app* app) {
  AppState state{};
  state.character = std::make_unique<rengine::lsg::CharacterRuntime>(rengine::lsg::builtin_profile(0));
  app->userData = &state;
  app->onAppCmd = on_command;
  app->onInputEvent = on_input;
  const auto start = std::chrono::steady_clock::now();

  while (true) {
    int events = 0;
    android_poll_source* source = nullptr;
    while (ALooper_pollOnce(state.renderer.ready() ? 0 : -1,
                            nullptr,
                            &events,
                            reinterpret_cast<void**>(&source)) >= 0) {
      if (source != nullptr) {
        source->process(app, source);
      }
      if (app->destroyRequested != 0) {
        state.renderer.shutdown();
        return;
      }
    }

    if (state.renderer.ready()) {
      const auto now = std::chrono::steady_clock::now();
      const float seconds = std::chrono::duration<float>(now - start).count();
      if (!state.renderer.draw_frame(seconds)) {
        __android_log_write(ANDROID_LOG_WARN, kTag, "Frame failed or swapchain needs recreation");
      }
    }
  }
}
