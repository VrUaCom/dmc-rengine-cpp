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
#include <cstdio>
#include <memory>

namespace {
constexpr const char* kTag = "RengineLSG";
constexpr std::int64_t kLongPressMs = 600;

std::int64_t monotonic_ms() {
  return std::chrono::duration_cast<std::chrono::milliseconds>(
      std::chrono::steady_clock::now().time_since_epoch()).count();
}

struct AppState {
  rengine::lsg::VulkanRenderer renderer;
  std::unique_ptr<rengine::lsg::CharacterRuntime> character;
  android_app* app{};
  std::uint32_t character_index{};
  bool detail_enabled{true};
  bool skeleton_enabled{};
  bool has_window{};
  bool dragging{};
  bool hud_candidate{};
  bool moved{};
  bool tooltip_visible{};
  int pressed_ui_row{-1};
  std::int64_t press_start_ms{};
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

const char* mode_name(rengine::lsg::DiagnosticRenderMode mode) {
  using rengine::lsg::DiagnosticRenderMode;
  switch (mode) {
    case DiagnosticRenderMode::genome_perspective: return "GENOME_PERSPECTIVE";
    case DiagnosticRenderMode::raw_perspective: return "RAW_PERSPECTIVE";
    case DiagnosticRenderMode::raw_orthographic: return "RAW_ORTHOGRAPHIC";
    case DiagnosticRenderMode::genome_joint_debug: return "GENOME_JOINT_DEBUG";
  }
  return "UNKNOWN";
}

const char* lighting_name(rengine::lsg::LightingPreset preset) {
  using rengine::lsg::LightingPreset;
  switch (preset) {
    case LightingPreset::morning: return "MORNING";
    case LightingPreset::noon: return "NOON";
    case LightingPreset::evening: return "EVENING";
    case LightingPreset::night: return "NIGHT";
  }
  return "UNKNOWN";
}

const char* filter_name(rengine::lsg::OpticalFilterPreset preset) {
  using rengine::lsg::OpticalFilterPreset;
  switch (preset) {
    case OpticalFilterPreset::clear: return "CLEAR";
    case OpticalFilterPreset::tinted: return "TINTED";
    case OpticalFilterPreset::polarized_approx: return "POLARIZED_APPROX";
  }
  return "UNKNOWN";
}

void log_renderer_diagnostics(AppState& state, const char* reason, float fps = 0.0f, float cpu_ms = 0.0f) {
  const auto d = state.renderer.diagnostics();
  char message[900]{};
  std::snprintf(message, sizeof(message),
      "%s mode=%s skeleton=%s time=%s filter=%s scene_lum=%.3f eye_lum=%.3f pupil_target=%.4f pupil=%.4f window=%ux%u swapchain=%ux%u logical=%ux%u rotation=%u aspect=%.4f fov=%.3f distance=%.3fm near=%.3fm far=%.1fm gpu_est=%llu fps=%.1f cpu_frame=%.2fms",
      reason, mode_name(d.mode), state.skeleton_enabled ? "ON" : "OFF",
      lighting_name(d.lighting_preset), filter_name(d.optical_filter),
      d.scene_luminance, d.effective_eye_luminance,
      d.pupil_target_radius, d.pupil_current_radius,
      d.window_width, d.window_height,
      d.swapchain_width, d.swapchain_height,
      d.logical_width, d.logical_height,
      d.surface_rotation, d.logical_aspect, d.fov_y_radians, d.camera_distance_m,
      d.near_plane_m, d.far_plane_m,
      static_cast<unsigned long long>(d.estimated_gpu_bytes), fps, cpu_ms);
  log_info(message);
}

void select_character(AppState& state, std::uint32_t index) {
  state.character_index = index & 1u;
  state.character = std::make_unique<rengine::lsg::CharacterRuntime>(rengine::lsg::builtin_profile(state.character_index));
  log_info(state.character_index == 0 ? "Character 0 selected" : "Character 1 selected");
}

void toggle_detail(AppState& state) {
  state.detail_enabled = !state.detail_enabled;
  log_info(state.detail_enabled ? "Procedural detail ON" : "Procedural detail OFF");
}

void toggle_skeleton(AppState& state) {
  using rengine::lsg::DiagnosticRenderMode;
  state.skeleton_enabled = !state.skeleton_enabled;
  state.renderer.set_diagnostic_mode(state.skeleton_enabled
      ? DiagnosticRenderMode::genome_joint_debug
      : DiagnosticRenderMode::genome_perspective);
  log_info(state.skeleton_enabled ? "Joint Debug / Skeleton ON" : "Joint Debug / Skeleton OFF");
  log_renderer_diagnostics(state, "Skeleton toggle");
}

void cycle_diagnostic_mode(AppState& state) {
  using rengine::lsg::DiagnosticRenderMode;
  state.skeleton_enabled = false;
  const auto current = state.renderer.diagnostic_mode();
  const auto next = current == DiagnosticRenderMode::genome_perspective ? DiagnosticRenderMode::raw_perspective
                  : current == DiagnosticRenderMode::raw_perspective ? DiagnosticRenderMode::raw_orthographic
                                                                    : DiagnosticRenderMode::genome_perspective;
  state.renderer.set_diagnostic_mode(next);
  log_renderer_diagnostics(state, "Diagnostic mode changed");
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
        if (state->renderer.initialize(app->window, app->activity->assetManager)) {
          log_info("Vulkan LSG viewer initialized");
          log_renderer_diagnostics(*state, "Renderer init");
        } else {
          __android_log_write(ANDROID_LOG_ERROR, kTag, "Vulkan initialization failed");
        }
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

int ui_row_from_point(float x, float y, float width, float height) {
  if (width <= 0.0f || height <= 0.0f) return -1;
  const float nx = x / width;
  const float ny = y / height;
  if (nx < 0.020f || nx > 0.185f) return -1;
  for (int row = 0; row < 10; ++row) {
    const float center_y = 0.07f + static_cast<float>(row) * 0.07f;
    if (std::abs(ny - center_y) <= 0.029f) return row;
  }
  return -1;
}

bool point_in_ui_panel(float x, float y, float width, float height) {
  if (width <= 0.0f || height <= 0.0f) return false;
  const float nx = x / width;
  const float ny = y / height;
  return nx >= 0.010f && nx <= 0.195f && ny >= 0.035f && ny <= 0.820f;
}

void clear_tooltip(AppState& state) {
  state.tooltip_visible = false;
  state.pressed_ui_row = -1;
  state.press_start_ms = 0;
  state.renderer.set_ui_tooltip_row(-1);
}

void update_long_press(AppState& state) {
  if (!state.hud_candidate || state.moved || state.tooltip_visible || state.pressed_ui_row < 0) return;
  if (monotonic_ms() - state.press_start_ms < kLongPressMs) return;
  state.tooltip_visible = true;
  state.renderer.set_ui_tooltip_row(state.pressed_ui_row);
  log_info("R&D tooltip shown");
}

void cycle_physiology(AppState& state) {
  using rengine::lsg::PhysiologyPreset;
  const auto current = state.renderer.physiology_preset();
  const auto next = current == PhysiologyPreset::normal ? PhysiologyPreset::exercise
                  : current == PhysiologyPreset::exercise ? PhysiologyPreset::cold
                  : current == PhysiologyPreset::cold ? PhysiologyPreset::hot
                                                      : PhysiologyPreset::normal;
  state.renderer.set_physiology_preset(next);
  log_info(next == PhysiologyPreset::normal ? "Physiology Normal"
           : next == PhysiologyPreset::exercise ? "Physiology Exercise"
           : next == PhysiologyPreset::cold ? "Physiology Cold"
                                             : "Physiology Hot");
}

void cycle_eye_mode(AppState& state) {
  using rengine::lsg::EyeDiagnosticMode;
  const auto current = state.renderer.eye_diagnostic_mode();
  const auto next = current == EyeDiagnosticMode::normal ? EyeDiagnosticMode::components
                  : current == EyeDiagnosticMode::components ? EyeDiagnosticMode::iris_only
                  : current == EyeDiagnosticMode::iris_only ? EyeDiagnosticMode::cornea_only
                                                            : EyeDiagnosticMode::normal;
  state.renderer.set_eye_diagnostic_mode(next);
  log_info(next == EyeDiagnosticMode::normal ? "Eyes Normal"
           : next == EyeDiagnosticMode::components ? "Eyes Components"
           : next == EyeDiagnosticMode::iris_only ? "Eyes Iris Only"
                                                   : "Eyes Cornea Only");
}

void cycle_lighting_time(AppState& state) {
  using rengine::lsg::LightingPreset;
  const auto current = state.renderer.lighting_preset();
  const auto next = current == LightingPreset::morning ? LightingPreset::noon
                  : current == LightingPreset::noon ? LightingPreset::evening
                  : current == LightingPreset::evening ? LightingPreset::night
                                                       : LightingPreset::morning;
  state.renderer.set_lighting_preset(next);
  log_info(next == LightingPreset::morning ? "Time Morning"
           : next == LightingPreset::noon ? "Time Noon"
           : next == LightingPreset::evening ? "Time Evening"
                                             : "Time Night");
  log_renderer_diagnostics(state, "Time preset changed");
}

void handle_ui_row(AppState& state, int row) {
  switch (row) {
    case 0: select_character(state, 0); break;
    case 1: select_character(state, 1); break;
    case 2: toggle_detail(state); break;
    case 3: toggle_skeleton(state); break;
    case 4: cycle_camera_preset(state); break;
    case 5: cycle_diagnostic_mode(state); break;
    case 6:
      state.renderer.reset_camera_view();
      log_info("Camera view reset");
      log_renderer_diagnostics(state, "Camera reset");
      break;
    case 7: cycle_physiology(state); break;
    case 8: cycle_eye_mode(state); break;
    case 9: cycle_lighting_time(state); break;
    default: break;
  }
}

std::int32_t on_input(android_app* app, AInputEvent* event) {
  auto* state = static_cast<AppState*>(app->userData);
  const int type = AInputEvent_getType(event);
  if (type == AINPUT_EVENT_TYPE_KEY && AKeyEvent_getAction(event) == AKEY_EVENT_ACTION_UP) {
    switch (AKeyEvent_getKeyCode(event)) {
      case AKEYCODE_0: select_character(*state, 0); return 1;
      case AKEYCODE_1: select_character(*state, 1); return 1;
      case AKEYCODE_D: toggle_detail(*state); return 1;
      case AKEYCODE_S: toggle_skeleton(*state); return 1;
      case AKEYCODE_M: cycle_diagnostic_mode(*state); return 1;
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
    state->pressed_ui_row = ui_row_from_point(state->down_x, state->down_y, width, height);
    state->hud_candidate = state->pressed_ui_row >= 0;
    state->dragging = !state->hud_candidate;
    state->moved = false;
    state->tooltip_visible = false;
    state->renderer.set_ui_tooltip_row(-1);
    state->press_start_ms = state->hud_candidate ? monotonic_ms() : 0;
    state->last_pinch_distance = 0.0f;
    return 1;
  }

  if (masked == AMOTION_EVENT_ACTION_POINTER_DOWN && count >= 2) {
    clear_tooltip(*state);
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
    if (state->hud_candidate && movement > threshold) {
      state->hud_candidate = false;
      clear_tooltip(*state);
    }
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
      if (state->hud_candidate) {
        const int row = ui_row_from_point(x, y, width, height);
        if (!state->tooltip_visible && row >= 0 && row == state->pressed_ui_row) {
          handle_ui_row(*state, row);
        }
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
    clear_tooltip(*state);
    state->dragging = false; state->hud_candidate = false; state->moved = false; state->last_pinch_distance = 0.0f; return 1;
  }
  return 0;
}
} // namespace

void android_main(android_app* app) {
  AppState state{}; state.app = app; select_character(state, 0);
  app->userData = &state; app->onAppCmd = on_command; app->onInputEvent = on_input;
  const auto start = std::chrono::steady_clock::now();
  auto telemetry_start = start;
  std::uint32_t telemetry_frames = 0;

  while (true) {
    int events = 0; android_poll_source* source = nullptr;
    while (ALooper_pollOnce(state.renderer.ready() ? 0 : -1, nullptr, &events, reinterpret_cast<void**>(&source)) >= 0) {
      if (source != nullptr) source->process(app, source);
      if (app->destroyRequested != 0) { state.renderer.shutdown(); return; }
    }
    if (state.renderer.ready()) {
      update_long_press(state);
      const auto frame_begin = std::chrono::steady_clock::now();
      const float seconds = std::chrono::duration<float>(frame_begin - start).count();
      if (!state.renderer.draw_frame(seconds, state.character_index, state.detail_enabled)) {
        __android_log_write(ANDROID_LOG_WARN, kTag, "Frame failed or swapchain needs recreation");
      } else {
        ++telemetry_frames;
      }
      const auto now = std::chrono::steady_clock::now();
      const float interval = std::chrono::duration<float>(now - telemetry_start).count();
      if (interval >= 1.0f) {
        const float fps = static_cast<float>(telemetry_frames) / interval;
        const float cpu_ms = telemetry_frames == 0 ? 0.0f : interval * 1000.0f / static_cast<float>(telemetry_frames);
        log_renderer_diagnostics(state, "Frame telemetry", fps, cpu_ms);
        telemetry_start = now;
        telemetry_frames = 0;
      }
    }
  }
}
