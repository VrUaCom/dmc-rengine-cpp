#include "rengine/lsg/camera.hpp"
#include "rengine/lsg/genome.hpp"
#include "rengine/lsg/runtime.hpp"
#include "vulkan_renderer.hpp"

#include <android/input.h>
#include <android/log.h>
#include <android/native_window.h>
#include <android_native_app_glue.h>

#include <algorithm>
#include <chrono>
#include <cmath>
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
  bool has_window{};
  bool dragging{};
  bool hud_candidate{};
  bool moved{};
  float down_x{};
  float down_y{};
  float last_x{};
  float last_y{};
  float last_pinch_distance{};
  std::int64_t last_tap_ms{-1000};
  float last_tap_x{};
  float last_tap_y{};
};

void log_info(const char* message) { __android_log_write(ANDROID_LOG_INFO, kTag, message); }

void select_character(AppState& state, std::uint32_t index) {
  state.character_index = index & 1u;
  state.character = std::make_unique<rengine::lsg::CharacterRuntime>(rengine::lsg::builtin_profile(state.character_index));
  log_info(state.character_index == 0 ? "Character 0 selected" : "Character 1 selected");
}

void toggle_detail(AppState& state) {
  state.detail_enabled = !state.detail_enabled;
  log_info(state.detail_enabled ? "Procedural detail ON" : "Procedural detail OFF");
}

void cycle_camera_preset(AppState& state) {
  using rengine::lsg::CameraPreset;
  const auto current = state.renderer.camera_state().preset;
  const auto next = current == CameraPreset::full_body ? CameraPreset::portrait
                  : current == CameraPreset::portrait ? CameraPreset::extreme_close_up
                                                     : CameraPreset::full_body;
  state.renderer.set_camera_preset(next);
  log_info(next == CameraPreset::full_body ? "Camera Full Body"
           : next == CameraPreset::portrait ? "Camera Portrait"
                                           : "Camera Extreme Close-up");
}

void on_command(android_app* app, std::int32_t command) {
  auto* state = static_cast<AppState*>(app->userData);
  switch (command) {
    case APP_CMD_INIT_WINDOW:
      state->has_window = app->window != nullptr;
      if (state->has_window) {
        if (state->renderer.initialize(app->window, app->activity->assetManager)) log_info("Vulkan LSG viewer initialized");
        else __android_log_write(ANDROID_LOG_ERROR, kTag, "Vulkan initialization failed");
      }
      break;
    case APP_CMD_TERM_WINDOW:
      state->renderer.shutdown(); state->has_window = false; break;
    default: break;
  }
}

float pointer_distance(AInputEvent* event) {
  if (AMotionEvent_getPointerCount(event) < 2) return 0.0f;
  const float dx = AMotionEvent_getX(event, 1) - AMotionEvent_getX(event, 0);
  const float dy = AMotionEvent_getY(event, 1) - AMotionEvent_getY(event, 0);
  return std::sqrt(dx * dx + dy * dy);
}

void handle_hud_tap(AppState& state, float x, float width) {
  if (x < width / 3.0f) select_character(state, 0);
  else if (x < (width * 2.0f) / 3.0f) select_character(state, 1);
  else toggle_detail(state);
}

std::int32_t on_input(android_app* app, AInputEvent* event) {
  auto* state = static_cast<AppState*>(app->userData);
  const int type = AInputEvent_getType(event);
  if (type == AINPUT_EVENT_TYPE_KEY && AKeyEvent_getAction(event) == AKEY_EVENT_ACTION_UP) {
    switch (AKeyEvent_getKeyCode(event)) {
      case AKEYCODE_0: select_character(*state, 0); return 1;
      case AKEYCODE_1: select_character(*state, 1); return 1;
      case AKEYCODE_D: toggle_detail(*state); return 1;
      case AKEYCODE_F: state->renderer.set_camera_preset(rengine::lsg::CameraPreset::full_body); return 1;
      case AKEYCODE_P: state->renderer.set_camera_preset(rengine::lsg::CameraPreset::portrait); return 1;
      case AKEYCODE_C: state->renderer.set_camera_preset(rengine::lsg::CameraPreset::extreme_close_up); return 1;
      default: return 0;
    }
  }
  if (type != AINPUT_EVENT_TYPE_MOTION || app->window == nullptr) return 0;

  const float width = static_cast<float>(ANativeWindow_getWidth(app->window));
  const float height = static_cast<float>(ANativeWindow_getHeight(app->window));
  if (width <= 0.0f || height <= 0.0f) return 0;
  const int action = AMotionEvent_getAction(event);
  const int masked = action & AMOTION_EVENT_ACTION_MASK;
  const std::size_t count = AMotionEvent_getPointerCount(event);

  if (masked == AMOTION_EVENT_ACTION_DOWN) {
    state->down_x = state->last_x = AMotionEvent_getX(event, 0);
    state->down_y = state->last_y = AMotionEvent_getY(event, 0);
    state->hud_candidate = state->down_y >= height * 0.70f;
    state->dragging = !state->hud_candidate;
    state->moved = false;
    state->last_pinch_distance = 0.0f;
    return 1;
  }

  if (masked == AMOTION_EVENT_ACTION_POINTER_DOWN && count >= 2) {
    state->hud_candidate = false; state->dragging = false; state->moved = true;
    state->last_pinch_distance = pointer_distance(event); return 1;
  }

  if (masked == AMOTION_EVENT_ACTION_MOVE) {
    if (count >= 2) {
      const float current = pointer_distance(event);
      if (state->last_pinch_distance > 1.0f && current > 1.0f) state->renderer.zoom_camera(current / state->last_pinch_distance);
      state->last_pinch_distance = current; state->hud_candidate = false; state->moved = true; return 1;
    }
    const float x = AMotionEvent_getX(event, 0), y = AMotionEvent_getY(event, 0);
    const float movement = std::hypot(x - state->down_x, y - state->down_y);
    const float threshold = 0.025f * std::min(width, height);
    if (movement > threshold) state->moved = true;
    if (state->hud_candidate && movement > threshold) state->hud_candidate = false;
    if (!state->hud_candidate) {
      if (!state->dragging) { state->dragging = true; state->last_x = x; state->last_y = y; }
      else {
        state->renderer.orbit_camera((x - state->last_x) / width, -(y - state->last_y) / height);
        state->last_x = x; state->last_y = y;
      }
    }
    return 1;
  }

  if (masked == AMOTION_EVENT_ACTION_POINTER_UP) {
    state->last_pinch_distance = 0.0f; state->moved = true;
    if (count > 1) {
      const std::size_t action_index = static_cast<std::size_t>((action & AMOTION_EVENT_ACTION_POINTER_INDEX_MASK) >> AMOTION_EVENT_ACTION_POINTER_INDEX_SHIFT);
      const std::size_t remaining = action_index == 0 ? 1u : 0u;
      state->last_x = AMotionEvent_getX(event, remaining); state->last_y = AMotionEvent_getY(event, remaining);
      state->dragging = true;
    }
    return 1;
  }

  if (masked == AMOTION_EVENT_ACTION_UP || masked == AMOTION_EVENT_ACTION_CANCEL) {
    const float x = AMotionEvent_getX(event, 0), y = AMotionEvent_getY(event, 0);
    if (masked == AMOTION_EVENT_ACTION_UP) {
      if (state->hud_candidate && y >= height * 0.70f) {
        handle_hud_tap(*state, x, width);
      } else if (!state->moved) {
        const std::int64_t now_ms = AMotionEvent_getEventTime(event);
        const float tap_distance = std::hypot(x - state->last_tap_x, y - state->last_tap_y);
        if (now_ms - state->last_tap_ms <= 350 && tap_distance < 0.10f * std::min(width, height)) {
          cycle_camera_preset(*state); state->last_tap_ms = -1000;
        } else {
          state->last_tap_ms = now_ms; state->last_tap_x = x; state->last_tap_y = y;
        }
      }
    }
    state->dragging = false; state->hud_candidate = false; state->moved = false; state->last_pinch_distance = 0.0f; return 1;
  }
  return 0;
}
} // namespace

void android_main(android_app* app) {
  AppState state{}; state.app = app; select_character(state, 0);
  app->userData = &state; app->onAppCmd = on_command; app->onInputEvent = on_input;
  const auto start = std::chrono::steady_clock::now();
  while (true) {
    int events = 0; android_poll_source* source = nullptr;
    while (ALooper_pollOnce(state.renderer.ready() ? 0 : -1, nullptr, &events, reinterpret_cast<void**>(&source)) >= 0) {
      if (source != nullptr) source->process(app, source);
      if (app->destroyRequested != 0) { state.renderer.shutdown(); return; }
    }
    if (state.renderer.ready()) {
      const float seconds = std::chrono::duration<float>(std::chrono::steady_clock::now() - start).count();
      if (!state.renderer.draw_frame(seconds, state.character_index, state.detail_enabled)) {
        __android_log_write(ANDROID_LOG_WARN, kTag, "Frame failed or swapchain needs recreation");
      }
    }
  }
}
