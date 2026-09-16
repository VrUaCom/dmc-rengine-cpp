#include "rengine/lsg/genome.hpp"
#include "rengine/lsg/runtime.hpp"
#include "vulkan_renderer.hpp"

#include <android/input.h>
#include <android/log.h>
#include <android/native_window.h>
#include <android_native_app_glue.h>

#include <chrono>
#include <cstdint>
#include <memory>

namespace {

constexpr const char* kTag = "RengineLSG";

struct AppState {
  rengine::lsg::VulkanRenderer renderer;
  std::unique_ptr<rengine::lsg::CharacterRuntime> character;
  android_app* app{};
  std::uint32_t character_index{};
  bool detail_enabled{true};
  bool debug_enabled{true};
  bool has_window{false};
};

void log_info(const char* message) {
  __android_log_write(ANDROID_LOG_INFO, kTag, message);
}

void select_character(AppState& state, std::uint32_t index) {
  state.character_index = index & 1u;
  state.character = std::make_unique<rengine::lsg::CharacterRuntime>(
      rengine::lsg::builtin_profile(state.character_index));
  log_info(state.character_index == 0 ? "Character 0 selected" : "Character 1 selected");
}

void toggle_detail(AppState& state) {
  state.detail_enabled = !state.detail_enabled;
  log_info(state.detail_enabled ? "Procedural detail ON" : "Procedural detail OFF");
}

void on_command(android_app* app, std::int32_t command) {
  auto* state = static_cast<AppState*>(app->userData);
  switch (command) {
    case APP_CMD_INIT_WINDOW:
      state->has_window = app->window != nullptr;
      if (state->has_window) {
        if (state->renderer.initialize(app->window, app->activity->assetManager)) {
          log_info("Vulkan 1.2 LSG graphics pipeline initialized");
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
  const int type = AInputEvent_getType(event);
  if (type == AINPUT_EVENT_TYPE_KEY && AKeyEvent_getAction(event) == AKEY_EVENT_ACTION_UP) {
    switch (AKeyEvent_getKeyCode(event)) {
      case AKEYCODE_0:
        select_character(*state, 0);
        return 1;
      case AKEYCODE_1:
        select_character(*state, 1);
        return 1;
      case AKEYCODE_D:
        toggle_detail(*state);
        return 1;
      default:
        return 0;
    }
  }

  if (type == AINPUT_EVENT_TYPE_MOTION &&
      (AMotionEvent_getAction(event) & AMOTION_EVENT_ACTION_MASK) == AMOTION_EVENT_ACTION_UP &&
      app->window != nullptr) {
    const float x = AMotionEvent_getX(event, 0);
    const float y = AMotionEvent_getY(event, 0);
    const float width = static_cast<float>(ANativeWindow_getWidth(app->window));
    const float height = static_cast<float>(ANativeWindow_getHeight(app->window));
    if (width <= 0.0f || height <= 0.0f) return 0;

    // The Vulkan HUD occupies the lower band: [0] [1] [D]. Keep the hitboxes
    // identical to the visible controls instead of using invisible full-screen zones.
    if (y >= height * 0.70f) {
      if (x < width / 3.0f) select_character(*state, 0);
      else if (x < (width * 2.0f) / 3.0f) select_character(*state, 1);
      else toggle_detail(*state);
      return 1;
    }
  }
  return 0;
}

}  // namespace

void android_main(android_app* app) {
  AppState state{};
  state.app = app;
  select_character(state, 0);
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
      if (source != nullptr) source->process(app, source);
      if (app->destroyRequested != 0) {
        state.renderer.shutdown();
        return;
      }
    }

    if (state.renderer.ready()) {
      const auto now = std::chrono::steady_clock::now();
      const float seconds = std::chrono::duration<float>(now - start).count();
      if (!state.renderer.draw_frame(seconds, state.character_index, state.detail_enabled)) {
        __android_log_write(ANDROID_LOG_WARN, kTag, "Frame failed or swapchain needs recreation");
      }
    }
  }
}
